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
    double media;
    int dumpShmID;
    DumpArea* dump;

    dumpShmID = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "fillExpirationTime");
    dump = (DumpArea*)getShmAddress(dumpShmID, 0, errorHandler, "fillExpirationTime");

    media = ((double)(SO_MIN_VITA + SO_MAX_VITA)) / 2;
    for (i = 0; i < SO_MERCI * SO_DAYS; i++) {
        S->expirationTimes[i] = random_int(SO_MIN_VITA, SO_MAX_VITA);
        dump->expTimeVariance += mod(((double)S->expirationTimes[i]) - media);
    }
    shmDetach(dump, errorHandler, "fillExpirationTime");
}

void fillMagazine(Supplies* S, int day, int* supplies) {
    int i;

    for (i = 0; i < SO_MERCI; i++) {
        S->magazine[day][i] = supplies[i];
        addNotExpiredGood(supplies[i], i, PORT, 1,0);
    }

}



int getExpirationTime(Supplies S, int tipoMerce, int giornoDistribuzione) {
    return S.expirationTimes[(giornoDistribuzione * SO_MERCI) + tipoMerce];
}


void printSupplies(Supplies s, FILE* stream) {
    int i;
    int j;
    fprintf(stream,"SUPPLIES:\n");
    for (i = 0; i < SO_DAYS; i++) {
        
        fprintf(stream,"GIORNO %d: [ ", i);
        for (j = 0; j < SO_MERCI; j++) {
            fprintf(stream,"%d, ", s.magazine[i][j]);
        }
        fprintf(stream,"]\n");
    }

    fprintf(stream,"EXP TIMES:\n[");
    
    for (i = 0; i < SO_DAYS * SO_MERCI; i++) {
        fprintf(stream,"%d, ", s.expirationTimes[i]);
    }
    fprintf(stream,"]\n");
    fprintf(stream,"--------------------------------------\n");

}


void decrementExpTimes(Supplies* S, int day) {
    int i;
    for (i = 0; i < SO_MERCI * SO_DAYS; i++) {
        if (i < SO_MERCI * day && S->expirationTimes[i] > 0 ) {
            S->expirationTimes[i] --;
        }
    }
}


void removeExpiredGoods(Supplies* S) {
    int i;
    for (i = 0; i < SO_MERCI * SO_DAYS; i++) {
        if (S->expirationTimes[i] == 0 && S->magazine[i / SO_MERCI][i % SO_MERCI] > 0) {
            addExpiredGood(S->magazine[i / SO_MERCI][i % SO_MERCI], i % SO_MERCI, PORT);
            S->magazine[i / SO_MERCI][i % SO_MERCI] = 0;
        }
    }
}

/*
    in sintesi maggiore è la quantità, maggiore è il valore assegnato
    maggiore è il tempo di vita rimanente, minore è il valore,
    l'ho aggiustata con una divisione
    
*/
