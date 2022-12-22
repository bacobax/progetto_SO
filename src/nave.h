#ifndef NAVE_H
#define NAVE_H


#include "../config1.h"

#define SHIPSHMKEY 8080
/*
 KEY della coda di messaggi alla quale tutte le navi
 si interfacceranno per leggere se è arrivato un msg dal master
 perchè è scattato un nuovo giorno
*/
#define SHIPQUEUEKEY 9090  
#define NEWDAY_TYPE_MSG 1

struct product {
    int product_type;    
    int expirationTime;
    int weight;
};
typedef struct product Product;

struct ship {
    int shipID;
    double x;
    double y;
    int weight;
    Product products[SO_CAPACITY];
};
typedef struct ship* Ship;


int checkCapacity(); /* ritorna il numero di ton presenti sulla nave */

int availableCapacity(); /* ritorna il numero di ton disponibili sulla nave */

double generateCord(); /* genere una coordinata double */

Ship initShip(int shipID);

void printLoadShip(Product* products);

void printShip(Ship ship);

int addProduct(Ship ship, Product p);

int findProduct(Product* products, Product p); /* ritorna l'indice del vettore in cui il prodotto è contenuto */

int removeProduct(Ship ship, int product_index);


/*

CODICE DELLA VECCHIA VERSIONE, DA TENERE PERCHÈ POTREBBE
SERVIRCI

void newDayListenerShip();  chiamarlo ogni volta nel while(1) o lasciare che il processo in async non termini?

void travel(int portID);

void accessPort(int portID, struct port_offer product);

*/

#endif
