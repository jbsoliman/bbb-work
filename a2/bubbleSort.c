//BubbleSort

#include <stdio.h>


void numSwap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main(void) {
    int i = 1;
    int j = 2;
    numSwap(&i,&j);
    printf("%d\n%d\n", i,j);
    return 0;
}