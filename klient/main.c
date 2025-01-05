#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../common/inputStruktura.h"
#include "../common/prekazky.h"
#include "../common/vykreslenieStruct.h"

#include "kreslenie.c"
#include "menu.c"

#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define FIFO_INPUT "../../fifo_files/input"
#define SHM_NAME "/sem.shared_vykreslenie_RJ_"
#define SEMAPHORE_KLIENT_NAME "/shared_semaphore_klient_RJ"
#define SEMAPHORE_SERVER_NAME "/shared_semaphore_server_RJ"

int main(int argc, char* argv[]) {
  



  sem_unlink(SEMAPHORE_KLIENT_NAME);
  sem_unlink(SEMAPHORE_SERVER_NAME);


  // Vytvorenie semaforu pre server
  sem_t *semServer = sem_open(SEMAPHORE_SERVER_NAME, O_CREAT, 0666, 1); // Inicializovaný na 0
  if (semServer == SEM_FAILED) {
      perror("sem_open");
      exit(EXIT_FAILURE);
  }
  // Vytvorenie semaforu pre klienta
  sem_t *semKlient = sem_open(SEMAPHORE_KLIENT_NAME, O_CREAT, 0666, 0); // Inicializovaný na 0
  if (semKlient == SEM_FAILED) {
      perror("sem_open");
      exit(EXIT_FAILURE);
  }

  // Input struktura (prazdna)
  Input input;
  memset(&input, 0, sizeof(input));

  menu(&input);

  // V tomto momente uz mame input z menu

  if(input.pripojenieNaPrebiehajucu) {
    // kod na vypisanie prebiehajucich
  }


  // ZDIELANA PAMAT - VYKRESLENIE
  //Otvorenie zdieľanej pamäte
  char menoSimulacie[256] = SHM_NAME;
  strcat(menoSimulacie, input.suborUlozenia);
  memset(&menoSimulacie[strlen(menoSimulacie)-4], 0, 4);
  printf("%s\n", menoSimulacie);
  int shm_vykreslenie_fd = shm_open(menoSimulacie, O_CREAT | O_RDWR, 0666);
  if (shm_vykreslenie_fd == -1) {
      perror("shm_open");
      exit(EXIT_FAILURE);
  }
  size_t vykreslenieSize = sizeof(Vykreslenie_shm);
  // Nastavenie veľkosti zdieľanej pamäte
  if (ftruncate(shm_vykreslenie_fd, vykreslenieSize) == -1) {
      perror("ftruncate");
      exit(EXIT_FAILURE);
  }
  // Mapovanie pamäte
  Vykreslenie_shm *vykreslenie = mmap(NULL, vykreslenieSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_vykreslenie_fd, 0);
  if (vykreslenie == MAP_FAILED) {
    perror("mmap");
      exit(EXIT_FAILURE);
  }
  vykreslenie->pripojenie = input.pripojenie;

  // VYTVORENIE TEXTAKU NA INPUT
  if(input.opatovneSpustenie == 0) { // NOVA SIMULACIA
    char cesta[300] = "../../input_files/";
    strcat(cesta, input.suborUlozenia);
    FILE *fileInput = fopen(cesta, "w");

    //if (fileInput == NULL) {
    //    // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
    //    perror("Chyba pri otváraní súboru");
    //}

    // Zápis textu do súboru
    fprintf(fileInput, "%s\n", input.mapaSubor);
    fprintf(fileInput, "%d\n", input.maxX);
    fprintf(fileInput, "%d\n", input.maxY);
    fprintf(fileInput, "%f\n", input.pVpred);
    fprintf(fileInput, "%f\n", input.pVzad);
    fprintf(fileInput, "%f\n", input.pVlavo);
    fprintf(fileInput, "%f\n", input.pVpravo);
    fprintf(fileInput, "%d\n", input.k);
    fprintf(fileInput, "%s\n", input.suborUlozenia);
    fprintf(fileInput, "%d\n", input.pripojenie);

    // Zatvorenie súboru
    fclose(fileInput);
  } else {  // OPATOVNE SPUSTENIE SIMULACIE

    char cesta[300] = "../../input_files/";
    strcat(cesta, input.suborUlozenia);
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
    strcat(input.mapaSubor, buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    // maxX
    buffer[strlen(buffer) - 1] = '\0';
    input.maxX = atoi(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //maxY
    buffer[strlen(buffer) - 1] = '\0';
    input.maxY = atoi(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVpred
    buffer[strlen(buffer) - 1] = '\0';
    input.pVpred = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVzad
    buffer[strlen(buffer) - 1] = '\0';
    input.pVzad = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVlavo
    buffer[strlen(buffer) - 1] = '\0';
    input.pVlavo = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVpravo
    buffer[strlen(buffer) - 1] = '\0';
    input.pVpravo = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //k
    buffer[strlen(buffer) - 1] = '\0';
    input.k = atoi(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //nazov simulacie
    buffer[strlen(buffer) - 1] = '\0';
    //input.suborUlozenia = buffer;

    fgets(buffer, sizeof(buffer), fileInput);
    //pripojenie (0 alebo 1)
    buffer[strlen(buffer) - 1] = '\0';
    input.pripojenie = atoi(buffer);

    fclose(fileInput);
  }

  // debug
  printf("Veľkosť mapy je: %d\n", (input.maxX*2+1) * (input.maxY*2+1));
  printf("Nazov mapy je: %s \n", input.mapaSubor);
  printf("Dlzka nazva mapy: %d \n", (int)strlen(input.mapaSubor));
  printf("Nazov simulacie je: %s \n", input.suborUlozenia);
  printf("Opatovne spustenie: %d\n", input.opatovneSpustenie);
  printf("maxX: %d\n", input.maxX);
  printf("maxY: %d\n", input.maxY);
  printf("pVpred: %f\n", input.pVpred);
  printf("pVzad: %f\n", input.pVzad);
  printf("pVpravo: %f\n", input.pVpravo);
  printf("pVlavo: %f\n", input.pVlavo);
  printf("k: %d\n", input.k);
  printf("pocet replikacii: %d\n", input.pocetReplikacii);
  printf("mozu sa ostatni pripojit: %d\n", input.pripojenie);
  printf("Zapnute vykreslenie: %d\n", input.vykreslenie);

  // V tomto momente uz mame naplnenu strukturu isto isto (nacitavanie zo suboru)
  // Uz vieme co ideme robit, len to treba spravit


  // FIFO INPUT vytvorenie
  // Skontroluj, či FIFO existuje
    if (access(FIFO_INPUT, F_OK) == -1) {
        // FIFO neexistuje, pokus o jeho vytvorenie
        if (mkfifo(FIFO_INPUT, 0666) == -1) {
            perror("Chyba pri vytváraní FIFO");
            return 1;
        }
    } else {
        printf("FIFO už existuje: %s\n", FIFO_INPUT);
    }

  




  // spustenie
  int statusServera = system("../server/server &"); // ampersant na to aby bezal server na pozadi
  if (statusServera == -1) {
      perror("Chyba pri spusteni programu server");
  }


  // Zapis do FIFO
  // Otvorenie FIFO na zápis
  int fd_input = open(FIFO_INPUT, O_WRONLY);
  if (fd_input == -1) {
      perror("open");
      exit(EXIT_FAILURE);
  }
  // Poslanie celej input struktury
  if (write(fd_input, &input, sizeof(Input)) == -1) {
    perror("write");
    close(fd_input);
    exit(EXIT_FAILURE);
  }

  close(fd_input);


  if(input.vykreslenie == 1) {

    usleep(500000);

    printf("Vykresluje sa 𓁹‿𓁹\n");

    while(vykreslenie->end == 0) {
      // Čakanie, kým sú údaje pripravené
      int hodnota;
      sem_getvalue(semKlient, &hodnota);
      printf("Klient semafor hodnota pred wait: %d\n", hodnota);
      sem_wait(semKlient);
      kresli(&input, vykreslenie->mapa.opilec, vykreslenie->mapa.prekazky.pocet, &(vykreslenie->mapa.prekazky));
      printf("Klient: x: %d, y: %d\n", vykreslenie->mapa.opilec.x, vykreslenie->mapa.opilec.y);
      usleep(20000);
      sem_post(semServer);
      //usleep(20000);
    }
    printf("Odišiel som z tohto pekelného loopu\n");

  }

  usleep(250000);
    printf("Server: pred result open\n");
  //NACITANIE VYSLEDKOV (POCTY KOLKO KRAT SA DOSTAL DO STREDU)
    // 1. Otvorenie existujúcej zdieľanej pamäte
    int shm_result_fd = shm_open("/sem.shared_result_RJ", O_RDONLY, 0666);
    if (shm_result_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    printf("Server: pred resultSize\n");
    // 2. Mapovanie pamäte
    size_t resultSize = (sizeof(int) * (input.maxX*2+1) * (input.maxY*2+1));
    char *newResult = mmap(0, resultSize, PROT_READ, MAP_SHARED, shm_result_fd, 0);
    if (newResult == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    printf("Server: pred polom\n");
    // Pole na uloženie prijatých hodnôt
    int result[input.maxY*2+1][input.maxX*2+1];
    memset(result, 0, sizeof(result));

    printf("Server: pred citanim\n");
    // Čítanie hodnôt zo zdielanej pamate
    for(int i = 0; i < input.maxY*2+1; i++) {
      for(int j = 0; j < input.maxX*2+1; j++) {
        result[i][j] = newResult[(i*(input.maxY*2+1)) + j];
      }
    }

    printf("Server: vypisanim\n");
    // Vypísanie prijatých hodnôt
    for (int i = 0; i < input.maxY*2+1; i++) {
      for(int j = 0; j < input.maxX*2+1; j++) {
        printf("%6d ", result[i][j]);
      }
      printf("\n");
    }


    printf("Server: pred oldResult\n");
  // nove vysledky
  float oldResult[input.maxY*2+1][input.maxX*2+1];
  memset(oldResult, 0, sizeof(oldResult));

    printf("Server: pred zapisom\n");
  // zapisanie vysledkov do suboru:
  char cesta[300] = "../../result_files/";
  strcat(cesta, input.suborUlozenia);
  FILE *fileResultTmp = fopen(cesta, "r");
  int pocetReplikaciiBefore;

    printf("Server: pred tmpFile\n");
  if(fileResultTmp == NULL) {
    pocetReplikaciiBefore = 0; // súbor neexistuje
  } else {
    fscanf(fileResultTmp, "%d", &pocetReplikaciiBefore);
    for(int i = 0; i < input.maxY*2+1; i++) {

      for(int j = 0; j < input.maxY*2+1; j++) {

        fscanf(fileResultTmp, "%f", &oldResult[i][j]);
        result[i][j] += oldResult[i][j];
      }
    }
    fclose(fileResultTmp);
  }

    printf("Server: pred fileResult\n");
  // prepísanie súboru
  FILE* fileResult;
  fileResult = fopen(cesta, "w");
  if(fileResult == NULL) {
    printf("chyba pri otvarani na zapis\n");
  }
  fprintf(fileResult, "%d\n", input.pocetReplikacii + pocetReplikaciiBefore);
  for (int j = 0; j < input.maxY*2+1; j++) {
      for(int k = 0; k < input.maxX*2+1; k++) {
        fprintf(fileResult, "%d ", result[j][k]);
      }
      fprintf(fileResult, "\n");
    }
  fclose(fileResult);

    printf("Server: pred unmap1\n");
  munmap(newResult, resultSize);
  close(shm_result_fd);
  //unlink(newResult);
  shm_unlink("/sem.shared_result_RJ");

    printf("Server: pred unmap2\n");
  munmap(vykreslenie, vykreslenieSize);
  close(shm_vykreslenie_fd);
  shm_unlink(menoSimulacie);

    printf("Server: pred close1\n");
  // Uvoľnenie zdrojov
  sem_close(semServer);
  sem_unlink(SEMAPHORE_SERVER_NAME);
  sem_close(semKlient);
  sem_unlink(SEMAPHORE_KLIENT_NAME);

    printf("Server: pred koncom\n");
  return 0;
}
