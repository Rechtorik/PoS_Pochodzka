#ifndef VYKRESLENIE_STRUCT
#define VYKRESLENIE_STRUCT

typedef struct Vykreslenie_shm{
  Mapa mapa;
  int end;
  int pripojenie;
  int pocetPripojenych;
  int replikacia;
  int pocetReplikacii;
  int zacX;
  int zacY;
} Vykreslenie_shm;

#endif // PREKAZKY
