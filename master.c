#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <config1.h>
#include "./utils/sem_utility.h"

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


void genera_porti() {
    for (int i = 0; i < SO_PORTI; i++) {
        int pid = fork();
        if (pid == 0) {
            execve("./bin/porto", NULL, NULL);
            exit(EXIT_FAILURE);
        }
        else if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char const* argv[])
{

    int semid = createSem(MASTKEY, 1, NULL);

    genera_navi();

    genera_porti();

    //TODO: Aggiungere removeSem alle funzioni dei semfaori

    return 0;
}
