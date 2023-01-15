#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>

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
    semid = useSem(DUMPSEMKEY, errorHandler, "lockAllGoodsDump");
    for(i=0; i<SO_MERCI; i++){
        mutexPro(semid, i, LOCK, errorHandler, "lockAllGoodsDump");
    }
}
void unlockAllGoodsDump(){
    int semid;
    int i;
    semid = useSem(DUMPSEMKEY, errorHandler, "lockAllGoodsDump");
    for(i=0; i<SO_MERCI; i++){
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
    DumpArea *arrGoods;
    shmid = createShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "create dump area");
    semid = createMultipleSem(DUMPSEMKEY, SO_MERCI, 1, errorHandler, "creazione semafori per regolare la zona di dump");
    logFileSemID = createSem(LOGFILESEMKEY, 1, errorHandler, "creazione semaforo per scrivere nel file di log");
    txFileSemID = createSem(TXFILESEMKEY, 1, errorHandler, "creazione semaforo per scrivere nel file di log tx");

   

    /*per cancellare il contenuto del logfile*/
    fclose(fopen("./logs/logfile.log", "w"));
    fclose(fopen("./logs/historyTransictions.log", "w"));
    fclose(fopen("./logs/cronologia.log", "w"));
    fclose(fopen("./logs/exitShipLog.log", "w"));
    
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
   
    shmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "shm del dump");
    semid = useSem(DUMPSEMKEY, errorHandler, "semafori del dump");
    
    dump = ((DumpArea*) getShmAddress(shmid, 0, errorHandler, "attach del tipo di merce del dump"));

    mutexPro(semid, type, LOCK, errorHandler, "addExpiredGood LOCK");

    if (where == PORT) {
        dump->types[type].expired_goods_on_port += quantity;
        dump->types[type].goods_on_port -= quantity;
        
    }
    else if (where == SHIP) {
        dump->types[type].expired_goods_on_ship += quantity;
        dump->types[type].goods_on_ship -= quantity;
        
    }
    else {
        
        throwError("Il contesto può solo essere PORT o SHIP", "addExpiredGood");
        exit(1);
    }

    mutexPro(semid, type, UNLOCK, errorHandler, "addExpiredGood UNLOCK");

    shmDetach(dump,errorHandler, "addExpiredGood");

}

void addNotExpiredGood(int quantity, int type, ctx where, int refilling, int idx) {
    int shmid;
    int semid;
    FILE *fp;
    DumpArea* dump;
    shmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "addNotExpiredGood");
    semid = useSem(DUMPSEMKEY, errorHandler , "addNotExpiredGood");
    fp = fopen("./logs/historyTransictions.log", "a+"); 
    
    dump = ((DumpArea*) getShmAddress(shmid, 0, errorHandler, "addNotExpiredGood"));

    mutexPro(semid, type, LOCK, errorHandler, "addNotExpiredGood LOCK");
    if(!refilling){
        fprintf(fp, "DUMP: %s IDX: %d, %s %d\n", (where == PORT ? "PORT" : "NAVE"),idx , (quantity <0 ? "tolgo" : "aggiungo") ,(quantity<0 ? -1 * quantity : quantity) );
    }
    if (where == PORT)
    {
        dump->types[type].goods_on_port += quantity;
    }
    else if (where == SHIP) {
        dump->types[type].goods_on_ship += quantity;
    }
    else {
        throwError("Il contesto può solo essere PORT o SHIP", "addNotExpiredGood");
        
        exit(1);
    }
    fclose(fp);
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
    
    shmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "addDeliveredGood");
    semid = useSem(DUMPSEMKEY, errorHandler, "addDeliveredGood");

    port = getPort(portIdx);

    dump = ((DumpArea*)getShmAddress(shmid, 0, errorHandler, "addDeliveredGood"));
    mutexPro(semid, type, LOCK, errorHandler, "addDeliveredGood LOCK");

    dump->types[type].delivered_goods += quantity;
    dump->types[type].goods_on_ship -= quantity;
    
    port->deliveredGoods += quantity;
    
    mutexPro(semid, type, UNLOCK, errorHandler, "addDeliveredGood LOCK");
    shmDetach(dump,errorHandler, "dump addDeliveredGood");
    shmDetach(port,errorHandler, "portArr addDeliveredGood");
}


void removeDumpArea() {
    int shmid;
    int logFileSemID;
    int semid;
    int txSemID;
    shmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler , "removeDumpArea");
    semid = useSem(DUMPSEMKEY, errorHandler, "removeDumpArea");
    logFileSemID = useSem(LOGFILESEMKEY, errorHandler, "removeDumpArea");
    txSemID = useSem(TXFILESEMKEY, errorHandler, "removeDumpArea");

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
    DumpArea *dump;
    Supplies s;

   
    logFileSemID = useSem(LOGFILESEMKEY, errorHandler, "printerCode");
    shmid = useShm(DUMPSHMKEY,sizeof(DumpArea), errorHandler , "printerCode->useShm del dump");
    dump = (DumpArea*)getShmAddress(shmid, 0, errorHandler ,"printerCode->dump");
    
    waitToRemoveDumpKey = useSem(WAITRMVDUMPKEY, errorHandler, "waitToRemoveDumpKey in printerCode");
    
    

    mutex(logFileSemID, LOCK, errorHandler, "printerCode LOCK");
    printf("Scrivo nel logifle %d\n" ,day);
    fp = fopen("./logs/logfile.log", "a+");
    if (fp == NULL) {
        throwError("Errore nell'apertura del file log", "printerCode");
        exit(EXIT_FAILURE);
    }
    if (day < SO_DAYS) {
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
    for (i = 0; i < SO_MERCI; i++) {
        fprintf(fp, "Tipo merce %d:\n", i);
        fprintf(fp, "\t- Non scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", dump->types[i].goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", dump->types[i].goods_on_ship);
        fprintf(fp, "\t- Scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", dump->types[i].expired_goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", dump->types[i].expired_goods_on_ship);
        fprintf(fp, "\t- Consegnata: %d\n" ,  dump->types[i].delivered_goods);
        expiredGoodsOnShips += dump->types[i].expired_goods_on_ship;
        expiredGoodsOnPorts +=  dump->types[i].expired_goods_on_port;
        deliveredGoods += dump->types[i].delivered_goods;
        notExpiredGoodsOnShips +=  dump->types[i].goods_on_ship;
        notExpiredGoodsOnPorts += dump->types[i].goods_on_port;
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
        merceDaRefillare = (SO_FILL/SO_DAYS)*(SO_DAYS-day);
        merceRefillata = SO_FILL - merceDaRefillare;
        
        fprintf(fp, "Tempo di vita medio della merce: %.3f\n", ((double)(SO_MAX_VITA+SO_MIN_VITA))/2);
        fprintf(fp, "Varianza tempo della merce: %.3f\n", dump->expTimeVariance / (SO_PORTI * SO_DAYS * SO_MERCI));
        fprintf(fp, "SO_DAYS: %d\n", SO_DAYS);
        fprintf(fp, "Merce rimasta in porto: %d\n", notExpiredGoodsOnPorts);
        fprintf(fp,"Merce rimasta in nave: %d\n" , notExpiredGoodsOnShips);
        fprintf(fp, "Merce scaduta in porto: %d\n" , expiredGoodsOnPorts);
        fprintf(fp, "Merce scaduta in nave: %d\n" , expiredGoodsOnShips);
        fprintf(fp, "Merce consegnata: %d (%.4f%% di SO_FILL)\n" , deliveredGoods,((double)( deliveredGoods* 100))/SO_FILL);
        fprintf(fp, "Tempo di viaggio medio viaggio tra porti: %f\n", mediaTempoViaggioFraPorti());
        fprintf(fp, "Tempo medio scaricamento di lotti: %f\n", (dump->tempoScaricamentoTot)/((double)(SO_PORTI * SO_DAYS * SO_MERCI)));
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
    shmDetach(dump, errorHandler , "printerCode dump");
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
