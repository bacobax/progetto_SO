#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>

#include "../utils/errorHandler.h"
#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"
#include "../utils/msg_utility.h"
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
    int i;
    int c;
    int k;
    GoodTypeInfo *arrGoods;
    shmid = createShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler, "create dump area");
    semid = createMultipleSem(DUMPSEMKEY, SO_MERCI, 1, errorHandler, "creazione semafori per regolare la zona di dump");
    logFileSemID = createSem(LOGFILESEMKEY, 1, errorHandler, "creazione semaforo per scrivere nel file di log");

   

    /*per cancellare il contenuto del logfile*/
    fclose(fopen("./logs/logfile.log", "w"));
    fclose(fopen("./logs/historyTransictions.log", "w"));
    
}

void addExpiredGood(int quantity, int type, ctx where) {
    int shmid;
    int semid;
    
    GoodTypeInfo* info;
   
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler, "shm del dump");
    semid = useSem(DUMPSEMKEY, errorHandler, "semafori del dump");
    
    info = ((GoodTypeInfo*) getShmAddress(shmid, 0, errorHandler, "attach del tipo di merce del dump")) + type;

    mutexPro(semid, type, LOCK, errorHandler, "addExpiredGood LOCK");

    if (where == PORT) {
        info->expired_goods_on_port += quantity;
        info->goods_on_port -= quantity;
        
    }
    else if (where == SHIP) {
        info->expired_goods_on_ship += quantity;
        info->goods_on_ship -= quantity;
        
    }
    else {
        
        throwError("Il contesto può solo essere PORT o SHIP", "addExpiredGood");
        exit(1);
    }

    mutexPro(semid, type, UNLOCK, errorHandler, "addExpiredGood UNLOCK");

    shmDetach(info - type,errorHandler, "addExpiredGood");

}

void addNotExpiredGood(int quantity, int type, ctx where, int refilling, int idx) {
    int shmid;
    int semid;
    FILE *fp;
    GoodTypeInfo* info;
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler, "addNotExpiredGood");
    semid = useSem(DUMPSEMKEY, errorHandler , "addNotExpiredGood");
    fp = fopen("./logs/historyTransictions.log", "a+"); 
    
    info = ((GoodTypeInfo*) getShmAddress(shmid, 0, errorHandler, "addNotExpiredGood")) + type;

    mutexPro(semid, type, LOCK, errorHandler, "addNotExpiredGood LOCK");
    if(!refilling){
        fprintf(fp, "DUMP: %s IDX: %d, %s %d\n", (where == PORT ? "PORT" : "NAVE"),idx , (quantity <0 ? "tolgo" : "aggiungo") ,(quantity<0 ? -1 * quantity : quantity) );
    }
    if (where == PORT)
    {
        info->goods_on_port += quantity;
    }
    else if (where == SHIP) {
        info->goods_on_ship += quantity;
    }
    else {
        throwError("Il contesto può solo essere PORT o SHIP", "addNotExpiredGood");
        
        exit(1);
    }
    fclose(fp);
    mutexPro(semid, type, UNLOCK, errorHandler, "addNotExpiredGood UNLOCK");
    shmDetach(info - type,errorHandler, "addNotExpiredGood");

}

void addDeliveredGood(int quantity, int type, int portIdx){
    int shmid;
    int semid;
    int portShmId;
    Port portArr;
    FILE* fp;
    GoodTypeInfo* info;
    
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler, "addDeliveredGood");
    semid = useSem(DUMPSEMKEY, errorHandler, "addDeliveredGood");
    portShmId = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler, "portShmid addDeliveredGood");

    portArr = ((Port)getShmAddress(portShmId,0,errorHandler,"addDeliveredGood portArr"));

    info = ((GoodTypeInfo*)getShmAddress(shmid, 0, errorHandler, "addDeliveredGood")) + type;
    mutexPro(semid, type, LOCK, errorHandler, "addDeliveredGood LOCK");

    info->delivered_goods += quantity;
    info->goods_on_ship -= quantity;
    
    portArr[portIdx].deliveredGoods += quantity;
    
    mutexPro(semid, type, UNLOCK, errorHandler, "addDeliveredGood LOCK");
    shmDetach(info - type,errorHandler, "info addDeliveredGood");
    shmDetach(portArr,errorHandler, "portArr addDeliveredGood");
}


void removeDumpArea() {
    int shmid;
    int logFileSemID;
    int semid;
    
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler , "removeDumpArea");
    semid = useSem(DUMPSEMKEY, errorHandler, "removeDumpArea");
    logFileSemID = useSem(LOGFILESEMKEY, errorHandler, "removeDumpArea");
    
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
    GoodTypeInfo *arr;
    Port portArr;
    Supplies s;

   
    logFileSemID = useSem(LOGFILESEMKEY, errorHandler, "printerCode");
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler , "printerCode->useShm del dump");
    portShmid = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler , "printerCode->useShm dei porti");
    arr = (GoodTypeInfo*)getShmAddress(shmid, 0, errorHandler ,"printerCode->arr");
    portArr = (Port)getShmAddress(portShmid, 0, errorHandler, "printerCode->portArr");
    
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
        fprintf(fp, "\t\ta) nei porti: %d\n", (arr + i)->goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", (arr + i)->goods_on_ship);
        fprintf(fp, "\t- Scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", (arr + i)->expired_goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", (arr + i)->expired_goods_on_ship);
        fprintf(fp, "\t- Consegnata: %d\n" ,  (arr + i)->delivered_goods);
        expiredGoodsOnShips += arr[i].expired_goods_on_ship;
        expiredGoodsOnPorts +=  arr[i].expired_goods_on_port;
        deliveredGoods += arr[i].delivered_goods;
        notExpiredGoodsOnShips +=  arr[i].goods_on_ship;
        notExpiredGoodsOnPorts += arr[i].goods_on_port;
    }

    sum = expiredGoodsOnPorts+ expiredGoodsOnShips + notExpiredGoodsOnPorts+notExpiredGoodsOnShips+deliveredGoods;

    printStatoNavi(fp);

    printStatoPorti(fp, portArr);
    

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
        fprintf(fp,"Merce rimasta in porto: %d\n" , notExpiredGoodsOnPorts);
        fprintf(fp,"Merce rimasta in nave: %d\n" , notExpiredGoodsOnShips);
        fprintf(fp, "Merce scaduta in porto: %d\n" , expiredGoodsOnPorts);
        fprintf(fp, "Merce scaduta in nave: %d\n" , expiredGoodsOnShips);
        fprintf(fp, "Merce consegnata: %d\n" , deliveredGoods);


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
    shmDetach(arr, errorHandler , "printerCode arr");
    shmDetach(portArr, errorHandler , "printerCode portArr");
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
