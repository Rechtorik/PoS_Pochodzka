#include <stdio.h>
#include <stdlib.h>
#include "../common/inputStruktura.h"
#include "../common/prekazky.h"

#include "kreslenie.c"
#include "menu.c"

#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define FIFO_RESULT "../../fifo_files/"

int main(int argc, char* argv[]) {

  // Veľkosť štruktúry
  size_t inputSize = sizeof(Input);

  // Vytvorenie zdieľanej pamäte
  int shm_fd = shm_open("/sem.shared_input_RJ", O_CREAT | O_RDWR, 0666);
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
  Input *input = mmap(NULL, inputSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (input == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
  }

  // ZDIELANA PAMAT - MAPA
  // Veľkosť štruktúry
    size_t mapaSize = sizeof(Mapa);

    // Vytvorenie zdieľanej pamäte
    int shm_mapa_fd = shm_open("/sem.shared_mapa_RJ", O_CREAT | O_RDWR, 0666);
    if (shm_mapa_fd == -1) {
      perror("shm_open");
      exit(EXIT_FAILURE);
    }

    // Nastavenie veľkosti pamäte
    if (ftruncate(shm_mapa_fd, mapaSize) == -1) {
      perror("ftruncate");
      exit(EXIT_FAILURE);
    }

    // Mapovanie pamäte
    Mapa *mapa = mmap(NULL, mapaSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_mapa_fd, 0);
    if (mapa == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
    }






  menu(input);
  
  // V tomto momente uz mame input z menu


  // VYTVORENIE TEXTAKU NA INPUT
  if(input->opatovneSpustenie == 0) { // NOVA SIMULACIA
    char cesta[300] = "../../input_files/";
    strcat(cesta, input->suborUlozenia);
    FILE *fileInput = fopen(cesta, "w");

    //if (fileInput == NULL) {
    //    // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
    //    perror("Chyba pri otváraní súboru");
    //}

    // Zápis textu do súboru
    fprintf(fileInput, "%s\n", input->mapaSubor);
    fprintf(fileInput, "%d\n", input->maxX);
    fprintf(fileInput, "%d\n", input->maxY);
    fprintf(fileInput, "%f\n", input->pVpred);
    fprintf(fileInput, "%f\n", input->pVzad);
    fprintf(fileInput, "%f\n", input->pVlavo);
    fprintf(fileInput, "%f\n", input->pVpravo);
    fprintf(fileInput, "%d\n", input->k);
    fprintf(fileInput, "%s\n", input->suborUlozenia);
    fprintf(fileInput, "%d\n", input->pripojenie);

    // Zatvorenie súboru
    fclose(fileInput);
  } else {  // OPATOVNE SPUSTENIE SIMULACIE

    char cesta[300] = "../../input_files/";
    strcat(cesta, input->suborUlozenia);
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
    strcat(input->mapaSubor, buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    // maxX
    buffer[strlen(buffer) - 1] = '\0';
    input->maxX = atoi(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //maxY
    buffer[strlen(buffer) - 1] = '\0';
    input->maxY = atoi(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVpred
    buffer[strlen(buffer) - 1] = '\0';
    input->pVpred = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVzad
    buffer[strlen(buffer) - 1] = '\0';
    input->pVzad = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVlavo
    buffer[strlen(buffer) - 1] = '\0';
    input->pVlavo = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVpravo
    buffer[strlen(buffer) - 1] = '\0';
    input->pVpravo = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //k
    buffer[strlen(buffer) - 1] = '\0';
    input->k = atoi(buffer);
  
    fgets(buffer, sizeof(buffer), fileInput);
    //nazov simulacie
    buffer[strlen(buffer) - 1] = '\0';
    //input->suborUlozenia = buffer;

    fgets(buffer, sizeof(buffer), fileInput);
    //pripojenie (0 alebo 1)
    buffer[strlen(buffer) - 1] = '\0';
    input->pripojenie = atoi(buffer);

    fclose(fileInput);
  }

  // debug
  printf("Veľkosť mapy je: %d\n", input->maxX*2+1 * input->maxY*2+1);
  printf("Nazov mapy je: %s \n", input->mapaSubor);
  printf("Dlzka nazva mapy: %d \n", (int)strlen(input->mapaSubor));
  printf("Nazov simulacie je: %s \n", input->suborUlozenia);
  printf("Opatovne spustenie: %d\n", input->opatovneSpustenie);
  printf("maxX: %d\n", input->maxX);
  printf("maxY: %d\n", input->maxY);
  printf("pVpred: %f\n", input->pVpred);
  printf("pVzad: %f\n", input->pVzad);
  printf("pVpravo: %f\n", input->pVpravo);
  printf("pVlavo: %f\n", input->pVlavo);
  printf("k: %d\n", input->k);
  printf("pocet replikacii: %d\n", input->pocetReplikacii);
  printf("mozu sa ostatni pripojit: %d\n", input->pripojenie);
  printf("Zapnute vykreslenie: %d\n", input->vykreslenie);

  // V tomto momente uz mame naplnenu strukturu isto isto (nacitavanie zo suboru)
  // Uz vieme co ideme robit, len to treba spravit


  


  char fifo_cesta[256] = FIFO_RESULT;
  strcat(fifo_cesta, input->suborUlozenia);
  for(int i = 0; i < 4; i++) {
    fifo_cesta[strlen(fifo_cesta)-1] = '\0';
  }
  
  // Skontroluj, či FIFO existuje
    if (access(fifo_cesta, F_OK) == -1) {
        // FIFO neexistuje, pokus o jeho vytvorenie
        if (mkfifo(fifo_cesta, 0666) == -1) {
            perror("Chyba pri vytváraní FIFO");
            return 1;
        }
    } else {
        printf("FIFO už existuje: %s\n", fifo_cesta);
    }


  // spustenie
  int statusServera = system("../server/server &"); // ampersant na to aby bezal server na pozadi
  if (statusServera == -1) {
      perror("Chyba pri spusteni programu server");
  }

  if(input->vykreslenie) {
    // kód vykreslenia
    printf("Vykresluje sa 𓁹‿𓁹\n");
    //Suradnice* prekazky = malloc(sizeof(Suradnice)*mapa->prekazky.pocet);
    
    usleep(500000);
    printf("Klient: pred vykreslenim\n");
    kresli(input, mapa->opilec, mapa->prekazky.pocet, &(mapa->prekazky));
  }

  //NACITANIE VYSLEDKOV (POCTY KOLKO KRAT SA DOSTAL DO STREDU)
  // Otvorenie FIFO na čítanie
    int fd_result = open(fifo_cesta, O_RDONLY);
    if (fd_result == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Pole na uloženie prijatých hodnôt
    float result[input->maxY*2+1][input->maxX*2+1];
    memset(result, 0, sizeof(result));

    // Čítanie hodnôt z FIFO
    for(int i = 0; i < input->maxY*2+1; i++) {
      if(read(fd_result, result[i], sizeof(float)*input->maxX*2+1) == -1) {
        perror("Chyba pri čítaní Klient result\n");
        return 1;
      }
    }

    // Vypísanie prijatých hodnôt
    for (int i = 0; i < input->maxY*2+1; i++) {
      for(int j = 0; j < input->maxX*2+1; j++) {
        printf("%6.2f ", result[i][j]);
      }
      printf("\n");
    }
    close(fd_result);


  // nove vysledky
  float oldResult[input->maxY*2+1][input->maxX*2+1];
  memset(oldResult, 0, sizeof(oldResult));

  // zapisanie vysledkov do suboru:
  char cesta[300] = "../../result_files/";
  strcat(cesta, input->suborUlozenia);
  FILE *fileResultTmp = fopen(cesta, "r");
  int pocetReplikaciiBefore;
  
  if(fileResultTmp == NULL) {
    pocetReplikaciiBefore = 0; // súbor neexistuje
  } else {
    fscanf(fileResultTmp, "%d", &pocetReplikaciiBefore);
    for(int i = 0; i < input->maxY*2+1; i++) {
      for(int j = 0; j < input->maxY*2+1; j++) {  
        fscanf(fileResultTmp, "%f", &oldResult[i][j]);
        result[i][j] += oldResult[i][j];
      }
    }
    fclose(fileResultTmp);
  }
  
  // prepísanie súboru
  FILE* fileResult;
  fileResult = fopen(cesta, "w");
  if(fileResult == NULL) {
    printf("chyba pri otvarani na zapis\n");
  }
  fprintf(fileResult, "%d\n", input->pocetReplikacii + pocetReplikaciiBefore);
  for (int j = 0; j < input->maxY*2+1; j++) {
      for(int k = 0; k < input->maxX*2+1; k++) {
        fprintf(fileResult, "%f ", result[j][k]);
      }
      fprintf(fileResult, "\n");
    }
  fclose(fileResult);

  // Odmapovanie pamäte a uvoľnenie zdrojov
  munmap(input, inputSize);
  close(shm_fd);
  shm_unlink("/sem.shared_input_RJ");
  
  munmap(mapa, mapaSize);
  close(shm_mapa_fd);
  shm_unlink("/sem.shared_mapa_RJ");
  
  return 0;
}
