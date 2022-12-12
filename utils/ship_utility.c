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
    int currentCapacity = checkCapacity(ship);
    if (currentCapacity < 0) return -1;
    return (SO_CAPACITY - currentCapacity);
}

double generateCord() {
    double range = (SO_LATO - 0); /* max-min */
    double div = RAND_MAX / range;
    return 0 + (rand() / div);
}
