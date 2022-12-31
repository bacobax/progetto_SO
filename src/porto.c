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


void recvDischargeHandler(long type, char* text) {

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
    

    Port porto;

    waitResponsesID = useSem(WAITFIRSTRESPONSES, errorHandler, "recvDischargerHandler->waitResponsesID useSem");
    sscanf(text, "%d %d", &quantity, &idNaveMittente);
    idx = type - 1;

    mutexPro(waitResponsesID, idNaveMittente, LOCK, errorHandler, "recvDischargerHandler->waitResponsesID LOCK");
        
    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler, "RecvDischargerHandler");

    porto = ((Port)getShmAddress(portShmId, 0, errorHandler, "RecvDischargerHandler")) + idx;

    keyMiaCoda = ftok("./src/porto.c" , idx);
    keyCodaNave = ftok("./src/nave.c" , idNaveMittente);

    if(keyMiaCoda == -1 || keyCodaNave == -1){
        perror("Errore Coda Nave");
    }

    myQueueID = useQueue(keyMiaCoda, errorHandler, "RecvDischargerHandler->myQueueID");

    shipQueueID = useQueue(keyCodaNave, errorHandler,"RecvDischargerHandler->shipQueueID");
    

    controlPortsDisponibilitySemID = useSem(PSEMVERIFYKEY, errorHandler,"RecvDischargerHandler->controlPortsDisponibilitySemID");

    waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler,"RecvDischargerHandler->waitToTravelSemID");




    
    printf("Port %d: Ricevuto messaggio da nave %d con quantità %d\n", getppid(), idNaveMittente, quantity);


    /* Operazione controllata da semaforo, per permettere di controllare le disponibilità per una richiesta solo quando non lo si sta già facendo per un altra*/
    mutexPro(controlPortsDisponibilitySemID, idx, LOCK, errorHandler, "RecvDischargerHandler->controlPortsDisponibilitySemID LOCK");
    res = trovaTipoEScadenza(&porto->supplies, &tipoTrovato, &dayTrovato, &dataScadenzaTrovata, quantity);
    mutexPro(controlPortsDisponibilitySemID, idx, UNLOCK, errorHandler, "RecvDischargerHandler->controlPortsDisponibilitySemID UNLOCK");
    printf("|Port %d: \n|\tTIPO TROVATO: %d \n|\tEXP TIME TROVATA:%d\n", getppid(), tipoTrovato, dataScadenzaTrovata);

    printf("Port %d: Ho trovato il tipo %d con data di scadenza %d\n", getppid(), tipoTrovato, dataScadenzaTrovata);


    if (res == -1) {
        msgSend(shipQueueID, "x", idx + 1, errorHandler,"risposta negativa recvDischargerHandler");
        printf("✅Port %d: MSGSND NEGATIVA RIUSCITA", getppid(), idNaveMittente, quantity);
        
    }
    else {
        
        sprintf(text, "%d %d", tipoTrovato, dataScadenzaTrovata);
        msgSend(shipQueueID, text, idx + 1, errorHandler,"risposta positiva recvDischargerHandler");
        printf("✅Port %d: MSGSND POSITIVA RIUSCITA", getppid(), idNaveMittente, quantity);
        
    }
    
    printf("\n\n[%d]Porto: RISPOSTA NAVE MANDATA\n\n", getpid());
/*
    mutexPro(waitResponsesID, idNaveMittente, LOCK, NULL);

*/
    
    messaggioRicevuto = msgRecv(myQueueID, idNaveMittente + 1, errorHandler, NULL, SYNC, "recvDischargerHandler->ricezione di sonostatoScelto");

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
    
    mutexPro(waitToTravelSemID, idNaveMittente, LOCK, NULL, "RecvDischargerHandler->waitToTravelSemID LOCK");
    getOneValue(waitToTravelSemID, idNaveMittente);        

}

void recvChargerHandler(long type, char* text) {
    int waitResponsesID;
    int quantity;
    int idNaveMittente;
    int tipoMerceRichiesto;
    int portShmId;
    Port porto;
    int idx;
    int keyMiaCoda; 
    int IDMiaCoda;
    int res;
    int keyCodaNave;
    int shipQueueID;
    int verifyRequestSemID;
    char rtext[MEXBSIZE];
    int sonostatoScelto;
    mex* messaggioRicevuto;
    int waitToTravelSemID;
    idx = (int)type - 1;

    waitResponsesID = useSem(WAITFIRSTRESPONSES, errorHandler, "recvChargerHandler->waitResponsesID");
    sscanf(text, "%d %d %d", &tipoMerceRichiesto, &quantity, &idNaveMittente);
    printf("PORTO %d, ricevuta richiesta di scaricare %d merce di tipo %d dalla nave %d\n", getppid(), quantity, tipoMerceRichiesto, idNaveMittente);

    
    mutexPro(waitResponsesID, idNaveMittente, LOCK, errorHandler,"recvChargerHandler->waitResponsesID LOCK");


    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler, "recvChargerHandler");

    porto = ((Port)getShmAddress(portShmId, 0, errorHandler, "recvChargerHandler")) + idx;

    keyMiaCoda = ftok("./src/porto.h", idx);
    if (keyMiaCoda == -1) {
        perror("Ftok keyMiaCoda");
        exit(EXIT_FAILURE);
    }

    keyCodaNave = ftok("./src/nave.c" , idNaveMittente);

    if(keyCodaNave == -1){
        perror("Errore Coda Nave");
    }

    shipQueueID = useQueue(keyCodaNave, errorHandler,  "recvChargerHandler->shipQueueID");
    

    IDMiaCoda = useQueue(keyMiaCoda, errorHandler,  "recvChargerHandler->IDMiaCoda");

    verifyRequestSemID = useSem(P2SEMVERIFYKEY, errorHandler,  "recvChargerHandler->verifyRequestSemID");

    waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler,"recvChargerHandler->waitToTtravelSemID");
    
    mutexPro(verifyRequestSemID, idx, LOCK, errorHandler, "recvChargerHandler->verifyRequestSemID LOCK");
    res = checkRequests(porto, tipoMerceRichiesto, quantity);
    mutexPro(verifyRequestSemID, idx, UNLOCK, errorHandler, "recvChargerHandler->verifyRequestSemID UNLOCK");

    if (res == -1) {
        printf("INVIO NOPE\n");
        msgSend(shipQueueID, "NOPE", idx + 1, errorHandler, "recvChargerHandler->invio risposta negativa");
    }
    else {
        printf("INVIO %d\n", res);
        
        sprintf(rtext, "%d", res);
        msgSend(shipQueueID, rtext, idx + 1, errorHandler, "recvChargerHandler->invio risposta positiva");
    }
    messaggioRicevuto = msgRecv(IDMiaCoda, idNaveMittente + 1, errorHandler, NULL, SYNC, "recvChargerHandler->ricezione di sonostatoScelto");

    sscanf(messaggioRicevuto->mtext, "%d", &sonostatoScelto);
    printf("Port %d: valore di sonostatoScelto = %d\n", idx, sonostatoScelto);
    if (sonostatoScelto == 0 && res != -1) {
        printf("Porto %d, non sono stato scelto anche se avevo trovato della rob\n", idx);
        porto->requests[tipoMerceRichiesto] = res;
    }


    if (sonostatoScelto == 1 && res == 1) {
        printf("Porto %d: sono stato scelto\n", idx);
        
        /* addNotExpiredGood(0 - quantity, tipoTrovato, PORT); */
    }
    mutexPro(waitToTravelSemID, idNaveMittente, LOCK, errorHandler, "recvChargerHandler->waitToTravelSemID LOCK");
    /*TODO: Fare una recv per sapere se sono stato scelto*/

    return;
}
void codicePorto(Port porto, int myQueueID, int shipsQueueID, int idx) {
    
   
    waitForStart();

    launchDischarger(recvDischargeHandler, idx);
    launchCharger(recvChargerHandler, idx);
    /* START */
    

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


