

#ifndef PORTO_H

#define PORTO_H

#include "../config1.h"
#include "../utils/vettoriInt.h"

struct port {
    int semidBanchina;

    int requests[SO_MERCI];
    int supplies[SO_MERCI];

    double x;
    double y;
};

typedef struct port* Port;

/* TODO: fare struttura porto senza puntatori */
Port initPort(int disponibility, int pIndex);

void freePort(Port p);

#endif