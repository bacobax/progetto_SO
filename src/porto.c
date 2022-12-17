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






//TODO: Bisogna fare la fork() e creare un processo "filler" che fa una costante recv e che non appena riceve il messaggio dal master che deve refillare, lo fa

int main(int argc, char const* argv[]) {
    int supplyDisponibility;
    int requestDisponibility;
    void (*oldHandler)(int);
    int idx;
    Port p;
    struct timespec tim, tim2;

    srand(time(NULL));
    
    oldHandler = signal(SIGUSR1, quitSignalHandler);
    if (oldHandler == SIG_ERR) {
        perror("signal");
        exit(1);
    }


    supplyDisponibility = atoi(argv[1]);
    requestDisponibility = atoi(argv[2]);
    idx = atoi(argv[3]);

    p = initPort(supplyDisponibility,requestDisponibility, idx);

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


