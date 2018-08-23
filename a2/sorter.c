//BubbleSort

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include "sorter.h"
#include <string.h>
#include <unistd.h>
#include "a2d.h"

//Defaults
#define SIZE_DEFAULT 100
#define MAX_SIZE 2100

//Globals
static int size = SIZE_DEFAULT;
static long long count;
static int *arr;
static _Bool sortCont = true;
pthread_t t1;
pthread_t t3; //thread for a2d display
pthread_mutex_t sortLock;
pthread_mutex_t sizeLock;

//Helper Functions
void setFlagTrue() {
    sortCont = true;
}

void setFlagFalse() {
    sortCont = false;
}


int genRanNum(int size) {
    return rand() % size; 
}

int *genArr(int size) {
    int *array = malloc(size *sizeof(int));

    for(int i = 0; i < size; i++) {
        array[i] = genRanNum(size);
    }
    
    return array;
}

void sort(int arr[], int size) {
    int tmp;
    for(int i = 0; i < size - 1; i++) {
        for(int j = 0; j < size - i - 1; j++) {
            if(arr[j] > arr[j+1]) {
                tmp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = tmp;
            }
        }
    }
}

int * getArrayCopy() {
    int arrSize = Sorter_getArrayLength();
    if(arrSize == 0) {
        return NULL;
    } else {
        int *p = malloc(arrSize * sizeof(int));
        if(p == NULL) {
            perror("ERROR allocating memory for array");
        }
        memcpy(p, arr, arrSize * sizeof(int));
        return p;
    }
}

void printArray(int *arr) {
    for(int i = 0; i < size; i++) {
        if(i == size -1) {
            printf("%d\n", arr[i]);
        } else {
            printf("%d, ", arr[i]);
        }
    }
}

void initArrSorter() {
    pthread_mutex_lock(&sortLock);
    int knobSetting;
    while(sortCont) {
        knobSetting = A2d_knobValue();
        Sorter_setArraySize(knobSetting);
        arr = genArr(size);
        sort(arr, size);
        free(arr);
        count++;
    }
    pthread_mutex_unlock(&sortLock);
}



//Public Exposed Functions
void Sorter_startSorting(void) {
    count = 0;
    if (pthread_mutex_init(&sortLock, NULL) != 0) {
        printf("Init lock has failed\n");
    }
    if (pthread_mutex_init(&sizeLock, NULL) != 0) {
        printf("Init lock has failed\n");
    }
    int ret1 = pthread_create(&t1, NULL, (void *) &initArrSorter, NULL);
    if(ret1) {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", ret1);
    }
    int ret3 = pthread_create(&t3, NULL, (void *) &A2d_displayArraySize, NULL);
    if(ret3) {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", ret1);
    }
}

void Sorter_stopSorting(void) {
    pthread_mutex_destroy(&sortLock);
    pthread_mutex_destroy(&sortLock);
    setFlagFalse();
    A2d_setA2dFlagFalse();
    pthread_join(t1, NULL);
    pthread_join(t3,NULL);
}

void Sorter_setArraySize (int newSize) {
    pthread_mutex_lock(&sizeLock);
    if (newSize < 0 || newSize > MAX_SIZE) {
        printf("Cannot set size, select size between 1 and 2100\n");
    } else {
            size = newSize;
    }
    pthread_mutex_unlock(&sizeLock);
}

int Sorter_getArrayLength(void) {
    return size;
}

long long Sorter_getNumberArraysSorted(void) {
    return count;
}

int * Sorter_getArray() {
    return getArrayCopy();
}

