void generujMapu(SIMPAM* args, Prekazky* prekazky) {


    int pocetPokusov = 0; // 10 pokusov na najdenie miesta
    int napocitavanie = 0;
    int pocetPrekazok = 0;
    if(args->maxY < 2 || args->maxX < 2) {
      pocetPrekazok = 0;
  } else {
      pocetPrekazok = ((args->maxX*args->maxY)/2);
  }

     while((napocitavanie < pocetPrekazok) && (pocetPokusov < 20)) {

        int prekX = rand() % (2 * args->maxX + 1);
        int prekY = rand() % (2 * args->maxY + 1);
        bool validna = true;

        // Kontrola okolia
        for (int i = -1; i <= 1; i++) { // kontroluje okolie x 
            for (int j = -1; j <= 1; j++) { // detto pre y

                int noveX = prekX + i;
                int noveY = prekY + j;

                if (noveX > 0 && noveX < 2 * args->maxX && 
                  noveY > 0 && noveY < 2 * args->maxY &&
                  !(noveX == args->maxX && noveY == args->maxY)) {
                  if (args->mapa[noveY][noveX] == 1) {
                    validna = false;
                    break;
                  }
                } else {
                  validna = false;
                }
            }
            if (!validna) break;
        }

        // Ak je pozícia validná, pridaj prekážku
        if (validna) {
            pocetPokusov = 0;
            args->mapa[prekY][prekX] = 1;
            napocitavanie++;

            prekazky->prekazky[prekazky->pocet].x = prekX;
            prekazky->prekazky[prekazky->pocet].y = prekY;
            prekazky->pocet++;
        } else { pocetPokusov++; }
  }
}
