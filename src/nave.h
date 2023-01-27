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


int availableCapacity(Ship ship); /* ritorna il numero di ton disponibili sulla nave */


Ship initShip(int shipID);

/*stampa la nave nel file di log*/
void printShip(Ship ship);

/* int addProduct(Ship ship, Product p, Port port); */

/*
    funzione che sceglie la quantità da caricare prendendo il minimo tra:
        1) massima offerta dei porti di cui esiste una domanda
        2) propria capacità disponibile in ton
*/
int chooseQuantityToCharge(Ship ship);

/*
    ritorna l'indice del prodotto da consegnare (scelto in base alla data di scadenza)
*/
int chooseProductToDelivery(Ship ship);
/*
    inizializza a -1 tutti i campi di ogni elemento dell'array offers
*/
void initArrayOffers(PortOffer* offers);
/*
    ritorna il numero di porti che hanno da offrire <quantityToCharge> ton di merce di qualsiasi tipo;
    riempie port_offers con tutte le risposte affermative dei porti, indicando per ciascuna di loro
    1) porto che ha offerto
    2) tipo di merce
    3) tempo di vita rimanente
    4) il peso in tonnellate
*/
int communicatePortsForChargeV1(Ship ship, int quantityToCharge, PortOffer* port_offers);


/*
    ritorna direttamente il porto migliore dove scaricare (domanda per il tipo di merce p->product_type più alta);
    riempie arrayResponses con la domanda di ciascun porto non-decrementata (perchè ogni porto con domanda = 0 risponde in maniera negativa,
    mentre gli altri decrementano la loro domanda di p->weight, massimo fino a 0);
    quantoPossoScaricare viene settato alla domanda originaria del porto, se ques'ultima è stata decrementata fino a 0, ovviamente,
    conseguenza logica di ciò, in questo caso quantoPossoScaricare <= p->weight.
    In sintesi quantoPossoScaricare = min{richiesta del porto, p->weight}

*/
int communicatePortsForDischargeV1(Ship ship, Product p, int* quantoPossoScaricare, int* arrayResponses);

/*
    tramite l'array delle offerte di tutti i porti, la nave sceglie in quale andare a caricare,
    sceglie il porto con offerta con massima/minima expTime
*/
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
