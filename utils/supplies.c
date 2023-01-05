#include "supplies.h"
#include "../config1.h"
#include "./support.h"
#include "../src/dump.h"
#include "../src/porto.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include <stdio.h>
#include <stdlib.h>

void fillExpirationTime(Supplies* S) {
    int i;
    for (i = 0; i < SO_MERCI * SO_DAYS; i++) {
        S->expirationTimes[i] = random_int(SO_MIN_VITA, SO_MAX_VITA);
    }
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


void printSupplies(Supplies s) {
    int i;
    int j;
    printf("SUPPLIES:\n");
    for (i = 0; i < SO_DAYS; i++) {
        
        printf("GIORNO %d: [ ", i);
        for (j = 0; j < SO_MERCI; j++) {
            printf("%d, ", s.magazine[i][j]);
        }
        printf("]\n");
    }

    printf("EXP TIMES:\n[");
    
    for (i = 0; i < SO_DAYS * SO_MERCI; i++) {
        printf("%d, ", s.expirationTimes[i]);
    }
    printf("]\n");
    printf("--------------------------------------\n");

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
