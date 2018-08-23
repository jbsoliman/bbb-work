// Assumes ADC cape already loaded:
// root@beaglebone:/# echo BB-ADC > /sys/devices/platform/bone_capemgr/slots
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "a2d.h"
#include <stdbool.h>

#define A2D_FILE_VOLTAGE0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define A2D_VOLTAGE_REF_V 1.8
#define A2D_MAX_READING 4095

_Bool a2dFlag = true;

int getVoltage0Reading()
{
// Open file
FILE *f = fopen(A2D_FILE_VOLTAGE0, "r");
if (!f) {
printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
printf("try: echo BB-ADC > /sys/devices/platform/bone_capemgr/slots\n");
exit(-1);
}
// Get reading
int a2dReading = 0;
int itemsRead = fscanf(f, "%d", &a2dReading);
if (itemsRead <= 0) {
printf("ERROR: Unable to read values from voltage input file.\n");
exit(-1);
}
// Close file
fclose(f);
return a2dReading;
}

double convertReadingtoArraySize(int voltageReading) {

int a2dReadingPWL[10] = {0,500,100,1500,2000,2500,3000,3500,4000,4100};
int arraySizePWL[10] = {1,20,60,120,250,300,500,800,1200,2100};

int i = 0;

for (i = 0; i < 10; i++) {
  if (a2dReadingPWL[i] < voltageReading && a2dReadingPWL[i+1] > voltageReading) {
    double s = voltageReading;
    double a = a2dReadingPWL[i];
    double b = a2dReadingPWL[i+1];
    double m = arraySizePWL[i];
    double n = arraySizePWL[i+1];

    return((((s-a)/(b-a))*(n-m))+m);
  }
}

return 0;

}

double A2d_knobValue() {
  int voltage = getVoltage0Reading();
  int arrayConvert = convertReadingtoArraySize(voltage);
  return arrayConvert;
}

void A2d_setA2dFlagFalse() {
  a2dFlag = false;
}

int A2d_displayArraySize() {
  while (a2dFlag) {
    //int reading = getVoltage0Reading();
    int arraySize = A2d_knobValue();
    //double voltage = ((double)reading / A2D_MAX_READING) * A2D_VOLTAGE_REF_V;
    ///printf("Value %5d ==> %5.2fV\n", reading, voltage);
    printf("Array Size = %d\n",arraySize);
    sleep(1);
    }
    return 0;
}





// int main() {
//   displayArraySize();
//   return 0;
// }
