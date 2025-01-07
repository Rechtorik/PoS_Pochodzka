#ifndef VYKRESLENIE_STRUCT
#define VYKRESLENIE_STRUCT

typedef struct Vykreslenie_shm{
  pthread_mutex_t mutexSemafory;
  pthread_mutex_t mutexResult;
  Mapa mapa;
  int end;
  int pripojenie;
  int pocetPripojenych;
  int replikacia;
  int pocetReplikacii;
  int k;
  int pocetKrokov;
  int zacX;
  int zacY;
} Vykreslenie_shm;

#endif // PREKAZKY
