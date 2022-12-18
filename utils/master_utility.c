#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "../src/porto.h"
#include "../config1.h"
#include "./vettoriInt.h"
#include "./support.h"
#include "./shm_utility.h"
#include "./sem_utility.h"
#include "./msg_utility.h"
void genera_navi() {
    int i;
    int pid;
    for (i = 0; i < SO_NAVI; i++) {
        pid = fork();
        if (pid == 0) {
            /*
                da passare le coordinate
            */

            execve("./bin/nave", NULL, NULL);
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
    printf("M: libero la lista"); /*! da fixare, come mai non la stampa??? */
    intFreeList(quantiesSupplies);

}

void wait_all(int n_px) {
    int pid;
    int i;
    for (i = 0; i < n_px; i++) {
        pid = wait(NULL);
    }
}


void sigusr1sigHandler(int s) {
    printf("Non faccio nulla\n");
    return;
}

void aspettaConfigs(int waitConfigSemID) {
    mutex(waitConfigSemID, WAITZERO, errorHandler);
}


void mySettedMain(void (*codiceMaster)(int semid, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID)) {
    int semid;
    int reservePrintSem;
    int reservePortsResourceSem;
    int portsShmid;
    int shipsShmid;
    int semBanchineID;
    int msgRefillerID;
    int waitconfigSemID;
    
    srand(time(NULL));

    if (signal(SIGUSR1, sigusr1sigHandler) == SIG_ERR) {
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
    /*shipsShmid = createShm(SSHMKEY, SO_NAVI * sizeof(struct ship), errorHandler);*/


    /*creazione banchine*/
    semBanchineID = createMultipleSem(BANCHINESEMKY, SO_PORTI, SO_BANCHINE, errorHandler);


    if (portsShmid == EEXIST || shipsShmid == EEXIST) {
        perror("Le shm esistono già\n");
        exit(EXIT_FAILURE);
    }
    /*il codice del master manco la usa*/
    msgRefillerID = createQueue(REFILLERQUEUE, errorHandler);

    reservePortsResourceSem = createMultipleSem(RESPORTSBUFFERS, SO_PORTI, 1, errorHandler);

    codiceMaster(semid, portsShmid, shipsShmid, reservePrintSem, waitconfigSemID);


    kill(0, SIGUSR1); /* uccide tutti i figli */

    removeSem(semid, errorHandler);
    removeSem(reservePrintSem, errorHandler);
    removeSem(semBanchineID, errorHandler);
    removeSem(reservePortsResourceSem, errorHandler);
    removeSem(waitconfigSemID, errorHandler);
    
    /*removeShm(shipsShmid, errorHandler);*/
    removeShm(portsShmid, errorHandler);

    removeQueue(msgRefillerID, errorHandler);
    printf("Ciao");

}
