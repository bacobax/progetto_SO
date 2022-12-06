#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config1.h"
#include "./utils/sem_utility.h"
#include "./utils/support.h"
#include "./utils/vettoriInt.h"

void genera_navi() {
    for (int i = 0; i < SO_NAVI; i++) {
        int pid = fork();
        if (pid == 0) {
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
            int quantity = intElementAt(quanties, i);
            char strQuantity[50];
            sprintf(strQuantity, "%d", quantity);

            char* temp[] = { "porto",strQuantity,NULL };

            execve("./bin/porto", temp, NULL);

            intFreeList(quanties);
            exit(EXIT_FAILURE);
        }
        else if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char const* argv[]) {

    int semid = createSem(MASTKEY, 1, NULL);

    genera_navi();

    genera_porti(SO_FILL, SO_PORTI);



    mutex(semid, LOCK, errorHandler);
    //TODO: Aggiungere removeSem alle funzioni dei semfaori

    printf("Ciao");
    return 0;
}
