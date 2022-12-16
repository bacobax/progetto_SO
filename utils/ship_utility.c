#include <stdio.h>
#include <stdlib.h>
#include "../src/nave.h"
#include "../src/porto.h"
#include "../utils/loadShip.h"
#include "../config1.h"
#include "./support.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


int checkCapacity(Ship ship) {
    if (ship->load->weightLoad == 0) return 0;
    return ship->load->weightLoad;
}

int availableCapacity(Ship ship) {

    int currentCapacity;

    currentCapacity = checkCapacity(ship);
    if (currentCapacity < 0) return -1;
    return (SO_CAPACITY - currentCapacity);
}

double generateCord() {

    double range, div;

    range = (SO_LATO); /* max-min */
    div = RAND_MAX / range;
    return (rand() / div);
}

Ship initShip() {

    Ship ship;

    if (signal(SIGUSR1, quitSignalHandler) == SIG_ERR) {              /* imposto l'handler per la signal SIGUSR1 */
        perror("Error trying to set a signal handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }

    /* inizializziamo la nave */
    ship = (struct ship*)malloc(sizeof(struct ship));
    ship->x = generateCord();
    ship->y = generateCord();
    ship->capacity = 0;
    /*
        load sarà NULL all'inizio
    */
    ship->load = initLoadShip();

    return ship;
}

void printShip(void* ship, int id_ship){
    printf("[%d]: Nave\n", id_ship);
    
    printf("coords: [x:%f, y:%f]\n", (((Ship)ship)->x), (((Ship)ship)->y));
    
    printf("ton trasporati:%d\n", availableCapacity((Ship) ship));

    printf("carico trasportato:\n");
    printLoadShip(((Ship)ship)->load);
    
    printf("______________________________________________\n");

}

void travel(Ship ship, int portID){

    Port p;
    double dt_x, dt_y, result, nanosleep_arg;
    struct timespec arg;

    int portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler); /* prendo l'id della shm del porto */

    p = ((Port) getShmAddress(portShmId, 0, errorHandler)) + portID; /* prelevo la struttura del porto alla portID-esima posizione nella shm */

    /* imposto la formula per il calcolo della distanza*/
    
    dt_x = p->x - ship->x;
    dt_y = p->y - ship->y;

    result = pow(dt_x, 2) + pow(dt_y, 2);

    arg.tv_sec = (time_t) (sqrt(result)) / SO_SPEED;
    arg.tv_nsec = 0;

    nanosleep(&arg, NULL);

    /* Dopo aver fatto la nanosleep la nave si trova esattamente sulle coordinate del porto
       quindi aggiorniamo le sue coordinate
    */

    ship->x = p->x;
    ship->y = p->y;
}
