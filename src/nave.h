#ifndef NAVE_H
#define NAVE_H


#include "../utils/loadShip.h"

#define SHIPSHMKEY 8080

struct ship {
    int shipID;
    double x;
    double y;
    int capacity;
    loadShip load;
};
typedef struct ship* Ship;

Ship ship; /* puntatore come variabile globale alla struttura della nave */

struct port_offer{
    int product_type;
    int expirationTime;
};



int checkCapacity(); /* ritorna il numero di ton presenti sulla nave */

int availableCapacity(); /* ritorna il numero di ton disponibili sulla nave */

double generateCord(); /* genere una coordinata double */

Ship initShip(int shipID);

void printShip(int id_ship);

void travel(int portID); 

#endif
