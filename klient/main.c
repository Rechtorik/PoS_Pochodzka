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

#define FIFO_INPUT "../../fifo_files/input"
#define SHM_VYKRESLENIE_NAME "/shared_vykreslenie_RJ_"
#define SHM_RESULT_NAME "/shared_result_RJ_"
#define SEMAPHORE_KLIENT_NAME "/shared_semaphore_klient_RJ_"
#define SEMAPHORE_SERVER_NAME "/shared_semaphore_server_RJ_"


int proces_vykreslenie = 1;

// Funkcia vlákna na ukončenie výstupu
void *waitForK(void *arg) {
    Vykreslenie_shm* par = (Vykreslenie_shm*)arg;

    int ch;
    while (par->end == 0) {  // Pokračuje, kým end nie je 1
        ch = getchar();
        if (ch == 'k') {  // Detekcia CTRL+D
            if(par->pocetPripojenych > 1) {
                par->pocetPripojenych--;
                proces_vykreslenie = 0;
            } else {
                par->end = 1; // Nastavenie end na 1
            }
            break;        // Ukončenie cyklu
        }
    }

    pthread_exit(NULL);
}













int main(int argc, char* argv[]) {

  // Input struktura (prazdna)
  Input input;
  memset(&input, 0, sizeof(input));

  int menuExit = menu(&input);

  if(menuExit) return 0;

  // V tomto momente uz mame input z menu

  if(input.pripojenieNaPrebiehajucu) { //                             🔄🔌

    const char *shm_dir = "/dev/shm"; // Adresár pre zdieľanú pamäť
    struct dirent *entry;

    // Otvorenie adresára /dev/shm
    DIR *dir = opendir(shm_dir);
    if (dir == NULL) {
        perror("Nepodarilo sa otvoriť /dev/shm");
        exit(EXIT_FAILURE);
    }

    printf("Prebiehajúce simulácie, zadajte 'exit' na vypnutie:\n");
    printf("---------------------------------------\n");

    // Iterácia cez obsah adresára
    while ((entry = readdir(dir)) != NULL) {
        // Skontroluj, či názov obsahuje "vykreslenie"
        char* retazec = NULL;
        retazec = strstr(entry->d_name, "vykreslenie_RJ");
        if (retazec != NULL) {
            printf("  -> %s\n", retazec + sizeof("vykreslenie_RJ")); // Vypíše len záznamy obsahujúce "vykreslenie"
        }

    }
    closedir(dir);

    printf("\nZadajte názov simulácie, na ktorú sa chcete pripojiť: \n\n");
    char nazovSimulacie[256];
    memset(nazovSimulacie, 0, sizeof(nazovSimulacie));
    
    while (getchar() != '\n'); // vycistenie bufferu
    fgets(nazovSimulacie, sizeof(nazovSimulacie), stdin);  // Načítanie celej línie
    //while (getchar() != '\n'); // vycistenie bufferu
    nazovSimulacie[strlen(nazovSimulacie)-1] = '\0';

    if(strcmp(nazovSimulacie, "exit") == 0) { printf("Aplikácia sa ukončuje...\n"); return 0; }
    
    //Pripojenie sa na zdielanun pamat
    char menoVykreslenia[256] = SHM_VYKRESLENIE_NAME;
    strcat(menoVykreslenia, nazovSimulacie);
    int shm_vykreslenie_fd = shm_open(menoVykreslenia, O_RDWR, 0666);
    if (shm_vykreslenie_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    size_t vykreslenieSize = sizeof(Vykreslenie_shm);
    Vykreslenie_shm* vykreslenie = mmap(0, vykreslenieSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_vykreslenie_fd, 0);
    if (vykreslenie == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }


    
    
    if(vykreslenie->pripojenie) {
        pthread_t thread;

        // Vytvorenie vlákna
        if (pthread_create(&thread, NULL, waitForK, vykreslenie) != 0) {
            perror("Chyba pri vytváraní vlákna");
            exit(EXIT_FAILURE);
        }

        vykreslenie->pocetPripojenych++;
        while(proces_vykreslenie && vykreslenie->end == 0) {
        kresli(vykreslenie, 0);
        }
        // Čakáme na ukončenie vlákna
        if (pthread_join(thread, NULL) != 0) {
            perror("Chyba pri čakaní na vlákno");
            exit(EXIT_FAILURE);
        }
    } else {
      printf("Na túto simuláciu nieje dovolené pripojenie!\n");
    }

    

    return 0; //                                                      🔄🔌
  }


  // ZDIELANA PAMAT - VYKRESLENIE
  //Otvorenie zdieľanej pamäte
  char menoSimulacie[256] = SHM_VYKRESLENIE_NAME;
  strcat(menoSimulacie, input.suborUlozenia);
  memset(&menoSimulacie[strlen(menoSimulacie)-4], 0, 4); // odstranenie .txt
  //printf("%s\n", menoSimulacie);
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
  vykreslenie->pocetPripojenych++;

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

    // Pokus o vymazanie súboru result
    char resultFile[256] = "../../result_files/";
    strcat(resultFile, input.suborUlozenia);
    remove(resultFile); 

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
  /*
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
*/
  

  // V tomto momente uz mame naplnenu strukturu isto isto (nacitavanie zo suboru)
  // Uz vieme co ideme robit, len to treba spravit

  vykreslenie->mapa.maxX = input.maxX;
  vykreslenie->mapa.maxY = input.maxY;
  vykreslenie->pocetReplikacii = input.pocetReplikacii;

  // FIFO INPUT vytvorenie
  // Skontroluj, či FIFO existuje
    if (access(FIFO_INPUT, F_OK) == -1) {
        // FIFO neexistuje, pokus o jeho vytvorenie
        if (mkfifo(FIFO_INPUT, 0666) == -1) {
            perror("Chyba pri vytváraní FIFO");
            return 1;
        }
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
  
  char semKlientName[256] = SEMAPHORE_KLIENT_NAME;
  strcat(semKlientName, input.suborUlozenia);
  memset(&semKlientName[strlen(semKlientName) - 4], 0, 4);
  char semServerName[256] = SEMAPHORE_SERVER_NAME;
  strcat(semServerName, input.suborUlozenia);
  memset(&semServerName[strlen(semServerName) - 4], 0, 4);

  usleep(100000);

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


  if(input.vykreslenie == 1) {
    
    pthread_t thread;

    // Vytvorenie vlákna
    if (pthread_create(&thread, NULL, waitForK, vykreslenie) != 0) {
        perror("Chyba pri vytváraní vlákna");
        exit(EXIT_FAILURE);
    }

    
    usleep(500000);
    //printf("Vykresluje sa 𓁹‿𓁹\n");

    while(vykreslenie->end == 0) {
      sem_wait(semKlient);
      kresli(vykreslenie, 1);
      usleep(20000);
      sem_post(semServer);
    }
    //printf("Odišiel som z tohto pekelného loopu\n");
    
    // Čakáme na ukončenie vlákna
    if (pthread_join(thread, NULL) != 0) {
        perror("Chyba pri čakaní na vlákno");
        exit(EXIT_FAILURE);
    }
      sem_post(semServer); // pre istotu
  }

  usleep(250000);
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


    printf("\nPočty úspešných replikácií z %d replikácií:\n", input.pocetReplikacii);
    // Vypísanie prijatých hodnôt
    for (int i = 0; i < input.maxY*2+1; i++) {
      for(int j = 0; j < input.maxX*2+1; j++) {
        printf("%3d ", result[i][j]);
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

  munmap(newResult, resultSize);
  close(shm_result_fd);

  int pocetPripojenych = vykreslenie->pocetPripojenych;
  munmap(vykreslenie, vykreslenieSize);
  close(shm_vykreslenie_fd);
  if(pocetPripojenych == 1) { shm_unlink(menoSimulacie); shm_unlink(menoResultu); }

  // Uvoľnenie zdrojov
  sem_close(semServer);
  sem_unlink(semServerName);
  sem_close(semKlient);
  sem_unlink(semKlientName);
  
  printf("Koniec🗿\n");

  return 0;
}
