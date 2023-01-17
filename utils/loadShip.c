#include <stdio.h>
#include <stdlib.h>
#include "./loadShip.h"
#include "../config1.h"
#include "../src/dump.h"
#include "./errorHandler.h"
#include "../src/nave.h"
loadShip initLoadShip() {
    loadShip ret;
    ret = (struct load*)malloc(sizeof(struct load));
    ret->first = NULL;
    ret->last = NULL;
    
    ret->length = 0;
    return ret;
}

Product initProduct(int weight, int type, int expTime, int portID, int dd) {
    Product p = (Product)malloc(sizeof(struct productNode_));
    p->distributionDay = dd;
    p->expirationTime = expTime;
    p->next = NULL;
    p->weight = weight;
    p->product_type = type;
    p->portID = portID;
    return p;
}

void addProduct(Ship ship, Product p, int idx, Port port) {
    if (SO_CAPACITY - ship->weight >= p->weight) {
        

        if (ship->loadship->length == 0) {
            ship->loadship->last = p;
            ship->loadship->first = p;
        }
        else {
            
            ship->loadship->last->next = p;
            ship->loadship->last = p;
        }
        ship->loadship->length += 1;
        ship->weight+= p->weight;
        addNotExpiredGood(p->weight, p->product_type, SHIP, 0, ship->shipID);
        port->sentGoods += p->weight;

    }
    else {
        throwError("capacità insufficente", "loadShip Addproduct");
    }
    
}

Product productAt(loadShip l, int idx) {
    Product aux;
    int i = 0;
    for (aux = l->first; aux != NULL; aux = aux->next) {
        if (i == idx)return aux;
        i++;
    }
    return NULL;
}

Product findProduct(loadShip list, int product_type) { /* cerca un prodotto per product_type e restutuisce il nodo*/

    Product aux = list->first;

    while(aux != NULL){

        if(aux->product_type == product_type){
            return aux;
        }
        aux = aux->next;
    }
    
    return NULL;
}
/*
int getProductId(loadShip list, int product_type){ /* cerca un prodotto per product_type e restituire l'id del primo prodotto che trova nella lista
    Product aux = list->first;

    while(aux != NULL){

        if(aux->product_type == product_type){
            return aux->id_product;
        }
        aux = aux->next;
    }
    
    return -1;
}
*/

void removeProduct(Ship ship, int index) {

    Product aux;
    Product innerAux;
    int i = 0;
    int peso;
    if (index == 0) {
        
        aux = ship->loadship->first;
        peso = aux->weight;
        ship->loadship->first = ship->loadship->first->next;
        free(aux);
        ship->loadship->length -= 1;
        ship->weight -= peso;
        return;
    }
    for (aux = ship->loadship->first; aux != NULL; aux = aux->next) {
        if (i == index - 1) {
            peso = aux->next->weight;
            innerAux = aux->next->next;
            free(aux->next);
            aux->next = innerAux;
            ship->loadship->length -= 1;
            ship->weight -= peso;
            return;
        }
        i++;
    }

    throwError("Prodotto non trovato, impossibile rimuoverlo dalla lista\n", "removeProduct");
}

void printLoadShip(loadShip list) {
    
    Product aux;

    printf("[ ");
    aux = list->first;
    while (aux != NULL) {
        printf("product_type:%d weight:%d expiration_time:%d , ",aux->product_type, aux->weight, aux->expirationTime);
        aux = aux->next;
    }
    printf(" ]\n");
}

void freeLoadShip(loadShip list) {
    Product aux;
    while (list->first != NULL) {
        aux = list->first;
        list->first = list->first->next;
        free(aux);
    }
    free(list);
}



