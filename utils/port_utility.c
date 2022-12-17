#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include "../src/porto.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include "./support.h"
#include "./vettoriInt.h"

Port initPort(int disponibility, int pIndex) {

    int portShmId;
    int length;
    Port p;
    int* requests;
    int* supplies;
    int i;
    int j;
    srand(time(NULL));
    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);

    p = ((Port)getShmAddress(portShmId, 0, errorHandler)) + pIndex;


    requests = toArray(distribute(disponibility, SO_MERCI), &length);
    supplies = toArray(distribute(disponibility/SO_DAYS, SO_MERCI), &length);

    copyArray(p->requests, requests, length);
    fillMagazine(&p->supplies, 0, supplies);
    fillExpirationTime(&p->supplies);
    //copyArray(p->supplies, supplies, length);

    for (i = 0; i < SO_MERCI; i++) {
        int c = rand() % 2;
        if (c == 1) {
            p->requests[i] = 0;
        }
        else if (c == 0) {
            p->supplies.magazine[0][i] = 0;
        }
        else {
            printf("Errore nella rand");
            exit(EXIT_FAILURE);
        }
    }

    //*Azzero tutti tipi delle risorse degli altri giorni
    for (i = 1; i < SO_DAYS; i++) {
        for (j = 0; j < SO_MERCI; j++) {
            p->supplies.magazine[i][j] = 0;
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
        p->x = (double)rand() / (double)(RAND_MAX / (SO_LATO));
        p->y = (double)rand() / (double)(RAND_MAX / (SO_LATO));
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

    printSupplies(((Port)p)->supplies);

    printf("coords:\n");
    printf("x: %f\n", ((Port)p)->x);
    printf("y: %f\n", ((Port)p)->y);

    printf("______________________________________________\n");

}
