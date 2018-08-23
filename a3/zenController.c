#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>


#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x1c
#define CTRL_REG1 0x2A
#define OUT_X_MSB 0x01
#define OUT_X_LSB 0x02
#define OUT_Y_MSB 0x03
#define OUT_Y_LSB 0x04
#define OUT_Z_MSB 0x05
#define OUT_Z_LSB 0x06



const char* position[] = {"up","right","down","left","push","none"};
const char* directionDir[] = {"/sys/class/gpio/gpio26/value","/sys/class/gpio/gpio47/value","/sys/class/gpio/gpio46/value","/sys/class/gpio/gpio65/value","/sys/class/gpio/gpio27/value"};


static pthread_t ZencapeThread;





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


static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
{// To read a register, must first write the address
int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
  if (res != sizeof(regAddr)) {
    perror("I2C: Unable to write to i2c register.");
    exit(1);
  }
// Now read the value and return it
  char value = 0;
  res =read(i2cFileDesc, &value, sizeof(value));
  if (res != sizeof(value)) {
    perror("I2C: Unable to read from i2c register");
    exit(1);
  }
  return value;
}




void writeToFile(char *fileName, int gpio) {
  // Use fopen() to open the file for write access.
FILE *pfile = fopen(fileName, "w");
if (pfile == NULL) {
printf("ERROR: Unable to open export file.\n");
exit(1);
}
// Write to data to the file using fprintf():
fprintf(pfile, "%d", gpio);

// Close the file using fclose():
fclose(pfile);
}








const char * readFromFile(const char *fileName){
FILE *file = fopen(fileName, "r");

if (file == NULL) {
  printf("ERROR: Unable to open file (%s) for read\n", fileName);
  exit(-1);
}

// Read string (line)
//const int max_length = 1024;
char* buff;
buff = malloc(sizeof(char)*100);
fgets(buff, 2, file);
// Close
fclose(file);

return buff;
}

//returns current joystick position
const char * Zencape_checkDir() {
  int i = 0;
  for (i = 0; i < 5; i++) {
    const char * line = readFromFile(directionDir[i]);
    if (strcmp(line, "0")) {
    }
    else {
      //should return correct direction joystick is in
      return position[i];
    }
  }
  //Returns none if no direction is pressed on joystick
  return position[5];


}

void Zencape_debounce(const char* value) {

  nanosleep((const struct timespec[]){{0,100000000}},NULL);
  if(strcmp(value,Zencape_checkDir())) {
      printf("Stage Changed\n");

  }
}

void Zencape_stop() {
  pthread_join(ZencapeThread, NULL);
}

void Zencape_checkState() {
  while(1) {
  const char* value = Zencape_checkDir();
  if(strcmp(value, "none")) {
    printf("%s\n",value);
    Zencape_debounce(value);
  }
  nanosleep((const struct timespec[]){{0,10000000}},NULL);
  }
}



void Zencape_init() {
  int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

  writeI2cReg(i2cFileDesc, CTRL_REG1, 0x01);
  printf("i2c register reads: %d\n", readI2cReg(i2cFileDesc, OUT_X_LSB));
  writeToFile("/sys/class/gpio/export",26);
  writeToFile("/sys/class/gpio/export",47);
  writeToFile("/sys/class/gpio/export",46);
  writeToFile("/sys/class/gpio/export",65);
  writeToFile("/sys/class/gpio/export",27);
  close(i2cFileDesc);

  pthread_create(&ZencapeThread, NULL, (void *) &Zencape_checkDir, NULL);

}
