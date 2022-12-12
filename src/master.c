#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./master.h"
#include "./porto.h"


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

    intList* quanties = distribute(risorse, n_porti);
    int i;
    int pid;
    int* quantity;/*Solo perchè elementAt ritorna un puntatore a intero (perchè almeno in caso di errore ritorna NULL)*/
    char strQuantity[50];
    char strIdx[50];
    for (i = 0; i < n_porti; i++) {
        pid = fork();
        if (pid == 0) {

            quantity = (int*)malloc(sizeof(int));
            quantity = intElementAt(quanties, i);

            sprintf(strQuantity, "%d", *quantity);
            free(quantity);


            sprintf(strIdx, "%d", i);


            char* temp[] = { "porto",strQuantity, strIdx, NULL };

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
    intFreeList(quanties);

}

void wait_all(int n_px) {
    int pid;
    int i;
    for (i = 0; i < n_px; i++) {
        pid = wait(NULL);
    }
}


void codiceMaster(int semid, int portsShmid, int shipsShmid, int reservePrintSem) {
    struct timespec tim, tim2;
    int i;

    /*  per ora ho usato solo semid */
    genera_porti(SO_FILL, SO_PORTI); /* da tradurre in inglese */


    mutex(semid, LOCK, errorHandler);


    /*
    genera_navi()
    mutex(semid, LOCK, errorHandler);
    */

    printf("Master: ciao\n");

    tim.tv_sec = 1;
    tim.tv_nsec = 0;
    for (i = 0; i < SO_DAYS; i++) {
        printf("Master: dormo\n");

        /* TODO: funzione dump */
        nanosleep(&tim, &tim2);
    }
}

int main(int argc, char const* argv[]) {
    mySettedMain(codiceMaster);
    return 0;
}
