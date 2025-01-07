void* vytvorenieDatovoduInput(void* args) {
  // FIFO INPUT vytvorenie
  // Skontroluj, či FIFO existuje
    if (access(FIFO_INPUT, F_OK) == -1) {
        // FIFO neexistuje, pokus o jeho vytvorenie
        if (mkfifo(FIFO_INPUT, 0666) == -1) {
            perror("Chyba pri vytváraní FIFO");
            return NULL;
        }
    }
  return NULL;
}

void* vytvorenieInputFileOpatovnaSimulacia(void* args) {
  ProcesPamat* par = (ProcesPamat*)args;

    char cesta[300] = "../../input_files/";
    strcat(cesta, par->input->suborUlozenia);
    FILE *fileInput = fopen(cesta, "r");

    //if (fileInput == NULL) {
    //    // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
    //    perror("Chyba pri otváraní súboru");
    //}

    char buffer[256]; // Buffer na uloženie prečítaného obsahu
    // Čítanie obsahu riadok po riadku

    fgets(buffer, sizeof(buffer), fileInput);
    // nazovMapy
    buffer[strlen(buffer) - 1] = '\0';
    strcat(par->input->mapaSubor, buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    // maxX
    buffer[strlen(buffer) - 1] = '\0';
    par->input->maxX = atoi(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //maxY
    buffer[strlen(buffer) - 1] = '\0';
    par->input->maxY = atoi(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVpred
    buffer[strlen(buffer) - 1] = '\0';
    par->input->pVpred = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVzad
    buffer[strlen(buffer) - 1] = '\0';
    par->input->pVzad = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVlavo
    buffer[strlen(buffer) - 1] = '\0';
    par->input->pVlavo = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //pVpravo
    buffer[strlen(buffer) - 1] = '\0';
    par->input->pVpravo = atof(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //k
    buffer[strlen(buffer) - 1] = '\0';
    par->input->k = atoi(buffer);

    fgets(buffer, sizeof(buffer), fileInput);
    //nazov simulacie
    buffer[strlen(buffer) - 1] = '\0';
    //par->input->suborUlozenia = buffer;

    fgets(buffer, sizeof(buffer), fileInput);
    //pripojenie (0 alebo 1)
    buffer[strlen(buffer) - 1] = '\0';
    //par->input->pripojenie = atoi(buffer);

    fclose(fileInput);

  return NULL;
}

void* vytvorenieInputFileNovaSimulacia(void* args) {
  ProcesPamat* par = (ProcesPamat*)args;

    char cesta[300] = "../../input_files/";
    strcat(cesta, par->input->suborUlozenia);
    FILE *fileInput = fopen(cesta, "w");

    if (fileInput == NULL) {
        // Ak sa súbor nepodarí otvoriť, vypíše sa chybová hláška
        perror("Chyba pri otváraní súboru");
    }

    // Zápis textu do súboru
    fprintf(fileInput, "%s\n", par->input->mapaSubor);
    fprintf(fileInput, "%d\n", par->input->maxX);
    fprintf(fileInput, "%d\n", par->input->maxY);
    fprintf(fileInput, "%f\n", par->input->pVpred);
    fprintf(fileInput, "%f\n", par->input->pVzad);
    fprintf(fileInput, "%f\n", par->input->pVlavo);
    fprintf(fileInput, "%f\n", par->input->pVpravo);
    fprintf(fileInput, "%d\n", par->input->k);
    fprintf(fileInput, "%s\n", par->input->suborUlozenia);
    fprintf(fileInput, "%d\n", par->input->pripojenie);

    // Zatvorenie súboru
    fclose(fileInput);

    // Pokus o vymazanie súboru result
    char resultFile[256] = "../../result_files/";
    strcat(resultFile, par->input->suborUlozenia);
    remove(resultFile);

  return NULL;
}

void* vykreslovanie(void* args) {
  ProcesPamat* par = (ProcesPamat*)args;

  //printf("Vykresluje sa 𓁹‿𓁹\n");
  if(par->owner) {
    //usleep(500000);
    while(par->vykreslenie->end == 0) {
      sem_wait(par->semKlient);
      kresli(par->vykreslenie, par->owner);
      usleep(20000);
      sem_post(par->semServer);
    }
  } else {
    while(par->proces_vykreslenie) {
      //sem_wait(par->semKlient);
      kresli(par->vykreslenie, par->owner);
      //usleep(20000);
      //sem_post(par->semServer);
    }
  }


  return NULL;
}


// Funkcia vlákna na ukončenie výstupu
void *waitForK(void *arg) {
  ProcesPamat* par = (ProcesPamat*)arg;

  while (par->vykreslenie->end == 0) {
    int ch = getchar();
    if (ch == 'k') {  // Detekcia 'k'
      //if(par->pocetPripojenych > 1) {
      if(!par->owner) {
        //par->pocetPripojenych--;
        par->proces_vykreslenie = 0;
        break;
      } else {
        par->vykreslenie->end = 1; // Nastavenie end na 1
      }
      break;
    }
  }

  return NULL;
}
