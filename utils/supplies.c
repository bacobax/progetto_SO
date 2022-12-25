#include "supplies.h"
#include "../config1.h"
#include "./support.h"
#include "../src/dump.h"
#include "./sem_utility.h"
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
        addNotExpiredGood(supplies[i], i, PORT);
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
double getValue(int quantity, int scadenza) {
    return quantity / (double)scadenza;
}

int trovaTipoEScadenza(Supplies* S, int* tipo, int* dayTrovato, int* scadenza, int quantity) {
    int i;
    int j;
    /*
    il valore dev'essere di tipo double perchè sennò le volte che 0 < quantity/scadenza < 1,
    cioè quando il tempo di vita rimanente è maggiore della quantità, value diventa 0
    */
    double value = 0;
    int ton;
    double currentValue;
    int currentScadenza;
    int res;
    int semid;
    semid = useSem(RESPRINTKEY, errorHandler);

    
    *tipo = -1;
    *scadenza = -1;
    *dayTrovato = -1;
    mutex(semid, LOCK, NULL);
    printf("✋🏼✋🏼✋🏼✋🏼✋🏼✋🏼✋🏼✋🏼✋🏼✋🏼Valori della merce:\n");
    for (i = 0; i < SO_DAYS; i++) {
        for (j = 0; j < SO_MERCI; j++) {
            
            ton = S->magazine[i][j];
            
            currentScadenza = getExpirationTime(*S, j, i);
            currentValue = getValue(ton, currentScadenza);
            printf("👾%f, ", currentValue);
            if (ton >= quantity && currentValue > value) {
                value = currentValue;
                *tipo = j;
                *dayTrovato = i;
                *scadenza = currentScadenza;
            }
        }
        printf("\n");
        
    }
    mutex(semid, UNLOCK, NULL);

    if (*tipo == -1 && *scadenza == -1 && *dayTrovato == -1) {
        res = -1;
    }
    else {
        res = 1;
    }
    return res;

}

