#include <stdio.h>
#include <stdlib.h>
#include "../src/nave.h"
#include "../utils/loadShip.h"
#include "../config1.h"
#include <stdio.h>
#include <stdlib.h>


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

    range = (SO_LATO - 0); /* max-min */
    div = RAND_MAX / range;
    return 0 + (rand() / div);
}

Ship initShip() {

    Ship ship;

    if (signal(SIGUSR1, shipSignalHandler) == SIG_ERR) {              /* imposto l'handler per la signal SIGUSR1 */
        perror("Error trying to set a signal handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }

    /* inizializziamo la nave */
    ship = (struct ship*)malloc(sizeof(struct ship));
    ship->cords[0] = generateCord();
    ship->cords[1] = generateCord();
    ship->capacity = 0;
    /*
        load sarà NULL all'inizio
    */
    ship->load = initLoadShip();

    return ship;
}

void printShip(void* ship, int id_ship){
    printf("[%d]: Nave\n", id_ship);
    
    printf("coords: [x:%f, y:%f]\n", ((Ship)ship->cords[0]), ((Ship)ship->cords[1]));
    
    printf("ton trasporati:%d\n", availableCapacity((Ship) ship));

    printf("carico trasportato:\n");
    printLoadShip((Ship)ship->load);
    
    printf("______________________________________________\n");

}
