#include<iostream>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>
#include<mpi.h>
#define CL_SILENCE_DEPRECATION
#include <OpenCL/cl.h>

const int matrixSize = 100;

int a[matrixSize][matrixSize];
int b[matrixSize][matrixSize];
int c[matrixSize][matrixSize];

cl_device_id deviceId;
cl_context context;
cl_program program;
cl_kernel kernel;
cl_command_queue queue;
cl_event event = NULL;
cl_mem bufferA, bufferB, bufferC;

int err;
const int TS = 4;
const size_t local[2] = {TS, TS};
const size_t global[2] = {matrixSize, matrixSize};

using namespace std;

void generateMatrix() {
	int seed = 10;
	srand(seed);

	for (int i = 0; i < matrixSize; i++) {
		for (int j = 0; j < matrixSize; j++) {
			a[i][j] = rand() % 10 + 1;
			b[i][j] = rand() % 10 + 1;
		}
	}
}

void printMatrix(int a[matrixSize][matrixSize]) {
	for (int i = 0; i < matrixSize; i++) {
		printf("%d", a[i][0]);
		for (int j = 1; j < matrixSize; j++) {
			printf(", %d", a[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void multiplyMatrices(int a[matrixSize][matrixSize], int b[matrixSize][matrixSize], int c[matrixSize][matrixSize]) {
	for (int i = 0; i < matrixSize; i++) {
		for (int j = 0; j < matrixSize; j++) {
			c[i][j] = 0;
			for (int k = 0; k < matrixSize; k++) {
				c[i][j] += a[i][k] * b[k][j];
			}
		}
	}
}

cl_program buildProgram(cl_context localContext, cl_device_id localDeviceId, const char* filename);
cl_device_id createDevice();

void freeMemory();
void setupKernelMemory();
void copyKernelArgs();
void setupKernelMemory();
void setupOpenCLDeviceContextQueueKernel();

int main(int argc, char *argv[]) {
	// Generate Matrices;
	clock_t timer;
	double timeTaken = 0;

	int numTasks, rank, tag = 1;
	int numThreads = atoi(argv[1]), chunkSize = 10;

	MPI_Status Stat;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	const int rowsPerTask = ceil((float) matrixSize / (numTasks - 1));
	int taskResult[rowsPerTask][matrixSize];
	
	if (rank == 0) {
		timer = clock();
	
		generateMatrix();		
	
		// Send data to all processes
		for (int i = 1; i < numTasks; i++) {
			MPI_Send(&a, matrixSize * matrixSize, MPI_INT, i, tag, MPI_COMM_WORLD);
			MPI_Send(&b, matrixSize * matrixSize, MPI_INT, i, tag, MPI_COMM_WORLD);
		}
	
		int cIndex = 0;
		for (int i = 1; i < numTasks; i++) {
			MPI_Recv(&taskResult, rowsPerTask * matrixSize, MPI_INT, i, tag, MPI_COMM_WORLD, &Stat);
	
			for (int j = 0; j < rowsPerTask; j++) {
				for (int k = 0; k < matrixSize; k++) {
					c[cIndex + j][k] = taskResult[j][k];
				}
			}
	
			cIndex += rowsPerTask;
		}

		printMatrix(c);

		setupOpenCLDeviceContextQueueKernel();
		setupKernelMemory();
		copyKernelArgs();

		clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
		clWaitForEvents(1, &event);

		clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, matrixSize * matrixSize * sizeof(int), c, 0, NULL, NULL);
		printMatrix(c);
		freeMemory();

	} else if (rank > 0) {
	
		MPI_Recv(&a, matrixSize * matrixSize, MPI_INT, 0, tag, MPI_COMM_WORLD, &Stat);
		MPI_Recv(&b, matrixSize * matrixSize, MPI_INT, 0, tag, MPI_COMM_WORLD, &Stat);
	
		int lowerBound = (rank - 1) * rowsPerTask;
		int upperBound = lowerBound + rowsPerTask;
	
		if (upperBound > matrixSize) {
			upperBound = matrixSize;
		}
	
		if (lowerBound < upperBound) {
			for (int i = lowerBound; i < upperBound; i++) {
				for (int j = 0; j < matrixSize; j++) {
					taskResult[i - lowerBound][j] = 0;
					for (int k = 0; k < matrixSize; k++) {
						taskResult[i - lowerBound][j] += a[i][k] * b[k][j];
					}
				}
			}
	
		}
	
		MPI_Send(&taskResult, rowsPerTask * matrixSize, MPI_INT, 0, tag, MPI_COMM_WORLD);
	}
	if (rank == 0) {
	
		timeTaken = (clock() - timer) / (double) CLOCKS_PER_SEC;
	
		printf("MPI: %fs\n", timeTaken);
	}
	
	MPI_Finalize();
	return 0;
}

void setupOpenCLDeviceContextQueueKernel() {
	deviceId = createDevice();
	cl_int err;
	context = clCreateContext(NULL, 1, &deviceId, NULL, NULL, &err);

	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}

	program = buildProgram(context, deviceId, "matrixMultiplication.cl");
	queue = clCreateCommandQueue(context, deviceId, 0, &err);

	if (err < 0) {
		perror("Couldn't create a command queue");
		exit(1);
	}

	kernel = clCreateKernel(program, "matrixMultiplication", &err);

	if (err < 0) {
		perror("Couldn't create a kernel");
		printf("error =%d", err);
		exit(1);
	}
}

void freeMemory() {
	clReleaseKernel(kernel);
	clReleaseMemObject(bufferA);
	clReleaseMemObject(bufferB);
	clReleaseMemObject(bufferC);

	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
}

void copyKernelArgs() {
	clSetKernelArg(kernel, 0, sizeof(int), (void*) &matrixSize);
	clSetKernelArg(kernel, 1, sizeof(int), (void*) &matrixSize);
	clSetKernelArg(kernel, 2, sizeof(int), (void*) &matrixSize);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*) &bufferA);
	clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*) &bufferB);
	clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*) &bufferC);

	if (err < 0) {
		perror("Couldn't create a kernel argument");
		printf("error = %d", err);
		exit(1);
	}
}

void setupKernelMemory() {
	bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY, matrixSize * matrixSize * sizeof(int), NULL, NULL);
	bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY, matrixSize * matrixSize * sizeof(int), NULL, NULL);
	bufferC = clCreateBuffer(context, CL_MEM_READ_WRITE, matrixSize * matrixSize * sizeof(int), NULL, NULL);

	clEnqueueWriteBuffer(queue, bufferA, CL_TRUE, 0, matrixSize * matrixSize * sizeof(int), a, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, bufferB, CL_TRUE, 0, matrixSize * matrixSize * sizeof(int), b, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, bufferC, CL_TRUE, 0, matrixSize * matrixSize * sizeof(int), c, 0, NULL, NULL);
}

cl_program buildProgram(cl_context localContext, cl_device_id localDeviceId, const char* filename) {
	cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
  

   program_handle = fopen(filename, "r");
   if (program_handle == NULL) {
	  perror("Couldn't find the program file");
	  exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);


   program = clCreateProgramWithSource(localContext, 1, (const char**) &program_buffer, &program_size, &err);
   if (err < 0) {
	  perror("Couldn't create the program");
	  exit(1);
   }
   free(program_buffer);


   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if (err < 0) {
	  clGetProgramBuildInfo(program, localDeviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	  program_log = (char*) malloc(log_size + 1);
	  program_log[log_size] = '\0';
	  clGetProgramBuildInfo(program, localDeviceId, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
	  printf("%s\n", program_log);
	  free(program_log);
	  exit(1);
   }

   return program;
}

cl_device_id createDevice() {

	cl_platform_id platform;
	cl_device_id dev;
	int err;

	/* Identify a platform */
	err = clGetPlatformIDs(1, &platform, NULL);
	if(err < 0) {
	  perror("Couldn't identify a platform");
	  exit(1);
	} 

	// Access a device
	// GPU
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
	if (err == CL_DEVICE_NOT_FOUND) {
	  // CPU
	  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
	}
	if (err < 0) {
	  perror("Couldn't access any devices");
	  exit(1);   
	}

	return dev;
}
