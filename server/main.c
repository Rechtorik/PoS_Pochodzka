#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "../common/inputStruktura.h"
#include "../common/prekazky.h"

// zdielana pamat
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#define FIFO_RESULT "../../fifo_files/"

typedef struct {
  int maxX;
  int maxY;
  int pVpred; // 80
  int pVzad; //30
  int pVlavo; //56
  int pVpravo; //21
  int x;
  int y;
  int nVpred;
  int nVzad;
  int nVpravo;
  int nVlavo;
  int k;
  int reps;
  pthread_mutex_t mutex;
  int ** mapa;
  float **statPocetKrokov;
  float **dostalSaDoStredu;


}SIMPAM;

int velkostMapy(SIMPAM* args) {
    int rozmerX = 2*args->maxX + 1;
    int rozmerY =2*args->maxY + 1;
    return  rozmerX*rozmerY;

}
void generujMapu(SIMPAM* args) {
    
    int napocitavanie = 0;
    int pocetPrekazok = rand() % (args->maxX + args->maxY);
    int max = pocetPrekazok;
    while (napocitavanie < pocetPrekazok && max > 0) {
        
        max--;
        int prekX = rand() % (2 * args->maxX + 1);  
        int prekY = rand() % (2 * args->maxY + 1); 
   
      if (prekX < 0 || prekX > 2 * args->maxX || prekY < 0 || prekY > 2 * args->maxY) {
            printf("Generated invalid index prekX=%d, prekY=%d\n", prekX, prekY);
            continue; // Ignorovať túto iteráciu
        }
        bool validna = true;
      

        // Kontrola okolia
        for (int i = -1; i <= 1; i++) { // kontroluje okolie x
            for (int j = -1; j <= 1; j++) { // detto pre y
          
                int noveX = prekX + i;
                int noveY = prekY + j;

                if (noveX >= 0 && noveX <= 2 * args->maxX && 
                    noveY >= 0 && noveY <= 2 * args->maxY) {
                    printf("%d %d\n",noveX,noveY);
                    if (args->mapa[noveX][noveY] == 1) {
                        validna = false;
                        break;
                    }
                }
            }
            if (!validna) break;
        }


        // Ak je pozícia validná, pridaj prekážku
        if (validna) {
            args->mapa[prekX][prekY] = 1;
            napocitavanie++;
        }
  }
}

int vyberSmer(void* args){

  SIMPAM* input = (SIMPAM*)args;

  int random = rand()%(input->pVpred + input->pVzad + input->pVlavo + input->pVpravo);
  int volba = 0;
  if(random < input->pVpred){
    // pohne sa vpred
    volba = 1;
  }else if (random < input->pVpred+input->pVzad) {
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
    int posun = vyberSmer(args);  // Získanie smeru pohybu
    int newI = args->x;
    int newJ = args->y;

    // Aktualizácia súradníc na základe smeru
    if (posun == 1) {  // Posun doprava (j++)
        newJ = (args->y + 1 > args->maxY) ? 0 : args->y + 1;
    } else if (posun == 2) {  // Posun doľava (j--)
        newJ = (args->y - 1 < 0) ? args->maxY : args->y - 1;
    } else if (posun == 3) {  // Posun nahor (i--)
        newI = (args->x - 1 < 0) ? args->maxX : args->x - 1;
    } else if (posun == 4) {  // Posun nadol (i++)
        newI = (args->x + 1 > args->maxX) ? 0 : args->x + 1;
    } else {
        return;
    }

    // Kontrola, či je nové políčko blokované
    if (args->mapa[newI][newJ] == 1) {
        return;
    }

    // Ak pohyb nie je blokovaný, aktualizujeme súradnice
    args->x = newI;
    args->y = newJ;
    
}
void replikuj(SIMPAM *args) {
    for (int i = 0; i <= 2 * args->maxX; i++) {
        for (int j = 0; j <= 2 * args->maxY; j++) {
            int totalSteps = 0;

            for (int r = 0; r < args->reps; r++) {
                args->x = i;  // Nastavenie počiatočnej pozície
                args->y = j;
                int steps = 0;

                while (steps < args->k) {  // Maximálny počet krokov
                    // Ak sa dostaneme na cieľové políčko (hodnota 2), ukončíme pohyb
                    if (args->mapa[args->x][args->y] == 2) {
                        args->dostalSaDoStredu[i][j] += 1;
                        break;
                    }

                    // Pohyb na základe pravidiel
                    zmenPoziciu(args);
                    steps++;
                }

                if (steps == args->k && args->mapa[args->x][args->y] != 2) {
                    printf("NEDOSTAL SA na [%d][%d] po %d krokoch.\n", i, j, args->k);
                   // args->statPocetKrokov[i][j] = 0;
                } else {
                    totalSteps += steps;
                }
            }

            // Vypočíta sa priemerný počet krokov pre danú počiatočnú pozíciu, ak sa niekedy dostal
            if (totalSteps > 0) {
                double averageSteps = (double)totalSteps / args->reps;
                printf("Startovacia pozícia [%d][%d]: Priemerný počet krokov = %.2f\n", i, j, averageSteps);
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


  // JOJO PRIDAL ▄︻デ══━一💥
  // Veľkosť štruktúry
  size_t inputSize = sizeof(Input);

  // Pripojenie k zdieľanej pamäti
  int shm_fd = shm_open("/sem.shared_input_RJ", O_RDWR, 0666);
  if (shm_fd == -1) {
      perror("shm_open");
      exit(EXIT_FAILURE);
  }

  // Mapovanie pamäte
  Input *inputJojo = mmap(NULL, inputSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (inputJojo == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
  }

    char cesta[300] = "../../map_files/";
    strcat(cesta, inputJojo->mapaSubor);
    printf("Server cesta mapy: %s\n", cesta);
    FILE *mapInput = fopen(cesta, "r");

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
  input->maxX = inputJojo->maxX;
  input->maxY = inputJojo->maxY;
  input->pVpred = inputJojo->pVpred;
  input->pVzad = inputJojo->pVzad;
  input->pVpravo = inputJojo->pVpravo;
  input->pVlavo = inputJojo->pVlavo;
  input->k = inputJojo->k;
  input->x = 0;
  input->y = 0;
  input->nVpred = 0;
  input->nVzad = 0;
  input->nVpravo = 0;
  input->nVlavo = 0;
  input->reps = inputJojo->pocetReplikacii;
  bool generujem = false;
  FILE* mapInput;


input->mapa = malloc((2*input->maxX + 1) * sizeof(int*));
  for(int i = 0; i < (2*input->maxX +1); i++){
    input->mapa[i] = malloc((2*input->maxY + 1) * sizeof(int));
  }
 input->statPocetKrokov = malloc((2*input->maxX +1)*sizeof(float*));
  for(int i = 0; i < (2*input->maxX +1); i++){
    input->statPocetKrokov[i] = malloc((2*input->maxY + 1)*sizeof(float));
  }
  input->dostalSaDoStredu = malloc((2*input->maxX + 1) * sizeof(float*));
  for(int i = 0; i < (2*input->maxX + 1);i++){
    input->dostalSaDoStredu[i] = malloc((2*input->maxY + 1) * sizeof(float));
  }
for(int i = 0; i <= 2 * input->maxX; i++) {
    for(int j = 0; j <= 2 * input->maxY; j++){
      input->mapa[i][j] = 0;
      input->statPocetKrokov[i][j] = 0.0;
      input->dostalSaDoStredu[i][j] = 0.0;
      }
  }




            printf("Server: pred prekazkami\n");
  // JOJO PRIDAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // JOJO PRIDAL ▄︻デ══━一💥
  Prekazky prekazky;
  prekazky.pocet = 0;



for (int i = 0; i < 2 * input->maxX + 1; i++) {
    if (input->mapa[i] == NULL) {
        printf("Memory allocation failed - mapa[%d]\n", i);
        exit(EXIT_FAILURE);
    }
    if (input->statPocetKrokov[i] == NULL) {
        printf("Memory allocation failed - statPocetKrokov[%d]\n", i);
        exit(EXIT_FAILURE);
    }
    if (input->dostalSaDoStredu[i] == NULL) {
        printf("Memory allocation failed - dostalSaDoStredu[%d]\n", i);
        exit(EXIT_FAILURE);
    }
}
printf("Memory allocation successful\n");


  if(strlen(inputJojo->mapaSubor) == 0){
    generujMapu(input);
    generujem = true;
  } else {
  char cesta[300] = "../../map_files/";
  strcat(cesta, inputJojo->mapaSubor);
  mapInput = fopen(cesta, "r");
  generujem = false;
  }

    //if (mapInput == NULL) {
    //    // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
    //    perror("Chyba pri otváraní súboru");
    //}

  
  int i,j;


 if(!generujem) { 
  while (fscanf(mapInput, "%d %d", &i, &j) == 2) {
        if (i >= 0 && i <=2*input->maxX && j >= 0 && j <=2*input->maxY) {
            input->mapa[i][j] = 1;  // Nastav hodnotu na 1, ak sú súradnice platné
      
            printf("Server: pred zapisovanim prekazok\n");
            // JOJO PRIDAL
            prekazky.prekazky[prekazky.pocet].x = i;
            prekazky.prekazky[prekazky.pocet].y = j;
            prekazky.pocet++;
            printf("Server: po zapisovani prekazok\n");


        } else {
            printf("Súradnice (%d, %d) sú mimo rozsah!\n", i, j);
        }
    }
  } 

    input->mapa[input->maxX][input->maxY] = 2;

    int velkost = velkostMapy(input);
    printf("Veľkosť mapy: %d\n", velkost);
    replikuj(input);
    






    // JOJO PRIDAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // JOJO PRIDAL ▄︻デ══━一💥
    // Veľkosť štruktúry
    size_t mapaSize = sizeof(Mapa);

    // Pripojenie k zdieľanej pamäti
    printf("Server: Pred open\n");
    int shm_mapa_fd = shm_open("/sem.shared_mapa_RJ", O_RDWR, 0666);
    if (shm_mapa_fd == -1) {
      perror("shm_open");
      exit(EXIT_FAILURE);
    }

    // Mapovanie pamäte
    printf("Server: Pred mmap\n");
    Mapa* mapaJojo = mmap(NULL, mapaSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_mapa_fd, 0);
    if (mapaJojo == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
    }


    mapaJojo->opilec.x = input->x;
    mapaJojo->opilec.y = input->y;
    mapaJojo->prekazky = prekazky;

    // Odmapovanie pamäte a zatvorenie deskriptora
    printf("Server: Pred unmap\n");
    munmap(mapaJojo, mapaSize);
    printf("Server: Pred close\n");
    close(shm_mapa_fd);
    printf("Server: konec\n");
    //shm_unlink("../shared_input_jojo");
    //shm_unlink("/sem.shared_mapa_RJ");









  printf("Pole:\n");
    for (int x = 0; x <= 2*input->maxX; x++) {
        for (int y = 0; y <= 2*input-> maxY; y++) {
            printf("%d ", input->mapa[x][y]);
        }
        printf("\n");
    }
  printf("Stats:\n");
  for (int x = 0; x <= 2*input->maxX; x++ ) {
    for(int y = 0; y<= 2*input->maxY; y++) {
      printf("%6.1f", input->statPocetKrokov[x][y]);
    }
    printf("\n");
  }
printf("kolko krat sa dostal do stredu z %d iteracii:\n", input -> reps);
  for (int x = 0; x <= 2*input->maxX; x++ ) {
    for(int y = 0; y<= 2*input->maxY; y++) {
      printf("%6.1f", input->dostalSaDoStredu[x][y]);
    }
    printf("\n");
  }
  

  

    // JOJO PRIDAL ▄︻デ══━一💥
    // Otvorenie FIFO na zápis
    char fifo_cesta[256] = FIFO_RESULT;
    strcat(fifo_cesta, inputJojo->suborUlozenia);
    for(int p = 0; p < 4; p++) {
      fifo_cesta[strlen(fifo_cesta)-1] = '\0';
    }
    int fd_result = open(fifo_cesta, O_WRONLY);
    if (fd_result == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // float**
    // input->statPocetKrokov 
    // Zápis hodnôt do FIFO
    for(int r = 0; r < (input->maxY)*2+1; r++) { // po riadkoch
      if (write(fd_result, input->statPocetKrokov[r], sizeof(float)*input->maxX*2+1) == -1) {
        perror("write");
        close(fd_result);
        exit(EXIT_FAILURE);
      } 
    }
    
    close(fd_result);


  // Odmapovanie pamäte a zatvorenie deskriptora
  munmap(inputJojo, inputSize);
  close(shm_fd);
  //shm_unlink("../shared_input_jojo");
  shm_unlink("/sem.shared_input_RJ");

  // Vytvorenie pomenovanej FIFO
    //if (mkfifo(FIFO_RESULT, 0666) == -1) {
    //    perror("mkfifo");
    //    exit(EXIT_FAILURE);
    //}
  
  for(int p = 0; p < (2*input->maxX + 1);p++) {
    free(input->mapa[p]);
  }
 for(int p = 0; p < (2*input->maxX + 1);p++) {
    free(input->statPocetKrokov[p]);
  }

 for(int p = 0; p < (2*input->maxX + 1);p++) {
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
