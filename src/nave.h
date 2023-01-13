#ifndef NAVE_H
#define NAVE_H


#include "../config1.h"
#include "./porto.h"
#define SDCHQUEUEKEY 9092  /* key coda messaggi per azioni di scaricamento*/

struct product {
    int product_type;    
    int expirationTime;
    int weight;
};
typedef struct product Product;

struct port_offer{
    int product_type;
    int expirationTime;
    int distributionDay;
    int weight;
};
typedef struct port_offer PortOffer;


struct ship {
    int shipID;
    double x;
    double y;
    int weight;
    int pid;
    
    Product products[SO_CAPACITY];
    PortOffer promisedProduct;
    unsigned short storm;
    unsigned short dead;
    unsigned short weatherTarget;
    unsigned short inSea;
};
typedef struct ship* Ship;



/* TUTTE LE FUNZIONI SOTTOSTANTI SONO RELATIVE ALLA NAVE*/

int checkCapacity(); /* ritorna il numero di ton presenti sulla nave */

int availableCapacity(); /* ritorna il numero di ton disponibili sulla nave */


Ship initShip(int shipID);

void printLoadShip(Product* products);

void printShip(Ship ship);

int addProduct(Ship ship, Product p, Port port);

int findProduct(Product* products, Product p); /* ritorna l'indice del vettore in cui il prodotto è contenuto */

int removeProduct(Ship ship, int product_index);

int chooseQuantityToCharge(Ship ship);

int chooseProductToDelivery(Ship ship);

void initArrayOffers(PortOffer* offers);

int communicatePortsForCharge(Ship ship, int quantityToCharge, PortOffer* port_offers);
int communicatePortsForDischarge(Ship ship, Product p, int* quantoPossoScaricare);  

int portResponsesForCharge(Ship ship, PortOffer* port_offers);
int choosePortForCharge(PortOffer* port_offers, int idx);

void replyToPortsForCharge(Ship ship, int portID);
void replyToPortsForDischarge(Ship ship, int portID);


void travel(Ship ship, int portID, int* day);

void accessPortForCharge(Ship ship, int portID);
void accessPortForDischarge(Ship ship, int portID, int product_index, int quantoPossoScaricare);

void updateExpTimeShip(Ship ship);

void chargeProducts(Ship ship, int quantityToCharge, int* day, unsigned int* terminateValue);
void dischargeProducts(Ship ship, int* day, unsigned int* terminateValue);

void exitNave(Ship s);

void printStatoNavi(FILE* fp);

void waitEndDay();

void waitToTravel(Ship ship);

void initPromisedProduct(Ship ship, PortOffer port_offer, int quantityToCharge);

void checkTerminateValue(Ship ship, unsigned int* terminateValue);


int getPierSem();

int getShipSem();

void checkShipDead(Ship ship);
int deliverProduct(Ship ship, Port port, int product_index, Product p, int portID, int firstProd);

#endif
