#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<time.h>
#include<stack>

using namespace std;

const int seed = 10;
const int arrayLength = 50;

int array[arrayLength];

void generateArray() {
    srand(seed);
    for (int i = 0; i < arrayLength; i++) {
        array[i] = rand() % arrayLength + 1;
    }
}

void printArray() {
    cout << array[0];
    for (int i = 1; i < arrayLength; i++) {
        cout << ", " << array[i];
    }
    cout << "\n\n";
}

int partitionArray(long low, long high) {
    int pivot = array[low + (high - low) / 2];
    int i = low - 1;
    int j = high + 1;

    while (true) {
        do {
            i++;
        } while (array[i] < pivot);

        do {
            j--;
        } while (array[j] > pivot);

        if (i >= j) {
            return j;
        }

        int tempI = array[i];
        array[i] = array[j];
        array[j] = tempI;
    }
}

void sortArray(long low, long high) {
    if (low < high) {
        int pivot = partitionArray(low, high);

        sortArray(low, pivot);
        sortArray(pivot + 1, high);
    }
}

void parallelSortArray(long low, long high) {
    if (low < high) {
        int pivot = partitionArray(low, high);

        // OpenMP
        // Once again OpenMP the dog has made me use a { on the next line
        #pragma omp parallel sections
        {
            #pragma omp section
            sortArray(low, pivot);

            #pragma omp section
            sortArray(pivot + 1, high);
        }
    }
}

int main() {
    clock_t timer;
    double timeTaken = 0;

    generateArray();

    // Sequential
    timer = clock();

    sortArray(0, arrayLength - 1);

    timeTaken = (clock() - timer) / (double) CLOCKS_PER_SEC;
    printArray();
    generateArray();
    printArray();
    cout << "Time Taken: " << timeTaken << "\n";

    timer = clock();

    parallelSortArray(0, arrayLength - 1);

    timeTaken = (clock() - timer) / (double) CLOCKS_PER_SEC;
    printArray();
    cout << "Time Taken: " << timeTaken << "\n";

    return 0;
}
