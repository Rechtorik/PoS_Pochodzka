#ifndef PREKAZKY
#define PREKAZKY

typedef struct Suradnice{
  int x;
  int y;
} Suradnice;

typedef struct Prekazky{
  int pocet;
  Suradnice prekazky[100];
} Prekazky;

typedef struct Mapa{
  int maxX;
  int maxY;
  Suradnice opilec; 
  Prekazky prekazky;
} Mapa;

#endif // PREKAZKY
