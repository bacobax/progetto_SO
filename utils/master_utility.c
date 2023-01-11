#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include "../src/porto.h"
#include "../src/nave.h"
#include "../src/dump.h"
#include "../config1.h"
#include "./errorHandler.h"
#include "./vettoriInt.h"
#include "./support.h"
#include "./shm_utility.h"
#include "./sem_utility.h"
#include "./msg_utility.h"
#include "./supplies.h"


void genera_navi() {
    int i;
    int pid;
    char* argv[3];
    for (i = 0; i < SO_NAVI; i++) {  /* provo a creare due navi*/
        pid = fork();
        if (pid == 0) {

            char s[50];
            sprintf(s, "%d", i);
            argv[0] = "nave";
            argv[1] = s;
            argv[2] = NULL;
        
            execve("./bin/nave", argv, NULL);

            exit(EXIT_FAILURE);
        }
        else if (pid == -1) {
            throwError("fork", "generaNavi");
            exit(EXIT_FAILURE);
        }
    }
}

void genera_porti(int risorse, int n_porti) {

    intList* quantiesSupplies;
    intList* quantiesRequests;
    
    int i;
    int pid;
    int* quantity;/*Solo perchè elementAt ritorna un puntatore a intero (perchè almeno in caso di errore ritorna NULL)*/
    char strQuantitySupply[50];
    char strQuantityRequest[50];
    char strIdx[50];
    char* temp[5];
    quantiesSupplies = distribute(risorse, n_porti);
    quantiesRequests = distribute(SO_FILL, n_porti);
    
    for (i = 0; i < n_porti; i++) {
        pid = fork();
        if (pid == 0) {
            printf("PORTO %d: %d\n",i, getpid());
            quantity = (int*)malloc(sizeof(int));
            
            quantity = intElementAt(quantiesSupplies, i);
            sprintf(strQuantitySupply, "%d", *quantity);
            
            quantity = intElementAt(quantiesRequests, i);
            sprintf(strQuantityRequest, "%d", *quantity);
            
            free(quantity);


            sprintf(strIdx, "%d", i);

            temp[0] = "porto";
            temp[1] = strQuantitySupply;
            temp[2] = strQuantityRequest;
            temp[3] = strIdx;
            temp[4] = NULL;

            execve("./bin/porto", temp, NULL);

            throwError("execve" , "generaPorti");
            exit(EXIT_FAILURE);
        }
        else if (pid == -1) {
            throwError("fork", "generaPorti");
            exit(EXIT_FAILURE);
        }
        printf("Generato porto %d\n", i);
    }
    printf("M: libero la lista\n"); /*! da fixare, come mai non la stampa??? */
    intFreeList(quantiesSupplies);

}

void wait_all(int n_px) {
    int pid;
    int i;
    for (i = 0; i < n_px; i++) {
        pid = wait(NULL);
    }
}


void mastersighandler(int s) {
    printf("Master kill signal però non faccio nulla\n");
    return;
}

void aspettaConfigs(int waitConfigSemID) {
    mutex(waitConfigSemID, WAITZERO, errorHandler, "aspettaConfigs");
}


void creaCodePorti() {
    int i;
    int msgQueue;

    for (i = 0; i < SO_PORTI; i++) {
        msgQueue = createQueue(ftok("./src/porto.c" , i), errorHandler, "creaCodePorti");
        printf("msgQueue id:%d port:%d\n", msgQueue, i);
    }
}
void creaCodePortiDischarge() {
     int i;
    int msgQueue;

    for (i = 0; i < SO_PORTI; i++) {
        msgQueue = createQueue(ftok("./src/porto.h" , i), errorHandler, "creaCodePortiDischarge");
        printf("msgQueue id:%d port:%d\n", msgQueue, i);
    }
}


void creaCodeNavi() {
    int i;
    int msgQueue;

    for (i = 0; i < SO_NAVI; i++) {
        msgQueue = createQueue(ftok("./src/nave.c" , i), errorHandler , "creaCodeNavi");
        printf("msgQueue id:%d nave:%d\n", msgQueue, i);
    }
}


void distruggiCodePorti() {
    int i;
    int msgQueue;
    for (i = 0; i < SO_PORTI; i++) {
        msgQueue = useQueue(ftok("./src/porto.c" , i), errorHandler , "distruggiCodePorti");
        removeQueue(msgQueue, errorHandler , "distruggiCodePorti");
    }
}
void distruggiCodePortiDischarge() {
    int i;
    int msgQueue;
    for (i = 0; i < SO_PORTI; i++) {
        msgQueue = useQueue(ftok("./src/porto.h" , i), errorHandler, "distruggiCodePortiDischarge");
        removeQueue(msgQueue, errorHandler, "distruggiCodePortiDischarge");
    }
}


void distruggiCodeNavi() {
    int i;
    int msgQueue;
    
    for (i = 0; i < SO_NAVI; i++) {
        msgQueue = useQueue(ftok("./src/nave.c", i), errorHandler, "distruggiCodeNavi");
        
        removeQueue(msgQueue, errorHandler, "distruggiCodeNavi");
    }
}

void mySettedMain(void (*codiceMaster)(int startSimulationSemID, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID, int msgRefillerID, int waitEndDaySemID, int* day, int waitEndDayShipsSemID)) {
    int startSimulationSemID;
    int reservePrintSem;
    int reservePortsResourceSem;
    int portsShmid;
    int shipsShmid;
    int endShmID;
    int dayShmID;
    int semBanchineID;
    int semShipsID;
    int msgRefillerID;
    int waitconfigSemID;
    int rwExpTimesPortSemID;
    int waitEndDaySemID;
    int portRequestsQueueID;
    int controlPortsDisponibilitySemID;
    int waitToTravelsemID;
    int waitResponsesID;
    int portsDischargeQueue;
    int verifyRequestPortSemID;
    int portDischargeRequestsQueueID;
    int waitToRemoveDump;
    int i;
    unsigned int* terminateValue;
    int* day;
    int waitPortsSemID;
    int waitShipsSemID;
    int waitEndDayShipSemID;

    struct sigaction new_sig_action;
    sigset_t new_sig_set;

    sigemptyset(&new_sig_set);
    sigaddset(&new_sig_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &new_sig_set, NULL);
    srand(time(NULL));

   
    initErrorHandler();

    startSimulationSemID = createSem(MASTKEY, 1, errorHandler, "creazione sem startSimulationSemID");
    reservePrintSem = createSem(RESPRINTKEY, 1, errorHandler, "creazione sem reservePrintSem");
    controlPortsDisponibilitySemID = createMultipleSem(PSEMVERIFYKEY, SO_PORTI, 1, errorHandler, "creazione sem per controllare i supplies in ordine");
    /*
    !dovrà essere SO_PORTI + SO_NAVI
    */

   waitPortsSemID = createSem(WAITPORTSSEM, SO_PORTI + 1, errorHandler, "master crazione waitPortsSemID");
   waitShipsSemID = createSem(WAITSHIPSSEM, SO_NAVI, errorHandler, "master creazione waitShipsSemID");

    waitEndDayShipSemID = createSem(WAITENDDAYSHIPSEM, 0, errorHandler, "master create waitEndDayShipSem");

    waitconfigSemID = createSem(WAITCONFIGKEY, SO_PORTI+ SO_NAVI+ 1, errorHandler, "creazione sem waitconfig");

    
    createDumpArea();
    portsShmid = createShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler , "creazione shm dei porti");
    shipsShmid = createShm(SSHMKEY, SO_NAVI * sizeof(struct ship), errorHandler ,"creazione shm delle navi");
    endShmID = createShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "crazione shm intero terminazione programma");
    terminateValue = (unsigned int*)getShmAddress(endShmID, 0, errorHandler, "master getShmAddress endShm");

    dayShmID = createShm(DAYWORLDSHM, sizeof(int), errorHandler, "crazione day shm master");
    day = (int*) getShmAddress(dayShmID, 0, errorHandler, "master getShmAddress day");


    /*creazione banchine*/
    semBanchineID = createMultipleSem(BANCHINESEMKY, SO_PORTI, SO_BANCHINE, errorHandler, "creazione semaforo banchine");

    if (portsShmid == EEXIST || shipsShmid == EEXIST) {
        throwError("Le shm esistono già\n" , "mySettedMain");
        exit(EXIT_FAILURE);
    }
    /*il codice del master manco la usa*/
    msgRefillerID = createQueue(REFILLERQUEUE, errorHandler, "creazione coda del refiller");

    /* msgShipQueueID = createQueue(SCHQUEUEKEY, errorHandler); */

    /* creare queue navi per fase di scaricamento TO-DO*/

    reservePortsResourceSem = createMultipleSem(RESPORTSBUFFERS, SO_PORTI, 1, errorHandler, "creazione sem per scrivere nei supplies di un porto");

    rwExpTimesPortSemID = createMultipleSem(WREXPTIMESSEM, SO_PORTI, 1, errorHandler, "sem per leggere e scrivere negli exp time di un porto");

    semShipsID = createMultipleSem(SEMSHIPKEY, SO_NAVI, 1, errorHandler, "creazione sem per modificare carico di una nave");

    /*
        semaforo che serve al master per eseguirne la waitzero alla fine di ogni giorno
        in sintesi il master aspetta a passare il giorno finchè tutti i porti non hanno ricevuto la loro merce
    */
    waitEndDaySemID = createSem(WAITENDDAYKEY, SO_PORTI, errorHandler, "creazione sem waitEndDaySem");
    portRequestsQueueID = createQueue(PQUERECHKEY, errorHandler, "creazione della coda delle richieste di carico dei porti");
    portDischargeRequestsQueueID = createQueue(PQUEREDCHKEY, errorHandler, "creazione della coda delle richieste di scarico dei porti");
    
    /* TO-DO creare coda porti richieste per fase di scaricamento*/

    creaCodePorti();
    creaCodePortiDischarge();
    creaCodeNavi();
    /* TO-DO creare code porti per fase di scaricamento*/

    waitToTravelsemID = createMultipleSem(WAITTOTRAVELKEY, SO_NAVI, SO_PORTI, errorHandler , "creazione waitToTravelSem");
    getAllVAlues(waitToTravelsemID, SO_NAVI);
    
    waitResponsesID = createMultipleSem(WAITFIRSTRESPONSES, SO_NAVI, 1, errorHandler, "creazione waitResponsesSem");

    verifyRequestPortSemID = createMultipleSem(P2SEMVERIFYKEY, SO_PORTI*SO_MERCI, 1, errorHandler, "creazione verifyRequestPortSemID");
    
    waitToRemoveDump = createSem(WAITRMVDUMPKEY, 1, errorHandler, "craezione semaforo remove dump");


    codiceMaster(startSimulationSemID, portsShmid, shipsShmid, reservePrintSem, waitconfigSemID, msgRefillerID, waitEndDaySemID, day, waitEndDayShipSemID);
    /* kill(0, SIGUSR1);  uccide tutti i figli */
    printf("SETTATO A 1 TERMINATE VALUE, ASPETTO FIGLI...\n");

    *terminateValue = 1;
    /*
    wait_all(SO_NAVI + SO_PORTI + (SO_PORTI * 3));
    */
    mutex(waitPortsSemID, WAITZERO, errorHandler, "master mutex WAITZERO on ports");
    printf("FACCIO IL PRINT DEL DUMP DEL %d ESIMO GIORNO\n", SO_DAYS);
    printDump(ASYNC , *day, 1);
    printf("MASTER: FACCIO LA WAITZERO...\n");
    mutex(waitToRemoveDump, WAITZERO, errorHandler, "mutex waitzero remove dump");
    printf("MASTER: HO PASSATO LA WAITZERO...\n");
    lockAllGoodsDump();
    printf("master sono ancora vivo dopo kill\n");
    removeSem(startSimulationSemID, errorHandler, "startSimulationSemID");
    removeSem(reservePrintSem, errorHandler, "reservePrintSem");
    removeSem(semBanchineID, errorHandler, "semBanchineID");
    removeSem(reservePortsResourceSem, errorHandler, "reservePortsResourceSem");
    removeSem(waitconfigSemID, errorHandler, "waitconfigSemID");
    removeSem(rwExpTimesPortSemID, errorHandler , "rwExpTimesPortSemID");
    removeSem(semShipsID, errorHandler, "semShipsID");
    removeSem(waitEndDaySemID, errorHandler, "waitEndDaySemID");
    removeSem(controlPortsDisponibilitySemID, errorHandler, "controlPortsDisponibilitySemID");
    removeSem(verifyRequestPortSemID, errorHandler, "verifyRequestPortSemID");
    removeSem(waitToRemoveDump, errorHandler, "waitToRemoveDump");
    removeSem(waitToTravelsemID, errorHandler, "waitToTravelsemID");
    removeSem(waitResponsesID, errorHandler, "waitResponsesID");

    removeSem(waitPortsSemID, errorHandler, "waitPortsSemID");
    removeSem(waitShipsSemID, errorHandler, "waitShipsSemID");
    removeSem(waitEndDayShipSemID, errorHandler, "waitEndDayShipSemID");
    printf("master tutti i sem sono stati rimoessi\n");

    removeShm(shipsShmid, errorHandler, "shipsShmid");
    removeShm(portsShmid, errorHandler, "portsShmid");
    shmDetach(terminateValue, errorHandler, "master terminateValue detach");
    shmDetach(day, errorHandler, "master day detach");
    removeShm(endShmID, errorHandler, "endShmID");
    removeShm(dayShmID, errorHandler, "daySmID");
    removeDumpArea();
    printf("master tutte le shm sono state rimosse\n");

    removeQueue(msgRefillerID, errorHandler, "msgRefillerID");
    printf("coda di refiller rimossa\n");

    removeQueue(portDischargeRequestsQueueID, errorHandler , "portDischargeRequestsQueueID");
    removeQueue(portRequestsQueueID, errorHandler, "portRequestsQueueID");
    distruggiCodePorti();
    distruggiCodePortiDischarge();
    printf("coda dei porti rimossa\n");
    
    distruggiCodeNavi();
    printf("coda delle navi rimossa\n");

    printf("master tutte le code sono state rimosse\n");
    
    printf("Master, ho rimosso tutto\n");
    removeErrorHandler();
}

void refillCode(intList* l, int msgRefillerID, int giorno) {
    int i;
    long type;
    char supportText[MEXBSIZE];
    for (i = 0; i < l->length; i++) {
        sprintf(supportText, "%d|%d", giorno, *(intElementAt(l, i)));
        type = i + 1;
        /*
        printf("Invio messaggio alla coda %d con il seguente testo: %s con tipo %ld\n", msgRefillerID, supportText, type);

        */
        /* Invio messaggio alla coda 458752 con il seguente testo: 0|20 con tipo 0 */
        msgSend(msgRefillerID, supportText, type, errorHandler,0, "refillCode");
    }

}

void refillPorts(int opt, int msgRefillerID, int quantitaAlGiorno, int giorno) {
    intList* l;
    
    int pid;
    l = distribute(quantitaAlGiorno, SO_PORTI);
    if (opt == SYNC) {
        refillCode(l, msgRefillerID, giorno);
        intFreeList(l);
    }
    else if (opt == ASYNC) {
        pid = fork();
        if (pid == -1) {
            throwError("Errore nella fork in refillPorts\n","refillPorts");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            refillCode(l, msgRefillerID, giorno);
            intFreeList(l);
            exit(EXIT_SUCCESS);
        }
        else {
            /*
                Al padre non servono
            */
            intFreeList(l);
        }
    }
    else {
        throwError("refillPorts: Inserire SYNC o ASYNC (0 o 1) come opt\n","refillPorts");
        exit(EXIT_FAILURE);

    }

}

void childExpirePortCode(Port p, int day, int idx) {
    int rwExpTimesPortSemID;
    int portBufferSemID;
    rwExpTimesPortSemID = useSem(WREXPTIMESSEM, errorHandler, "useSem rwExpTimesPortSemID in childExpirePortCode");
    portBufferSemID = useSem(RESPORTSBUFFERS, errorHandler, "useSem portBufferSemID in childExpirePortCode");
    
    mutexPro(rwExpTimesPortSemID, idx, LOCK, errorHandler , "rwExpTimesPortSemID LOCK in childExpirePortCode");
    
    decrementExpTimes(&p->supplies, day);
    
    mutexPro(rwExpTimesPortSemID, idx, UNLOCK, errorHandler, "rwExpTimesPortSemID UNLOCK in childExpirePortCode");

    mutexPro(portBufferSemID, idx, LOCK, errorHandler, "portBufferSemID LOCK in childExpirePortCode");
    
    removeExpiredGoods(&p->supplies);
    mutexPro(portBufferSemID, idx, UNLOCK, errorHandler, "portBufferSemID UNLOCK in childExpirePortCode");
    

}

void childExpireShipCode(Ship ship){
    int semShipID;
    semShipID = useSem(SEMSHIPKEY, errorHandler, "childExpireShipCode");
    
    mutexPro(semShipID, ship->shipID, LOCK, errorHandler, "childExpireShipCode LOCK");
    
    if (ship->dead) {
        mutexPro(semShipID, ship->shipID, UNLOCK, errorHandler, "childExpireShipCode UNLOCK");    
        exit(EXIT_SUCCESS);
    }
    
    updateExpTimeShip(ship);

    mutexPro(semShipID, ship->shipID, UNLOCK, errorHandler, "childExpireShipCode UNLOCK");    
}

void expirePortsGoods(int day) {
    int portShmID;
    int i;
    Port ports;
    int pid;
    portShmID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler, "expirePortsGoods");
    ports = getShmAddress(portShmID, 0, errorHandler, "expirePortsGoods");
    for (i = 0; i < SO_PORTI; i++) {
        pid = fork();
        if (pid == -1) {
            throwError("fork nel gestore delle risorse","expirePortsGoods");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            childExpirePortCode(ports + i, day, i);
            exit(EXIT_SUCCESS);
        }
    }
    shmDetach(ports,errorHandler, "expirePortsGoods");
}

void expireShipGoods(){
    int shipShmID;
    int i;
    Ship ships;
    int pid;
    shipShmID = useShm(SSHMKEY, sizeof(struct ship) * SO_NAVI, errorHandler,"expireShipGoods");
    for(i=0; i<SO_NAVI; i++) {
        pid = fork();
        if(pid == -1){
            throwError("fork nel gestore risorse","expireShipGoods");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            
            ships = (Ship)getShmAddress(shipShmID, 0, errorHandler, "expireShipGoods");
            childExpireShipCode(ships + i);
            exit(EXIT_SUCCESS);
        }
    }
}


FILE* genera_meteo() {
    int pid;
    FILE* fp;

    return popen("./bin/meteo", "w");
    

}

int countAliveShips(){
    int waitShipSemID = useSem(WAITSHIPSSEM, errorHandler, "nave waitShipSemID");   
    return getOneValue(waitShipSemID, 0);
}

void resetWeatherTargets(Port arrPort, Ship arrShip){
    int i;
    for(i = 0; i< SO_NAVI; i++){
        if(arrShip[i].weatherTarget == 1) {
            arrShip[i].weatherTarget = 0;
        }
    }

    for(i = 0; i< SO_PORTI; i++){
        if(arrPort[i].weatherTarget == 1){
            arrPort[i].weatherTarget = 0;
        }
    }
}

