#include "supplies.h"
#include "../config1.h"
#include "./support.h"
#include "../src/dump.h"
#include "../src/porto.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include "./errorHandler.h"
#include <stdio.h>
#include <stdlib.h>

void fillExpirationTime(Supplies* S) {
    int i;
    double mean;
    int dumpShmID;
    DumpArea* dump;
    int so_max_vita;
    int so_min_vita;
    int so_days;
    int so_merci;
    int* expTimes;
    
    so_max_vita = SO_("MAX_VITA");
    so_min_vita = SO_("MIN_VITA");
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");

    expTimes = (int*)getShmAddress(S->expirationTimesID, 0, errorHandler, "fillExpirationTime expTimes");
    
    dumpShmID = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "fillExpirationTime");
    dump = (DumpArea*)getShmAddress(dumpShmID, 0, errorHandler, "fillExpirationTime");

    mean = ((double)(so_min_vita + so_max_vita)) / 2;
    for (i = 0; i < so_merci * so_days; i++) {
        expTimes[i] = random_int(so_min_vita, so_max_vita);
        dump->expTimeVariance += mod(((double)expTimes[i]) - mean);
    }
    shmDetach(expTimes, errorHandler, "fillExpirationTime expTimes");
    shmDetach(dump, errorHandler, "fillExpirationTime");
}

void fillMagazine(int* magazine, int day, int* supplies) {
    int i;
    int dumpShmid;
    DumpArea* dump;
    int so_loadspeed;
    int so_merci = SO_("MERCI");
    so_loadspeed = SO_("LOADSPEED");
    dumpShmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "fillMagazine");
    dump = (DumpArea*)getShmAddress(dumpShmid, 0, errorHandler, "fillMagazine");
    for (i = 0; i < so_merci; i++) {
        setMagazineVal(magazine, day, i, supplies[i]);
        dump->tempoScaricamentoTot += ((double)(supplies[i])) / so_loadspeed;
        addNotExpiredGood(supplies[i], i, PORT, 1, 0);
    }
    shmDetach(dump, errorHandler, "fillMagazine");
}



int getExpirationTime(Supplies S, int tipoMerce, int giornoDistribuzione) {
    int* expTimes;
    int ret;
    int so_merci = SO_("MERCI");
    expTimes = (int*)getShmAddress(S.expirationTimesID, 0, errorHandler, "getExpirationTime");
    ret = expTimes[(giornoDistribuzione * so_merci) + tipoMerce];
    shmDetach(expTimes, errorHandler, "getExpirationTime");
    return ret;
}


void printSupplies(Supplies s, FILE* stream, int* magazine) {
    int i;
    int j;
    int so_days;
    int so_merci;
    int* expTimes;
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");
    fprintf(stream, "SUPPLIES:\n");
    for (i = 0; i < so_days; i++) {
        
        fprintf(stream,"GIORNO %d: [ ", i);
        for (j = 0; j < so_merci; j++) {
            fprintf(stream,"%d, ", getMagazineVal(magazine,i,j));
        }
        fprintf(stream,"]\n");
    }

    fprintf(stream,"EXP TIMES:\n[");


    expTimes = (int*)getShmAddress(s.expirationTimesID, 0, errorHandler, "printSupplies");
    for (i = 0; i < so_days * so_merci; i++) {
        fprintf(stream,"%d, ", expTimes[i]);
    }
    shmDetach(expTimes, errorHandler, "printSupplies expTimes");
    fprintf(stream, "]\n");
    fprintf(stream,"--------------------------------------\n");

}


void decrementExpTimes(Supplies* S, int day) {
    int i;
    int so_days;
    int so_merci;
    int* expTimes = (int*)getShmAddress(S->expirationTimesID,0,errorHandler, "decrementExpTimes");
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");
    for (i = 0; i < so_merci * so_days; i++) {
        if (i < so_merci * day && expTimes[i] > 0 ) {
            expTimes[i] --;
        }
    }
    shmDetach(expTimes, errorHandler, "decrementExpTimes");
}


void removeExpiredGoods(Supplies* S, int* magazine) {
    int i;
    int so_merci;
    int so_days;
    int* expTimes;
    expTimes = (int*)getShmAddress(S->expirationTimesID, 0, errorHandler, "removeExpiredGoods");

    so_merci = SO_("MERCI");
    so_days = SO_("DAYS");
    for (i = 0; i < so_merci * so_days; i++) {
        
        if (expTimes== 0 && getMagazineVal(magazine,i/so_merci,i%so_merci) > 0) {
            addExpiredGood(getMagazineVal(magazine, i / so_merci, i % so_merci), i % so_merci, PORT);
            setMagazineVal(magazine, i / so_merci, i % so_merci, 0);
        }
    }
    shmDetach(expTimes, errorHandler, "removeExpiredGoods");
}

int validSupply(Supplies s, int type, int* magazine){
    
    int i;
    int j;
    int so_days = SO_("DAYS");
    for (i = 0; i < so_days; i++) {
        if (getMagazineVal(magazine, i, type) > 0) {
            return 1;
        }
    }
    return 0;
}


int getMagazineVal(int* magazine, int day, int type) {
    
    return magazine[SO_("MERCI") * day + type];
}

void setMagazineVal(int* magazine, int day, int type, int v) {
    magazine[SO_("MERCI") * day + type] = v;
}

void addMagazineVal(int* magazine, int day, int type, int v) {
    magazine[SO_("MERCI") * day + type] += v;
}
