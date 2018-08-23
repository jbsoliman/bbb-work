#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include "udpclient.h"
#include "sorter.h"
#include <math.h>

#define BUFSIZE 1024
#define SHORT_MSG_BUFFER 40
#define NUM_ARGS 2
#define BITS_FOR_ARR_TO_STR 15 //32-bit INT = 12 bits + 2 bits for ',' & ' ' + 1 

_Bool keepRunFlag = true;
pthread_t t2;
int sockfd;
int portno;
socklen_t addr_size;
struct sockaddr_in serveraddr;
struct sockaddr_in clientaddr;
char *buf;
char *bufCpy;
char *bufArr[NUM_ARGS];
int reset;
int n;
int argStr2Int;
char *ptr;

int sendDatagram(char *inputStr) {
    int inputLen = strlen(inputStr) + 1; //+1 for additional null terminating character(s)
    int q = sendto(sockfd, inputStr, inputLen, 0,
                (struct sockaddr *)&clientaddr, addr_size);
    if (q < 0) { perror("ERROR in sendto"); }
    return q;
}


void appToEnd(char *inputStr, char lineBreak) {
    int inputLen = strlen(inputStr);
    inputStr[inputLen] = lineBreak;
    inputStr[inputLen+1] = '\0';
}

void UDP_stop() {
    pthread_join(t2, NULL);
    Sorter_stopSorting();
    keepRunFlag = false;
}

void UDP_client() {
    portno = 12345;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR OPENING SOCKET!\n");
        exit(1);
    }

    //Reset port & server when killing program
    reset = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &reset, sizeof(int));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);
    addr_size = sizeof(clientaddr);

    if(bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("ERROR BINDING PORT\n");
        exit(1);
    }

    //MESSAGES TO DISPLAY STRINGS BASED ON COMMAND
    char help[] = "\nAccepted command examples:\n \
    count         -- display number arrays sorted.\n \
    get length    -- display length of array currently being sorted\n \
    get array     -- display the full array being sorted\n \
    get 10        -- display the tenth element of array currently being sorted\n \
    stop          -- cause the server program to end\n";
    char stopMsg[] = "This is the stop message!\n";
    char invalidMsg[] = "This is an invalid command!\n";
    char lineBreak[] = "\n\0";
    //------------------------------------------------------
    buf = malloc((BUFSIZE+1) * sizeof(char)); // +1 for additioinal terminating characters
    bufCpy = malloc((BUFSIZE+1) * sizeof(char)); 

    	while (keepRunFlag) {
		memset(buf, '\0', BUFSIZE); //Clear buffer / fill it with null terminating characters
		n = recvfrom(sockfd, buf, BUFSIZE, 0,
			     (struct sockaddr *)&clientaddr, &addr_size);
		if (n < 0) { perror("ERROR in recvfrom"); }

		printf("server received %d bytes\n", n);

        //Parse buffer
        strcpy(bufCpy, buf);
        char *p = strtok (bufCpy, " ");       
        bufArr[0] = p;
        p = strtok (NULL, " ");
        bufArr[1] = p;

        // printf("Tokenizer output:\n");
        // printf("[0] = %s\n", bufArr[0]);
        // printf("[1] = %s\n", bufArr[1]);

        //Conversion of output
        if(bufArr[1] != NULL) {
            argStr2Int = (int) strtol(bufArr[1], &ptr, 10);
        }

        if(strcmp(buf, "help\n") == 0) {
            printf("Help command recognized!\n");
            n = sendDatagram(help);
        } else if(strcmp(buf, "count\n") == 0) {
            printf("Count command recognized!\n");
            int ctn = Sorter_getNumberArraysSorted();
            char snum[SHORT_MSG_BUFFER]; 
            sprintf(snum, "Number of arrays sorted: %d\n", ctn);
            n = sendDatagram(snum);
        } else if(strcmp(buf, "get array\n") == 0) {
            printf("Get array recognized!\n");
            int *arrCopy = Sorter_getArray();
            int arrCopyLen = Sorter_getArrayLength();
            if(arrCopyLen == 0) {
                char *tooSmallMsg = "[] Empty Array\n";
                n = sendDatagram(tooSmallMsg);
            } else {
                char *arrString = malloc(BUFSIZE * BITS_FOR_ARR_TO_STR); 
                int index = 0;
                for(int i = 0; i< arrCopyLen; i++) {
                    index += sprintf(&arrString[index], "%d ,", arrCopy[i]);
                }
                int arrStrLen = strlen(arrString)+1;
                if(arrStrLen >= BUFSIZE) {
                    double loopCount = 0;
                    loopCount = (double) arrStrLen / (double) BUFSIZE;
                    int ceilLoopCount = ceil(loopCount);
                    for(int i = 0; i < ceilLoopCount; i++) {
                        char subString[BUFSIZE];
                        strncpy(subString, arrString+(i*BUFSIZE), BUFSIZE);
                        n = sendDatagram(subString);
                        n = sendDatagram(lineBreak);
                    }
                } else {
                    n = sendto(sockfd, arrString, arrStrLen, 0,
                    (struct sockaddr *)&clientaddr, addr_size);
                }
                free(arrCopy);
                free(arrString);
            }
        } else if(strcmp(buf, "stop\n") == 0) {
            printf("Stop command recognized!\n");
            n = sendDatagram(stopMsg);
            UDP_stop();
            break;
        } else if(strcmp(buf, "get length\n") == 0) {
            printf("Get element recognized!\n");
            int len = Sorter_getArrayLength();
            char slen[SHORT_MSG_BUFFER];
            sprintf(slen, "Current Array Length: %d\n", len);
            n = sendDatagram(slen);
        } else if (strcmp(bufArr[0], "get") == 0 && bufArr[1] != NULL){
            int arrayLen = Sorter_getArrayLength();
            int *arrCopy4elem = Sorter_getArray();
            char inValMsg[BUFSIZE];
            sprintf(inValMsg, "Invalid Argument. Must be betwen 1 and %d (array length)\n", arrayLen);
            if(argStr2Int < 1 || argStr2Int > arrayLen) {
                n = sendDatagram(inValMsg);
            } else if(arrayLen == 0) {
                n = sendDatagram(inValMsg);
            } else {
                char valLenMsg[BUFSIZE];
                if(argStr2Int == arrayLen) {
                    sprintf(valLenMsg, "Value %d = %d\n", argStr2Int, arrCopy4elem[argStr2Int-1]);
                } else {
                    sprintf(valLenMsg, "Value %d = %d\n", argStr2Int, arrCopy4elem[argStr2Int-1]);
                }
                n = sendDatagram(valLenMsg);
            }
            free(arrCopy4elem);
        } else {
            printf("Invalid command!\n");
            n = sendDatagram(invalidMsg);
        }
	}
    free(bufCpy);
    free(buf);

}

void UDP_init() {
    int ret2 = pthread_create(&t2, NULL, (void *) &UDP_client, NULL);
    if(ret2) {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", ret2);
    }
}
