#ifndef NAVE_H
#define NAVE_H


#include "../utils/loadShip.h"

#define SHIPSHMKEY 8080

struct ship {
    double cords[2];
    int capacity;
    loadShip load;
};
typedef struct ship* Ship;


int checkCapacity(Ship ship); /* ritorna il numero di ton presenti sulla nave */

int availableCapacity(Ship ship); /* ritorna il numero di ton disponibili sulla nave */

double generateCord(); /* genere una coordinata double */

Ship initShip();

void printShip();

#endif
