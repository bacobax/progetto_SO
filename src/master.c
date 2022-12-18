#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"

#include "./master.h"


void codiceMaster(int semid, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID) {
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

    printf("M: Finito generazione\n");
    mutex(semid, LOCK, errorHandler);
    aspettaConfigs(waitconfigSemID);

    /*
    genera_navi()
    mutex(semid, LOCK, errorHandler);
    */

    printf("Master: ciao\n");

    
    for (i = 0; i < SO_DAYS; i++) {
        printf("Master: dormo\n");
        nanosecsleep(NANOS_MULT);
        /* TODO: funzione dump */
    }
}

int main(int argc, char const* argv[]) {
    mySettedMain(codiceMaster);
    return 0;
}
