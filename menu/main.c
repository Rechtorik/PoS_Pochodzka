#include <stdio.h>
#include <stdlib.h>
#include "../common/inputStruktura.c"
#include "menu.c"
#include <string.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int main() {

  // Veľkosť štruktúry
  size_t inputSize = sizeof(Input);

  // Vytvorenie zdieľanej pamäte
  int shm_fd = shm_open("/shared_input", O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
      perror("shm_open");
      exit(EXIT_FAILURE);
  }

  // Nastavenie veľkosti pamäte
  if (ftruncate(shm_fd, inputSize) == -1) {
      perror("ftruncate");
      exit(EXIT_FAILURE);
  }

  // Mapovanie pamäte
  Input *i = mmap(NULL, inputSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (i == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
  }

  printf("Zdieľaná pamäť bola inicializovaná. Čakám na úpravy...\n");

  menu(i);
  printf("Nazov mapy je: %s \n", i->mapaSubor);
  printf("Nazov simulacie je: %s \n", i->suborUlozenia);
  printf("Opatovne spustenie: %d\n", i->opatovneSpustenie);
  printf("maxX: %d\n", i->maxX);
  printf("maxY: %d\n", i->maxY);
  printf("pVpred: %f\n", i->pVpred);
  printf("pVzad: %f\n", i->pVzad);
  printf("pVpravo: %f\n", i->pVpravo);
  printf("pVlavo: %f\n", i->pVlavo);
  printf("k: %d\n", i->k);
  printf("pocet replikacii: %d\n", i->pocetReplikacii);
  printf("mozu sa ostatni pripojit: %d\n", i->pripojenie);

  // VYTVORENIE TEXTAKU NA INPUT
  if(i->opatovneSpustenie == 0) { // NOVA SIMULACIA
    char cesta[300] = "../../input_files/";
    strcat(cesta, i->suborUlozenia);
    FILE *fileInput = fopen(cesta, "w");

    //if (fileInput == NULL) {
    //    // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
    //    perror("Chyba pri otváraní súboru");
    //}

    // Zápis textu do súboru
    fprintf(fileInput, "%s\n", i->mapaSubor);
    fprintf(fileInput, "%d\n", i->maxX);
    fprintf(fileInput, "%d\n", i->maxY);
    fprintf(fileInput, "%f\n", i->pVpred);
    fprintf(fileInput, "%f\n", i->pVzad);
    fprintf(fileInput, "%f\n", i->pVlavo);
    fprintf(fileInput, "%f\n", i->pVpravo);
    fprintf(fileInput, "%d\n", i->k);
    fprintf(fileInput, "%s\n", i->suborUlozenia);
    fprintf(fileInput, "%d\n", i->pripojenie);

    // Zatvorenie súboru
    fclose(fileInput);
  }

  // spustenie
  int statusServera = system("../server/server");
  if (statusServera == -1) {
      perror("Chyba pri spusteni programu");
  }


  // Odmapovanie pamäte a uvoľnenie zdrojov
  munmap(i, inputSize);
  close(shm_fd);
  //shm_unlink("/shared_input");
  return 0;
}

