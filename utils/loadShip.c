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

void addProduct(Ship ship, Product p,Port port) {
    /*char text[64];*/
    if (SO_CAPACITY - ship->weight >= p->weight)
    {

        if(p == NULL){
            printf("PRODOTTO NULLO\n");
            /*throwError("Prodotto nullo", "addProduct");*/
            /*logShip(ship->shipID, "PRODOTTO NULL");*/
        }
        

        if (ship->loadship->length == 0) {
            /*logShip(ship->shipID, "ramo true in addproduct");*/

            ship->loadship->last = p;
            ship->loadship->first = p;
        
           
        }else{
            ship->loadship->last->next = p;
            ship->loadship->last = ship->loadship->last->next;
        }
        /*sprintf(text, "ship->loadship->last->weight = %d\n", ship->loadship->last->weight );*/
        /*logShip(ship->shipID, text);*/
            
        /*logShip(ship->shipID, "dopo add");*/
        ship->loadship->length += 1;
        ship->weight+= p->weight;
        /*printShip(ship);*/
        
        addNotExpiredGood(p->weight, p->product_type, SHIP, 0, ship->shipID);
        port->sentGoods += p->weight;
        
    }
    else {
        /*
        throwError("capacità insufficente", "loadShip Addproduct");*/
        printf("IMPOSSIBILE AGGIUNGERE PRODOTTO SO_CAPACITY MASSIMA!\n");
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
    int i;
    int peso;
    i = 0;
    aux = ship->loadship->first; 

    if (index < 0 || index >= ship->loadship->length) {
        /*return throwError("Out of bound removeProduct", "removeProduct");*/
        printf("INDEX NON VALIDO PER RIMUOVERE IL PRODOTTO\n");
        return;
    }
    
    printf("index del prodotto da rimuovere:%d\n");
    if (index == 0) {
        
        aux = ship->loadship->first;
        peso = aux->weight;
        ship->loadship->first = ship->loadship->first->next;
        free(aux);
        
        ship->loadship->length -= 1;
        ship->weight -= peso;
        return;
    }

    if(index == ship->loadship->length-1){
        printf("vogio rimuovere l'ultimo, i:%d\n", index);
        peso = ship->loadship->last->weight;
        while (i < index - 1) {
            aux = aux->next;
            i++;
        }
        ship->loadship->last = aux;
        ship->loadship->last->next = NULL;

        ship->loadship->length -= 1;
        ship->weight -= peso;
        return;
    }

    while (aux != NULL)
    {

        if (i + 1 == index ) {
            peso = aux->next->weight;
            innerAux = aux->next->next;
            free(aux->next);
            aux->next = innerAux;
            
            ship->loadship->length -= 1;
            ship->weight -= peso;
            return;
        }
        i++;
        aux = aux->next;
    }

    printf("PRODOTTO NON TROVATO NELLA LISTA PER ESSERE RIMOSSO\n");
}

void printLoadShip(loadShip list, FILE* stream) {
    
    Product aux;

    fprintf(stream, "[ ");
    aux = list->first;
    while (aux != NULL) {
        fprintf(stream, "product_type:%d weight:%d expiration_time:%d dd:%d portID: %d, \n",aux->product_type, aux->weight, aux->expirationTime, aux->distributionDay, aux->portID);
        aux = aux->next;
    }
    fprintf(stream, " ]\n");
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

int weigthSum(loadShip l){
    Product aux;
    int sum = 0;
    for (aux = l->first; aux != NULL; aux = aux->next){
        sum += aux->weight;
    }
    return sum;
}
