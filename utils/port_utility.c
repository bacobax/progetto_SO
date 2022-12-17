#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "../src/porto.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include "./msg_utility.h"
#include "./support.h"
#include "./vettoriInt.h"

void refillerQuitHandler(int sig) {
    printf("refiller: ricevuto segnale di terminazione\n");
    exit(EXIT_SUCCESS);
}


Port initPort(int supplyDisponibility,int requestDisponibility, int pIndex) {

    int portShmId;
    int length;
    Port p;
    int* requests;
    int* supplies;
    int i;
    int j;
    
    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);

    p = ((Port)getShmAddress(portShmId, 0, errorHandler)) + pIndex;


    requests = toArray(distribute(requestDisponibility, SO_MERCI), &length);
    supplies = toArray(distribute(supplyDisponibility, SO_MERCI), &length);

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

/*
    si assume che i messaggi siano sempre scritti con questo 'pattern': giorno|quantita
*/
void mexParse(const char* mex, int* intDay, int* intQuantity) {
    int sizeDay;
    int sizeQuantity;
    int i;
    int c;
    int j;
    for (i = 0; *(mex + i); i++) {
        if(mex[i]=='|'){
            sizeDay = i;
            break;
        }
    }
    printf("Lunghezza stringa giorno: %d\n", sizeDay);
    
    c = 0;
    
    for (i = i + 1; i < strlen(mex); i++) {
        c++;
    }
    sizeQuantity = c;
    
    printf("Lunghezza stringa quantità: %d\n", sizeQuantity);
    
    char day[sizeDay]; //"23"
    char quantity[sizeQuantity];//"12"

    for(i=0; i<sizeDay; i++){
        day[i] = mex[i];
    }
    i++;
    for (j = 0; j < sizeQuantity; j++) {
        quantity[j] = mex[i];
        i++;
    }
    
    *intDay = atoi(day);
    *intQuantity = atoi(quantity);

}


/*
    23|32
*/

void refill(long type, char* text) {
    int portBufferSem;
    int day;
    int quantity;
    int portShmID;
    Port p;
    int* quanties;
    int length;
    portShmID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler);
    p = (Port)getShmAddress(portShmID, 0, errorHandler) + type;

    quanties = toArray(distribute(quantity , SO_MERCI), &length);

    

    mexParse(text, &day, &quantity);
    portBufferSem = useSem(RESPORTSBUFFERS, errorHandler);


    
    mutexPro(portBufferSem, (int)type, LOCK, errorHandler);

    fillMagazine(&p->supplies, day, quanties);
    
    mutexPro(portBufferSem, (int)type, UNLOCK, errorHandler);

    shmDetach(p, errorHandler);
}

void refillerCode(int idx) {

    if (signal(SIGUSR1, refillerQuitHandler) == SIG_ERR) {
        perror("Refiller: non riesco a settare il signal handler\n");
        exit(EXIT_FAILURE);
    }

    int refillerID = useQueue(REFILLERQUEUE, errorHandler);

    while (1) {
        msgRecv(refillerID, (long)idx, errorHandler, refill, ASYNC);
    }
}

void launchRefiller(int idx) {
    int pid = fork();

    if (pid == -1) {
        perror("Error launching the refiller");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        refillerCode(idx);
        exit(EXIT_FAILURE);
    }
}