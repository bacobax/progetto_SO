#ifndef NAVE_H
#define NAVE_H


#include "../config1.h"

//#define SCHQUEUEKEY 9091   /* key coda messaggi per azioni di caricamento*/
#define SDCHQUEUEKEY 9092  /* key coda messaggi per azioni di scaricamento*/

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

struct product_for_delivery{
    int product_type;
    int weight;
};
typedef struct product_for_delivery ProductToDelivery;


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

int chooseProductToDelivery(Ship ship);

void initArrayOffers(PortOffer* offers);

void callPortsForCharge(Ship ship, int quantityToCharge);
void callPortsForDischarge(Ship ship, Product p);  

int portResponsesForCharge(Ship ship, PortOffer* port_offers);
int choosePortForCharge(PortOffer* port_offers);

void replyToPortsForCharge(Ship ship, int portID);

int portResponsesForDischarge();



void travel(Ship ship, int portID);


void accessPortForCharge(Ship ship, int portID, PortOffer offer_choosen, int weight);
void accessPortForDischarge(Ship ship, int portID, int product_index);

void updateExpTimeShip(Ship ship);

void chargeProducts(Ship ship, int quantityToCharge);
void dischargeProducts(Ship ship);

#endif
