

#ifndef PORTO_H

#define PORTO_H

#include "../config1.h"
#include "../utils/vettoriInt.h"

struct port {
    int semidBanchina;
    int requests[];
    int supplies[];
};

typedef struct port* Port;

//TODO: fare struttura porto senza puntatori
Port initPort(int disponibility);

void freePort(Port p);

#endif