#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/ipc.h>

#include "../utils/errorHandler.h"
#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"
#include "../utils/msg_utility.h"
#include "../utils/support.h"
#include "../config1.h"
#include "./dump.h"
#include "./porto.h"
#include "./nave.h"
void lockAllGoodsDump(){
    int semid;
    int i;
    int so_merci = SO_("MERCI");
    semid = useSem(DUMPSEMKEY, errorHandler, "lockAllGoodsDump");
    for(i=0; i<so_merci; i++){
        mutexPro(semid, i, LOCK, errorHandler, "lockAllGoodsDump");
    }
}
void unlockAllGoodsDump(){
    int semid;
    int i;
    int so_merci = SO_("MERCI");
    semid = useSem(DUMPSEMKEY, errorHandler, "lockAllGoodsDump");
    for(i=0; i<so_merci; i++){
        mutexPro(semid, i, UNLOCK, errorHandler, "lockAllGoodsDump");
    }
}

void createDumpArea(){
    int shmid;
    int semid;
    int logFileSemID;
    int txFileSemID;
    int i;
    int c;
    int k;
    int so_merci = SO_("MERCI");
    DumpArea *dumpArea;
    shmid = createShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "create dump area");
    semid = createMultipleSem(DUMPSEMKEY, so_merci, 1, errorHandler, "creazione semafori per regolare la zona di dump");
    logFileSemID = createSem(LOGFILESEMKEY, 1, errorHandler, "creazione semaforo per scrivere nel file di log");
    txFileSemID = createSem(TXFILESEMKEY, 1, errorHandler, "creazione semaforo per scrivere nel file di log tx");

    dumpArea = (DumpArea*)getShmAddress(shmid, 0, errorHandler, "createDumpArea");

    dumpArea->typesInfoID = createShm(ftok("./src/dump.c", 0), sizeof(GoodTypeInfo) * so_merci, errorHandler, "createArrDumpArea");

    shmDetach(dumpArea, errorHandler, "createArrDumpArea");

    /*per cancellare il contenuto del logfile*/
    fclose(fopen("./logs/logfile.log", "w"));
    fclose(fopen("./logs/historyTransictions.log", "w"));
    fclose(fopen("./logs/cronologia.log", "w"));
    fclose(fopen("./logs/exitShipLog.log", "w"));
    fclose(fopen("./logs/logNavi.log", "w"));
}

void transactionPrinterCode(int idxNave, int idxPorto, int carico, int ton, int tipoMerce) {
    FILE* fp;
    int txSemID;
    int* day;
    int dayShmID;
    dayShmID = useShm(DAYWORLDSHM, sizeof(int), errorHandler, "DAYWORLDSHM transactionPrinterCode");
    day = (int*)getShmAddress(dayShmID, 0, errorHandler, "DAYWORLDSHM transactionPrinterCode");
    txSemID = useSem(TXFILESEMKEY, errorHandler, "removeDumpArea");
    fp = fopen("./logs/cronologia.log", "a+");
    mutex(txSemID, LOCK, errorHandler, "mutex(txSemID, LOCK");
    
    fprintf(fp, "DAY: %d:🚢 %d %s %d|%d %s Porto %d\n", *day, idxNave, (carico ? "⏪️" : "⏩️"), ton, tipoMerce, (carico ? "⏪️" : "⏩️"), idxPorto);

    mutex(txSemID, UNLOCK, errorHandler, "mutex(txSemID, UNLOCK");
    fclose(fp);
    shmDetach(day, errorHandler, "DAYWORLDSHM transactionPrinterCode");
}

void printTransaction(int idxNave, int idxPorto, int carico, int ton, int tipoMerce) {
    int pid;
    pid = fork();
    if (pid == -1) {
        throwError("errore nella fork", "printTransaction");
    }
    if (pid == 0) {
        transactionPrinterCode(idxNave, idxPorto, carico, ton, tipoMerce);
        exit(0);
    }
}

void addExpiredGood(int quantity, int type, ctx where) {
    int shmid;
    int semid;
    
    DumpArea* dump;
    GoodTypeInfo* types;
    shmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "shm del dump");
    semid = useSem(DUMPSEMKEY, errorHandler, "semafori del dump");
    
    dump = ((DumpArea*) getShmAddress(shmid, 0, errorHandler, "attach del tipo di merce del dump"));

    mutexPro(semid, type, LOCK, errorHandler, "addExpiredGood LOCK");
    types = (GoodTypeInfo*)getShmAddress(dump->typesInfoID,0,errorHandler, "addExpiredGood types");
    if (where == PORT) {
        types[type].expired_goods_on_port += quantity;
        types[type].goods_on_port -= quantity;
        
    }
    else if (where == SHIP) {
        types[type].expired_goods_on_ship += quantity;
        types[type].goods_on_ship -= quantity;
        
    }
    else {
        
        throwError("Il contesto può solo essere PORT o SHIP", "addExpiredGood");
        exit(1);
    }
    shmDetach(types, errorHandler, "addExpiredGood types");
    mutexPro(semid, type, UNLOCK, errorHandler, "addExpiredGood UNLOCK");

    shmDetach(dump,errorHandler, "addExpiredGood");

}

void addNotExpiredGood(int quantity, int type, ctx where, int refilling, int idx) {
    int shmid;
    int semid;
    FILE *fp;
    DumpArea* dump;
    GoodTypeInfo* types;
    
    shmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "addNotExpiredGood");
    semid = useSem(DUMPSEMKEY, errorHandler , "addNotExpiredGood");
    fp = fopen("./logs/historyTransictions.log", "a+"); 
    
    dump = ((DumpArea*) getShmAddress(shmid, 0, errorHandler, "addNotExpiredGood"));

    mutexPro(semid, type, LOCK, errorHandler, "addNotExpiredGood LOCK");
    types = (GoodTypeInfo*)getShmAddress(dump->typesInfoID,0,errorHandler, "addNotExpiredGood types");
    
    if (!refilling) {
        fprintf(fp, "DUMP: %s IDX: %d, %s %d\n", (where == PORT ? "PORT" : "NAVE"),idx , (quantity <0 ? "tolgo" : "aggiungo") ,(quantity<0 ? -1 * quantity : quantity) );
    }
    if (where == PORT)
    {
        types[type].goods_on_port += quantity;
    }
    else if (where == SHIP) {
        types[type].goods_on_ship += quantity;
    }
    else {
        throwError("Il contesto può solo essere PORT o SHIP", "addNotExpiredGood");
        
        exit(1);
    }
    fclose(fp);
    shmDetach(types, errorHandler, "addNotExpiredGood types");
    
    mutexPro(semid, type, UNLOCK, errorHandler, "addNotExpiredGood UNLOCK");
    shmDetach(dump,errorHandler, "addNotExpiredGood");

}

void addDeliveredGood(int quantity, int type, int portIdx){
    int shmid;
    int semid;
    int portShmId;
    Port port;
    FILE* fp;
    DumpArea* dump;
    GoodTypeInfo* types;
    
    shmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "addDeliveredGood");
    semid = useSem(DUMPSEMKEY, errorHandler, "addDeliveredGood");

    port = getPort(portIdx);

    dump = ((DumpArea*)getShmAddress(shmid, 0, errorHandler, "addDeliveredGood"));
    mutexPro(semid, type, LOCK, errorHandler, "addDeliveredGood LOCK");
    types = (GoodTypeInfo*)getShmAddress(dump->typesInfoID,0,errorHandler, "addDeliveredGood types");

    types[type].goods_on_ship -= quantity;
    types[type].delivered_goods += quantity;
    
    port->deliveredGoods += quantity;
    
    mutexPro(semid, type, UNLOCK, errorHandler, "addDeliveredGood LOCK");
    shmDetach(types, errorHandler, "addDeliveredGood types");
    
    shmDetach(dump, errorHandler, "dump addDeliveredGood");
    detachPort(port, portIdx);
}


void removeDumpArea() {
    int shmid;
    int logFileSemID;
    int semid;
    int txSemID;
    DumpArea* area;
    shmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "removeDumpArea");
    semid = useSem(DUMPSEMKEY, errorHandler, "removeDumpArea");
    logFileSemID = useSem(LOGFILESEMKEY, errorHandler, "removeDumpArea");
    txSemID = useSem(TXFILESEMKEY, errorHandler, "removeDumpArea");

    area = (DumpArea*)getShmAddress(shmid, 0, errorHandler, "removeDumpArea");

    removeShm(area->typesInfoID, errorHandler, "removeDumpArea");
    shmDetach(area, errorHandler, "removeDumpArea");
    removeSem(txSemID, errorHandler, "removeDumpArea");
    removeSem(semid, errorHandler, "removeDumpArea");
    removeSem(logFileSemID, errorHandler , "removeDumpArea");
    removeShm(shmid, errorHandler, "removeDumpArea");
}
void signalHandler(int s){
    printf("NON FACCIO NULLA DUMP\n");
    return;
}
void printerCode(int day, int last) {
    FILE* fp;
    int logFileSemID;
    int shmid;
    int portShmid;
    int i;
    int sum;
    int merceRefillata;
    int merceDaRefillare;
    int waitToRemoveDumpKey;
    int deliveredGoods;
    int notExpiredGoodsOnPorts;
    int notExpiredGoodsOnShips;
    int expiredGoodsOnPorts;
    int expiredGoodsOnShips;
    double media;
    double varianza;
    int so_fill = SO_("FILL");
    int so_days = SO_("DAYS");
    int so_max_vita = SO_("MAX_VITA");
    int so_min_vita = SO_("MIN_VITA");
    int so_merci = SO_("MERCI");
    int so_porti = SO_("PORTI");
    DumpArea* dump;
    Supplies s;
    GoodTypeInfo* types;

   
    logFileSemID = useSem(LOGFILESEMKEY, errorHandler, "printerCode");
    shmid = useShm(DUMPSHMKEY,sizeof(DumpArea), errorHandler , "printerCode->useShm del dump");
    dump = (DumpArea*)getShmAddress(shmid, 0, errorHandler ,"printerCode->dump");
    
    waitToRemoveDumpKey = useSem(WAITRMVDUMPKEY, errorHandler, "waitToRemoveDumpKey in printerCode");
    
    

    mutex(logFileSemID, LOCK, errorHandler, "printerCode LOCK");
    types = (GoodTypeInfo*)getShmAddress(dump->typesInfoID,0,errorHandler, "addDeliveredGood types");
    
    printf("Scrivo nel logifle %d\n", day);
    fp = fopen("./logs/logfile.log", "a+");
    if (fp == NULL) {
        throwError("Errore nell'apertura del file log", "printerCode");
        exit(EXIT_FAILURE);
    }
    if (day < so_days) {
    fprintf(fp, "------------------Day %d -----------------\n", day);
        
    }
    else {
            fprintf(fp, "------------------STATO FINALE -----------------\n");

    }
    sum = 0;
    lockAllGoodsDump();
    deliveredGoods = 0;
    notExpiredGoodsOnPorts = 0;
    notExpiredGoodsOnShips = 0;
    expiredGoodsOnPorts = 0;
    expiredGoodsOnShips = 0;
    for (i = 0; i < so_merci; i++) {
        fprintf(fp, "Tipo merce %d:\n", i);
        fprintf(fp, "\t- Non scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", types[i].goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", types[i].goods_on_ship);
        fprintf(fp, "\t- Scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", types[i].expired_goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", types[i].expired_goods_on_ship);
        fprintf(fp, "\t- Consegnata: %d\n" ,  types[i].delivered_goods);
        expiredGoodsOnShips += types[i].expired_goods_on_ship;
        expiredGoodsOnPorts +=  types[i].expired_goods_on_port;
        deliveredGoods += types[i].delivered_goods;
        notExpiredGoodsOnShips +=  types[i].goods_on_ship;
        notExpiredGoodsOnPorts += types[i].goods_on_port;
    }

    sum = expiredGoodsOnPorts+ expiredGoodsOnShips + notExpiredGoodsOnPorts+notExpiredGoodsOnShips+deliveredGoods;

    printStatoNavi(fp);

    printStatoPorti(fp);
    

    /*
        SO_FILL = 10
        se SO_DAYS = 2
        Q.TA GG = 5
        Se day = 1
        merceDaRefillare = 5
        merceRefillata = 5
    */
    if(last){
        merceDaRefillare = (so_fill/so_days)*(so_days-day);
        merceRefillata = so_fill - merceDaRefillare;
        media = ((double)(so_max_vita + so_min_vita)) / 2;
        varianza = dump->expTimeVariance / (so_porti * so_days * so_merci);
        fprintf(fp, "Tempo di vita medio della merce: %.3f\n", media);
        fprintf(fp, "Varianza tempo della merce: %.3f\n", varianza);
        fprintf(fp, "Coefficente di variazione tempo della merce: %.3f%%\n", (varianza/media)*100);
        fprintf(fp, "SO_DAYS: %d\n", so_days);
        fprintf(fp, "Merce rimasta in porto: %d\n", notExpiredGoodsOnPorts);
        fprintf(fp,"Merce rimasta in nave: %d\n" , notExpiredGoodsOnShips);
        fprintf(fp, "Merce scaduta in porto: %d\n" , expiredGoodsOnPorts);
        fprintf(fp, "Merce scaduta in nave: %d\n" , expiredGoodsOnShips);
        fprintf(fp, "Merce consegnata: %d (%.4f%% di SO_FILL)\n" , deliveredGoods,((double)( deliveredGoods* 100))/so_fill);
        fprintf(fp, "Tempo di viaggio medio viaggio tra porti: %f\n", mediaTempoViaggioFraPorti());
        fprintf(fp, "Tempo medio scaricamento di lotti: %f\n", (dump->tempoScaricamentoTot)/((double)(so_porti * so_days * so_merci)));
        /*
            SO_FILL/100 = Merce_conse/x
            x = M_C*100
        */

        fprintf(fp, "TOTALE MERCE: %d <==> IN GIOCO: %d\n", sum, merceRefillata);
        if (sum ==  merceRefillata) {
            fprintf(fp, "✅");
        }
        else {
            fprintf(fp, "❌");   
        }
    }
    
    unlockAllGoodsDump();
    fclose(fp);
    shmDetach(types, errorHandler, "addDeliveredGood types");
    
    shmDetach(dump, errorHandler, "printerCode dump");
    mutex(logFileSemID, UNLOCK, errorHandler , "printerCode UNLOCK");
    
    if(last){
        printf("Faccio la lock\n");
        mutex(waitToRemoveDumpKey, LOCK, errorHandler, "LOCK in waitToRemoveDumpID");
    }
}

void printDump(int mod, int day, int last) {
    int pid;
    if(mod == ASYNC){
        pid = fork();
        if (pid == -1) {
            throwError("Errore nel forkare il dump printer\n", "printDump");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            printerCode(day,last);

            exit(EXIT_SUCCESS);
        }    
    }else{
        printerCode(day, last);
    }
}


void logShip(int shipID, char* msg){
    FILE* fp;
    fp = fopen("./logs/logNavi.log", "a+");

    fprintf(fp, "[%d]Nave: %s\n", shipID, msg);

    fclose(fp);
}