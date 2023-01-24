#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "../config1.h"
#include "./nave.h"
#include "./porto.h"
#include "../utils/errorHandler.h"
#include "../utils/support.h"
#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"

void malestormRoutine() {
    int victimIdx;
    int i;
    Ship victimShip;
    int shipShmID;
    intList* shipsList;
    int* a;
    int victim;
    int semShipID;
    int so_malestorm;
    int so_navi = SO_("NAVI");
    so_malestorm = SO_("MAELSTROM");
    semShipID = useSem(SEMSHIPKEY, errorHandler, "childExpireShipCode");
     
    
    shipShmID = useShm(SSHMKEY, sizeof(struct ship) * so_navi, errorHandler, "shipShmID in malestormRoutine");
    a = (int*)malloc(sizeof(int) * so_navi);
    for(i=0; i<so_navi; i++){
        a[i] = i;
    }
    shipsList = intInitFromArray(a, so_navi); 
    free(a);
    while (1) {
        if (shipsList->length > 0) {
            victimIdx = random_int(0, shipsList->length - 1);
            printf("METEO: nave vittima per malestorm:%d\n", victimIdx);
            victim = *(intElementAt(shipsList, victimIdx));
            printf("Tra %d ore killo la nave %d\n", so_malestorm, victim);
            nanosecsleep((double)(0.04166667 * NANOS_MULT) * so_malestorm);

            
            victimShip = ((Ship)getShmAddress(shipShmID, 0, errorHandler, "getShmAddress in malestormRoutine di shipsArray")) + victim;


            mutexPro(semShipID, victim, LOCK, errorHandler, "semShipID LOCK");
        
            victimShip->dead = 1;
            
            mutexPro(semShipID, victim, UNLOCK, errorHandler, "semShipID UNLOCK");
            
            shmDetach(victimShip - victim, errorHandler, "malestormRoutine");
            intRemove(shipsList, victimIdx);
            
        }
        else {
            printf("MALESTORM: Ho rimosso tutte le navi\n");
            exit(EXIT_SUCCESS);
        }
        
    }
        
}
void malestormHandler() {
    int pid;
    pid = fork();

    if (pid == -1) {
        throwError("fork in launchStorm", "malestormHandler");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        malestormRoutine();
        exit(EXIT_FAILURE);
    }
}


void stormRoutine(){
    int shmShipID;
    Ship ship;
    int victimIdx;
    int so_navi = SO_("NAVI");
    victimIdx = random_int(0, so_navi - 1);
    shmShipID = useShm(SSHMKEY, sizeof(struct ship) * so_navi, errorHandler, "meteo stormRoutine");
    ship = ((Ship) getShmAddress(shmShipID, 0, errorHandler, "getShmAddress meteo stormRoutine")) + victimIdx;
    ship->storm = 1;
    shmDetach(ship - victimIdx,errorHandler,"stormRoutine");
}

void launchStorm() {
    int pid = fork();
    if(pid == -1){
        throwError("fork in launchStorm","launchStorm");
        exit(EXIT_FAILURE);
    } else if(pid == 0){
        stormRoutine();
        exit(EXIT_SUCCESS);
    }
}

void swellRoutine(){
    Port port;
    int victimIdx;
    int so_porti = SO_("PORTI");
    victimIdx = random_int(0, so_porti - 1);
    port =getPort(victimIdx);
    port->swell = 1;
    detachPort(port, victimIdx);
}

void launchSwell(){
    int pid = fork();
    if(pid == -1){
        throwError("fork in launchSwell","launchSwell");
        exit(EXIT_FAILURE);
    } else if(pid == 0){
        swellRoutine();
        exit(EXIT_SUCCESS);
    } 
}

int main(int argc, char* argv[]) {
    int day;
    int endShmID;
    int aspettoMortePortiSemID;
    char str[128];
    signal(SIGCHLD, SIG_IGN);
    
    endShmID = useShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "getEndDayShmID meteo");
    aspettoMortePortiSemID = useSem(WAITPORTSSEM, errorHandler, "aspettoMortePortiSemID in portCode");
    

    checkInConfig();
    waitForStart();

    if (WITH_MALESTORM) {
        malestormHandler();     
    }

    printf("Meteo partito...\n");

    while (fgets(str, 128, stdin) != NULL) {
        day = atoi(str);
        if (day == -1) {
            break;
        }
        
        launchStorm();
        launchSwell();
        
    }
    mutex(aspettoMortePortiSemID, LOCK, errorHandler, "LOCK su aspettoMortePortiSemID");
    printf("Meteo: termino\n");
    exit(EXIT_SUCCESS);
        
}

