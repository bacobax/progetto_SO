#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "../config1.h"
#include "./nave.h"
#include "../utils/errorHandler.h"
#include "../utils/support.h"
#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"

/*
    TODO: testare + aggiornare il dump con merce affondata


*/
void malestormRoutine() {
    int victimIdx;
    Ship victimShip;
    int shipShmID;
    int waitShipSemID;
    
    waitShipSemID = useSem(WAITSHIPSSEM, errorHandler, "nave waitShipSemID");

    shipShmID = useShm(SSHMKEY, sizeof(struct ship) * SO_NAVI, errorHandler, "shipShmID in malestormRoutine");
    while (1) {
        victimIdx = random_int(0, SO_NAVI - 1);
        nanosecsleep(0.04166667 * NANOS_MULT * SO_MAELSTROM);

        
        victimShip = ((Ship)getShmAddress(shipShmID, 0, errorHandler, "getShmAddress in malestormRoutine di shipsArray")) + victimIdx;
        
        mutex(waitShipSemID, LOCK, errorHandler, "nave mutex LOCK waitShipSemID");

        kill(shipsArray[victimIdx].pid, SIGTERM);
        
        shmDetach(victimShip - victimIdx, errorHandler, "malestormRoutine");
        printf("🌊🌊🌊🌊🌊🌊\nUccisa la nave %d\n🌊🌊🌊🌊🌊🌊\n", victimIdx);
    }
        
}
void malestormHandler() {
    int pid;
    pid = fork();

    if (pid == -1) {
        perror("fork in launchStorm");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        malestormRoutine();
        exit(EXIT_FAILURE);
    }
}

/*
    TODO: utilizzare nella struct della nave un array [storm,swell] e se uno dei due campi viene settato ad 1
          allora quella nave è stata scelta dal meteo come target

          RICORDARSI DI RISETTRE IL VALORA A 0 DOPO
*/

void stormRoutine(){
    int shmID;
    unsigned int* value;
    int victimIdx;
    shmID = useShm(STORMSWELLSHMKEY, sizeof(unsigned int) * 2, errorHandler, "meteo stormRoutine");
    value = (unsigned int*) getShmAddress(shmID, 0, errorHandler, "getShmAddress meteo stormRoutine");
    victimIdx = random_int(0, SO_NAVI - 1);
    *value = victimIdx;
    shmDetach(value,errorHandler,"stormRoutine");
}

void launchStorm() {
    int pid = fork();
    if(pid == -1){
        perror("fork in launchStorm");
        exit(EXIT_FAILURE);
    } else if(pid == 0){
        stormRoutine();
        exit(EXIT_SUCCESS);
    }
}

void swellRoutine(){
    int shmID;
    unsigned int* value;
    int victimIdx;
    shmID = useShm(STORMSWELLSHMKEY, sizeof(unsigned int) * 2, errorHandler, "meteo stormRoutine");
    value = ((unsigned int*) getShmAddress(shmID, 0, errorHandler, "getShmAddress meteo stormRoutine")) + 1;
    victimIdx = random_int(0, SO_NAVI - 1);
    *value = victimIdx;
    shmDetach(value-1,errorHandler, "swellRoutine");
}

void launchSwell(){
    int pid = fork();
    if(pid == -1){
        perror("fork in launchSwell");
        exit(EXIT_FAILURE);
    } else if(pid == 0){
        swellRoutine();
        exit(EXIT_SUCCESS);
    }
    
}

int main(int argc, char* argv[]) {
    int day;
    int* terminateValue;
    int endShmID;
    int aspettoMortePortiSemID;
    char str[128];
    
    endShmID = useShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "getEndDayShmID meteo");
    terminateValue = (int*) getShmAddress(endShmID, 0, errorHandler, "meteo terminateValue");

    
    aspettoMortePortiSemID = useSem(WAITPORTSSEM, errorHandler, "aspettoMortePortiSemID in codicePorto");
    

    checkInConfig();
    printf("Meteo chekInConfig finita...\n");
    waitForStart();
    printf("Meteo partito...\n");

    malestormHandler();

    while (fgets(str, 128, stdin) != NULL) {
        day = atoi(str);
        printf("Giorno %d\n", day);

        launchStorm();
        launchSwell();

    }
    mutex(aspettoMortePortiSemID, LOCK, errorHandler, "LOCK su aspettoMortePortiSemID");
    printf("Meteo: termino\n");
    exit(EXIT_SUCCESS);
        

}