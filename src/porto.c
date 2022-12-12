#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include <sys/types.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./porto.h"


/*copia il contenuto di un array in un altro array
  assumendo ovviamente che a.length >= a1.length */
void copyArray(int a[], int* a1, int length) {
    int i;
    for (i = 0; i < length; i++) {
        a[i] = a1[i];
    }
}

Port initPort(int disponibility, int pIndex) {

    int portShmId;
    int length;
    Port p;
    int* requests;
    int* supplies;
    int i;
    srand(time(NULL));
    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);

    p = ((Port)getShmAddress(portShmId, 0, errorHandler)) + pIndex;


    requests = toArray(distribute(disponibility, SO_MERCI), &length);
    supplies = toArray(distribute(disponibility, SO_MERCI), &length);

    copyArray(p->requests, requests, length);
    copyArray(p->supplies, supplies, length);

    for (i = 0; i < SO_MERCI; i++) {
        int c = rand() % 2;
        if (c == 1) {
            p->requests[i] = 0;
        }
        else if (c == 0) {
            p->supplies[i] = 0;
        }
        else {
            printf("Errore nella rand");
            exit(EXIT_FAILURE);
        }
    }


    if (pIndex == 0) {
        p->x = 0;
        p->y = 0;
    }
    else if (pIndex == 1) {
        p->x = SO_LATO;
        p->y = 0;
    }
    else if (pIndex == 2) {
        p->x = SO_LATO;
        p->y = SO_LATO;
    }
    else if (pIndex == 3) {
        p->x = 0;
        p->y = SO_LATO;
    }
    else {
        p->x = (double)rand() / (double)(RAND_MAX / (SO_LATO - 1));
        p->y = (double)rand() / (double)(RAND_MAX / (SO_LATO - 1));
    }



    return p;
}



void printPorto(void* p, int idx) {

    int i;
    printf("Porto %d:\n", idx);
    printf("DOMANDE:\n");
    for (i = 0; i < SO_MERCI; i++) {
        printf("%d, \n", ((Port)p)->requests[i]);
    }

    printf("OFFERTE:\n");
    for (i = 0; i < SO_MERCI; i++) {
        printf("%d, \n", ((Port)p)->supplies[i]);
    }

    printf("coords:\n");
    printf("x: %f\n", ((Port)p)->x);
    printf("y: %f\n", ((Port)p)->y);

    printf("______________________________________________\n");

}



int main(int argc, char const* argv[]) {
    int disponibility;
    void (*oldHandler)(int);
    int idx;
    Port p;
    struct timespec tim, tim2;

    oldHandler = signal(SIGUSR1, quitSignalHandler);
    if (oldHandler == SIG_ERR) {
        perror("signal");
        exit(1);
    }


    disponibility = atoi(argv[1]);
    idx = atoi(argv[2]);

    p = initPort(disponibility, idx);

    reservePrint(printPorto, p, idx);


    waitForStart();


    /* START */

    tim.tv_sec = 1;
    tim.tv_nsec = 0;

    while (1) {
        printf("Porto %d: dormo\n", idx);
        nanosleep(&tim, NULL);
    }


    return 1;

}


