#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "../common/inputStruktura.h"
#include "../common/prekazky.h"
#include "../common/vykreslenieStruct.h"
#include <errno.h>
// zdielana pamat
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#define FIFO_INPUT "../../fifo_files/input"
#define SHM_RESULT_NAME "/shared_result_RJ_"
#define SHM_VYKRESLENIE_NAME "/shared_vykreslenie_RJ_"
#define SEMAPHORE_KLIENT_NAME "/shared_semaphore_klient_RJ_"
#define SEMAPHORE_SERVER_NAME "/shared_semaphore_server_RJ_"

typedef struct {
  int maxX;
  int maxY;
  float pVpred; // 80
  float pVzad; //30
  float pVlavo; //56
  float pVpravo; //21
  int x;
  int y;
  int nVpred;
  int nVzad;
  int nVpravo;
  int nVlavo;
  int k;
  int reps;
//  pthread_mutex_t mutex;
  int ** mapa;
  float **statPocetKrokov;
  float **dostalSaDoStredu;


}SIMPAM;

int velkostMapy(SIMPAM* args) {
    int rozmerX = 2*args->maxX + 1;
    int rozmerY =2*args->maxY + 1;
    return  rozmerX*rozmerY;

}
void generujMapu(SIMPAM* args, Prekazky* prekazky) {


    int pocetPokusov = 0; // 10 pokusov na najdenie miesta
    int napocitavanie = 0;
    int pocetPrekazok = 0;
    if(args->maxY < 2 || args->maxX < 2) {
      pocetPrekazok = 0;
  } else {
      pocetPrekazok = ((args->maxX*args->maxY)/2);//rand() % ((args->maxX*args->maxY)/2);
  }

     while((napocitavanie < pocetPrekazok) && (pocetPokusov < 10)) {

        int prekX = rand() % (2 * args->maxX + 1);
        int prekY = rand() % (2 * args->maxY + 1);
        printf("Pocet prekazok %d\n", pocetPrekazok);
        printf("x %d\n", prekX);
        printf("y %d\n", prekY);
        printf("napocitavanie %d\n", napocitavanie);
        

     /* while (prekX < 0 || prekX > 2 * args->maxX || prekY < 0 || prekY > 2 * args->maxY) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
            printf("Generated invalid index prekX=%d, prekY=%d\n", prekX, prekY);
            prekX = rand() % (2 * args->maxX + 1);
            prekY = rand() % (2 * args->maxY + 1);

        }*/
        bool validna = true;

        // Kontrola okolia
        for (int i = -1; i <= 1; i++) { // kontroluje okolie x //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
            for (int j = -1; j <= 1; j++) { // detto pre y

                int noveX = prekX + i;
                int noveY = prekY + j;

                if (noveX > 0 && noveX < 2 * args->maxX && //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
                    noveY > 0 && noveY < 2 * args->maxY &&
                    !(noveX == args->maxX && noveY == args->maxY)) {
                    if (args->mapa[noveY][noveX] == 1) {
                        validna = false;
                        break;
                    }
                } else { 
                  validna = false;
                }
            }
            if (!validna) break;
        }

        // Ak je pozícia validná, pridaj prekážku
        if (validna) {
            pocetPokusov = 0;
            args->mapa[prekY][prekX] = 1;
            napocitavanie++;
            //printf("Prekazky: %d %d\n",prekX,prekY);

            prekazky->prekazky[prekazky->pocet].x = prekX;
            prekazky->prekazky[prekazky->pocet].y = prekY;
            prekazky->pocet++;
        } else { pocetPokusov++; }
  }
}

int vyberSmer(void* args){

  SIMPAM* input = (SIMPAM*)args;
  float random = (float)rand()/(float)RAND_MAX;
  //int random = rand()%(input->pVpred + input->pVzad + input->pVlavo + input->pVpravo);
  int volba = 0;
  if(random < input->pVpred){
    // pohne sa vpred
    volba = 1;
  }else if (random < input->pVpred + input->pVzad) {
    //pohne sa vzad
    volba = 2;
  } else if(random < input->pVpred + input->pVzad + input->pVlavo) {
    //pohne sa vlavo
    volba = 3;
  }else {
    //pohne sa vpravo
    volba = 4;
  }
    return volba;
}

void zmenPoziciu(SIMPAM *args) {
    //int posun = vyberSmer(args);  // Získanie smeru pohybu

    int newX = args->x;
    int newY = args->y;
    bool ok = false;

    while(!ok){
      int posun = vyberSmer(args);  // Získanie smeru pohybu
      newX = args->x;
      newY = args->y;
      // Aktualizácia súradníc na základe smeru
    if (posun == 1) {  // Posun DOLE (j++) //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
        newY = (args->y + 1 > 2*args->maxY) ? 0 : args->y + 1;
    } else if (posun == 2) {  // Posun HORE (j--)
        newY = (args->y - 1 < 0) ? 2*args->maxY : args->y - 1;
    } else if (posun == 3) {  // Posun VLAVO (i--)
        newX = (args->x - 1 < 0) ? 2*args->maxX : args->x - 1;
    } else if (posun == 4) {  // Posun VPRAVO (i++)
        newX = (args->x + 1 > 2*args->maxX) ? 0 : args->x + 1;
    }
    //🩸🩸🩸🩸🩸🩸
    // Kontrola, či je nové políčko blokované //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    if (args->mapa[newY][newX] != 1) {
      ok = true;
    }
  }
    // Ak pohyb nie je blokovaný, aktualizujeme súradnice //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    args->x = newX;
    args->y = newY;
}
void replikuj(SIMPAM *args, Vykreslenie_shm* update_shm, sem_t* semServer, sem_t* semKlient) {
  for (int i = 0; i <= 2 * args->maxY; i++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
        for (int j = 0; j <= 2 * args->maxX; j++) {
            int totalSteps = 0; // ZA VSETKY REPS 1 policko

            for (int r = 0; r < args->reps; r++) {
                if(args->mapa[i][j] == 1) { break; }
                if(args->mapa[i][j] == 2) { break; }
                args->x = j;  // Nastavenie počiatočnej pozície
                args->y = i;
                int steps = 0;
                //printf("Server: ideme novu replikaciu\n");
                while (steps < args->k) {  // Maximálny počet krokov
                    // Ak sa dostaneme na cieľové políčko (hodnota 2), ukončíme pohyb
                    if (args->mapa[args->y][args->x] == 2) {
                        args->dostalSaDoStredu[i][j] += 1;
                        break;
                    }

                   // printf("\nServer pred pohybom: x:%d y:%d\n", args->x, args->y);
                    // Pohyb na základe pravidiel
                    zmenPoziciu(args);

                    // Kód vo while cykluse JOJO PRIDAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    if(update_shm != NULL){
                      if(update_shm->end) sem_post(semServer);
                      int hodnota;
                      sem_getvalue(semServer, &hodnota);
                      //printf("Server semafor hodnota pred wait: %d\n", hodnota);
                      sem_wait(semServer);
                      // Aktualizácia údajov
                      update_shm->mapa.opilec.x = args->x;  // Náhodná hodnota
                      update_shm->mapa.opilec.y = args->y;  // Náhodná hodnota
                      update_shm->replikacia = r;
                      update_shm->zacX = j;
                      update_shm->zacY = i;
                      update_shm->k = args->k;
                      update_shm->pocetKrokov = steps;
                      

                      sem_post(semKlient);
                    }
                    //printf("Server: x:%d y:%d\n", args->x, args->y);
                    //printf("Server: krok cislo %d\n", steps);
                    //printf("Server: replikacia cislo %d\n", r);
                    //printf("Server: startovacia pozicia [%d][%d]\n", j, i);
                    steps++;
                }

                if (steps == args->k && args->mapa[args->y][args->x] != 2) {
                    //printf("NEDOSTAL SA na [%d][%d] po %d krokoch.\n", i, j, args->k);
                   // args->statPocetKrokov[i][j] = 0;
                } else {
                    totalSteps += steps;
                }
            }

            // Vypočíta sa priemerný počet krokov pre danú počiatočnú pozíciu, ak sa niekedy dostal
            if (totalSteps > 0) {
                double averageSteps = (double)totalSteps / args->reps;
                //printf("Startovacia pozícia [%d][%d]: Priemerný počet krokov = %.2f\n", i, j, averageSteps);
                args->statPocetKrokov[i][j] = averageSteps;
            }
          if(args->mapa[i][j] == 1){
            args->statPocetKrokov[i][j] = 0.0;
            args->dostalSaDoStredu[i][j] = 0.0;

           }
        }
    }
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
  //printf("Server: Pred open\n");
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
  //printf("Server: Pred mmap\n");
  Vykreslenie_shm* vykreslenie = mmap(NULL, vykreslenieSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_vykreslenie_fd, 0);
  if (vykreslenie == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
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
  
  //printf("Server: meno sem je %s\n", semServerName);
  //printf("Server: meno sem je %s\n", semKlientName);

  //sem_unlink(semKlientName);
  //sem_unlink(semServerName);
 
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

    //char cestaMapa[300] = "../../map_files/";
    //strcat(cestaMapa, inputJojo->mapaSubor);
    //printf("Server cesta mapy: %s\n", cestaMapa);
    //FILE *mapInputJojo = fopen(cestaMapa, "r");

    //if (mapInput == NULL) {
    //    // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
    //    perror("Chyba pri otváraní súboru");
    //}

  srand(time(NULL));

  SIMPAM* input = malloc(sizeof(SIMPAM));

  //input->maxX = atoi(argv[1]);
  //input->maxY = atoi(argv[2]);
  //input->pVpred = atof(argv[3]);
  //input->pVzad = atof(argv[4]);
  //input->pVpravo = atof(argv[5]);
  //input->pVlavo = atof(argv[6]);
  //input->k = atoi(argv[7]);
  //input->x = 0;
  //input->y = 0;
  //input->nVpred = 0;
  //input->nVzad = 0;
  //input->nVpravo = 0;
  //input->nVlavo = 0;
  //input->reps = 100000;


  // JOJO PRIDAL ▄︻デ══━一💥
  input->maxX = inputJojo.maxX;
  input->maxY = inputJojo.maxY;
  input->pVpred = inputJojo.pVpred;
  input->pVzad = inputJojo.pVzad;
  input->pVpravo = inputJojo.pVpravo;
  input->pVlavo = inputJojo.pVlavo;
  input->k = inputJojo.k;
  input->x = 0;
  input->y = 0;
  input->nVpred = 0;
  input->nVzad = 0;
  input->nVpravo = 0;
  input->nVlavo = 0;
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


  // JOJO PRIDAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // JOJO PRIDAL ▄︻デ══━一💥
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

    //if (mapInput == NULL) {
    //    // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
    //    perror("Chyba pri otváraní súboru");
    //}
//꧁𝔂𝓪𝓼𝓼 𝓺𝓾𝓮𝓮𝓷꧂

//꧁𝔂𝓪𝓼𝓼 𝓺𝓾𝓮𝓮𝓷꧂




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

    //int velkost = velkostMapy(input);
    //printf("Veľkosť mapy: %d\n", velkost);



    // V tomto momente uz mame map

    // JOJO PRIDAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // JOJO PRIDAL ▄︻デ══━一💥
    

    vykreslenie->mapa.opilec.x = input->x;
    vykreslenie->mapa.opilec.y = input->y;
    vykreslenie->mapa.prekazky = prekazky;


    // ZDIELANA PAMAT - VYKRESLOVANIE UPDATE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!🎨🖌️
    if(inputJojo.vykreslenie) {
      vykreslenie->end = 0;
    //printf("Server: Pred replikuj s vykreslenim\n");
      replikuj(input, vykreslenie, semServer, semKlient);
      vykreslenie->end = 1;
      sem_post(semKlient);
  } else {
    //printf("Server: Pred replikuj bez vykreslenia\n");
      replikuj(input, NULL, NULL, NULL);
  }
    //printf("Server: Po replikaciach\n");

  usleep(50000);

  //🐻🐻🐻🐻🐻🐻🐻🐻🐻🐻🐻🐻🐻🐻🐻

  /*printf("Mapa:\n"); //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    for (int y = 0; y <= 2*input->maxY; y++) {
        for (int x = 0; x <= 2*input-> maxX; x++) {
            printf("%d ", input->mapa[y][x]);
        }
        printf("\n");
    }*/
  printf("Priemerné počty krokov do stredu:\n"); //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
  for (int y = 0; y <= 2*input->maxY; y++ ) {
    for(int x = 0; x<= 2*input->maxX; x++) {
      printf("%6.1f", input->statPocetKrokov[y][x]);
    }
    printf("\n");
  }/*
printf("Počty úspešných replikácií z %d replikácií:\n", input -> reps); //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
  for (int y = 0; y <= 2*input->maxY; y++ ) {
    for(int x = 0; x<= 2*input->maxX; x++) {
      printf("%6.1f", input->dostalSaDoStredu[y][x]);
    }
    printf("\n");
  }*/
//printf("KONIEC\n");

    // JOJO PRIDAL ▄︻デ══━一💥
    // float**
    // input->statPocetKrokov
    // Zápis hodnôt do RESULT
    for(int r = 0; r < input->maxY*2+1; r++) { // po riadkoch
      for(int s = 0; s < input->maxX*2+1; s++) {
        result[(r*(input->maxY*2+1)) + s] = (int)input->dostalSaDoStredu[r][s];
      }
    }

    munmap(vykreslenie, vykreslenieSize);
    close(shm_vykreslenie_fd);

    munmap(result, resultSize);
    close(shm_result_fd);

    sem_close(semKlient);
    sem_close(semServer);

  for(int p = 0; p < (2*input->maxY + 1);p++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    free(input->mapa[p]);
  }
 for(int p = 0; p < (2*input->maxY + 1);p++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    free(input->statPocetKrokov[p]);
  }

 for(int p = 0; p < (2*input->maxY + 1);p++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
    free(input->dostalSaDoStredu[p]);
  }
  //printf("KONIEC\n");
  free(input->dostalSaDoStredu);
  free(input->statPocetKrokov);
  free(input->mapa);
  free(input);
  if(!generujem){
  fclose(mapInput);}
  //printf("KONIEC\n");
  return 0;
}
