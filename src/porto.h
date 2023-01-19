

#ifndef PORTO_H

#define PORTO_H


/*Queste funzioni sono implementate in /utils/port_utility.c*/
#include <stdio.h>
#include "../config1.h"
#include "../utils/vettoriInt.h"
#include "../utils/supplies.h"
#include "../utils/support.h"

#define PQUEUEDCHKEY 5000    /* key coda messaggi per azioni di scaricamento*/

#define PQUERECHKEY 3999    
#define PQUEREDCHKEY 3998

#define PSEMVERIFYKEY 3997
#define P2SEMVERIFYKEY 3995
#define WAITTOTRAVELKEY 3996


struct port {


    int requests[SO_MERCI];
    unsigned short swell;
    unsigned short weatherTarget;
    Supplies supplies;
    double x;
    double y;
    int deliveredGoods;
    int sentGoods;
};

typedef struct port* Port;

/*
    Funzione che inizializza il porto distribuendo la quantità di risorse del primo giorno e la domanda totale di quel porto e assegnandogli le coordinate
*/
Port initPort(int supplyDisponibility, int requestDisponibility, int pIndex);


void printPorto(Port p, int idx, FILE* stream);

void launchRefiller(int idx);
/*
    void freePort(Port p)
*/


/*
funzione per separare la logica della configurazione del porto da quella della sua routine
*/
void mySettedPort(int supplyDisponibility, int requestDisponibility, int idx, void(*codicePorto)(int idx, int endShmId,int aspettoMortePortiSemID, int aspettoMorteNaviSemID));

/*
    forka il figlio che gestisce le navi che vogliono caricare dal porto
*/

void launchDischarger(void (*recvHandler)(long,char*), int idx);
void launchCharger(void (*recvHandler)(long, char*), int idx);

int checkRequests(Port p, int type, int quantity);

int allRequestsZero();
intList *haSensoContinuare();

intList *getAllOtherTypeRequests(int idx);

intList* getAllTypeSupplies(Port portArr);
intList* tipiDiMerceOfferti(Port p);
intList* tipiDiMerceRichiesti(Port p);
double getValue(int quantity, int scadenza, int tipo, int idx);
Port getPort(int portID);

/*
    algoritmo che trova la migliore coppia di coordinate della matrice (tipo merce, giorno di distribuzione della merce) che corrisponde
    alla migliore quantità disponibile da offrire di fronte alla richiesta di merce pari a {{quantity}}
*/
int trovaTipoEScadenza(Supplies* S, int* tipo, int* dayTrovato, int* scadenza, int quantity, int idx);

void printStatoPorti(FILE *fp);
void restorePromisedGoods(Port porto, int dayTrovato, int tipoTrovato, int quantity, int myPortIdx);
void detachPort(Port port, int portID);

#endif
