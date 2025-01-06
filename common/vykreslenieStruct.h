#ifndef VYKRESLENIE_STRUCT
#define VYKRESLENIE_STRUCT

typedef struct Vykreslenie_shm{
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
