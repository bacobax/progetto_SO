#ifndef NAVE_H
#define NAVE_H

#include "../utils/loadShip.h"
#include "../config1.h"
#include "./porto.h"
#define SDCHQUEUEKEY 9092  /* key coda messaggi per azioni di scaricamento*/


struct port_offer{
    int product_type;
    int expirationTime;
    int distributionDay;
    int weight;
    int portID;
};
typedef struct port_offer PortOffer;

struct ship {
    int shipID;
    double x;
    double y;
    
    int pid;
    int weight;
    loadShip loadship;
    unsigned short storm;
    unsigned short dead;
    unsigned short weatherTarget;
    unsigned short inSea;
};
typedef struct ship* Ship;



/* TUTTE LE FUNZIONI SOTTOSTANTI SONO RELATIVE ALLA NAVE*/

int checkCapacity(); /* ritorna il numero di ton presenti sulla nave */

int availableCapacity(Ship ship); /* ritorna il numero di ton disponibili sulla nave */


Ship initShip(int shipID);

void printShip(Ship ship);

/* int addProduct(Ship ship, Product p, Port port); */


int chooseQuantityToCharge(Ship ship);

int chooseProductToDelivery(Ship ship);

void initArrayOffers(PortOffer* offers);
int communicatePortsForChargeV1(int quantityToCharge, PortOffer* port_offers);

int communicatePortsForDischargeV1(Ship ship, Product p, int* quantoPossoScaricare, int* arrayResponses);


int portResponsesForCharge(Ship ship, PortOffer* port_offers);
int choosePortForCharge(PortOffer* port_offers, int idx);
void replyToPortsForChargeV1(int portID, PortOffer* port_offers);

void replyToPortsForDischargeV1(Ship ship, int portID, int quantoPossoScaricare, int* portResponses, Product prod);


void travelCharge(Ship ship, int portID, int* day, PortOffer* port_offers);

void travelDischarge(Ship ship, int portID, int* day, Product prod, int* portResponses);


void accessPortForChargeV1(Ship ship, int portID, PortOffer* port_offers);

void accessPortForDischargeV1(Ship ship, int portID, Product p,int product_index, int quantoPossoScaricare);

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
int deliverProduct(Ship ship, Port port, int product_index, Product p, int portID, int firstProd, int quantoPossoScaricare);

void addProduct(Ship ship, Product p, Port port);
void removeProduct(Ship ship, int index);
void removePWhere(Ship s, int(*f)(Product p));

void removeExpiredGoodsOnShip(Ship ship);

int isScadutaProduct(Product prod);

#endif
