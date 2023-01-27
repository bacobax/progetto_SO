

#ifndef PORTO_H

#define PORTO_H


/*Queste funzioni sono implementate in /utils/port_utility.c*/
#include <stdio.h>
#include "../config1.h"
#include "../utils/vettoriInt.h"
#include "../utils/supplies.h"
#include "../utils/support.h"


#define P2SEMVERIFYKEY 3995

struct port {


    int requestsID;
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

/* funzione che stampa il porto*/
void printPort(Port p, int idx, FILE* stream);

void launchRefiller(int idx);

/*
    funzione per separare la logica della configurazione del porto da quella della sua routine
*/
void mySettedPort(int supplyDisponibility, int requestDisponibility, int idx, void(*portCode)(int idx, int endShmId,int aspettoMortePortiSemID, int aspettoMorteNaviSemID));


/*
    dato un tipo da voler scaricare e un peso da voler scaricare, viene detratto alla richiesta del tipo di merce [type] del porto p
    la quantità richiesta, fino ad arrivare a 0
*/
int checkRequests(Port p, int type, int quantity);

  
/*
    Questa funzione ritorna una lista di valori interi che indicano l'intersezione tra la lista di merci richieste (tra tutti i porti)
    e la lista di merci offerte (tra tutti i porti)
*/ 
intList *getTypeToCharge();

/*
    funzione che ritorna una lista di tipi di merce richiesta da tutti i porti tranne che dal porto con indice = idx
*/
intList *getAllOtherTypeRequests(int idx, Port portArr);

/*
    lista contenente i tipi di merce offerti dal porto p
*/
intList* suppliesTypes(Port p);
/*
    lista contenente i tipi di merce richieste dal porto p
*/
intList* requestsTypes(Port p);
/*
    funzione che attribuisce un valore al lotto offerto riferito
*/
double getValue(int quantity, int scadenza, int tipo, Port p, int idx);

/*
    funzione che ritorna l'indirizzo del porto con offset portID dall'indirizzo base dell'array di porti in shm
*/
Port getPort(int portID);

/*
    algoritmo che trova la migliore coppia di coordinate della matrice (tipo merce, giorno di distribuzione della merce) che corrisponde
    alla migliore quantità disponibile da offrire di fronte alla richiesta di merce pari a {{quantity}}
*/
int findTypeAndExpTime(Port port, int* tipo, int* dayTrovato, int* scadenza, int quantity, int idx);

/*
    funzione per stampare sul file le statistiche di tutti i porti
    (esempio: quante banchine occupate ha, numero di merci spedite, numero di merci ricevute)
*/
void printPortsState(FILE *fp);
/*
    serve per ripristinare la quantità di merci presente prima dell'inizio della comunicazione tra una nave ed un porto,
    viene fatta ogni qualvolta una nave non riesce a portare a compimento un operazione di carico verso un porto
    (magari viene uccisa, o magari termina perchè non le rimane abbastanza tempo).
*/
void restorePromisedGoods(Port porto, int dayTrovato, int tipoTrovato, int quantity, int myPortIdx);
/*
    sgancia il segmento di memoria condivisa relativa ai porti dal processo chiamante
*/
void detachPort(Port port, int portID);
/*
    aggancia il segmento di memoria condivisa del magazzino relativo al porto (passato per parametro)
    al processo chiamante
*/
int* getMagazine(Port port);

#endif
