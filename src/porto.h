

#ifndef PORTO_H

#define PORTO_H


/*Queste funzioni sono implementate in /utils/port_utility.c*/

#include "../config1.h"
#include "../utils/vettoriInt.h"
#include "../utils/supplies.h"

#define PQUEUEKEY 4000
#define PQUEREQKEY 3999

struct port {

    int requests[SO_MERCI];
    Supplies supplies;
    double x;
    double y;
};

typedef struct port* Port;

/*
    Funzione che inizializza il porto distribuendo la quantità di risorse del primo giorno e la domanda totale di quel porto e assegnandogli le coordinate
*/
Port initPort(int supplyDisponibility, int requestDisponibility, int pIndex);


void printPorto(void* p, int idx);

void launchRefiller(int idx);
/*
    void freePort(Port p)
*/


/*
funzione per separare la logica della configurazione del porto da quella della sua routine
*/
void mySettedPort(int supplyDisponibility, int requestDisponibility, int idx, void(*codicePorto)(Port porto, int myQueueID, int shipsQueueID, int idx));

/*
    forka il figlio che gestisce le navi che vogliono caricare dal porto
*/
void launchGoodsDispatcher(int myQueueID, Port porto, int idx, int shipsQueueID);
#endif
