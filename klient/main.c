#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../common/inputStruktura.h"
#include "../common/prekazky.h"
#include "../common/vykreslenieStruct.h"

#include "kreslenie.c"
#include "menu.c"

#include <dirent.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// define
#define FIFO_INPUT "../../fifo_files/input"
#define SHM_VYKRESLENIE_NAME "/shared_vykreslenie_RJ_"
#define SHM_RESULT_NAME "/shared_result_RJ_"
#define SEMAPHORE_KLIENT_NAME "/shared_semaphore_klient_RJ_"
#define SEMAPHORE_SERVER_NAME "/shared_semaphore_server_RJ_"

typedef struct ProcesPamat{
  sem_t* semKlient;
  sem_t* semServer;
  Vykreslenie_shm* vykreslenie;
  int owner;
  int proces_vykreslenie;
  Input* input;
}ProcesPamat;

// musi byt tu, aby to poznalo ProcesPamat
#include "workers.c"

int main(int argc, char* argv[]) {

  // Input struktura (prazdna)
  Input input;
  memset(&input, 0, sizeof(input));
  int menuExit = menu(&input);
  if(menuExit) return 0;

  // V tomto momente uz mame input z menu
  ProcesPamat pamat;
  pamat.input = &input;
  pamat.proces_vykreslenie = 0;

  if(input.pripojenieNaPrebiehajucu) {
    pamat.owner = 0;
    const char *shm_dir = "/dev/shm"; // Adresár pre zdieľanú pamäť
    struct dirent *entry;

    // Otvorenie adresara /dev/shm/
    DIR *dir = opendir(shm_dir);
    if (dir == NULL) {
        perror("Nepodarilo sa otvoriť /dev/shm");
        exit(EXIT_FAILURE);
    }

    printf("Prebiehajúce simulácie, zadajte 'exit' na vypnutie:\n");
    printf("---------------------------------------\n");

    // Iterácia cez obsah adresára
    while ((entry = readdir(dir)) != NULL) {
      // Skontroluj, či názov obsahuje "vykreslenie_RJ"
      char* retazec = NULL;
      retazec = strstr(entry->d_name, "vykreslenie_RJ");
      if (retazec != NULL) {
          printf("  -> %s\n", retazec + sizeof("vykreslenie_RJ")); // Vypíše to co je za vykreslenie_RJ_
      }
    }
    closedir(dir);

    printf("\nZadajte názov simulácie, na ktorú sa chcete pripojiť: \n\n");
    char nazovSimulacie[256];
    memset(nazovSimulacie, 0, sizeof(nazovSimulacie));
    
    while (getchar() != '\n'); // vycistenie bufferu
    fgets(nazovSimulacie, sizeof(nazovSimulacie), stdin);
    nazovSimulacie[strlen(nazovSimulacie)-1] = '\0';

    if(strcmp(nazovSimulacie, "exit") == 0) { printf("Aplikácia sa ukončuje...\n"); return 0; }
    
    //Pripojenie sa na zdielanu pamat
    char menoVykreslenia[256] = SHM_VYKRESLENIE_NAME;
    strcat(menoVykreslenia, nazovSimulacie);

    int shm_vykreslenie_fd = shm_open(menoVykreslenia, O_RDONLY, 0666);
    if (shm_vykreslenie_fd == -1) {
      perror("shm_open");
      exit(EXIT_FAILURE);
    }

    size_t vykreslenieSize = sizeof(Vykreslenie_shm);
    Vykreslenie_shm* vykreslenie = mmap(0, vykreslenieSize, PROT_READ, MAP_SHARED, shm_vykreslenie_fd, 0);
    if (vykreslenie == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
    }

    pamat.vykreslenie = vykreslenie;
    
    if(vykreslenie->pripojenie) {
      pthread_t threadInput;
      if (pthread_create(&threadInput, NULL, waitForK, &pamat) != 0) {
          perror("Chyba pri vytváraní vlákna");
          exit(EXIT_FAILURE);
      }
      pamat.proces_vykreslenie = 1;
      pthread_t threadVykreslovanie;
      if (pthread_create(&threadVykreslovanie, NULL, vykreslovanie, &pamat) != 0) {
          perror("Chyba pri vytváraní vlákna");
          exit(EXIT_FAILURE);
      }
      if (pthread_join(threadInput, NULL) != 0) {
          perror("Chyba pri čakaní na vlákno");
          exit(EXIT_FAILURE);
      }
      if (pthread_join(threadVykreslovanie, NULL) != 0) {
          perror("Chyba pri čakaní na vlákno");
          exit(EXIT_FAILURE);
      }
    } else {
      printf("Na túto simuláciu nieje dovolené pripojenie!\n");
    }

    // Odmapovanie a zatvorenie
    munmap(vykreslenie, vykreslenieSize);
    close(shm_vykreslenie_fd);

    return 0;
  }

  // TU ZACINA PROGRAM PRE VYTVARANIE SIMULACIE
  pamat.owner = 1;
  pthread_t threadInputTextak; // vzdy len jedno vlakno, ale rozdiel co bude robit

  // VYTVORENIE TEXTAKU NA INPUT - VLAKNA
  if(input.opatovneSpustenie == 0) { // NOVA SIMULACIA
    if (pthread_create(&threadInputTextak, NULL, vytvorenieInputFileNovaSimulacia, &pamat) != 0) {
      perror("Chyba pri vytváraní vlákna");
      exit(EXIT_FAILURE);
    }
  } else {  // OPATOVNE SPUSTENIE SIMULACIE
    if (pthread_create(&threadInputTextak, NULL, vytvorenieInputFileOpatovnaSimulacia, &pamat) != 0) {
      perror("Chyba pri vytváraní vlákna");
      exit(EXIT_FAILURE);
    }
  }

  // V tomto momente uz mame naplnenu strukturu isto isto (nacitavanie zo suboru)
  // Uz vieme co ideme robit, len to treba spravit
  
  pthread_t threadDatovodInput;

  // Vytvorenie vlákna
  if (pthread_create(&threadDatovodInput, NULL, vytvorenieDatovoduInput, NULL) != 0) {
    perror("Chyba pri vytváraní vlákna");
    exit(EXIT_FAILURE);
  }
    
  // Čakáme na ukončenie vlákna
  if (pthread_join(threadInputTextak, NULL) != 0) {
    perror("Chyba pri čakaní na vlákno");
    exit(EXIT_FAILURE);
  }
  // Čakáme na ukončenie vlákna
  if (pthread_join(threadDatovodInput, NULL) != 0) {
    perror("Chyba pri čakaní na vlákno");
    exit(EXIT_FAILURE);
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

  // ODTIALTO TREBA SYNCHRONIZOVAT - OBA POCESY UZ MAJU INPUT
  
  usleep(100000); // cakanie pokym server vytvori vykreslenie - mutexy sa nachadzaju v shm, ktoru musi vytvorit server
    
  // ZDIELANA PAMAT - VYKRESLENIE
  //Otvorenie zdieľanej pamäte
  char menoSimulacie[256] = SHM_VYKRESLENIE_NAME;
  strcat(menoSimulacie, input.suborUlozenia);
  memset(&menoSimulacie[strlen(menoSimulacie)-4], 0, 4); // odstranenie .txt
  //printf("%s\n", menoSimulacie);
  int shm_vykreslenie_fd = shm_open(menoSimulacie, O_RDWR, 0666);
  if (shm_vykreslenie_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  size_t vykreslenieSize = sizeof(Vykreslenie_shm);
  // Mapovanie pamäte
  Vykreslenie_shm *vykreslenie = mmap(NULL, vykreslenieSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_vykreslenie_fd, 0);
  if (vykreslenie == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  vykreslenie->pocetPripojenych++;
  
  char semKlientName[256] = SEMAPHORE_KLIENT_NAME;
  strcat(semKlientName, input.suborUlozenia);
  memset(&semKlientName[strlen(semKlientName) - 4], 0, 4);
  char semServerName[256] = SEMAPHORE_SERVER_NAME;
  strcat(semServerName, input.suborUlozenia);
  memset(&semServerName[strlen(semServerName) - 4], 0, 4);


  pthread_mutex_lock(&vykreslenie->mutexSemafory); // cakanie na signal od servera ze semafory su vytvorene


  // Pripojenie semaforov
  sem_t *semServer = sem_open(semServerName, 0);
  if (semServer == SEM_FAILED) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  // Vytvorenie semaforu pre klienta
  sem_t *semKlient = sem_open(semKlientName, 0);
  if (semKlient == SEM_FAILED) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  pamat.semServer = semServer;
  pamat.semKlient = semKlient;
  pamat.vykreslenie = vykreslenie;


  pthread_mutex_unlock(&vykreslenie->mutexSemafory); // odomknutie pre istotu, uz sa pouzivat nebude


  if(input.vykreslenie == 1) {
    
    pthread_t threadInput;

    // Vytvorenie vlákna
    if (pthread_create(&threadInput, NULL, waitForK, &pamat) != 0) {
      perror("Chyba pri vytváraní vlákna");
      exit(EXIT_FAILURE);
    }

    pthread_t threadVykreslovanie;

    // Vytvorenie vlákna
    if (pthread_create(&threadVykreslovanie, NULL, vykreslovanie, &pamat) != 0) {
      perror("Chyba pri vytváraní vlákna");
      exit(EXIT_FAILURE);
    }

    // Čakáme na ukončenie vlákna
    if (pthread_join(threadInput, NULL) != 0) {
      perror("Chyba pri čakaní na vlákno");
      exit(EXIT_FAILURE);
    }
    // Čakáme na ukončenie vlákna
    if (pthread_join(threadVykreslovanie, NULL) != 0) {
      perror("Chyba pri čakaní na vlákno");
      exit(EXIT_FAILURE);
    }
      sem_post(semServer); // pre istotu
  }


  pthread_mutex_lock(&vykreslenie->mutexResult); // cakanie na vysledky od servera


  //NACITANIE VYSLEDKOV (POCTY KOLKO KRAT SA DOSTAL DO STREDU)
  // 1. Otvorenie existujúcej zdieľanej pamäte
  char menoResultu[256] = SHM_RESULT_NAME;
  strcat(menoResultu, input.suborUlozenia);
  memset(&menoResultu[strlen(menoResultu) - 4], 0, 4);
  int shm_result_fd = shm_open(menoResultu, O_RDONLY, 0666);
  if (shm_result_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  // 2. Mapovanie pamäte
  size_t resultSize = (sizeof(int) * (input.maxX*2+1) * (input.maxY*2+1));
  int* newResult = mmap(0, resultSize, PROT_READ, MAP_SHARED, shm_result_fd, 0);
  if (newResult == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  // Pole na uloženie prijatých hodnôt
  int result[input.maxY*2+1][input.maxX*2+1];
  memset(result, 0, sizeof(result));

  // Čítanie hodnôt zo zdielanej pamate
  for(int i = 0; i < input.maxY*2+1; i++) {
    for(int j = 0; j < input.maxX*2+1; j++) {
      result[i][j] = newResult[(i*(input.maxY*2+1)) + j];
    }
  }


  pthread_mutex_unlock(&vykreslenie->mutexResult); // odokmkunutie len pre istotu, uz sa pouzivat nebude 


  printf("\nPočty úspešných replikácií z %d replikácií:\n", input.pocetReplikacii);
  // Vypísanie prijatých hodnôt
  for (int i = 0; i < input.maxY*2+1; i++) {
    for(int j = 0; j < input.maxX*2+1; j++) {
      if(result[i][j] > 0) {
        printf("\033[1m\033[36m%3d \033[0m\033[0m", result[i][j]); // výpis vo farbe
      } else {
        printf("%3d ", result[i][j]);
      }
    }
    printf("\n");
  }

  // nove vysledky
  float oldResult[input.maxY*2+1][input.maxX*2+1];
  memset(oldResult, 0, sizeof(oldResult));

  // zapisanie vysledkov do suboru:
  char cesta[300] = "../../result_files/";
  strcat(cesta, input.suborUlozenia);
  FILE *fileResultTmp = fopen(cesta, "r");
  int pocetReplikaciiBefore;

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

  // Uvoľnenie zdrojov
  munmap(newResult, resultSize);
  close(shm_result_fd);
  
  pthread_mutex_destroy(&vykreslenie->mutexSemafory); // znicenie mutexov
  pthread_mutex_destroy(&vykreslenie->mutexResult);
 
  int pocetPripojenych = vykreslenie->pocetPripojenych;
  munmap(vykreslenie, vykreslenieSize);
  close(shm_vykreslenie_fd);
  if(pocetPripojenych == 1) { shm_unlink(menoSimulacie); shm_unlink(menoResultu); }

  sem_close(semServer);
  sem_unlink(semServerName);
  sem_close(semKlient);
  sem_unlink(semKlientName);
  printf("Koniec programu\n");

  return 0;
}
