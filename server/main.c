#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "../common/inputStruktura.c"

// zdielana pamat
#include <sys/mman.h>
#include <fcntl.h>

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


}SIMPAM;

int velkostMapy(SIMPAM* args) {
    int rozmerX = 2*args->maxX + 1;
    int rozmerY =2*args->maxY + 1;
    return  rozmerX*rozmerY;

}

int vyberSmer(void* args){

  SIMPAM* input = (SIMPAM*)args;

  int random = rand()%(input->pVpred + input->pVzad + input->pVlavo + input->pVpravo);
  int volba = 0;
  if(random < input->pVpred){
    //input->y++; // pohne sa vpred
    volba = 1;
  }else if (random < input->pVpred+input->pVzad) {
    //input->y--;//pohne sa vzad
    volba = 2;
  } else if(random < input->pVpred + input->pVzad + input->pVlavo) {
    //input->x--;//pohne sa vlavo
    volba = 3;
  }else {
    //input->x++;//pohne sa vpravo
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
        //printf("Neplatný smer pohybu: %d\n", posun);
        return;
    }

    // Kontrola, či je nové políčko blokované
    if (args->mapa[newI][newJ] == 1) {
       // printf("Pohyb na blokované políčko [%d][%d] zamietnutý.\n", newI, newJ);
        return;
    }

    // Ak pohyb nie je blokovaný, aktualizujeme súradnice
    args->x = newI;
    args->y = newJ;
    //printf("Presun na [%d][%d] úspešný.\n", args->x, args->y);
}
void replikuj(SIMPAM *args) {
    for (int i = 0; i < 2 * args->maxX; i++) {
        for (int j = 0; j < 2 * args->maxY; j++) {
            int totalSteps = 0;

            for (int r = 0; r < args->reps; r++) {
                args->x = i;  // Nastavenie počiatočnej pozície
                args->y = j;
                int steps = 0;

                while (steps < args->k) {  // Maximálny počet krokov
                    // Ak sa dostaneme na cieľové políčko (hodnota 2), ukončíme pohyb
                    if (args->mapa[args->x][args->y] == 2) {
                        //printf("Cieľ dosiahnutý na [%d][%d] po %d krokoch.\n", i, j, steps);
                        break;
                    }

                    // Pohyb na základe pravidiel
                    zmenPoziciu(args);
                    steps++;
                }

                if (steps == args->k && args->mapa[args->x][args->y] != 2) {
                    printf("NEDOSTAL SA na [%d][%d] po %d krokoch.\n", i, j, args->k);
                    args->statPocetKrokov[i][j] = 0;
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
        }
    }
}
/* void zmenPoziciu(SIMPAM* args){

  int posun = vyberSmer(args);

  //printf("Povodne suradnice: x: %d, y: %d\n",args->x,args->y);

  if(posun == 1) {
    args->nVpred++;
    if(args->y < args->maxY){
    args->y++;
   // printf("Vpred\n");
  } else {
	args->y = (-1)*args->maxY;
    //	printf("teleportacia spredu dozadu\n");
  }
  } else if(posun == 2){
    args->nVzad++;
    if(args->y > args->maxY*(-1)){
    args->y--;
    //printf("Vzad\n");
  } else {
	args->y = args->maxY;
    //	printf("teleportacia spredu dozadu\n");
  }
  } else if(posun == 3) {
    args->nVlavo++;
    if(args->x > args->maxX*(-1)){
    args->x--;
   // printf("Vlavo\n");
  } else {
	args->x = args->maxX;
   // 	printf("teleportacia zlava doprava\n");
  }

  } else if (posun == 4) {
    args->nVpravo++;
     if(args->x < args->maxX){
    args->x++;
   // printf("Vpravo\n");
  } else {
    args->x = (-1)*args->maxX;
   // printf("teleportacia sprava dolava\n");
    }
  } else {
    printf("error\n");
  }

 // printf("Nove suradnice: x: %d, y: %d\n******************************\n",args->x,args->y);

  }


void replikuj(SIMPAM* args) {
    for (int i = -args->maxX; i <= args->maxX; i++) {
        for (int j = -args->maxY; j <= args->maxY; j++) {
            int total = 0;

            for (int a = 0; a < args->reps; a++) {
                args->x = i;
                args->y = j;
                int num = 0;

                while (!(args->x == 0 && args->y == 0)&&num!=args->k) {
                    num++;
                    zmenPoziciu(args);
                }

                total += num;
            }

            // Výpočet priemeru
            double average = (double)total / args->reps;
            printf("Startovacia pozicia: x = %d, y = %d, priemerny pocet krokov: %.2f\n", i, j, average);
        }
    }
}*/



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

  printf("pred malloc\n"); 

  input->mapa = malloc((2*input->maxX + 1) * sizeof(int*));
  for(int i = 0; i < (2*input->maxX +1); i++){
    input->mapa[i] = malloc((2*input->maxY + 1) * sizeof(int));
  }


  printf("pred malloc\n"); 
  input->statPocetKrokov = malloc((2*input->maxX +1)*sizeof(float*));
  for(int i = 0; i < (2*input->maxX +1); i++){
    input->statPocetKrokov[i] = malloc((2*input->maxY + 1)*sizeof(float));
  }

  printf("pred malloc\n"); 
  for(int i = 0; i <= 2*input->maxX; i++) {
    for(int j = 0; j <= 2*input->maxY; j++){
      input->mapa[i][j] = 0;
      input->statPocetKrokov[i][j] = 0.0;
      }
  }

  printf("pred malloc\n"); 
  int i,j;
  while (fscanf(mapInput, "%d %d", &i, &j) == 2) {
        if (i >= 0 && i <=2*input->maxX && j >= 0 && j <=2*input->maxY) {
            input->mapa[i][j] = 1;  // Nastav hodnotu na 1, ak sú súradnice platné
        } else {
            printf("Súradnice (%d, %d) sú mimo rozsah!\n", i, j);
        }
    }

    input->mapa[input->maxX][input->maxY] = 2;
  
    int velkost = velkostMapy(input);
    printf("Veľkosť mapy: %d\n", velkost);
    replikuj(input);

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


  // JOJO PRIDAL ▄︻デ══━一💥
  // Odmapovanie pamäte a zatvorenie deskriptora
  munmap(inputJojo, inputSize);
  close(shm_fd);
  //shm_unlink("../shared_input_jojo");
  shm_unlink("/sem.shared_input_RJ");
  
  
  for(int p = 0; p < (2*input->maxX);p++) {
    free(input->mapa[p]);
    //free(input->statPocetKrokov[i]);
  }
 for(int p = 0; p < (2*input->maxX);p++) {
    //free(input->mapa[i]);
    free(input->statPocetKrokov[p]);
  }
  free(input->statPocetKrokov);
  free(input->mapa);
  free(input);
  fclose(mapInput);
  return 0;
}
