#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "../common/inputStruktura.h"
#include "../common/prekazky.h"
#include "../common/vykreslenieStruct.h"
#include "strukturaPamat.h"
#include "generatorMapy.c"
#include "replikacie.c"
#include "pohyb.c"
#include <errno.h>
// zdielana pamat
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#define FIFO_INPUT "../../fifo_files/input"
#define SHM_RESULT_NAME "/shared_result_RJ_"
#define SHM_VYKRESLENIE_NAME "/shared_vykreslenie_RJ_"
#define SEMAPHORE_KLIENT_NAME "/shared_semaphore_klient_RJ_"
#define SEMAPHORE_SERVER_NAME "/shared_semaphore_server_RJ_"


int velkostMapy(SIMPAM* args) {
  int rozmerX = 2*args->maxX + 1;
  int rozmerY =2*args->maxY + 1;
  return  rozmerX*rozmerY;

}

int main(int argc, char *argv[]){
 
  // PRIJATIE FIFO INPUTU
  // Otvorenie FIFO na čítanie
  int fd_input = open(FIFO_INPUT, O_RDONLY);
  if (fd_input == -1) {
    perror("open");

    exit(EXIT_FAILURE);
  }

  Input inputJojo;
  memset(&inputJojo, 0, sizeof(inputJojo));
  // Čítanie hodnôt z FIFO
  if(read(fd_input, &inputJojo, sizeof(Input)) == -1) {
    perror("Chyba pri čítaní Klient result\n");
    return 1;
  }
  close(fd_input);

  // Pripojenie k zdieľanej pamäti
  char menoSimulacie[256] = SHM_VYKRESLENIE_NAME;
  strcat(menoSimulacie, inputJojo.suborUlozenia);
  memset(&menoSimulacie[strlen(menoSimulacie)-4], 0, 4);
  //printf("%s\n", menoSimulacie);

  int shm_vykreslenie_fd = shm_open(menoSimulacie, O_CREAT | O_RDWR, 0666);
  if (shm_vykreslenie_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  size_t vykreslenieSize = sizeof(Vykreslenie_shm);
  if(ftruncate(shm_vykreslenie_fd, vykreslenieSize) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  // Mapovanie pamäte
  Vykreslenie_shm* vykreslenie = mmap(NULL, vykreslenieSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_vykreslenie_fd, 0);
  if (vykreslenie == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  // priprava mutexu
  // Inicializácia mutexu s PTHREAD_PROCESS_SHARED
  pthread_mutexattr_t attrSemafory;
  pthread_mutexattr_init(&attrSemafory);
  pthread_mutexattr_setpshared(&attrSemafory, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&vykreslenie->mutexSemafory, &attrSemafory);
  
  pthread_mutexattr_t attrResult;
  pthread_mutexattr_init(&attrResult);
  pthread_mutexattr_setpshared(&attrResult, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&vykreslenie->mutexResult, &attrResult);

  pthread_mutex_lock(&vykreslenie->mutexSemafory); // zamknutie mutexu na vytvorenie semaforov a vykreslenia
  pthread_mutex_lock(&vykreslenie->mutexResult); // zamknutie mutexu na vytvorenie semaforov a vykreslenia

  //printf("Server: Po mmap\n");
  vykreslenie->pripojenie = inputJojo.pripojenie;
  vykreslenie->mapa.maxX = inputJojo.maxX;
  vykreslenie->mapa.maxY = inputJojo.maxY;
  vykreslenie->pocetReplikacii = inputJojo.pocetReplikacii;
  
  char semKlientName[256] = SEMAPHORE_KLIENT_NAME;
  strcat(semKlientName, inputJojo.suborUlozenia);
  memset(&semKlientName[strlen(semKlientName) - 4], 0, 4);
  char semServerName[256] = SEMAPHORE_SERVER_NAME;
  strcat(semServerName, inputJojo.suborUlozenia);
  memset(&semServerName[strlen(semServerName) - 4], 0, 4);

// vytvorenie semaforov
    sem_t *semKlient = sem_open(semKlientName, O_CREAT, 0666, 0);
    if (semKlient == SEM_FAILED) {
      perror("sem_open");
      exit(EXIT_FAILURE);
    }
  // Pripojenie k existujúcemu semaforu  
    sem_t *semServer = sem_open(semServerName, O_CREAT, 0666, 1);
    if (semServer == SEM_FAILED) {
      perror("sem_open");
      exit(EXIT_FAILURE);
    }

  pthread_mutex_unlock(&vykreslenie->mutexSemafory);
  

  // VYTVORENIE ZDIELANEJ PAMATE - RESULT
  // 1. Vytvorenie zdieľanej pamäte
  char menoResultu[256] = SHM_RESULT_NAME;
  strcat(menoResultu, inputJojo.suborUlozenia);
  memset(&menoResultu[strlen(menoResultu) - 4], 0, 4);
  int shm_result_fd = shm_open(menoResultu, O_CREAT | O_RDWR, 0666);
  if (shm_result_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  size_t resultSize = sizeof(int)*((inputJojo.maxY*2+1) * (inputJojo.maxX*2+1));
  // 2. Nastavenie veľkosti zdieľanej pamäte
  if (ftruncate(shm_result_fd, resultSize) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  // 3. Mapovanie pamäte
  int* result = mmap(0, resultSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_result_fd, 0);
  if (result == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

   
  srand(time(NULL));

  SIMPAM* input = malloc(sizeof(SIMPAM));
  input->maxX = inputJojo.maxX;
  input->maxY = inputJojo.maxY;
  input->pVpred = inputJojo.pVpred;
  input->pVzad = inputJojo.pVzad;
  input->pVpravo = inputJojo.pVpravo;
  input->pVlavo = inputJojo.pVlavo;
  input->k = inputJojo.k;
  input->x = 0;
  input->y = 0;
  input->reps = inputJojo.pocetReplikacii;
  bool generujem = false;

  if ((input->pVpred + input->pVzad + input->pVlavo + input->pVpravo) != 1) {
  float x = input->pVpred + input->pVzad + input->pVlavo + input->pVpravo;
  input->pVpred = input->pVpred/x;
  input->pVzad = input->pVzad/x;
  input->pVpravo = input->pVpravo/x;
  input->pVlavo = input->pVlavo/x;
  }

  FILE* mapInput;

  input->mapa = malloc((2*input->maxY + 1) * sizeof(int*)); //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
  for(int i = 0; i < (2*input->maxY +1); i++){
    input->mapa[i] = malloc((2 * input->maxX + 1) * sizeof(int));
  }
   input->statPocetKrokov = malloc((2*input->maxY +1)*sizeof(float*)); //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    for(int i = 0; i < (2*input->maxY +1); i++){
      input->statPocetKrokov[i] = malloc((2*input->maxX + 1)*sizeof(float));
    }
    input->dostalSaDoStredu = malloc((2*input->maxY + 1) * sizeof(float*));//☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    for(int i = 0; i < (2*input->maxY + 1);i++){
      input->dostalSaDoStredu[i] = malloc((2*input->maxX + 1) * sizeof(float));
    }

  for(int i = 0; i <= 2 * input->maxY; i++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    for(int j = 0; j <= 2 * input->maxX; j++){
      input->mapa[i][j] = 0;
      input->statPocetKrokov[i][j] = 0.0;
      input->dostalSaDoStredu[i][j] = 0.0;
      }
    }


  Prekazky prekazky;
  prekazky.pocet = 0;


  for (int i = 0; i < 2 * input->maxY + 1; i++) { // ☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    if (input->mapa[i] == NULL) {
      printf("Memory allocation failed - mapa[%d]\n", i);
      exit(EXIT_FAILURE);
    }
    if (input->statPocetKrokov[i] == NULL) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
      printf("Memory allocation failed - statPocetKrokov[%d]\n", i);
      exit(EXIT_FAILURE);
    }
    if (input->dostalSaDoStredu[i] == NULL) {//☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
      printf("Memory allocation failed - dostalSaDoStredu[%d]\n", i);
      exit(EXIT_FAILURE);
      }
  }
  printf("Memory allocation successful\n");


  if(strlen(inputJojo.mapaSubor) == 0){
    generujMapu(input, &prekazky);
    generujem = true;
  } else {
  char cesta[300] = "../../map_files/";
  strcat(cesta, inputJojo.mapaSubor);
  mapInput = fopen(cesta, "r");
  generujem = false;
  }

  int px,py;

  if(!generujem) {
  while (fscanf(mapInput, "%d %d", &px, &py) == 2) {
    if (px >= 0 && px <=2*input->maxX && py >= 0 && py <=2*input->maxY) {
      input->mapa[py][px] = 1;  // Nastav hodnotu na 1, ak sú súradnice platné
      // JOJO PRIDAL
      prekazky.prekazky[prekazky.pocet].x = px;
      prekazky.prekazky[prekazky.pocet].y = py;
      prekazky.pocet++;
    } else {
      //printf("Súradnice (%d, %d) sú mimo rozsah!\n", py, px);
    }
    }
  } // aby nepada ak je vacsi subor ako mapa

    input->mapa[input->maxY][input->maxX] = 2;

   
    // V tomto momente uz mame map

    
    vykreslenie->mapa.opilec.x = input->x;
    vykreslenie->mapa.opilec.y = input->y;
    vykreslenie->mapa.prekazky = prekazky;



    // ZDIELANA PAMAT - VYKRESLOVANIE UPDATE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!🎨🖌️
    if(inputJojo.vykreslenie) {
      
    vykreslenie->end = 0;
      replikuj(input, vykreslenie, semServer, semKlient);
      vykreslenie->end = 1;
      sem_post(semKlient);
    
  } else {
    replikuj(input, NULL, NULL, NULL);
  }
  printf("Priemerné počty krokov do stredu:\n"); //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
  for (int y = 0; y <= 2*input->maxY; y++ ) {
    for(int x = 0; x<= 2*input->maxX; x++) {
      if(input->statPocetKrokov[y][x] > 0){
        printf("\033[36m%6.1f\033[0m", input->statPocetKrokov[y][x]);
      } 
      else {
        printf("%6.1f", input->statPocetKrokov[y][x]);
      }
    }
    printf("\n");
  }    // Zápis hodnôt do RESULT
    for(int r = 0; r < input->maxY*2+1; r++) { // po riadkoch
      for(int s = 0; s < input->maxX*2+1; s++) {
        result[(r*(input->maxY*2+1)) + s] = (int)input->dostalSaDoStredu[r][s];
      }
    }



  pthread_mutex_unlock(&vykreslenie->mutexResult);


    // čistenie po klientovi
    munmap(vykreslenie, vykreslenieSize);
    close(shm_vykreslenie_fd);

    munmap(result, resultSize);
    close(shm_result_fd);

    sem_close(semKlient);
    sem_close(semServer);

    pthread_mutexattr_destroy(&attrSemafory);
    pthread_mutexattr_destroy(&attrResult);

  // čistenie po serveri
  for(int p = 0; p < (2*input->maxY + 1);p++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    free(input->mapa[p]);
  }
 for(int p = 0; p < (2*input->maxY + 1);p++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    free(input->statPocetKrokov[p]);
  }

 for(int p = 0; p < (2*input->maxY + 1);p++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    free(input->dostalSaDoStredu[p]);
  }
  free(input->dostalSaDoStredu);
  free(input->statPocetKrokov);
  free(input->mapa);
  free(input);
  if(!generujem){
  fclose(mapInput);}
  return 0;
}
