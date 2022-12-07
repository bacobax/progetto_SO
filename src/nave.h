#ifndef NAVE_H
#define NAVE_H

#include "../config1.h"
#include "../utils/vettoriInt.h"

struct ship{
    double cords[2];
    intList* capacity;
};

typedef struct ship* Ship;

Ship initShip(double cords[], int );


#endif