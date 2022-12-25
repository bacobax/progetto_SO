#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"

#include "./master.h"
#include "./dump.h"


void codiceMaster(int startSimulationSemID, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID, int msgRefillerID, int waitEndDaySemID) {
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
    quantitaPrimoGiorno = quantitaAlGiorno + (resto);

    printf("Quantità primo giorno: %d\n" , quantitaPrimoGiorno);
    
    /*  per ora ho usato solo startSimulationSemID */
    genera_porti(quantitaPrimoGiorno, SO_PORTI); /* da tradurre in inglese */


   
    genera_navi();

    printf("M: Finito generazione\n");
    aspettaConfigs(waitconfigSemID);
    mutex(startSimulationSemID, LOCK, errorHandler);

    
    /*
    mutex(semid, LOCK, errorHandler);
    */


    
    for (i = 0; i < SO_DAYS; i++) {
        printDump(i);
        
        printf("Master: dormo\n");
        if (i > 0) {
            expirePortsGoods(i);
            expireShipGoods();
            refillPorts(SYNC, msgRefillerID, quantitaAlGiorno, i);
            mutex(waitEndDaySemID, WAITZERO, errorHandler);
            
        }
        #ifndef __linux__
        nanosecsleep(NANOS_MULT);
        #endif
        #ifdef __linux__
        sleep(1);
        #endif

        /*expireShipGoods();
        
            kill(0, SIGALRM);
        */
        /* TODO: funzione dump */
    }
    printDump(i);
    #ifndef __linux__
    nanosecsleep(NANOS_MULT);
    #endif
    sleep(1);
}

int main(int argc, char const* argv[]) {
    mySettedMain(codiceMaster);
    return 0;
}
