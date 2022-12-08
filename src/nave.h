#ifndef NAVE_H
#define NAVE_H


#include "../utils/loadShip.h"


typedef struct ship{
    double cords[2];
    int capacity;
    loadShip* load;
} Ship;

Ship* initShip();

int checkCapacity(loadShip* load); // ritorna il numero di ton presenti sulla nave

int availableCapacity(int currentCapacity); // ritorna il numero di ton disponibili sulla nave 




#endif