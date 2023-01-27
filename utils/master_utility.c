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


void create_ships() {
    int i;
    int pid;
    char* argv[3];
    FILE* fp;
    int so_navi = SO_("NAVI");
    for (i = 0; i < so_navi; i++) {  /* provo a creare due navi*/
        pid = fork();
        if (pid == 0) {

            char s[50];
            sprintf(s, "%d", i);
            argv[0] = "nave";
            argv[1] = s;
            argv[2] = NULL;
            fp = fopen("./logs/exitShipLog.log", "a+");
            fprintf(fp,"[%d]Nave IDX %d: creata\n",getpid(),i);
            fclose(fp);

            execve("./bin/nave", argv, NULL);

            exit(EXIT_FAILURE);
        }
        else if (pid == -1) {
            throwError("fork", "generaNavi");
            exit(EXIT_FAILURE);
        }
    }
}

void create_ports(int risorse, int n_porti) {

    intList* quantiesSupplies;
    intList* quantiesRequests;
    int so_fill = SO_("FILL");
    
    int i;
    int pid;
    int* quantity;/*Solo perchè elementAt ritorna un puntatore a intero (perchè almeno in caso di errore ritorna NULL)*/
    char strQuantitySupply[50];
    char strQuantityRequest[50];
    char strIdx[50];
    char* temp[5];
    quantiesSupplies = distributeV1(risorse, n_porti);
    quantiesRequests = distributeV1(so_fill, n_porti);
    
    for (i = 0; i < n_porti; i++) {
        pid = fork();
        if (pid == 0) {
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
    intFreeList(quantiesSupplies);

}

void wait_all(int n_px) {
    int pid;
    int i;
    for (i = 0; i < n_px; i++) {
        pid = wait(NULL);
    }
}




void aspettaConfigs(int waitConfigSemID) {
    mutex(waitConfigSemID, WAITZERO, errorHandler, "aspettaConfigs");
}

void creaShmPorti(){
    int shmid;
    int i;
    char text[512];
    int ftok_val;
    int so_porti = SO_("PORTI");

    shmid = createShm(PSHMKEY, sizeof(struct port) * so_porti, errorHandler, "creaShmPorti");

}

void distruggiShmPorti(){
    int shmid;
    int i;
    int so_porti = SO_("PORTI");
    Port portArr = getPort(0);
    for(i=0; i<so_porti; i++){
        removeShm(portArr[i].requestsID, errorHandler, "distruggi shmd porti request");
        removeShm(portArr[i].supplies.magazineID, errorHandler, "distruggi shmd porti magazine");
        removeShm(portArr[i].supplies.expirationTimesID, errorHandler, "distruggi shmd porti expTimes");
    }
    detachPort(portArr,0);
    removeShm(useShm(PSHMKEY, sizeof(struct port) * so_porti, errorHandler, "distruggiShmPorti"), errorHandler, "distruggiShmPorti");
}

void mySettedMain(void (*masterCode)(int startSimulationSemID, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID, int msgRefillerID, int waitEndDaySemID, int* day, int waitEndDayShipsSemID)) {
    int startSimulationSemID, reservePrintSem, reservePortsResourceSem, portsShmid, shipsShmid, endShmID, dayShmID, pierSemID, semShipsID, verifyAllPortsSemID;
    int msgRefillerID, waitconfigSemID, rwExpTimesPortSemID, waitEndDaySemID, controlPortsDisponibilitySemID, waitResponsesID;   
    int verifyRequestPortSemID, waitToRemoveDump, i, waitPortsSemID, waitShipsSemID, waitEndDayShipSemID, so_porti, so_navi, so_banchine, so_merci;
    unsigned int* terminateValue;

    int* day;
    struct sigaction new_sig_action;
    sigset_t new_sig_set;

    so_porti = SO_("PORTI");
    so_navi = SO_("NAVI");
    so_banchine = SO_("BANCHINE");
    so_merci = SO_("MERCI");
    signal(SIGCHLD, SIG_IGN);
    sigemptyset(&new_sig_set);
    sigaddset(&new_sig_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &new_sig_set, NULL);
    srand(time(NULL));


   
    initErrorHandler();

    startSimulationSemID = createSem(MASTKEY, 1, errorHandler, "creazione sem startSimulationSemID");
    reservePrintSem = createSem(RESPRINTKEY, 1, errorHandler, "creazione sem reservePrintSem");
    

    waitPortsSemID = createSem(WAITPORTSSEM, so_porti + 1, errorHandler, "master crazione waitPortsSemID");
    waitShipsSemID = createSem(WAITSHIPSSEM, so_navi, errorHandler, "master creazione waitShipsSemID");

    waitEndDayShipSemID = createSem(WAITENDDAYSHIPSEM, 0, errorHandler, "master create waitEndDayShipSem");

    waitconfigSemID = createSem(WAITCONFIGKEY, so_porti+ so_navi+ 1, errorHandler, "creazione sem waitconfig");

    
    createDumpArea();
    
    creaShmPorti();
    shipsShmid = createShm(SSHMKEY, so_navi * sizeof(struct ship), errorHandler, "creazione shm delle navi");
    endShmID = createShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "crazione shm intero terminazione programma");
    terminateValue = (unsigned int*)getShmAddress(endShmID, 0, errorHandler, "master getShmAddress endShm");

    dayShmID = createShm(DAYWORLDSHM, sizeof(int), errorHandler, "crazione day shm master");
    day = (int*) getShmAddress(dayShmID, 0, errorHandler, "master getShmAddress day");


    /*creazione banchine*/
    pierSemID = createMultipleSem(BANCHINESEMKY, so_porti, so_banchine, errorHandler, "creazione semaforo banchine");

    if (portsShmid == EEXIST || shipsShmid == EEXIST) {
        throwError("Le shm esistono già\n" , "mySettedMain");
        exit(EXIT_FAILURE);
    }
    /*il codice del master manco la usa*/
    msgRefillerID = createQueue(REFILLERQUEUE, errorHandler, "creazione coda del refiller");

    reservePortsResourceSem = createMultipleSem(RESPORTSBUFFERS, so_porti, 1, errorHandler, "creazione sem per scrivere nei supplies di un porto");

    rwExpTimesPortSemID = createMultipleSem(WREXPTIMESSEM, so_porti, 1, errorHandler, "sem per leggere e scrivere negli exp time di un porto");

    semShipsID = createMultipleSem(SEMSHIPKEY, so_navi, 1, errorHandler, "creazione sem per modificare carico di una nave");

    /*
        semaforo che serve al master per eseguirne la waitzero alla fine di ogni giorno
        in sintesi il master aspetta a passare il giorno finchè tutti i porti non hanno ricevuto la loro merce
    */
    waitEndDaySemID = createSem(WAITENDDAYKEY, so_porti, errorHandler, "creazione sem waitEndDaySem");
    
    
    waitResponsesID = createMultipleSem(WAITFIRSTRESPONSES, so_navi, 1, errorHandler, "creazione waitResponsesSem");

    verifyRequestPortSemID = createMultipleSem(P2SEMVERIFYKEY, so_porti*so_merci, 1, errorHandler, "creazione verifyRequestPortSemID");
    
    waitToRemoveDump = createSem(WAITRMVDUMPKEY, 1, errorHandler, "craezione semaforo remove dump");

    verifyAllPortsSemID = createSem(VERIFYALLPORTS,1,errorHandler,"creazioen verifyAllPortsSemID");

    masterCode(startSimulationSemID, portsShmid, shipsShmid, reservePrintSem, waitconfigSemID, msgRefillerID, waitEndDaySemID, day, waitEndDayShipSemID);

    *terminateValue = 1;
    
    mutex(waitPortsSemID, WAITZERO, errorHandler, "master mutex WAITZERO on ports");
   
    printDump(SYNC , *day, 1);
    
    mutex(waitToRemoveDump, WAITZERO, errorHandler, "mutex waitzero remove dump");
    
    lockAllGoodsDump();

    removeSem(startSimulationSemID, errorHandler, "startSimulationSemID");
    removeSem(reservePrintSem, errorHandler, "reservePrintSem");
    removeSem(pierSemID, errorHandler, "pierSemID");
    removeSem(reservePortsResourceSem, errorHandler, "reservePortsResourceSem");
    removeSem(waitconfigSemID, errorHandler, "waitconfigSemID");
    removeSem(rwExpTimesPortSemID, errorHandler , "rwExpTimesPortSemID");
    removeSem(semShipsID, errorHandler, "semShipsID");
    removeSem(waitEndDaySemID, errorHandler, "waitEndDaySemID");
    removeSem(controlPortsDisponibilitySemID, errorHandler, "controlPortsDisponibilitySemID");
    removeSem(verifyRequestPortSemID, errorHandler, "verifyRequestPortSemID");
    removeSem(waitToRemoveDump, errorHandler, "waitToRemoveDump");
    removeSem(waitResponsesID, errorHandler, "waitResponsesID");

    removeSem(waitPortsSemID, errorHandler, "waitPortsSemID");
    removeSem(waitShipsSemID, errorHandler, "waitShipsSemID");
    removeSem(waitEndDayShipSemID, errorHandler, "waitEndDayShipSemID");
    removeSem(verifyAllPortsSemID, errorHandler, "verifyAllPortsSemID");
    removeShm(shipsShmid, errorHandler, "shipsShmid");
    distruggiShmPorti();
    
    shmDetach(terminateValue, errorHandler, "master terminateValue detach");
    shmDetach(day, errorHandler, "master day detach");
    removeShm(endShmID, errorHandler, "endShmID");
    removeShm(dayShmID, errorHandler, "daySmID");
    removeDumpArea();

    removeQueue(msgRefillerID, errorHandler, "msgRefillerID");
    
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
        msgSend(msgRefillerID, supportText, type, errorHandler,0, "refillCode");
    }

}

void refillPorts(int opt, int msgRefillerID, int quantityPerDay, int giorno) {
    intList* l;
    int so_porti = SO_("PORTI");
    int pid;
    l = distributeV1(quantityPerDay, so_porti);
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
    int* magazine;
    int portBufferSemID;
    rwExpTimesPortSemID = useSem(WREXPTIMESSEM, errorHandler, "useSem rwExpTimesPortSemID in childExpirePortCode");
    portBufferSemID = useSem(RESPORTSBUFFERS, errorHandler, "useSem portBufferSemID in childExpirePortCode");
    
    mutexPro(rwExpTimesPortSemID, idx, LOCK, errorHandler , "rwExpTimesPortSemID LOCK in childExpirePortCode");
    
    decrementExpTimes(&p->supplies, day);
    
    mutexPro(rwExpTimesPortSemID, idx, UNLOCK, errorHandler, "rwExpTimesPortSemID UNLOCK in childExpirePortCode");

    mutexPro(portBufferSemID, idx, LOCK, errorHandler, "portBufferSemID LOCK in childExpirePortCode");
    magazine = getMagazine(p);
    removeExpiredGoods(&p->supplies, magazine);
    shmDetach(magazine, errorHandler, "childExpirePortCode magazine");
    mutexPro(portBufferSemID, idx, UNLOCK, errorHandler, "portBufferSemID UNLOCK in childExpirePortCode");
    
}

void expirePortsGoods(int day) {
    int i;
    Port port;
    int pid;
    int so_porti = SO_("PORTI");

    for (i = 0; i < so_porti; i++) {
        pid = fork();
        if (pid == -1) {
            throwError("fork nel gestore delle risorse","expirePortsGoods");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            port = getPort(i);
            childExpirePortCode(port, day, i);
            detachPort(port, i);
            exit(EXIT_SUCCESS);
        }
    }
}

FILE* create_weather() {
    int pid;
    FILE* fp;

    return popen("./bin/meteo", "w");
    
}

int countAliveShips(){
    int waitShipSemID = useSem(WAITSHIPSSEM, errorHandler, "nave waitShipSemID");   
    return getOneValue(waitShipSemID, 0);
}

void resetWeatherTargets(Ship arrShip){
    int i;
    Port p;
    int so_navi = SO_("NAVI");
    int so_porti = SO_("PORTI");

    for (i = 0; i < so_navi; i++)
    {
        if(arrShip[i].weatherTarget == 1) {
            arrShip[i].weatherTarget = 0;
        }
    }

    for(i = 0; i< so_porti; i++){
        p = getPort(i);
        if (p->weatherTarget == 1)
        {
            p->weatherTarget = 0;
        }
        detachPort(p,i);
    }
}

