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

void genera_porti(int risorse, int n_porti) {

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

void creaShmPorti(){
    int shmid;
    int i;
    char text[512];
    int ftok_val;
    int so_porti = SO_("PORTI");

    shmid = createShm(PSHMKEY, sizeof(struct port) * so_porti, errorHandler, "creaShmPorti");
/*
    for (i = 0; i < so_porti; i++) {
        ftok_val = ftok("./utils/port_utility.c", i);
        if (ftok_val == -1) {
            throwError("ftok valore -1", "getPort");
            exit(1);
        }
        shmid = createShm(ftok_val, sizeof(struct port), errorHandler, "creaShmPorti");
        printf("MASTER HO CREATO LA SHM PER IL PORTO:%d\n", i);
        // sprintf(text, "key generata da ftok:%d per il porto:%d", ftok_val, i);
        // throwError(text, "crea shm porti");
    }*/
    return;
}

void distruggiShmPorti(){
    int shmid;
    int i;
    int so_porti = SO_("PORTI");
    /*
for (i = 0; i < SO_("PORTI"); i++) {
        shmid = useShm(ftok("./utils/port_utility.c", i), sizeof(struct port), errorHandler, "distruggiShmPorti");
        removeShm(shmid , errorHandler, "distruggiShmPorti");
    }
    */
    removeShm(useShm(PSHMKEY, sizeof(struct port) * so_porti, errorHandler, "distruggiShmPorti"), errorHandler, "distruggiShmPorti");
    return;
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
    int so_porti;
    int so_navi;
    int so_banchine;
    int so_merci;
    int so_days;
    struct sigaction new_sig_action;
    sigset_t new_sig_set;

    so_porti = SO_("PORTI");
    so_navi = SO_("NAVI");
    so_banchine = SO_("BANCHINE");
    so_merci = SO_("MERCI");
    so_days = SO_("DAYS");
    signal(SIGCHLD, SIG_IGN);
    sigemptyset(&new_sig_set);
    sigaddset(&new_sig_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &new_sig_set, NULL);
    srand(time(NULL));

   
    initErrorHandler();

    startSimulationSemID = createSem(MASTKEY, 1, errorHandler, "creazione sem startSimulationSemID");
    reservePrintSem = createSem(RESPRINTKEY, 1, errorHandler, "creazione sem reservePrintSem");
    controlPortsDisponibilitySemID = createMultipleSem(PSEMVERIFYKEY, so_porti, 1, errorHandler, "creazione sem per controllare i supplies in ordine");
    /*
    !dovrà essere SO_PORTI + SO_NAVI
    */

   waitPortsSemID = createSem(WAITPORTSSEM, so_porti + 1, errorHandler, "master crazione waitPortsSemID");
   waitShipsSemID = createSem(WAITSHIPSSEM, so_navi, errorHandler, "master creazione waitShipsSemID");

    waitEndDayShipSemID = createSem(WAITENDDAYSHIPSEM, 0, errorHandler, "master create waitEndDayShipSem");

    waitconfigSemID = createSem(WAITCONFIGKEY, so_porti+ so_navi+ 1, errorHandler, "creazione sem waitconfig");

    
    createDumpArea();
    /*
        !deprecated
        portsShmid = createShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler , "creazione shm dei porti");
    
    */
    creaShmPorti();
    shipsShmid = createShm(SSHMKEY, so_navi * sizeof(struct ship), errorHandler, "creazione shm delle navi");
    endShmID = createShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "crazione shm intero terminazione programma");
    terminateValue = (unsigned int*)getShmAddress(endShmID, 0, errorHandler, "master getShmAddress endShm");

    dayShmID = createShm(DAYWORLDSHM, sizeof(int), errorHandler, "crazione day shm master");
    day = (int*) getShmAddress(dayShmID, 0, errorHandler, "master getShmAddress day");


    /*creazione banchine*/
    semBanchineID = createMultipleSem(BANCHINESEMKY, so_porti, so_banchine, errorHandler, "creazione semaforo banchine");

    if (portsShmid == EEXIST || shipsShmid == EEXIST) {
        throwError("Le shm esistono già\n" , "mySettedMain");
        exit(EXIT_FAILURE);
    }
    /*il codice del master manco la usa*/
    msgRefillerID = createQueue(REFILLERQUEUE, errorHandler, "creazione coda del refiller");

    /* msgShipQueueID = createQueue(SCHQUEUEKEY, errorHandler); */

    /* creare queue navi per fase di scaricamento TO-DO*/

    reservePortsResourceSem = createMultipleSem(RESPORTSBUFFERS, so_porti, 1, errorHandler, "creazione sem per scrivere nei supplies di un porto");

    rwExpTimesPortSemID = createMultipleSem(WREXPTIMESSEM, so_porti, 1, errorHandler, "sem per leggere e scrivere negli exp time di un porto");

    semShipsID = createMultipleSem(SEMSHIPKEY, so_navi, 1, errorHandler, "creazione sem per modificare carico di una nave");

    /*
        semaforo che serve al master per eseguirne la waitzero alla fine di ogni giorno
        in sintesi il master aspetta a passare il giorno finchè tutti i porti non hanno ricevuto la loro merce
    */
    waitEndDaySemID = createSem(WAITENDDAYKEY, so_porti, errorHandler, "creazione sem waitEndDaySem");
    /*
    portRequestsQueueID = createQueue(PQUERECHKEY, errorHandler, "creazione della coda delle richieste di carico dei porti");
    portDischargeRequestsQueueID = createQueue(PQUEREDCHKEY, errorHandler, "creazione della coda delle richieste di scarico dei porti");*/
    
    /* TO-DO creare coda porti richieste per fase di scaricamento*/
/*
    creaCodePorti();
    creaCodePortiDischarge();
    creaCodeNavi();*/
    /* TO-DO creare code porti per fase di scaricamento*/
/*
    waitToTravelsemID = createMultipleSem(WAITTOTRAVELKEY, SO_NAVI, SO_PORTI, errorHandler, "creazione waitToTravelSem");

*/
    
    waitResponsesID = createMultipleSem(WAITFIRSTRESPONSES, so_navi, 1, errorHandler, "creazione waitResponsesSem");

    verifyRequestPortSemID = createMultipleSem(P2SEMVERIFYKEY, so_porti*so_merci, 1, errorHandler, "creazione verifyRequestPortSemID");
    
    waitToRemoveDump = createSem(WAITRMVDUMPKEY, 1, errorHandler, "craezione semaforo remove dump");


    codiceMaster(startSimulationSemID, portsShmid, shipsShmid, reservePrintSem, waitconfigSemID, msgRefillerID, waitEndDaySemID, day, waitEndDayShipSemID);
    /* kill(0, SIGUSR1);  uccide tutti i figli */
    printf("SETTATO A 1 TERMINATE VALUE, ASPETTO FIGLI...\n");

    *terminateValue = 1;
    /*
    wait_all(SO_NAVI + SO_PORTI + (SO_PORTI * 3));
    */
    mutex(waitPortsSemID, WAITZERO, errorHandler, "master mutex WAITZERO on ports");
    printf("FACCIO IL PRINT DEL DUMP DEL %d ESIMO GIORNO\n", so_days);
    printDump(SYNC , *day, 1);
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
    /*removeSem(waitToTravelsemID, errorHandler, "waitToTravelsemID");*/
    removeSem(waitResponsesID, errorHandler, "waitResponsesID");

    removeSem(waitPortsSemID, errorHandler, "waitPortsSemID");
    removeSem(waitShipsSemID, errorHandler, "waitShipsSemID");
    removeSem(waitEndDayShipSemID, errorHandler, "waitEndDayShipSemID");
    printf("master tutti i sem sono stati rimoessi\n");

    removeShm(shipsShmid, errorHandler, "shipsShmid");
    distruggiShmPorti();
    /*
    removeShm(portsShmid, errorHandler, "portsShmid");

    */
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
    /*distruggiCodePorti();
    distruggiCodePortiDischarge();*/
    printf("coda dei porti rimossa\n");
    
    /*distruggiCodeNavi();*/
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
    int so_porti = SO_("PORTI");
    int pid;
    l = distributeV1(quantitaAlGiorno, so_porti);
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
/*
void childExpireShipCode(Ship ship) {
    int semShipID;
    semShipID = useSem(SEMSHIPKEY, errorHandler, "childExpireShipCode");
    printf("EXPIRER PID: %d\n" , getpid());
    logShip(ship->shipID, "expirer nave FACCIO LOCK semShipID");
    mutexPro(semShipID, ship->shipID, LOCK, errorHandler, "childExpireShipCode LOCK");
    logShip(ship->shipID, "expirer passo la LOCK");
    if (ship->dead) {
        mutexPro(semShipID, ship->shipID, UNLOCK, errorHandler, "childExpireShipCode UNLOCK");    
        exit(EXIT_SUCCESS);
    }
    
    updateExpTimeShip(ship);

    logShip(ship->shipID, "expirer nave FACCIO UNLOCK semShipID");
    mutexPro(semShipID, ship->shipID, UNLOCK, errorHandler, "childExpireShipCode UNLOCK");    
}
*/


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
/*
void expireShipGoods(){
    int shipShmID;
    int i;
    Ship ships;
    int pid;
    for(i=0; i<SO_NAVI; i++) {
        pid = fork();
        if(pid == -1){
            throwError("fork nel gestore risorse","expireShipGoods");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            shipShmID = useShm(SSHMKEY, sizeof(struct ship) * SO_NAVI, errorHandler,"expireShipGoods");
            
            ships = (Ship)getShmAddress(shipShmID, 0, errorHandler, "expireShipGoods");
            childExpireShipCode(ships + i);
            shmDetach(ships, errorHandler, "expireShipGoods");
            exit(EXIT_SUCCESS);
        }
    }
}
*/

FILE* genera_meteo() {
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

