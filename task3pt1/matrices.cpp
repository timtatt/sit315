#include<iostream>
#include<pthread.h>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>
#include<mpi.h>

const int matrixSize = 3;

int a[matrixSize][matrixSize];
int b[matrixSize][matrixSize];
int c[matrixSize][matrixSize];

// class Matrix {
//     int storageIndex;
//     int storageOffset;
//     int matrixSize;
//     int* data;
//
//     public:
//         Matrix(int* storage, int index, int size) {
//             data = storage;
//             storageIndex = index;
//             matrixSize = size;
//             storageOffset = storageIndex * size * size;
//         }
//         int Get(int i, int j) {
//             int k = storageOffset + (matrixSize * i) + j;
//             return (int) data[k];
//         }
//         void Set(int i, int j, int value) {
//             int k = storageOffset + (matrixSize * i) + j;
//             data[k] = value;
//         }
// };
//
// Matrix a(x, 0, matrixSize);
// Matrix b(x, 1, matrixSize);
// Matrix c(y, 0, matrixSize);

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

    for (int i = 0; i < matrixSize; i++) {
        cout << a[i][0];
        for (int j = 1; j < matrixSize; j++) {
            cout << ", " << a[i][j];
        }
        cout << "\n";
    }
    cout << "\n";

    for (int i = 0; i < matrixSize; i++) {
        cout << b[i][0];
        for (int j = 1; j < matrixSize; j++) {
            cout << ", " << b[i][j];
        }
        cout << "\n";
    }

}

// int* coordsFromNum(int num) {
//     static int coords[2];
//     int row = floor(num / matrixSize);
//     int col = num - row * matrixSize;
//
//     coords[0] = row;
//     coords[1] = col;
//     return coords;
// }

int main(int argc, char *argv[]) {
    // Generate Matrices;
    clock_t timer;
    double timeTaken = 0;

    // double **A, **B, **C, *tmp;
    int numTasks, rank, tag = 1;
    int numThreads = atoi(argv[1]), chunkSize = 10;

    omp_set_num_threads(numThreads);

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
    } else if (rank > 0) {

        MPI_Recv(&a, matrixSize * matrixSize, MPI_INT, 0, tag, MPI_COMM_WORLD, &Stat);
        MPI_Recv(&b, matrixSize * matrixSize, MPI_INT, 0, tag, MPI_COMM_WORLD, &Stat);

        int lowerBound = (rank - 1) * rowsPerTask;
        int upperBound = lowerBound + rowsPerTask;

        if (upperBound > matrixSize) {
            upperBound = matrixSize;
        }

        if (lowerBound < upperBound) {

            // Debugging Line
            // cout << "Rank: " << rank << ", LB: " << lowerBound << ", UB: " << upperBound << "\n" << endl;

            #pragma omp parallel for shared(a, b, taskResult, numThreads) schedule (static, chunkSize)
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
        cout << endl;

        for (int i = 0; i < matrixSize; i++) {
            cout << c[i][0];
            for (int j = 1; j < matrixSize; j++) {
                cout << ", " << c[i][j];
            }
            cout << "\n";
        }

        cout << "MPI: " << timeTaken << "s\n";
    }

    MPI_Finalize();
    return 0;
}
