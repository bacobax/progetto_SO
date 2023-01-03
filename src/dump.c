#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"
#include "../config1.h"
#include "./dump.h"
#include "./porto.h"
/* TODO: la shmDetach() è buggata, da risolvere */
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
    fclose(fopen("./logfile.log", "w"));

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
        perror("Il contesto può solo essere PORT o SHIP\n");
        exit(1);
    }

    mutexPro(semid, type, UNLOCK, errorHandler, "addExpiredGood UNLOCK");

    shmDetach(info - type,errorHandler, "addExpiredGood");

}

void addNotExpiredGood(int quantity, int type, ctx where) {
    int shmid;
    int semid;
    
    GoodTypeInfo* info;
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler, "addNotExpiredGood");
    semid = useSem(DUMPSEMKEY, errorHandler , "addNotExpiredGood");
    
    info = ((GoodTypeInfo*) getShmAddress(shmid, 0, errorHandler, "addNotExpiredGood")) + type;

    mutexPro(semid, type, LOCK, errorHandler, "addNotExpiredGood LOCK");

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

    mutexPro(semid, type, UNLOCK, errorHandler, "addNotExpiredGood UNLOCK");
    shmDetach(info - type,errorHandler, "addNotExpiredGood");

}

void addDeliveredGood(int quantity, int type){
    int shmid;
    int semid;
    
    GoodTypeInfo* info;
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler, "addDeliveredGood");
    semid = useSem(DUMPSEMKEY, errorHandler, "addDeliveredGood");
    
    info = ((GoodTypeInfo*) getShmAddress(shmid, 0, errorHandler, "addDeliveredGood")) + type;
    printf("\n\nPRIMA DELLA MUTEXPRO DI ADD_DELIVERED_GOOD type:%d\n\n",type);
    mutexPro(semid, type, LOCK, errorHandler, "addDeliveredGood LOCK");

    info->delivered_goods += quantity;
    info->goods_on_ship -= quantity;

    mutexPro(semid, type, UNLOCK, errorHandler, "addDeliveredGood LOCK");
    printf("\n\nDOPO LA MUTEXPRO DI ADD_DELIVERED_GOOD ANDATA A BUON FINE\n\n");
    shmDetach(info - type,errorHandler, "addDeliveredGood");
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

void printerCode(int day) {
    FILE* fp;
    int logFileSemID;
    int shmid;
    int portShmid;
    int i;
    int sum;
    int c;
    int k;
    GoodTypeInfo *arr;
    Port portArr;
    Supplies s;

    logFileSemID = useSem(LOGFILESEMKEY, errorHandler, "printerCode");
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler , "printerCode->useShm del dump");
    portShmid = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler , "printerCode->useShm dei porti");
    arr = (GoodTypeInfo*)getShmAddress(shmid, 0, errorHandler ,"printerCode->arr");
    portArr = (Port)getShmAddress(portShmid, 0, errorHandler, "printerCode->portArr");

    mutex(logFileSemID, LOCK, errorHandler, "printerCode LOCK");
    printf("Scrivo nel logifle\n");
    fp = fopen("./logfile.log", "a+");
    if (fp == NULL) {
        perror("Errore nell'apertura del file log");
        exit(EXIT_FAILURE);
    }
    if (day < SO_DAYS) {
    fprintf(fp, "------------------Day %d -----------------\n", day);
        
    }
    else {
            fprintf(fp, "------------------STATO FINALE -----------------\n", day);

    }
    sum = 0;
    for (i = 0; i < SO_MERCI; i++) {
        fprintf(fp, "Tipo merce %d:\n", i);
        fprintf(fp, "\t- Non scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", (arr + i)->goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", (arr + i)->goods_on_ship);
        fprintf(fp, "\t- Scaduta:\n");
        fprintf(fp, "\t\ta) nei porti: %d\n", (arr + i)->expired_goods_on_port);
        fprintf(fp, "\t\tb) in nave: %d\n", (arr + i)->expired_goods_on_ship);
        fprintf(fp, "\t- Consegnata: %d\n" ,  (arr + i)->delivered_goods);
        
        sum += arr[i].delivered_goods + arr[i].goods_on_port + arr[i].goods_on_ship + arr[i].expired_goods_on_ship + arr[i].expired_goods_on_port;
    }
    if (day == SO_DAYS) {
        for (i = 0; i < SO_PORTI; i++){
            fprintf(fp, "Porto %d:\n",i);
            fprintf(fp, "DOMANDE:\n");
            for (c = 0; c < SO_MERCI; c++) {
                fprintf(fp, "%d, \n", portArr[i].requests[c]);
            }

            s = portArr[i].supplies;

            fprintf(fp, "SUPPLIES:\n");
            for (c = 0; c < SO_DAYS; c++) {
                
                fprintf(fp,"GIORNO %d: [ ", c);
                for (k = 0; k < SO_MERCI; k++) {
                    fprintf(fp, "%d, ", s.magazine[c][k]);
                }
                fprintf(fp, "]\n");
            }

            fprintf(fp, "EXP TIMES:\n[");
            
            for (c = 0; c < SO_DAYS * SO_MERCI; c++) {
                fprintf(fp, "%d, ", s.expirationTimes[c]);
            }
            fprintf(fp,"]\n");
            fprintf(fp,"--------------------------------------\n");


            fprintf(fp,"coords:\n");
            fprintf(fp,"x: %f\n", portArr[i].x);
            fprintf(fp,"y: %f\n", portArr[i].y);

            fprintf(fp,"______________________________________________\n");
        }
            fprintf(fp, "TOTALE MERCE: %d <==> SO_FILL: %d\n", sum, SO_FILL);
        if (sum == SO_FILL) {
            fprintf(fp, "✅");
        }
        else {
            fprintf(fp, "❌");   
        }
        
    }

    fclose(fp);
    mutex(logFileSemID, UNLOCK, errorHandler , "printerCode UNLOCK");
    shmDetach(arr, errorHandler , "printerCode");
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
