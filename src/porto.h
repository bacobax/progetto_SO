

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

/*
    funzione che dato l'indice del porto in questione, crea un figlio che usa quell'indice per ottenere la relativa struttura dai in shm
    per decrementare la durata di vita dei materiali e per rimuovere quelli scaduti
*/
void updateExpTimes(int idx);

void mySettedPort(int supplyDisponibility, int requestDisponibility, int idx, void(*codicePorto)(Port porto, int myQueueID, int shipsQueueID));
#endif
