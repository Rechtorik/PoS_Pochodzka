#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include "../common/inputStruktura.h"

void novaSimulacia(Input* input);
void pripojenieSa(Input* input);
void opatovnaSimulacia(Input* input);
void koniec();

void simulaciaForm(Input* input);
void opatovneSpustenieForm(Input* input);

int menu(Input* input) {
  int choice;

  while (1) {
    system("clear");
    printf("\n========== MENU ==========");
    printf("\n1. Nová simulácia\n");
    printf("2. Pripojenie sa na prebiehajúcu simuláciu\n");
    printf("3. Opätovná simulácia\n");
    printf("4. Koniec\n");
    printf("==========================\n");
    printf("Zadajte svoju voľbu: ");

    if (scanf("%d", &choice) != 1) {
      printf("Neplatný vstup! Prosím, zadajte číslo.\n");
      while (getchar() != '\n'); // Vyčistenie bufferu
      continue;
    }

    switch (choice) {
      case 1:
        novaSimulacia(input);
        return 0;
        break;
      case 2:
        pripojenieSa(input);
        return 0;
        break;
      case 3:
        opatovnaSimulacia(input);
        return 0;
        break;
      case 4:
        koniec();
        return 1; // mozno dat int a return 0
        break;
      default:
        printf("Neplatná voľba! Skúste to znova.\n");
    }
  }
}


void novaSimulacia(Input* input) {
  input->pripojenieNaPrebiehajucu = 0;
  input->opatovneSpustenie = 0;
  int choice2;
  system("clear");

  printf("\n========== MENU ==========");
  printf("\n1. Generovaná mapa\n");
  printf("2. Použiť mapu zo súboru\n");
  printf("3. Použiť prázdnu mapu\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &choice2) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  if(choice2 == 1) {
    // GENEROVANA MAPA
    simulaciaForm(input);
  } else if(choice2 == 2) {
    // MAPA ZO SUBORU
    system("clear");
    // Zobrazenie menu
    printf("\n========== MENU ==========\n");
    printf("Zadajte názov mapy (.txt): ");
    while (getchar() != '\n'); // Vyčistenie bufferu
    if (fgets(input->mapaSubor, 256, stdin) == NULL) {
      printf("Chyba pri čítaní vstupu.\n");
    }
    input->mapaSubor[strcspn(input->mapaSubor, "\n")] = '\0';
    simulaciaForm(input);
  } else if(choice2 == 3) {
    // PRAZDNA MAPA
    strcat(input->mapaSubor, "empty.txt");
    simulaciaForm(input);
  }
}

void pripojenieSa(Input* input) {
  input->pripojenieNaPrebiehajucu = 1;
  input->opatovneSpustenie = 0;
}

void opatovnaSimulacia(Input* input) {
  input->pripojenieNaPrebiehajucu = 0;
  input->opatovneSpustenie = 1;
  opatovneSpustenieForm(input);
}

void koniec() {
  printf("Aplikácia sa ukončuje...\n");
}


void simulaciaForm(Input* input) {
  int maxX;
  int maxY;
  float pVpred;
  float pVzad;
  float pVlavo;
  float pVpravo;
  int k;
  int pocetReplikacii;
  int pripojenie;
  int vykreslenie;

  
  system("clear");
  // maxX
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj maxX\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &maxX) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // maxY
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj maxY\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &maxY) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // pVpred
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj pravdepodobnost pohybu dole\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%f", &pVpred) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // pVzad
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj pravdepodobnost pohybu hore\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%f", &pVzad) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // pVlavo
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj pravdepodobnost pohybu dolava\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%f", &pVlavo) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // pVpravo
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj pravdepodobnost pohybu doprava\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%f", &pVpravo) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // k
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj maximálny počet krokov (0 pre nekonečno)\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &k) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }
  if(k == 0) {
    k = INT_MAX;
  }

  system("clear");
  // pocetReplikacii
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj počet replikácií\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &pocetReplikacii) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

   system("clear");
  // pripojenie
  printf("\n========== MENU ==========");
  printf("\n1. Môžu sa ostatní pripojiť k simulácii?\n");
  printf("   1 => áno\n");
  printf("   0 => nie\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &pripojenie) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // vykreslenie
  printf("\n========== MENU ==========");
  printf("\n1. Chcete vykresliť priebeh?\n");
  printf("   1 => áno\n");
  printf("   0 => nie\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &vykreslenie) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // subor ulozenia
  printf("\n========== MENU ==========\n");
  printf("Zadajte názov simulacie (.txt): ");
  while (getchar() != '\n'); // Vyčistenie bufferu

  // Čítanie reťazca
  if (fgets(input->suborUlozenia, 256, stdin) == NULL) {
    printf("Chyba pri čítaní vstupu.\n");
  }

  system("clear");
  
  input->suborUlozenia[strcspn(input->suborUlozenia, "\n")] = '\0';

  // Tu mám všetky premenné načítané, teraz ich uložiť do štruktúry
  input->maxX = maxX;
  input->maxY = maxY;
  input->pVpred = pVpred;
  input->pVzad = pVzad;
  input->pVpravo = pVpravo;
  input->pVlavo = pVlavo;
  input->k = k;
  input->pocetReplikacii = pocetReplikacii;
  //input->suborUlozenia = suborUlozenia;
  input->pripojenie = pripojenie;
  input->vykreslenie = vykreslenie;
}

void opatovneSpustenieForm(Input* input) {
  int pocetReplikacii;
  int pripojenie;
 
  system("clear");
  // pocetReplikacii
  printf("\n========== MENU ==========");
  printf("\n1. Zadaj počet replikácií\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &pocetReplikacii) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

   system("clear");
  // pripojenie
  printf("\n========== MENU ==========");
  printf("\n1. Môžu sa ostatní pripojiť k simulácii?\n");
  printf("   1 => áno\n");
  printf("   0 => nie\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &pripojenie) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }

  system("clear");
  // subor ulozenia
  printf("\n========== MENU ==========\n");
  printf("Zadajte názov simulácie (.txt): \n");
  while (getchar() != '\n'); // Vyčistenie bufferu
    
    const char *input_dir = "../../input_files/";
    struct dirent *entry;

    // Otvorenie adresára /dev/shm/
    DIR *dir = opendir(input_dir);
    if (dir == NULL) {
        perror("Nepodarilo sa otvoriť input_files/ \n");
        exit(EXIT_FAILURE);
    }

    // Iterácia cez obsah adresára
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("  -> %s\n", entry->d_name);
        }
    }
    closedir(dir);
    printf("\n (づ ᴗ _ᴗ)づ: ");
  // Čítanie reťazca
  if (fgets(input->suborUlozenia, 256, stdin) == NULL) {
    printf("Chyba pri čítaní vstupu.\n");
  }

  system("clear");
  // vykreslenie
  printf("\n========== MENU ==========");
  printf("\n1. Chcete vykresliť priebeh?\n");
  printf("   1 => áno\n");
  printf("   0 => nie\n");
  printf("==========================\n");
  printf("Zadajte svoju voľbu: ");

  // Čítanie voľby používateľa
  if (scanf("%d", &(input->vykreslenie)) != 1) {
    printf("Neplatný vstup! Prosím, zadajte číslo.\n");
    while (getchar() != '\n'); // Vyčistenie bufferu
  }
  
  input->suborUlozenia[strcspn(input->suborUlozenia, "\n")] = '\0';

  // Tu mám všetky premenné načítané, teraz ich uložiť do štruktúry
  input->pocetReplikacii = pocetReplikacii;
  input->pripojenie = pripojenie;
}
