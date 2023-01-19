#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "../config1.h"
#include "../utils/errorHandler.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"
#include "../utils/shm_utility.h"

#include "./master.h"
#include "./dump.h"



void codiceMaster(int startSimulationSemID, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID, int msgRefillerID, int waitEndDaySemID, int* day, int waitEndDayShipsSemID) {
    int quantitaAlGiorno;
    int resto;
    int quantitaPrimoGiorno;
    FILE* meteoPipe;
    char pypeDay[128];
    int aliveShips;
    int so_fill;
    int so_days;
    int so_porti;
    int so_navi;
    int c;
    Ship ships;
    FILE* fp;
    so_fill = SO_("FILL");
    so_days = SO_("DAYS");
    so_porti = SO_("PORTI");
    so_navi = SO_("NAVI");
    aliveShips = so_navi;
    
    fclose(fopen("./logs/masterlog.log", "w"));
    fp = fopen("./logs/masterlog.log" , "a+");
    /*
    quantitaAlGiorno rappresenta la divisione di SO_FILL per SO_DAYS, solo che può darsi che SO_FILL non sia divisbile per SO_DAYS,
    la soluzione che ho pensato è che per tutti i giorni diversi dal primo si tiene in considerazione soltanto la parte intera della divisione
    mentre alle risorse del primo giorno vengono recuperate tutte le risorse perse prendendo solo la parte intera della divisione
    in altre parole alle risorse del primo giorno vengono aggiunti tutti i resti della divisione intera di "SO_FILL/SO_DAYS" per ogni giorno
    */
    quantitaAlGiorno = so_fill / so_days;
    resto = so_fill % so_days;
    quantitaPrimoGiorno = quantitaAlGiorno + (resto);

    fprintf(fp,"Quantità primo giorno: %d\n" , quantitaPrimoGiorno);
    
    /*  per ora ho usato solo startSimulationSemID */
    genera_porti(quantitaPrimoGiorno, so_porti); /* da tradurre in inglese */


    meteoPipe = genera_meteo();
    genera_navi();

    fprintf(fp,"M: Finito generazione\n");
    aspettaConfigs(waitconfigSemID);

   
    ships = (Ship)getShmAddress(shipsShmid, 0, errorHandler, "master");

    printf("PID MASTER: %d, press any number to continue\n", getpid());
    scanf("%d", &c);

    mutex(startSimulationSemID, LOCK, errorHandler,  "mesterCode -> startSimulationSemID LOCK");
    

    fprintf(fp,"✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅\n");

    for (*day = 0; aliveShips && *day < so_days; *day = *day + 1) {
        aliveShips = countAliveShips();
        
        if (aliveShips)
        {
            fprintf(meteoPipe, "%d\n", *day);
            fflush(meteoPipe);
            printDump(ASYNC, *day,0);
            fprintf(fp,"MASTER: DAY: %d\n", *day);
            fprintf(fp,"Master: dormo\n");
            if (*day > 0) {
                expirePortsGoods(*day);
                /*
                expireShipGoods();

                */
                refillPorts(ASYNC, msgRefillerID, quantitaAlGiorno, *day);
                mutex(waitEndDaySemID, WAITZERO, errorHandler, "mesterCode -> waitEndDaySemID WAITZERO");
                mutex(waitEndDayShipsSemID, WAITZERO, errorHandler, "mesterCode -> waitEndDayShipSemID WAITZERO");
                mutex(waitEndDaySemID, so_porti, errorHandler, "mesterCode -> waitEndDaySemID +SO_PORTI");
            
            }
             nanosecsleep(NANOS_MULT);
             resetWeatherTargets(ships);
        }
        else {
            fprintf(fp,"Terminazione simulazione per navi morte\n");
        }
    }

    fprintf(meteoPipe, "%d\n", EOF);
    fflush(meteoPipe);
    fprintf(fp,"Master, faccio la pclose\n");
    /*
    pclose(meteoPipe);

    */
    fprintf(fp,"ENTRO\n");
    if (!aliveShips) {
    fprintf(fp,"SPACCO\n");
        
        *day = *day - 1;
    }
    fprintf(fp,"CIAO\n");
    fclose(fp);
}

int main(int argc, char const* argv[]) {
    mySettedMain(codiceMaster);
    return 0;
}
