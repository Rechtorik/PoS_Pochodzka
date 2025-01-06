#include <stdio.h>
#include <stdlib.h>

#include "../common/prekazky.h"
#include "../common/inputStruktura.h"
#include "../common/vykreslenieStruct.h"

// Funkcia na vykreslenie mapy

int kresli(Vykreslenie_shm* vykreslenie, int tvorca) {
    system("clear"); // Vyčistenie konzoly
    printf("Počet sledovateľov:  %d\n", vykreslenie->pocetPripojenych);
    printf("Replikácia: %d / %d\n", vykreslenie->replikacia + 1, vykreslenie->pocetReplikacii);
    printf("Počiatočná pozícia: [%d][%d]\n", vykreslenie->zacX, vykreslenie->zacY);
    printf("Krok: %d/%d\n\n", vykreslenie->pocetKrokov + 1, vykreslenie->k);

    for (int i = 0; i < vykreslenie->mapa.maxY*2+1; i++) {
        for (int j = 0; j < vykreslenie->mapa.maxX*2+1; j++) {

          if(j == vykreslenie->mapa.maxX && i == vykreslenie->mapa.maxY) { printf("X "); continue; } // stred mapy
          if (i == vykreslenie->mapa.opilec.y && j == vykreslenie->mapa.opilec.x) { printf("O "); continue; } // Hráč
          for(int k = 0; k < vykreslenie->mapa.prekazky.pocet + 1; k++) {

            if(k == vykreslenie->mapa.prekazky.pocet) { printf(". "); break;}
            if(i == vykreslenie->mapa.prekazky.prekazky[k].y && j == vykreslenie->mapa.prekazky.prekazky[k].x) {
              printf("# "); // Prekazka
              break;
            }
          }
        }

        printf("\n");
    }

    if(tvorca) {
      printf("\nPre ukončenie vizualizácie stlačte 'k' a potom ENTER\n");
    } else {
      printf("\nSte v režime sledovania simulácie, pre ukončenie stlačte 'k' a potom ENTER\n");
      
    }
    usleep(300000);
  return 0;
}


