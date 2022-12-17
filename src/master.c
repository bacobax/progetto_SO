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
    /*
    quantitaAlGiorno rappresenta la divisione di SO_FILL per SO_DAYS, solo che può darsi che SO_FILL non sia divisbile per SO_DAYS,
    la soluzione che ho pensato è che per tutti i giorni diversi dal primo si tiene in considerazione soltanto la parte intera della divisione
    mentre alle risorse del primo giorno vengono recuperate tutte le risorse perse prendendo solo la parte intera della divisione
    in altre parole alle risorse del primo giorno vengono aggiunti tutti i resti della divisione intera di "SO_FILL/SO_DAYS" per ogni giorno
    */
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
