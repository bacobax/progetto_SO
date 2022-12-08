

#ifndef PORTO_H

#define PORTO_H

#include "../config1.h"
#include "../utils/vettoriInt.h"

struct port {
    intList* requests;
    intList* supplies;
};

typedef struct port* Port;


Port initPort(int disponibility);

void freePort(Port p);

#endif