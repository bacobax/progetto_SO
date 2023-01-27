#ifndef LOAD_SHIP_H
#define LOAD_SHIP_H
#include "../src/porto.h"
struct productNode_ { /* nodo utilizzato nella lista */
    int product_type; 
    int weight;
    int expirationTime;
    int distributionDay;
    int portID;
    struct productNode_* next;
};
typedef struct productNode_* Product;

struct load { /* lista implementata per il carico della nave */
    Product first;
    Product last;
    int length;
    
};
typedef struct load* loadShip;
/*
    inizializza e crea una lista vuota per il carico della nave
*/
loadShip initLoadShip();
/*
    dato un product_type, restituisce il nodo contenuto nella lista
    con quel product_type
*/
Product findProduct(loadShip list, int product_type);
/*
    dato un product_type, restituisce l'indice della posizione
    del nodo, contenuto nella lista, con quel product_type
*/
int getProductId(loadShip list, int product_type);

/*
    funzione per stampare il carico della nave, ossia la lista
*/
void printLoadShip(loadShip list, FILE* stream);
/*
    funzione che fa la free della lista
*/
void freeLoadShip(loadShip list);
/*
    dato un indice, restituisce il nodo contenuto nella lista
    alla posizione corrispondente con l'indice
*/
Product productAt(loadShip l, int idx);
/*
    inizializza un Prodotto quindi inizializza e crea un nuovo nodo
*/
Product initProduct(int weight, int type, int expTime, int portID, int dd);
/*
    ritorna la somma totale dei pesi dei prodotti contenuti nella lista
*/
int weigthSum(loadShip l);
/*
    dato un product, ritorna il suo tempo di vita rimanente interfacciandosi
    con la shm del porto da cui è stato prelevato
*/
int getProductExpTime(Product p);

#endif
