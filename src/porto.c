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

void recvDischargeHandler(long type, char *text)
{

    int quantity = -2;
    mex *messaggioRicevuto;
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
    Port arrPorts;
    Port porto;

    /*waitResponsesID = useSem(WAITFIRSTRESPONSES, errorHandler, "recvDischargerHandler->waitResponsesID useSem");*/
    sscanf(text, "%d %d", &quantity, &idNaveMittente);
    idx = type - 1;

    /*mutexPro(waitResponsesID, idNaveMittente, LOCK, errorHandler, "recvDischargerHandler->waitResponsesID LOCK");*/

    printf("PORTO %d, ricevuta richiesta di caricare %d quantità merce dalla nave %d\n", idx, quantity, idNaveMittente);

    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler, "RecvDischargerHandler");

    arrPorts = ((Port)getShmAddress(portShmId, 0, errorHandler, "RecvDischargerHandler"));
    porto = arrPorts + idx;

    keyMiaCoda = ftok("./src/porto.c", idx);
    keyCodaNave = ftok("./src/nave.c", idNaveMittente);

    if (keyMiaCoda == -1 || keyCodaNave == -1)
    {
        perror("Errore Coda Nave");
    }

    myQueueID = useQueue(keyMiaCoda, errorHandler, "RecvDischargerHandler->myQueueID");

    shipQueueID = useQueue(keyCodaNave, errorHandler, "RecvDischargerHandler->shipQueueID");

    controlPortsDisponibilitySemID = useSem(PSEMVERIFYKEY, errorHandler, "RecvDischargerHandler->controlPortsDisponibilitySemID");
    waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler, "RecvDischargerHandler->waitToTravelSemID");


    /* Operazione controllata da semaforo, per permettere di controllare le disponibilità per una richiesta solo quando non lo si sta già facendo per un altra*/
    mutexPro(controlPortsDisponibilitySemID, idx, LOCK, errorHandler, "RecvDischargerHandler->controlPortsDisponibilitySemID LOCK");
    res = trovaTipoEScadenza(&porto->supplies, &tipoTrovato, &dayTrovato, &dataScadenzaTrovata, quantity, arrPorts, idx);
    mutexPro(controlPortsDisponibilitySemID, idx, UNLOCK, errorHandler, "RecvDischargerHandler->controlPortsDisponibilitySemID UNLOCK");
    printf("|Port %d: \n|\tTIPO TROVATO: %d \n|\tEXP TIME TROVATA:%d\n\tPER NAVE: %d\n", idx, tipoTrovato, dataScadenzaTrovata, idNaveMittente);


    if (res == -1)
    {
        msgSend(shipQueueID, "x", idx + 1, errorHandler, 0, "risposta negativa recvDischargerHandler");
    }
    else
    {

        sprintf(text, "%d %d %d", tipoTrovato, dataScadenzaTrovata , dayTrovato);
        msgSend(shipQueueID, text, idx + 1, errorHandler, 0, "risposta positiva recvDischargerHandler");
    }

    /*
        mutexPro(waitResponsesID, idNaveMittente, LOCK, NULL);

    */

    messaggioRicevuto = msgRecv(myQueueID, idNaveMittente + 1, errorHandler, NULL, SYNC, "recvDischargerHandler->ricezione di sonostatoScelto");
    sscanf(messaggioRicevuto->mtext, "%d", &sonostatoScelto);
    
    printf("Port %d: valore di sonostatoScelto = %d\n dalla nave %d\n", idx, sonostatoScelto, idNaveMittente);
    if (sonostatoScelto == 0 && res != -1)
    {
        printf("Porto %d, non sono stato scelto anche se avevo trovato della rob\n", idx);
        porto->supplies.magazine[dayTrovato][tipoTrovato] += quantity;
        printf("PORTO %d: riaggiungo %d\n" , idx, quantity);
        
        addNotExpiredGood(quantity, tipoTrovato, PORT, 0, idx);
        /*
            SE LA MERCE E' SCADUTA MENTRE IL PORTO ASPETTAVA DI SAPERE SE E' STATO SCELTO
            SE LA MERCE FOSSE SCADUTA PRIMA IL PROBLEMA NON ESISTEREBBE
        */
        if(getExpirationTime(porto->supplies,tipoTrovato, dayTrovato)== 0){
            addExpiredGood(quantity, tipoTrovato, PORT);
        }
    }

    if (sonostatoScelto == 1 && res == 1)
    {
        printf("Porto %d: sono stato scelto\n", idx);

        /* addNotExpiredGood(0 - quantity, tipoTrovato, PORT); */
    }

    mutexPro(waitToTravelSemID, idNaveMittente, LOCK, NULL, "RecvDischargerHandler->waitToTravelSemID LOCK");
    /*getOneValue(waitToTravelSemID, idNaveMittente);*/
    shmDetach(arrPorts, errorHandler, "recvDischargerHandler");
    return;
}

void recvChargerHandler(long type, char *text)
{
    int quantity;
    int idNaveMittente;
    int tipoMerceRichiesto;
    int portShmId;
    Port porto;
    int keyMiaCoda;
    int IDMiaCoda;
    int res;
    int keyCodaNave;
    int shipQueueID;
    int verifyRequestSemID;
    char rtext[MEXBSIZE];
    char payload[MEXBSIZE];
    int richiestaScarico;
    int sonostatoScelto;
    mex *messaggioRicevuto;
    int waitToTravelSemID;
    int idx;
    idx = type - 1;


    sscanf(text, "%d %d %d", &tipoMerceRichiesto, &quantity, &idNaveMittente);

    printf("PORTO %d, ricevuta richiesta di scaricare %d merce di tipo %d dalla nave %d\n", idx, quantity, tipoMerceRichiesto, idNaveMittente);

    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler, "recvChargerHandler");

    porto = ((Port)getShmAddress(portShmId, 0, errorHandler, "recvChargerHandler")) + idx;

    keyMiaCoda = ftok("./src/porto.h", idx);
    if (keyMiaCoda == -1)
    {
        perror("Ftok keyMiaCoda");
        exit(EXIT_FAILURE);
    }

    keyCodaNave = ftok("./src/nave.c", idNaveMittente);

    if (keyCodaNave == -1)
    {
        perror("Errore Coda Nave");
    }

    shipQueueID = useQueue(keyCodaNave, errorHandler, "recvChargerHandler->shipQueueID");

    IDMiaCoda = useQueue(keyMiaCoda, errorHandler, "recvChargerHandler->IDMiaCoda");

    verifyRequestSemID = useSem(P2SEMVERIFYKEY, errorHandler, "recvChargerHandler->verifyRequestSemID");

    waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler, "recvChargerHandler->waitToTtravelSemID");

    mutexPro(verifyRequestSemID, idx, LOCK, errorHandler, "recvChargerHandler->verifyRequestSemID LOCK");
    res = checkRequests(porto, tipoMerceRichiesto, quantity);
    mutexPro(verifyRequestSemID, idx, UNLOCK, errorHandler, "recvChargerHandler->verifyRequestSemID UNLOCK");

    if (res == -1)
    {
        msgSend(shipQueueID, "NOPE", idx + 1, errorHandler, 0, "recvChargerHandler->invio risposta negativa");
    }
    else
    {

        sprintf(rtext, "%d", res);
        msgSend(shipQueueID, rtext, idx + 1, errorHandler, 0, "recvChargerHandler->invio risposta positiva");
    }

    messaggioRicevuto = msgRecv(IDMiaCoda, idNaveMittente + 1, errorHandler, NULL, SYNC, "recvChargerHandler->ricezione risposta");
    sscanf(messaggioRicevuto->mtext, "%d", &sonostatoScelto);

    printf("Port %d: valore di sonostatoScelto = %d dalla nave %d\n", idx, sonostatoScelto, idNaveMittente);
    if (sonostatoScelto == 0 && res != -1){
        printf("Porto %d, non sono stato scelto anche se avevo trovato della rob\n", idx);
        porto->requests[tipoMerceRichiesto] = res;
    }

    if (sonostatoScelto == 1 && res == 1)
    {
        printf("Porto %d: sono stato scelto\n", idx);
    }
    mutexPro(waitToTravelSemID, idNaveMittente, LOCK, errorHandler, "recvChargerHandler->waitToTravelSemID LOCK");
    shmDetach(porto - idx, errorHandler, "recvChargerHandler");
    return;
}

void codicePorto(int endShmId, int idx)
{
    int* endNow;
    int aspettoMortePortiSemID;
    int aspettoMorteNaviSemID;
    waitForStart();
    endNow =(int*)getShmAddress(endShmId, 0, errorHandler, "codicePorto");
    aspettoMortePortiSemID = useSem(WAITPORTSSEM, errorHandler, "aspettoMortePortiSemID in codicePorto");
    aspettoMorteNaviSemID = useSem(WAITSHIPSSEM, errorHandler, "aspettoMortePortiSemID in codicePorto");
    launchDischarger(recvDischargeHandler, idx);
    launchCharger(recvChargerHandler, idx);

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

    mySettedPort(supplyDisponibility, requestDisponibility, idx, codicePorto);
}

