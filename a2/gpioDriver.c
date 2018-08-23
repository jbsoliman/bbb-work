#include <stdio.h>
#include <stdlib.h>



void setExport() {

  FILE *fp1 = fopen("/sys/class/gpio/export","w");
  if (fp1 == NULL) {
    printf("Error opening file!\n");
  }

  fprintf(fp1,"44");

  fclose(fp1);



    FILE *fp2 = fopen("/sys/class/gpio/export","w");
  if (fp2 == NULL) {
    printf("Error opening file!\n");
  }

  fprintf(fp2,"61");

  fclose(fp2);


}



void setDirectionOut() {

  FILE *fp3 = fopen("/sys/class/gpio/gpio44/direction","w");
  if (fp3 == NULL) {
    printf("Error opening file!\n");
  }

  fprintf(fp3,"in");

  fclose(fp3);


    FILE *fp4 = fopen("/sys/class/gpio/gpio61/direction","w");
    if (fp4 == NULL) {
      printf("Error opening file!\n");
    }

    fprintf(fp4,"in");

    fclose(fp4);



}
