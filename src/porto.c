#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include "../config1.h"
#include "../utils/msg_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./dump.h"
#include "./porto.h"
#include "./nave.h"
#include "../utils/supplies.h"


void recvHandler(long type, char* text) {

            int quantity = -2;
            mex* messaggioRicevuto;
            int res;
            int tipoTrovato;
            int dayTrovato;
            int dataScadenzaTrovata;
            int sonostatoScelto;
            int idx;
            int idNaveMittente; 
            int portShmId;
            int myQueueID;
            int shipQueueID;
            int controlPortsDisponibilitySemID;
            int semid;
            int waitToTravelSemID;
            int keyMiaCoda;
            int keyCodaNave;
            int waitResponsesID;
            // semid = useSem(RESPRINTKEY, errorHandler);
            // mutex(semid, LOCK, NULL);

            Port porto;
            
            sscanf(text, "%d %d", &quantity, &idNaveMittente);
            idx = type - 1;
            
            portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);

            porto = ((Port)getShmAddress(portShmId, 0, errorHandler)) + idx;

            keyMiaCoda = ftok("./src/porto.c" , idx);
            keyCodaNave = ftok("./src/nave.c" , idNaveMittente);

            if(keyMiaCoda == -1 || keyCodaNave == -1){
                perror("Errore Coda Nave");
            }

            myQueueID = useQueue(keyMiaCoda, errorHandler);

            shipQueueID = useQueue(keyCodaNave, errorHandler);
            // printf("[%d]Porto - myQueueKEY:%d shipQueueKEY:%d\n", getpid(),keyMiaCoda, keyCodaNave);

            // printf("[%d]Porto - myQueueID:%d shipQueueID:%d\n", getpid(),myQueueID, shipQueueID);

            controlPortsDisponibilitySemID = useSem(PSEMVERIFYKEY, errorHandler);

            waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler);

            waitResponsesID = useSem(WAITFIRSTRESPONSES, errorHandler);
            
            printf("Port %d: Ricevuto messaggio da nave %d con quantità %d\n", getppid(), idNaveMittente, quantity);


            /* Operazione controllata da semaforo, per permettere di controllare le disponibilità per una richiesta solo quando non lo si sta già facendo per un altra*/
            mutexPro(controlPortsDisponibilitySemID, idx, LOCK, NULL);
            res = trovaTipoEScadenza(&porto->supplies, &tipoTrovato, &dayTrovato, &dataScadenzaTrovata, quantity);
            mutexPro(controlPortsDisponibilitySemID, idx, UNLOCK, NULL);

            printf("Port %d: Ho trovato il tipo %d con data di scadenza %d\n", getppid(), tipoTrovato, dataScadenzaTrovata);
            // mutex(semid, UNLOCK, NULL);


            if (res == -1) {
                msgSend(shipQueueID, "x", idx + 1, errorHandler);
            }
            else {
                
                sprintf(text, "%d %d", tipoTrovato, dataScadenzaTrovata);
                msgSend(shipQueueID, text, idx + 1, errorHandler);
            }
            mutexPro(waitResponsesID, idNaveMittente, LOCK, NULL);
            
            messaggioRicevuto = msgRecv(myQueueID, idNaveMittente + 1, errorHandler, NULL, SYNC);

            sscanf(messaggioRicevuto->mtext, "%d", &sonostatoScelto);
            printf("Port %d: valore di sonostatoScelto = %d\n", idx, sonostatoScelto);
            if (sonostatoScelto == 0 && res != -1) {
                printf("Porto %d, non sono stato scelto anche se avevo trovato della rob\n", idx);
                porto->supplies.magazine[dayTrovato][tipoTrovato] += quantity;
                addNotExpiredGood(quantity, tipoTrovato, PORT);
            }


            if (sonostatoScelto == 1 && res == 1) {
                printf("Porto %d: sono stato scelto\n", idx);
                
                /* addNotExpiredGood(0 - quantity, tipoTrovato, PORT); */
            }

            mutexPro(waitToTravelSemID, idNaveMittente, LOCK, NULL);

}


void codicePorto(Port porto, int myQueueID, int shipsQueueID, int idx) {
    
    int requestPortQueueID;
    
    requestPortQueueID = useQueue(PQUEREQCHKEY, errorHandler);
    waitForStart();

    /* START */
     while (1) {

        /*
            E' importante che sia sincrona la gestione del messaggio ricevuto
            perchè prima di poterne ricevere un altro il porto deve poter aver aggiornato le sue disponibilità
        */

        /*
            prendo il primo messaggio che arriva
        */
         msgRecv(requestPortQueueID, idx+1, errorHandler, recvHandler, ASYNC);
         

         
    }

    /* launchGoodsDispatcher(myQueueID, porto, idx, shipsQueueID); */


}

int main(int argc, char const* argv[]) {
    int supplyDisponibility;
    int requestDisponibility;
    int idx;

    supplyDisponibility = atoi(argv[1]);
    requestDisponibility = atoi(argv[2]);
    idx = atoi(argv[3]);

    mySettedPort(supplyDisponibility, requestDisponibility, idx, codicePorto);
    

}


