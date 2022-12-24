#ifndef NAVE_H
#define NAVE_H


#include "../config1.h"

#define SQUEUEKEY 4001

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

struct port_offer{
    int product_type;
    int expirationTime;
};
typedef struct port_offer PortOffer;

/* TUTTE LE FUNZIONI SOTTOSTANTI SONO RELATIVE ALLA NAVE*/

int checkCapacity(); /* ritorna il numero di ton presenti sulla nave */

int availableCapacity(); /* ritorna il numero di ton disponibili sulla nave */

double generateCord(); /* genere una coordinata double */

Ship initShip(int shipID);

void printLoadShip(Product* products);

void printShip(Ship ship);

int addProduct(Ship ship, Product p);

int findProduct(Product* products, Product p); /* ritorna l'indice del vettore in cui il prodotto è contenuto */

int removeProduct(Ship ship, int product_index);

int chooseQuantityToCharge(Ship ship);

void initArrayOffers(PortOffer* offers);

void callPorts(Ship ship, int quantityToCharge);

int portResponses(Ship ship, PortOffer* port_offers);

int choosePort(PortOffer* port_offers);

void replyToPorts(Ship ship, int portID);

void dischargeProducts(Ship ship);

/*void travel(Ship ship, int portID);*/

/*void accessPort(Ship ship, int portID, PortOffer offer);*/



void updateExpTimeShip(Ship ship);

#endif
