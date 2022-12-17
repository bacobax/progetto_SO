#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"

#include "./master.h"


void codiceMaster(int semid, int portsShmid, int shipsShmid, int reservePrintSem) {
    struct timespec tim, tim2;
    int i;
    int quantitaAlGiorno;
    int resto;
    int quantitaPrimoGiorno;
    quantitaAlGiorno = SO_FILL / SO_DAYS;
    resto = SO_FILL % SO_DAYS;
    quantitaPrimoGiorno = quantitaAlGiorno + (resto * SO_DAYS);


    
    /*  per ora ho usato solo semid */
    genera_porti(quantitaPrimoGiorno, SO_PORTI); /* da tradurre in inglese */


    mutex(semid, LOCK, errorHandler);


    /*
    genera_navi()
    mutex(semid, LOCK, errorHandler);
    */

    printf("Master: ciao\n");

    tim.tv_sec  = 1;
    tim.tv_nsec = 0;
    for (i = 0; i < SO_DAYS; i++) {
        printf("Master: dormo\n");

        /* TODO: funzione dump */
        nanosleep(&tim, &tim2);
    }
}

int main(int argc, char const* argv[]) {
    mySettedMain(codiceMaster);
    return 0;
}
