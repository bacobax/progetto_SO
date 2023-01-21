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
    int so_max_vita;
    int so_min_vita;
    int so_days;
    int so_merci;
    so_max_vita = SO_("MAX_VITA");
    so_min_vita = SO_("MIN_VITA");
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");
    dumpShmID = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "fillExpirationTime");
    dump = (DumpArea*)getShmAddress(dumpShmID, 0, errorHandler, "fillExpirationTime");

    media = ((double)(so_min_vita + so_max_vita)) / 2;
    for (i = 0; i < so_merci * so_days; i++) {
        S->expirationTimes[i] = random_int(so_min_vita, so_max_vita);
        dump->expTimeVariance += mod(((double)S->expirationTimes[i]) - media);
    }
    shmDetach(dump, errorHandler, "fillExpirationTime");
}

void fillMagazine(Supplies* S, int day, int* supplies) {
    int i;
    int dumpShmid;
    DumpArea* dump;
    dumpShmid = useShm(DUMPSHMKEY, sizeof(DumpArea), errorHandler, "fillMagazine");
    dump = (DumpArea*)getShmAddress(dumpShmid, 0, errorHandler, "fillMagazine");
    int so_merci = SO_("MERCI");
    int so_loadspeed = SO_("LOADSPEED");
    for (i = 0; i < so_merci; i++) {
        S->magazine[day][i] = supplies[i];
        dump->tempoScaricamentoTot += ((double)(S->magazine[day][i])) / so_loadspeed;
        addNotExpiredGood(supplies[i], i, PORT, 1, 0);
    }
    shmDetach(dump, errorHandler, "fillMagazine");
}



int getExpirationTime(Supplies S, int tipoMerce, int giornoDistribuzione) {
    int so_merci = SO_("MERCI");
    return S.expirationTimes[(giornoDistribuzione * so_merci) + tipoMerce];
}


void printSupplies(Supplies s, FILE* stream) {
    int i;
    int j;
    int so_days;
    int so_merci;
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");
    fprintf(stream, "SUPPLIES:\n");
    for (i = 0; i < so_days; i++) {
        
        fprintf(stream,"GIORNO %d: [ ", i);
        for (j = 0; j < so_merci; j++) {
            fprintf(stream,"%d, ", s.magazine[i][j]);
        }
        fprintf(stream,"]\n");
    }

    fprintf(stream,"EXP TIMES:\n[");
    
    for (i = 0; i < so_days * so_merci; i++) {
        fprintf(stream,"%d, ", s.expirationTimes[i]);
    }
    fprintf(stream,"]\n");
    fprintf(stream,"--------------------------------------\n");

}


void decrementExpTimes(Supplies* S, int day) {
    int i;
    int so_days;
    int so_merci;
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");
    for (i = 0; i < so_merci * so_days; i++) {
        if (i < so_merci * day && S->expirationTimes[i] > 0 ) {
            S->expirationTimes[i] --;
        }
    }
}


void removeExpiredGoods(Supplies* S) {
    int i;
    int so_merci;
    int so_days;
    so_merci = SO_("MERCI");
    so_days = SO_("DAYS");
    for (i = 0; i < so_merci * so_days; i++) {
        if (S->expirationTimes[i] == 0 && S->magazine[i / so_merci][i % so_merci] > 0) {
            addExpiredGood(S->magazine[i / so_merci][i % so_merci], i % so_merci, PORT);
            S->magazine[i / so_merci][i % so_merci] = 0;
        }
    }
}

/*
    in sintesi maggiore è la quantità, maggiore è il valore assegnato
    maggiore è il tempo di vita rimanente, minore è il valore,
    l'ho aggiustata con una divisione
    
*/

int validSupply(Supplies s, int type){
    /* ritorna true se c'è almeno un elemento maggiore di 0 in p.megazine[tutti i giorni][type]
        int magazine[SO_DAYS][SO_MERCI];
    */
    int i;
    int j;
    int so_days = SO_("DAYS");
    for (i = 0; i < so_days; i++) {
        if (s.magazine[i][type] > 0) {
            return 1;
        }
    }
    return 0;
}
