void replikuj(SIMPAM *args, Vykreslenie_shm* update_shm, sem_t* semServer, sem_t* semKlient) {
  for (int i = 0; i <= 2 * args->maxY; i++) { //☆.𓋼𓍊 𓆏 𓍊𓋼𓍊.☆
        for (int j = 0; j <= 2 * args->maxX; j++) {
            int totalSteps = 0; // ZA VSETKY REPS 1 policko

            for (int r = 0; r < args->reps; r++) {
                if(args->mapa[i][j] == 1) { break; }
                if(args->mapa[i][j] == 2) { break; }
                args->x = j;  // Nastavenie počiatočnej pozície
                args->y = i;
                int steps = 0;
                //printf("Server: ideme novu replikaciu\n");
                while (steps < args->k) {  // Maximálny počet krokov
                    // Ak sa dostaneme na cieľové políčko (hodnota 2), ukončíme pohyb
                    if (args->mapa[args->y][args->x] == 2) {
                        args->dostalSaDoStredu[i][j] += 1;
                        break;
                    }

                   // printf("\nServer pred pohybom: x:%d y:%d\n", args->x, args->y);
                    // Pohyb na základe pravidiel
                    zmenPoziciu(args);

                    // Kód vo while cykluse JOJO PRIDAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    if(update_shm != NULL){
                      if(update_shm->end) sem_post(semServer);
                      int hodnota;
                      sem_getvalue(semServer, &hodnota);
                      //printf("Server semafor hodnota pred wait: %d\n", hodnota);
                      sem_wait(semServer);
                      // Aktualizácia údajov
                      update_shm->mapa.opilec.x = args->x;  // Náhodná hodnota
                      update_shm->mapa.opilec.y = args->y;  // Náhodná hodnota
                      update_shm->replikacia = r;
                      update_shm->zacX = j;
                      update_shm->zacY = i;
                      update_shm->k = args->k;
                      update_shm->pocetKrokov = steps;

                      sem_post(semKlient);
                    }
                    //printf("Server: x:%d y:%d\n", args->x, args->y);
                    //printf("Server: krok cislo %d\n", steps);
                    //printf("Server: replikacia cislo %d\n", r);
                    //printf("Server: startovacia pozicia [%d][%d]\n", j, i);
                    steps++;
                }

                if (steps == args->k && args->mapa[args->y][args->x] != 2) {
                    //printf("NEDOSTAL SA na [%d][%d] po %d krokoch.\n", i, j, args->k);
                   // args->statPocetKrokov[i][j] = 0;
                } else {
                    totalSteps += steps;
                }
            }

            // Vypočíta sa priemerný počet krokov pre danú počiatočnú pozíciu, ak sa niekedy dostal
            if (totalSteps > 0) {
                double averageSteps = (double)totalSteps / args->reps;
                //printf("Startovacia pozícia [%d][%d]: Priemerný počet krokov = %.2f\n", i, j, averageSteps);
                args->statPocetKrokov[i][j] = averageSteps;
            }
          if(args->mapa[i][j] == 1){
            args->statPocetKrokov[i][j] = 0.0;
            args->dostalSaDoStredu[i][j] = 0.0;

           }
        }
    }
}
