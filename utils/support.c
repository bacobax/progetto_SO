#include "./vettoriInt.h"
#include "./sem_utility.h"
#include "../config1.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "./shm_utility.h"

#include "../src/master.h"
#include "../src/porto.h"

void ErrorHandler(int err) {
    perror("reservePrint->useSem");
}

void quitSignalHandler(int sig) {
    printf("Porto: ricevuto segnale di terminazione\n");
    exit(EXIT_SUCCESS);
}


intList* distribute(int quantity, int parts) {



    //per ciascuna parte, tranne l'ultima vale:

    //massimo: la quantità che gli verrebbe assegnata se le quantità fossero distribuite in parti uguali
    //questo perchè nel peggiore dei casi (in cui a tutte le quantita venga assegnato il massimo) la quantità totale <= quantity
    int max_q = quantity / parts;

    //minimo: la metà della quantità che gli verrebbe distribuita se le quantità fossero distribuite in parti uguali
    int min_q = quantity / parts / 2;
    intList* l = intInit();
    for (int i = 0; i < parts - 1; i++) {
        int random_q = rand() % max_q + min_q;
        intPush(l, random_q);
    }

    //per l'ultima quantità viene assegnata la quantità restate non ancora assegnata
    //questo per essere sicuro che la somma delle quantità sia = quantity
    int last_q = quantity - sum(l);
    intPush(l, last_q);
    return l;
}


void reservePrint(void (*printer)(void* obj, int idx), void* object, int idx) {
    int semid = useSem(RESPRINTKEY, ErrorHandler);

    mutex(semid, LOCK, NULL);

    printer(object, idx);

    mutex(semid, UNLOCK, NULL);
}


void sigusr1sigHandler(int s) {
    printf("Non faccio nulla\n");
    return;
}


void mySettedMain(void (*codiceMaster)(int semid, int portsShmid, int shipsShmid, int reservePrintSem)) {
    if (signal(SIGUSR1, sigusr1sigHandler) == SIG_ERR) {
        perror("signal\n");
        exit(EXIT_FAILURE);
    }


    int semid = createSem(MASTKEY, 1, NULL);
    int reservePrintSem = createSem(RESPRINTKEY, 1, NULL);


    if (semid == EEXIST) {
        semid = useSem(MASTKEY, NULL);
    }

    int portsShmid = createShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);
    int shipsShmid = createShm(SSHMKEY, SO_NAVI * sizeof(struct ship), errorHandler);

    if (portsShmid == EEXIST || shipsShmid == EEXIST) {
        perror("Le shm esistono già\n");
        exit(EXIT_FAILURE);
    }


    codiceMaster(semid, portsShmid, shipsShmid, reservePrintSem);


    kill(0, SIGUSR1); //uccide tutti i figli

    removeSem(semid, errorHandler);
    removeSem(reservePrintSem, errorHandler);

    removeShm(shipsShmid, errorHandler);
    removeShm(portsShmid, errorHandler);
    printf("Ciao");

}