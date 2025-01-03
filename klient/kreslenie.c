#include <stdio.h>
#include <stdlib.h>

#include "../common/prekazky.h"
#include "../common/inputStruktura.h"

// Funkcia na vykreslenie mapy
int kresli(Input* input, Suradnice player, int pocetPrekazok, Prekazky* prekazky) {
    //system("clear"); // Vyčistenie konzoly (použi "cls" na Windows)
    for (int i = 0; i < input->maxY*2+1; i++) {
        for (int j = 0; j < input->maxX*2+1; j++) {
            if(j == input->maxX && i == input->maxY) { printf("X "); continue; } // stred mapy
            if (i == player.y && j == player.x) {
                printf("P "); // Hráč
                continue;
            } else {
                for(int k = 0; k < pocetPrekazok; k++) {
                    if(i == prekazky->prekazky[k].x && j == prekazky->prekazky[k].y) {
                        printf("# "); // Prekazka
                        break;
                    }
                    if(k == pocetPrekazok-1) printf(". ");
                }
            }
        }
        printf("\n");
    }
  return 0;
}


