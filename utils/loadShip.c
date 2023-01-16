#include <stdio.h>
#include <stdlib.h>
#include "./loadShip.h"
#include "../config1.h"
#include "./errorHandler.h"
loadShip initLoadShip() {
    loadShip ret;
    ret = (struct load*)malloc(sizeof(struct load));
    ret->first = NULL;
    ret->last = NULL;
    ret->weightLoad = 0;
    ret->length = 0;
    return ret;
}

void addProduct(loadShip list, Product p, int idx, Port port) {
    if (SO_CAPACITY - list->weightLoad >= p->weight) {
       Product newNode;
        newNode = (struct productNode_*)malloc(sizeof(struct productNode_));
        
        if(list->last == NULL){
            newNode->id_product = 0;
        } else {
            newNode->id_product = (list->last->id_product) + 1;
        }
        
        newNode->product_type = p->product_type;
        newNode->weight = p->weight;
        newNode->expirationTime = p->expirationTime;
        newNode->next = NULL;

        if (list->length == 0) {
            list->last = newNode;
            list->first = newNode;
        }
        else {
            
            list->last->next = newNode;
            list->last = newNode;
        }
        list->length += 1;
        list->weightLoad += newNode->weight;
        addNotExpiredGood(p.weight, p.product_type, SHIP, 0, ship->shipID);
        port->sentGoods += p->weight;

    }
    else {
        throwError("capacità insufficente", "loadShip Addproduct");
    }
    
}

Product productAt(loadShip l, int idx) {
    Product aux;
    int i = 0;
    for (aux == l->first; aux != NULL; aux = aux->next) {
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

int getProductId(loadShip list, int product_type){ /* cerca un prodotto per product_type e restituire l'id del primo prodotto che trova nella lista*/
    Product aux = list->first;

    while(aux != NULL){

        if(aux->product_type == product_type){
            return aux->id_product;
        }
        aux = aux->next;
    }
    
    return -1;
}


void removeProduct(loadShip list, int idProduct) {
    Product innerAux;
    Product aux;
    
    aux = list->first;

    while (aux != NULL) {
        if (idProduct == aux->id_product) {
            innerAux = aux->next->next;
            list->length += -1;
            list->weightLoad = list->weightLoad - aux->weight;
            free(aux->next);
            aux->next = innerAux;
            return;
        }

        aux = aux->next;
    }

    printf("Prodotto non trovato, impossibile rimuoverlo dalla lista\n");
}

void printLoadShip(loadShip list) {
    
    Product aux;

    printf("[ ");
    aux = list->first;
    while (aux != NULL) {
        printf("id_product:%d product_type:%d weight:%d expiration_time:%d , ", aux->id_product, aux->product_type, aux->weight, aux->expirationTime);
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



