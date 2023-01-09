#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "../config1.h"
#include "../utils/errorHandler.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"

#include "./master.h"
#include "./dump.h"

void errHandler(int er) {
    perror("errore nella WAITZERO GIORNALIERA NEL MASTER");
}

void codiceMaster(int startSimulationSemID, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID, int msgRefillerID, int waitEndDaySemID, int* day, int waitEndDayShipsSemID) {
    
    int quantitaAlGiorno;
    int resto;
    int quantitaPrimoGiorno;
    FILE* meteoPipe;
    char pypeDay[128];
    int aliveShips=SO_NAVI;
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


    meteoPipe = genera_meteo();
    genera_navi();

    printf("M: Finito generazione\n");
    aspettaConfigs(waitconfigSemID);
    mutex(startSimulationSemID, LOCK, errorHandler,  "mesterCode -> startSimulationSemID LOCK");


    printf("✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅\n");

    
    for (*day = 0; *day < SO_DAYS && aliveShips>0; *day = *day + 1) {
        aliveShips = countAliveShips();
        if (aliveShips) {
            fprintf(meteoPipe, "%d\n", *day);
            fflush(meteoPipe);
            printDump(ASYNC, *day);
            printf("MASTER: DAY: %d\n", *day);
            printf("Master: dormo\n");
            if (*day > 0) {
                expirePortsGoods(*day);
                expireShipGoods();
                refillPorts(ASYNC, msgRefillerID, quantitaAlGiorno, *day);
                mutex(waitEndDaySemID, WAITZERO, errorHandler, "mesterCode -> waitEndDaySemID WAITZERO");
                mutex(waitEndDayShipsSemID, WAITZERO, errorHandler, "mesterCode -> waitEndDayShipSemID WAITZERO");
                mutex(waitEndDaySemID, SO_PORTI, errorHandler, "mesterCode -> waitEndDaySemID +SO_PORTI");
            
            }
             nanosecsleep(NANOS_MULT); 
        }
        else {
            printf("Terminazione simulazione per navi morte\n");
        }
        
    }
    fprintf(meteoPipe, "%d\n", EOF);
    fflush(meteoPipe);
    printf("Master, faccio la pclose\n");
        
    if (pclose(meteoPipe) == -1) {
        perror("meteoPipe close");
        exit(EXIT_FAILURE);
    }
    nanosecsleep(NANOS_MULT);
    
}

int main(int argc, char const* argv[]) {
    mySettedMain(codiceMaster);
    return 0;
}
