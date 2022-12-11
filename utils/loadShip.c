#include <stdio.h>
#include <stdlib.h>
#include "loadShip.h"

loadShip* initLoadShip() {
    loadShip* ret = malloc(sizeof(loadShip));
    ret->first = NULL;
    ret->last = NULL;
    ret->weightLoad = 0;
    ret->length = 0;
    return ret;
}

void addProduct(loadShip* list, Product* p) {
    Product* newNode = (Product*)malloc(sizeof(Product));
    newNode->id = p->id;
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
}

Product* findProduct(loadShip* list, int idProduct) { //! secondo me dovresti chiamarla productAt(), perchè la find() la potresti fare passando come parametro una funzione 

    if (idProduct >= list->length || idProduct < 0) return NULL;

    Product* aux = list->first;

    while (aux != NULL) {
        if (aux->id == idProduct) {
            return aux;
        }
        aux = aux->next;
    }
    return NULL;
}

void removeProduct(loadShip* list, int idProduct) {
    Product* aux = list->first;
    Product* innerAux;

    while (aux != NULL) {
        if (idProduct == aux->id) {
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

void printLoadShip(loadShip* list) {
    printf("[ ");
    Product* aux = list->first;
    while (aux != NULL) {
        printf("id:%d weight:%d expiration_time:%d , ", aux->id, aux->weight, aux->exprationTime);
        aux = aux->next;
    }
    printf(" ]\n");
}

void freeLoadShip(loadShip* list) {
    Product* aux;
    while (list->first != NULL) {
        aux = list->first;
        list->first = list->first->next;
        free(aux);
    }
    free(list);
}



