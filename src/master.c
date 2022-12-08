#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./master.h"
#include "./porto.h"


void genera_navi() {
    for (int i = 0; i < SO_NAVI; i++) {
        int pid = fork();
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

    for (int i = 0; i < n_porti; i++) {
        int pid = fork();
        if (pid == 0) {

            int* quantity = (int*)malloc(sizeof(int));
            quantity = intElementAt(quanties, i);

            char strQuantity[50];
            sprintf(strQuantity, "%d", *quantity);
            free(quantity);

            char strIdx[50];
            sprintf(strIdx, "%d", i);


            char* temp[] = { "porto",strQuantity, strIdx, NULL };

            execve("./bin/porto", temp, NULL);

            exit(EXIT_FAILURE);
        }
        else if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
    intFreeList(quanties);

}

void wait_all(int n_px) {
    for (int i = 0; i < n_px; i++) {
        int pid = wait(NULL);
    }
}

int main(int argc, char const* argv[]) {

    int semid = createSem(MASTKEY, 1, NULL);
    int reservePrintSem = createSem(RESPRINTKEY, 1, NULL);


    if (semid == EEXIST) {
        semid = useSem(MASTKEY, NULL);
    }

    int portsShmid = createShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);
    int shipsShmid = createShm(SSHMKEY, SO_NAVI * sizeof(struct ship), errorHandler);

    //genera_navi();




    genera_porti(SO_FILL, SO_PORTI); //da tradurre in inglese


    mutex(semid, LOCK, errorHandler);





    wait_all(SO_PORTI);
    //TODO: Aggiungere removeSem alle funzioni dei semfaori
    removeSem(semid, errorHandler);
    removeSem(reservePrintSem, errorHandler);

    removeShm(shipsShmid, errorHandler);
    removeShm(portsShmid, errorHandler);
    printf("Ciao");
    return 0;
}
