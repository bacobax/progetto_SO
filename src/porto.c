#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include "../config1.h"
#include "../utils/errorHandler.h"
#include "../utils/msg_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./dump.h"
#include "./porto.h"
#include "./nave.h"
#include "../utils/supplies.h"

void portCode(int endShmId, int idx, int aspettoMortePortiSemID,int aspettoMorteNaviSemID )
{
    int* endNow;
    endNow = (int*)getShmAddress(endShmId, 0, errorHandler, "portCode");
    /*
    launchDischarger(recvDischargeHandler, idx);
    launchCharger(recvChargerHandler, idx);*/
    waitForStart();
  
    

    /* START */
    while(1){



        sleep(1);
        if(*endNow){
            mutex(aspettoMorteNaviSemID, WAITZERO, errorHandler, "waitzero su aspettoMorteNaviSemID");
            printf("[%d] PORTO UCCIDO TUTTI I FIGLI\n", idx);
            kill(0, SIGUSR1);
            mutex(aspettoMortePortiSemID, LOCK, errorHandler, "LOCK su aspettoMortePortiSemID");
            printf("[%d]PORTO TERMINO\n", idx);
            shmDetach(endNow, errorHandler, "endNowDetach porto");
            exit(EXIT_SUCCESS);
        }

    }
    /* launchGoodsDispatcher(myQueueID, porto, idx, shipsQueueID); */
}

int main(int argc, char const *argv[])
{
    int supplyDisponibility;
    int requestDisponibility;
    int idx;

    supplyDisponibility = atoi(argv[1]);
    requestDisponibility = atoi(argv[2]);
    idx = atoi(argv[3]);

    mySettedPort(supplyDisponibility, requestDisponibility, idx, portCode);
}

