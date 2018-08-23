#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <stdbool.h>
#include "sorter.h"
#include <pthread.h>

#define I2C_DEVICE_ADDRESS 0x20
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15
#define NANO_SECOND_MULTIPLIER 1000000 //1 millisecond = 1000000 nanoseconds

static pthread_t t4;
static pthread_t t5;
_Bool sortPerSecFlag = true;
static int displayValue = 0;

// unsigned char zeros[2] = {0xa1,0x86};
// unsigned char ones[2] = {0x01,0x80};
// unsigned char twos[2] = {0x31,0x0e};
// unsigned char threes[2] = {0x21,0x8d};
// unsigned char fours[2] = {0x91,0x88};
// unsigned char fives[2] = {0xb0,0x8c};
// unsigned char sixes[2] = {0xb0,0xe};
// unsigned char sevens[2] = {0xa1,0x80};
// unsigned char eights[2] = {0xb1,0x8e};
// unsigned char nines[2] = {0xb1,0xd};

//unsigned char digits[10][2] = {zeros,ones,twos,threes,fours,fives,sixes,sevens,eights,nines};


unsigned char digits[10][2] = {
  {0xa1,0x86} ,
  {0x01,0x80} ,
  {0x31,0x0e} ,
  {0x21,0x8d} ,
  {0x91,0x88} ,
  {0xb0,0x8c} ,
  {0xb0,0x8e} ,
  {0xa1,0x80} ,
  {0xb1,0x8e} ,
  {0xb1,0x8d}


};

// Insert the above functions here...

static int initI2cBus(char* bus, int address)
{
    int i2cFileDesc = open(bus, O_RDWR);
    if (i2cFileDesc < 0) {
    printf("I2C: Unable to open bus for read/write (%s)\n", bus);
    perror("Error is:");
    exit(1);
    }
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
    perror("I2C: Unable to set I2C device to slave address.");
    exit(1);
    }
    return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr,
unsigned char value)
{
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write i2c register.");
        exit(1);
    }
}

// static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
// {
//     // To read a register, must first write the address
//     int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
//     if (res != sizeof(regAddr)) {
//     perror("I2C: Unable to write to i2c register.");
//     exit(1);
//     }
//     // Now read the value and return it
//     char value = 0;
//     res = read(i2cFileDesc, &value, sizeof(value));
//     if (res != sizeof(value)) {
//     perror("I2C: Unable to read from i2c register");
//     exit(1);
//     }
//     return value;
// }

void leftDigitOn() {
  FILE *fp = fopen("/sys/class/gpio/gpio44/value","w");
  if (fp == NULL) {
    printf("Error opening file!\n");
  }

  fprintf(fp,"1");

  fclose(fp);
}

void leftDigitOff() {
  FILE *fp = fopen("/sys/class/gpio/gpio44/value","w");
  if (fp == NULL) {
    printf("Error opening file!\n");
  }

  fprintf(fp,"0");

  fclose(fp);

}


void rightDigitOn() {
  FILE *fp = fopen("/sys/class/gpio/gpio61/value","w");
  if (fp == NULL) {
    printf("Error opening file!\n");
  }

  fprintf(fp,"1");

  fclose(fp);
}


void rightDigitOff() {
  FILE *fp = fopen("/sys/class/gpio/gpio61/value","w");
  if (fp == NULL) {
    printf("Error opening file!\n");
  }

  fprintf(fp,"0");

  fclose(fp);

}





int display()
{
  int displayDigit = displayValue;
  if (displayDigit > 99) {
    displayDigit = 99;
  }
  else if (displayDigit < 0) {
    displayDigit = 0;
  }
  int digitArray[2];
  int counter = 0;
  int second = 100;
  if (displayDigit < 10) {
    digitArray[1] = 0;
  }

  while(displayDigit != 0) {
    digitArray[counter] = displayDigit % 10;
    displayDigit /= 10;
    counter++;
  }



  long timeInterval = 5 * NANO_SECOND_MULTIPLIER;
  while(second > 0) {

    leftDigitOff();
    rightDigitOff();



    //printf("Drive display (assumes GPIO #61 and #44 are output and 1\n");
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

    writeI2cReg(i2cFileDesc, REG_OUTA, digits[digitArray[1]][0]);
    writeI2cReg(i2cFileDesc, REG_OUTB, digits[digitArray[1]][1]);

    leftDigitOn();

    nanosleep((const struct timespec[]){{0,timeInterval}},NULL);

    leftDigitOff();


    writeI2cReg(i2cFileDesc, REG_OUTA, digits[digitArray[0]][0]);
    writeI2cReg(i2cFileDesc, REG_OUTB, digits[digitArray[0]][1]);

    rightDigitOn();




    // Read a register:
    //readI2cReg(i2cFileDesc, REG_OUTA);
    //printf("Reg OUT-A = 0x%02x\n", regVal);
    // Cleanup I2C access;
    close(i2cFileDesc);
    second--;
  }
    return 0;
}



int displayArray (int array[10]) {
  int a = 0;
  while(a < 10) {


    int displayDigit = array[a];

    if (displayDigit > 99) {
      displayDigit = 99;
    }
    else if (displayDigit < 0) {
      displayDigit = 0;
    }



    display(displayDigit);
    a++;

  }

  return 0;
}

double arraySortPerSec() {
    long long counterStart = 0;
    long long counterEnd = 0;
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    counterStart = Sorter_getNumberArraysSorted();
    sleep(1);
    counterEnd = Sorter_getNumberArraysSorted();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    double sortPerSec = (counterEnd - counterStart) / cpu_time_used;
    return sortPerSec;
}


void setSortPerSecFlagOff() {
    sortPerSecFlag = false;
}

void infiniteSortsPerSecCalc() {
    double value = 0;
    while(sortPerSecFlag) {
        value = (int) arraySortPerSec();
        displayValue = value;

        display();
         int ret5 = pthread_create(&t4, NULL, (void *) &display, NULL);
        ret5 = ret5;


        //ret5 = 0;
    }
}

void SD_initSegDisplay() {
    int ret4 = pthread_create(&t4, NULL, (void *) &infiniteSortsPerSecCalc, NULL);
    if(ret4) {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", ret4);
    }




}

void SD_stopSegDisplay() {
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);
    setSortPerSecFlagOff();
}


// int main() {
//     int array[10] = {3,45,123,5132,12,35,32,0,3,2};
//     displayArray(array);
//     double sorts = arraySortPerSec();
//     printf("Sorts per second: %f\n", sorts);
//     //display(12);

//     return 0;
// }
