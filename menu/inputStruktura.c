#ifndef INPUT_STRUKTURA
#define INPUT_STRUKTURA

typedef struct Input{
  // OPATOVNE SPUSTENIE
  int opatovneSpustenie; // 1 ak ano, 0 ak nie
  // MAPA
  char* mapaSubor;   // súbor z ktoreho sa ma nacitat mapa
  int maxX;     // sirka mapy - ak je 5 tak do +5 a -5 (vratane)
  int maxY;     // vyska mapy - ak je 5 tak do +5 a -5 (vratane)
  // PRAVDEPODOBNOSTI
  float pVpred;   // pravdepodobnost DOLE
  float pVzad;    // pravdepodobnost HORE
  float pVlavo;   // pravdepodobnost DOLAVA
  float pVpravo;  // pravdepodobnost DOPRAVA
  // MAX POCET KROKOV
  int k;
  // POCET REPLIKACII
  int pocetReplikacii;
  // SUBOR ULOZENIA
  char* suborUlozenia;
  // MOZU SA OSTATNI PRIPOJIT?  1 => ano, 0 => nie
  int pripojenie;
  // PRIPOJENIE SA NA PREBIEHAJUCU SIMULACIU?  1 => ano, 0 => nie
  int pripojenieNaPrebiehajucu;
  // SUBOR SO SIMULACIOU KTORA SA BUDE OPATOVNE SPUSTAT
  char* suborSoSimulaciou;
} Input;

#endif // INPUT_STRUKTURA
