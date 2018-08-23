#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include "sorter.h"
#include "udpclient.h"
#include "segDisplay.h"
#include "gpioDriver.h"

//extern pthread_t t1;
//extern pthread_t t2;

int main(void) {


    printf("\n\nTo connect to UDP server, open new terminal with the following command:\n");
    printf("netcat -u 192.168.7.2 12345\n\n");
    //Sorter_setArraySize(500);
    UDP_init();
    Sorter_startSorting();
    SD_initSegDisplay();
    UDP_stop();
    SD_stopSegDisplay();
    printf("Done!\n");
    return 0;
}
