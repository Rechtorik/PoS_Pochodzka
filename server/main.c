#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

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

  void zmenPoziciu(SIMPAM* args){

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

                while (!(args->x == 0 && args->y == 0)) {
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
}



int main(int argc, char *argv[]){
  srand(time(NULL));

  SIMPAM* input = malloc(sizeof(SIMPAM));

  input->maxX = atoi(argv[1]);
  input->maxY = atoi(argv[2]);
  input->pVpred = atof(argv[3]);
  input->pVzad = atof(argv[4]);
  input->pVpravo = atof(argv[5]);
  input->pVlavo = atof(argv[6]);
  input->k = atoi(argv[7]);
  input->x = 0;
  input->y = 0;
  input->nVpred = 0;
  input->nVzad = 0;
  input->nVpravo = 0;
  input->nVlavo = 0;
  input->reps = 100000;
  
    int velkost = velkostMapy(input);
    printf("Veľkosť mapy: %d\n", velkost);
  replikuj(input);
  free(input);
  return 0;
}
