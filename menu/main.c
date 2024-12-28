#include <stdio.h>
#include <stdlib.h>
#include "inputStruktura.c"
#include "menu.c"

int main() {
  Input i;
  menu(&i);
  if(i.mapaSubor != NULL) printf("Cesta k suboru s mapou je %s \n", i.mapaSubor);
  if(i.suborUlozenia != NULL) printf("Cesta k suboru ulozenia je %s \n", i.suborUlozenia);
  if(i.suborSoSimulaciou != NULL) printf("Cesta k suboru predoslej simulacie %s \n", i.suborSoSimulaciou);
  printf("Opatovne spustenie: %d\n", i.opatovneSpustenie);
  printf("maxX: %d\n", i.maxX);
  printf("maxY: %d\n", i.maxY);
  printf("pVpred: %f\n", i.pVpred);
  printf("pVzad: %f\n", i.pVzad);
  printf("pVpravo: %f\n", i.pVpravo);
  printf("pVlavo: %f\n", i.pVlavo);
  printf("k: %d\n", i.k);
  printf("pocet replikacii: %d\n", i.pocetReplikacii);
  printf("mozu sa ostatni pripojit: %d\n", i.pripojenie);



  // dealokacia
  if(i.mapaSubor) free(i.mapaSubor);
  if(i.suborUlozenia) free(i.suborUlozenia);
  if(i.suborSoSimulaciou) free(i.suborSoSimulaciou);
  return 0;
}

