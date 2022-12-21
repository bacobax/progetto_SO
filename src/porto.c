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
#include "../utils/supplies.h"


/*
void updateExpTimeHandler(int sig) {
    if (idx == -1) {
        perror("unexpected error\n");
        exit(EXIT_FAILURE);
    }
        printf("Porto: ricevuto sigalarm\n");

    updateExpTimes(idx);

}
*/

//TODO: Bisogna fare la fork() e creare un processo "filler" che fa una costante recv e che non appena riceve il messaggio dal master che deve refillare, lo fa

int main(int argc, char const* argv[]) {
    int supplyDisponibility;
    int requestDisponibility;
    void (*oldHandler)(int);
    int i;
    Port p;
    int idx;

    /*
        questo perchè per qualche motivo srand(time(NULL)) non generava unici seed tra un processo unico e l'altro
        fonte della soluzione: https://stackoverflow.com/questions/35641747/why-does-each-child-process-generate-the-same-random-number-when-using-rand
        da quel che ho capito il bug era dovuto al fatto che le fork dei vari figli sono avvenute nello stesso secondo
    */
    srand((int)time(NULL) % getpid());

    
    oldHandler = signal(SIGUSR1, quitSignalHandler);
    if (oldHandler == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    /*
        oldHandler = signal(SIGALRM, updateExpTimeHandler);
            if (oldHandler == SIG_ERR) {
                perror("signal");
                exit(1);
            }

    */
    
    


    supplyDisponibility = atoi(argv[1]);
    requestDisponibility = atoi(argv[2]);
    idx = atoi(argv[3]);

    p = initPort(supplyDisponibility,requestDisponibility, idx);


    // shmDetach(p, errorHandler);

    launchRefiller(idx);

    
    waitForStart();

    printf("P: finito configurazione\n");
    checkInConfig();
    /* START */

    
    i = 0;
    while (i<SO_DAYS) {
        printf("Porto %d: dormo\n", idx);

        nanosecsleep(NANOS_MULT);
        i++;
    }


    return 1;

}


