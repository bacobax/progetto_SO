#ifndef NAVE_H
#define NAVE_H


#include "../utils/loadShip.h"

#define SHIPSHMKEY 8080
/*
 KEY della coda di messaggi alla quale tutte le navi
 si interfacceranno per leggere se è arrivato un msg dal master
 perchè è scattato un nuovo giorno
*/
#define SHIPQUEUEKEY 9090  
#define NEWDAY_TYPE_MSG 1

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

void newDayListenerShip(); /* chiamarlo ogni volta nel while(1) o lasciare che il processo in async non termini?*/

int chooseQuantityToCarghe();

void initArray(struct port_offer* offers);

int callPorts(int quantityToCharge);

int portResponses(struct port_offer* offers);

int choosePort(struct port_offer* offers);

void replyToPorts(int portID);

void travel(int portID);

void accessPort(int portID, struct port_offer product);

#endif
