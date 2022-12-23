#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"
#include "../config1.h"
#include "./dump.h"

void createDumpArea(){
    int shmid;
    int semid;
    int logFileSemID;
    int i;
    GoodTypeInfo* arrGoods;
    shmid = createShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler);
    semid = createMultipleSem(DUMPSEMKEY, SO_MERCI, 1, errorHandler);
    logFileSemID = createSem(LOGFILESEMKEY, 1, errorHandler);

    arrGoods = (GoodTypeInfo* )getShmAddress(shmid, 0, errorHandler);

    for (i = 0; i < SO_MERCI; i++) {
        arrGoods[i].delivered_goods = 0;
        arrGoods[i].goods_on_port = 0;
        arrGoods[i].goods_on_ship= 0;
        arrGoods[i].expired_goods_on_port= 0;
        arrGoods[i].expired_goods_on_ship= 0;
    }

    /*per cancellare il contenuto del logfile*/
    fclose(fopen("./logfile.log", "w"));

}

void addExpiredGood(int quantity, int type, ctx where) {
    int shmid;
    int semid;
    
    GoodTypeInfo* info;
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler);
    semid = useSem(DUMPSEMKEY, errorHandler);
    
    info = ((GoodTypeInfo*) getShmAddress(shmid, 0, errorHandler)) + type;
    info = ((GoodTypeInfo*)getShmAddress(shmid, 0, errorHandler)) + type;

    mutexPro(semid, type, LOCK, errorHandler);

    if (where == PORT) {
        info->expired_goods_on_port += quantity;
    }
    else if (where == SHIP) {
        info->expired_goods_on_ship += quantity;
    }
    else {
        perror("Il contesto può solo essere PORT o SHIP\n");
        exit(1);
    }

    mutexPro(semid, type, UNLOCK, errorHandler);




}

void addNotExpiredGood(int quantity, int type, ctx where) {
    int shmid;
    int semid;
    
    GoodTypeInfo* info;
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler);
    semid = useSem(DUMPSEMKEY, errorHandler);
    
    info = ((GoodTypeInfo*) getShmAddress(shmid, 0, errorHandler)) + type;

    mutexPro(semid, type, LOCK, errorHandler);

    if (where == PORT) {
        info->goods_on_port += quantity;
    }
    else if (where == SHIP) {
        info->goods_on_ship += quantity;
    }
    else {
        perror("Il contesto può solo essere PORT o SHIP\n");
        exit(1);
    }

    mutexPro(semid, type, UNLOCK, errorHandler);




}


void removeDumpArea() {
    int shmid;
    int logFileSemID;
    int semid;
    
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler);
    semid = useSem(DUMPSEMKEY, errorHandler);
    logFileSemID = useSem(LOGFILESEMKEY, errorHandler);
    
    removeSem(semid, errorHandler);
    removeSem(logFileSemID, errorHandler);
    removeShm(shmid, errorHandler);
}

void printerCode(int day) {
    FILE* fp;
    int logFileSemID;
    int shmid;
    int i;
    GoodTypeInfo* arr;

    logFileSemID = useSem(LOGFILESEMKEY, errorHandler);
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler);
    arr = (GoodTypeInfo*)getShmAddress(shmid, 0, errorHandler);

    printf("Merce di tipo %d nel porto: %d\n", 0, arr[0].goods_on_port);

    mutex(logFileSemID, LOCK, errorHandler);
    printf("Scrivo nel logifle\n");
    fp = fopen("./logfile.log", "a+");
    if (fp == NULL) {
        perror("Errore nell'apertura del file log");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "------------------Day %d -----------------\n", day);
    for (i = 0; i < SO_MERCI; i++) {
        fprintf(fp, "Tipo merce %d:\n", i);
        fprintf(fp, "\t- Non scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", (arr + i)->goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", (arr + i)->goods_on_ship);
        fprintf(fp, "\t- Scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", (arr + i)->expired_goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", (arr + i)->expired_goods_on_ship);
    }

    fclose(fp);
    mutex(logFileSemID, UNLOCK, errorHandler);
}

void printDump(int day) {
    int pid;


    pid = fork();
    if (pid == -1) {
        perror("Errore nel forkare il dump printer\n");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        printerCode(day);
        exit(EXIT_SUCCESS);
    }

    

    

    
}
