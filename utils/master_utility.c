#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "../src/porto.h"
#include "../src/nave.h"
#include "../config1.h"
#include "./vettoriInt.h"
#include "./support.h"
#include "./shm_utility.h"
#include "./sem_utility.h"
#include "./msg_utility.h"

void genera_navi() {
    int i;
    int pid;
    for (i = 0; i < 2; i++) {  /* provo a creare due navi*/
        pid = fork();
        if (pid == 0) {

            char s[50];
            sprintf(s, "%d", i);

            char* argv[] = {s, NULL};
            execve("./bin/nave", argv, NULL);

            exit(EXIT_FAILURE);
        }
        else if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
}

void genera_porti(int risorse, int n_porti) {

    intList* quantiesSupplies;
    intList* quantiesRequests;
    
    int i;
    int pid;
    int* quantity;/*Solo perchè elementAt ritorna un puntatore a intero (perchè almeno in caso di errore ritorna NULL)*/
    char strQuantitySupply[50];
    char strQuantityRequest[50];
    char strIdx[50];
    
    quantiesSupplies = distribute(risorse, n_porti);
    quantiesRequests = distribute(SO_FILL, n_porti);
    
    for (i = 0; i < n_porti; i++) {
        pid = fork();
        if (pid == 0) {

            quantity = (int*)malloc(sizeof(int));
            
            quantity = intElementAt(quantiesSupplies, i);
            sprintf(strQuantitySupply, "%d", *quantity);
            
            quantity = intElementAt(quantiesRequests, i);
            sprintf(strQuantityRequest, "%d", *quantity);
            
            free(quantity);


            sprintf(strIdx, "%d", i);


            char* temp[] = { "porto",strQuantitySupply, strQuantityRequest, strIdx , NULL };

            execve("./bin/porto", temp, NULL);

            perror("execve");
            exit(EXIT_FAILURE);
        }
        else if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        printf("Generato porto %d\n", i);
    }
    printf("M: libero la lista\n"); /*! da fixare, come mai non la stampa??? */
    intFreeList(quantiesSupplies);

}

void wait_all(int n_px) {
    int pid;
    int i;
    for (i = 0; i < n_px; i++) {
        pid = wait(NULL);
    }
}


void mastersighandler(int s) {
    printf("Non faccio nulla\n");
    return;
}

void aspettaConfigs(int waitConfigSemID) {
    mutex(waitConfigSemID, WAITZERO, errorHandler);
}


void mySettedMain(void (*codiceMaster)(int semid, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID, int msgRefillerID)) {
    int semid;
    int reservePrintSem;
    int reservePortsResourceSem;
    int portsShmid;
    int shipsShmid;
    int semBanchineID;
    int msgRefillerID;
    int waitconfigSemID;
    int rwExpTimesPortSemID;

    srand(time(NULL));

    if (signal(SIGUSR1, mastersighandler) == SIG_ERR) {
        perror("signal\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGALRM, mastersighandler) == SIG_ERR) {
        perror("signal\n");
        exit(EXIT_FAILURE);
    }


    semid = createSem(MASTKEY, 1, NULL);
    reservePrintSem = createSem(RESPRINTKEY, 1, NULL);

    /*
    !dovrà essere SO_PORTI + SO_NAVI
    */
    waitconfigSemID = createSem(WAITCONFIGKEY, SO_PORTI, errorHandler);
    
    portsShmid = createShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);
    shipsShmid = createShm(SSHMKEY, SO_NAVI * sizeof(struct ship), errorHandler);


    /*creazione banchine*/
    semBanchineID = createMultipleSem(BANCHINESEMKY, SO_PORTI, SO_BANCHINE, errorHandler);


    if (portsShmid == EEXIST || shipsShmid == EEXIST) {
        perror("Le shm esistono già\n");
        exit(EXIT_FAILURE);
    }
    /*il codice del master manco la usa*/
    msgRefillerID = createQueue(REFILLERQUEUE, errorHandler);

    reservePortsResourceSem = createMultipleSem(RESPORTSBUFFERS, SO_PORTI, 1, errorHandler);

    rwExpTimesPortSemID = createMultipleSem(WREXPTIMESSEM, SO_PORTI, 1, errorHandler);


    
    codiceMaster(semid, portsShmid, shipsShmid, reservePrintSem, waitconfigSemID, msgRefillerID);



    kill(0, SIGUSR1); /* uccide tutti i figli */

    removeSem(semid, errorHandler);
    removeSem(reservePrintSem, errorHandler);
    removeSem(semBanchineID, errorHandler);
    removeSem(reservePortsResourceSem, errorHandler);
    removeSem(waitconfigSemID, errorHandler);
    removeSem(rwExpTimesPortSemID, errorHandler);
    
    removeShm(shipsShmid, errorHandler);
    removeShm(portsShmid, errorHandler);

    removeQueue(msgRefillerID, errorHandler);
    printf("Ciao");

}

void refillCode(intList* l, int msgRefillerID, int giorno) {
    int i;
    long type;
    char supportText[MEXBSIZE];
    for (i = 0; i < l->length; i++) {
        sprintf(supportText, "%d|%d", giorno, *(intElementAt(l, i)));
        type = i+1;
        printf("Invio messaggio alla coda %d con il seguente testo: %s con tipo %ld\n", msgRefillerID, supportText, type);
        //Invio messaggio alla coda 458752 con il seguente testo: 0|20 con tipo 0
        msgSend(msgRefillerID, supportText, type, NULL);
    }

}

void refillPorts(int opt, int msgRefillerID, int quantitaAlGiorno, int giorno) {
    intList* l;
    
    int pid;
    l = distribute(quantitaAlGiorno, SO_PORTI);
    if (opt == SYNC) {
        refillCode(l, msgRefillerID, giorno);
        intFreeList(l);
    }
    else if (opt == ASYNC) {
        pid = fork();
        if (pid == -1) {
            perror("Errore nella fork in refillPorts\n");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            refillCode(l, msgRefillerID, giorno);
            intFreeList(l);
            exit(EXIT_SUCCESS);
        }
        else {
            /*
                Al padre non servono
            */
            intFreeList(l);
        }
    }
    else {
        perror("refillPorts: Inserire SYNC o ASYNC (0 o 1) come opt\n");
        exit(EXIT_FAILURE);

    }

}

