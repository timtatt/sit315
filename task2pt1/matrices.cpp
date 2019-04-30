#include<iostream>
#include<pthread.h>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>

const int matrixSize = 800;
const int numThreads = 4;

int rowsPerThread = floor(matrixSize / numThreads);
int a[matrixSize][matrixSize];
int b[matrixSize][matrixSize];
int c[matrixSize][matrixSize];

using namespace std;

void* calculateRow(void* threadArg) {
    long threadId = (long) threadArg;

    int lowerBound = (threadId * rowsPerThread);
    int upperBound = (threadId * rowsPerThread) + rowsPerThread;

    if (upperBound > matrixSize) {
        upperBound = matrixSize;
    }

    for (int i = lowerBound; i < upperBound; i++) {
        for (int j = 0; j < matrixSize; j++) {
            c[i][j] = 0;
            for (int k = 0; k < matrixSize; k++) {
                c[i][j] += a[i][j] * b[k][j];
            }
        }
    }

    pthread_exit(NULL);
}


int main() {
    // Generate Matrices;
    int seed = 10;
    clock_t timer;
    double timeTaken = 0;
    pthread_t threads[numThreads];

    srand(seed);

    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            a[i][j] = rand() % 10 + 1;
            b[i][j] = rand() % 10 + 1;
        }
    }


    timer = clock();

    // Sequential Matrix
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            c[i][j] = 0;
            for (int k = 0; k < matrixSize; k++) {
                c[i][j] += a[i][j] * b[k][j];
            }
        }
    }

    timeTaken = (clock() - timer) / (double) CLOCKS_PER_SEC;
    cout << "Sequential Time Taken: " << timeTaken << "s\n";

    timer = clock();

    // Threaded Matrix
    for (long i = 0; i < numThreads; i++) {
        pthread_create(&threads[i], NULL, calculateRow, (void*) i);
    }

    for (long i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    timeTaken = (clock() - timer) / (double) CLOCKS_PER_SEC;
    cout << "Parallel Time Taken: " << timeTaken << "s\n";

    timer = clock();

    // OpenMP (stupid fucker makes me use a next line {)
    #pragma omp parallel num_threads(numThreads)
    {
        long threadId = omp_get_thread_num();

        int lowerBound = (threadId * rowsPerThread);
        int upperBound = (threadId * rowsPerThread) + rowsPerThread;

        if (upperBound > matrixSize) {
            upperBound = matrixSize;
        }

        for (int i = lowerBound; i < upperBound; i++) {
            for (int j = 0; j < matrixSize; j++) {
                c[i][j] = 0;
                for (int k = 0; k < matrixSize; k++) {
                    c[i][j] += a[i][j] * b[k][j];
                }
            }
        }


    }

    timeTaken = (clock() - timer) / (double) CLOCKS_PER_SEC;
    cout << "OpenMP Time Taken: " << timeTaken << "s\n";

    return 0;
}
