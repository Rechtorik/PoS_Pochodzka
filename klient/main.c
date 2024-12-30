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
  int shm_fd = shm_open("/shared_input_jojo", O_CREAT | O_RDWR, 0666);
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

  menu(i);
  printf("Nazov mapy je: %s \n", i->mapaSubor);
  printf("Dlzka nazva mapy: %d \n", (int)strlen(i->mapaSubor));
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


  // V tomto momente uz mame input z menu


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
  } else {  // OPATOVNE SPUSTENIE SIMULACIE
    
    char cesta[300] = "../../input_files/";
    strcat(cesta, i->suborUlozenia);
    FILE *fileInput = fopen(cesta, "r");

    //if (fileInput == NULL) {
    //    // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
    //    perror("Chyba pri otváraní súboru");
    //}
    
    char buffer[256]; // Buffer na uloženie prečítaného obsahu
    // Čítanie obsahu riadok po riadku
    
    fgets(buffer, sizeof(buffer), fileInput);
    // nazovMapy
    buffer[strlen(buffer) - 1] = '\0';
    strcat(i->mapaSubor, buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    // maxX
    buffer[strlen(buffer) - 1] = '\0';
    i->maxX = atoi(buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    //maxY
    buffer[strlen(buffer) - 1] = '\0';
    i->maxY = atoi(buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    //pVpred
    buffer[strlen(buffer) - 1] = '\0';
    i->pVpred = atof(buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    //pVzad
    buffer[strlen(buffer) - 1] = '\0';
    i->pVzad = atof(buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    //pVlavo
    buffer[strlen(buffer) - 1] = '\0';
    i->pVlavo = atof(buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    //pVpravo
    buffer[strlen(buffer) - 1] = '\0';
    i->pVpravo = atof(buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    //k
    buffer[strlen(buffer) - 1] = '\0';
    i->k = atoi(buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    //nazovSimulacie
    buffer[strlen(buffer) - 1] = '\0';
    strcat(i->suborUlozenia, buffer);
    
    fgets(buffer, sizeof(buffer), fileInput);
    //pripojenie (0 alebo 1)
    buffer[strlen(buffer) - 1] = '\0';
    i->pripojenie = atoi(buffer);

    // Zatvorenie súboru
    fclose(fileInput);
  }

  // V tomto momente uz mame naplnenu strukturu isto isto (nacitavanie zo suboru)
  // Uz vieme co ideme robit, len to treba spravit


  // spustenie
  int statusServera = system("../server/server");
  if (statusServera == -1) {
      perror("Chyba pri spusteni programu server");
  }


  // Odmapovanie pamäte a uvoľnenie zdrojov
  munmap(i, inputSize);
  close(shm_fd);
  shm_unlink("/shared_input_jojo");
  return 0;
}

