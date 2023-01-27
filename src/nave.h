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

/* inizializza la nave in shm e aggancia il segmento di memoria al processo chiamante*/
Ship initShip(int shipID);

<<<<<<< HEAD
/* stampa le informazioni relative alla struttura dati della nave*/
=======
/*stampa la nave nel file di log*/
>>>>>>> 65356000b2f9f2813d00089f6d09a0a55cddf13e
void printShip(Ship ship);


/*
<<<<<<< HEAD

=======
    funzione che sceglie la quantità da caricare prendendo il minimo tra:
        1) massima offerta dei porti di cui esiste una domanda
        2) propria capacità disponibile in ton
>>>>>>> 65356000b2f9f2813d00089f6d09a0a55cddf13e
*/
int chooseQuantityToCharge(Ship ship);

/*
<<<<<<< HEAD
    scorre la lista della nave e ritorna ...
*/
int chooseProductToDelivery(Ship ship);
/*
    inizializza l'array delle offerte impostando tutti i campi a -1
*/
void initArrayOffers(PortOffer* offers);
/*

*/
int communicatePortsForChargeV1(Ship ship, int quantityToCharge, PortOffer* port_offers);
/*
=======
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
>>>>>>> 65356000b2f9f2813d00089f6d09a0a55cddf13e

*/
int communicatePortsForDischargeV1(Ship ship, Product p, int* quantoPossoScaricare, int* arrayResponses);

/*
    tramite l'array delle offerte di tutti i porti, la nave sceglie in quale andare a caricare,
    sceglie il porto con offerta con massima/minima expTime
*/
int choosePortForCharge(PortOffer* port_offers, int idx);
void replyToPortsForChargeV1(int portID, PortOffer* port_offers);

void replyToPortsForDischargeV1(Ship ship, int portID, int quantoPossoScaricare, int* portResponses, Product prod);

/*
    funzioni per la simulazione del viaggio della nave, in base all'azione in corso (carico/scarico)
    è presente una apposita travel
*/
void travelCharge(Ship ship, int portID, int* day, PortOffer* port_offers);
void travelDischarge(Ship ship, int portID, int* day, Product prod, int* portResponses);


void accessPortForChargeV1(Ship ship, int portID, PortOffer* port_offers);

void accessPortForDischargeV1(Ship ship, int portID, Product p,int product_index, int quantoPossoScaricare);

void updateExpTimeShip(Ship ship);

void chargeProducts(Ship ship, int quantityToCharge, int* day, unsigned int* terminateValue);
void dischargeProducts(Ship ship, int* day, unsigned int* terminateValue);

/*
    funzione che gestisce la terminazione della nave 
*/
void exitNave(Ship s);
/*
    stampa su file lo stato delle navi (quante sono in mare con merci a bordo, quante senza, ecc...)
*/
void printStatoNavi(FILE* fp);
/*
    funzione che fa attendere alla nave la fine del giorno
*/
void waitEndDay();

/*
 funzione che legge dalla shared memory utilizzata dal master,
 se il programma è finito perchè è arrivato al termine della
 simulazione
*/
void checkTerminateValue(Ship ship, unsigned int* terminateValue);

/*
 funzioni che ritornano l'id delle strutture IPC:
 la prima del semaforo delle banchine,
 la seconda del semaforo delle navi
*/
int getPierSem();
int getShipSem();

/*
 funzione che controlla se la nave è stata affondata da
 un malestorm
*/
void checkShipDead(Ship ship);
/*

*/
int deliverProduct(Ship ship, Port port, int product_index, Product p, int portID, int firstProd, int quantoPossoScaricare);
/*
 funzioni che aggiungono e rimuovono un prodotto al carico della nave,
 quindi aggiungono o tolgono nodi dalla lista
*/
void addProduct(Ship ship, Product p, Port port);
void removeProduct(Ship ship, int index);
/*

*/
void removePWhere(Ship s, int(*f)(Product p));

/*
    pulisce il carico delle navi dai prodotti scaduti
*/
void removeExpiredGoodsOnShip(Ship ship);

/*
 controlla che un determinato prodotto è scaduto o no,
 leggendo dalla shared memory del porto in cui è stato caricato
*/
int isScadutaProduct(Product prod);

#endif
