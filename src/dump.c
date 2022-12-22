#include <stdio.h>
#include <stdlib.h>

#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"
#include "../config1.h"
#include "./dump.h"

void createDumpArea(){
    int shmid;
    int semid;
    shmid = createShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler);
    semid = createMultipleSem(DUMPSEMKEY, SO_MERCI, 1, errorHandler);

}

void addExpiredGood(int quantity, int type, ctx where) {
    int shmid;
    int semid;
    
    GoodTypeInfo* info;
    shmid = useShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler);
    semid = useSem(DUMPSEMKEY, errorHandler);
    
    info = getShmAddress(shmid, 0, errorHandler) + type;

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
    
    info = getShmAddress(shmid, 0, errorHandler) + type;

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
    int semid;
    shmid = createShm(DUMPSHMKEY, SO_MERCI * sizeof(GoodTypeInfo), errorHandler);
    semid = createMultipleSem(DUMPSEMKEY, SO_MERCI, 1, errorHandler);
    removeSem(semid, errorHandler);
    removeShm(shmid, errorHandler);
}