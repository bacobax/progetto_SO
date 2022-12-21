

#ifndef PORTO_H

#define PORTO_H


/*Queste funzioni sono implementate in /utils/port_utility.c*/

#include "../config1.h"
#include "../utils/vettoriInt.h"
#include "../utils/supplies.h"

#define PQUEUEKEY 4000

struct port {

    int requests[SO_MERCI];
    Supplies supplies;
    double x;
    double y;
};

typedef struct port* Port;

/* TODO: fare struttura porto senza puntatori */
Port initPort(int supplyDisponibility, int requestDisponibility, int pIndex);


void printPorto(void* p, int idx);

void launchRefiller(int idx);
/*void freePort(Port p)*/ 

#endif
