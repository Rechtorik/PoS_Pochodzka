int vyberSmer(void* args){

  SIMPAM* input = (SIMPAM*)args;
  float random = (float)rand()/(float)RAND_MAX;
  int volba = 0;
  if(random < input->pVpred){
    // pohne sa vpred
    volba = 1;
  }else if (random < input->pVpred + input->pVzad) {
    //pohne sa vzad
    volba = 2;
  } else if(random < input->pVpred + input->pVzad + input->pVlavo) {
    //pohne sa vlavo
    volba = 3;
  }else {
    //pohne sa vpravo
    volba = 4;
  }
    return volba;
}

void zmenPoziciu(SIMPAM *args) {
    //int posun = vyberSmer(args);  // Získanie smeru pohybu

    int newX = args->x;
    int newY = args->y;
    bool ok = false;

    while(!ok){
      int posun = vyberSmer(args);  // Získanie smeru pohybu
      newX = args->x;
      newY = args->y;
      // Aktualizácia súradníc na základe smeru
    if (posun == 1) {  // Posun DOLE (j++) 
      newY = (args->y + 1 > 2*args->maxY) ? 0 : args->y + 1;
    } else if (posun == 2) {  // Posun HORE (j--)
      newY = (args->y - 1 < 0) ? 2*args->maxY : args->y - 1;
    } else if (posun == 3) {  // Posun VLAVO (i--)
      newX = (args->x - 1 < 0) ? 2*args->maxX : args->x - 1;
    } else if (posun == 4) {  // Posun VPRAVO (i++)
      newX = (args->x + 1 > 2*args->maxX) ? 0 : args->x + 1;
    }
    
    // Kontrola, či je nové políčko blokované 
    if (args->mapa[newY][newX] != 1) {
      ok = true;
    }
  }
    // Ak pohyb nie je blokovaný, aktualizujeme súradnice 
    args->x = newX;
    args->y = newY;
}
