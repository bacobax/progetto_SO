#include <stdio.h>
#include <stdlib.h>
#include "./loadShip.h"

loadShip initLoadShip() {
    loadShip ret = (struct load*)malloc(sizeof(struct load));
    ret->first = NULL;
    ret->last = NULL;
    ret->weightLoad = 0;
    ret->length = 0;
    return ret;
}

<<<<<< < HEAD
    void addProduct(loadShip * list, Product * p) {
    Product* newNode = (Product*)malloc(sizeof(Product));
    ====== =
        void addProduct(loadShip list, Product p) {
        Product newNode = (struct productNode_*)malloc(sizeof(struct productNode_));
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

    <<<<<< < HEAD
        Product * findProduct(loadShip * list, int idProduct) { //! secondo me dovresti chiamarla productAt(), perchè la find() la potresti fare passando come parametro una funzione 

        ====== =
            Product findProduct(loadShip list, int idProduct) {

            >>>>>> > cc8ba5a8e07904a87c55ea99444c9ecb01531bcf
                if (idProduct >= list->length || idProduct < 0) return NULL;

            Product aux = list->first;

            while (aux != NULL) {
                if (aux->id == idProduct) {
                    return aux;
                }
                aux = aux->next;
            }
            return NULL;
        }

        <<<<<< < HEAD
            void removeProduct(loadShip * list, int idProduct) {
            Product* aux = list->first;
            Product* innerAux;
            ====== =
                void removeProduct(loadShip list, int idProduct) {
                Product aux = list->first;
                Product innerAux;

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

            <<<<<< < HEAD
                void printLoadShip(loadShip * list) {
                ====== =
                    void printLoadShip(loadShip list) {
                    >>>>>> > cc8ba5a8e07904a87c55ea99444c9ecb01531bcf
                        printf("[ ");
                    Product aux = list->first;
                    while (aux != NULL) {
                        printf("id:%d weight:%d expiration_time:%d , ", aux->id, aux->weight, aux->expirationTime);
                        aux = aux->next;
                    }
                    printf(" ]\n");
                }

                <<<<<< < HEAD
                    void freeLoadShip(loadShip * list) {
                    Product* aux;
                    ====== =
                        void freeLoadShip(loadShip list) {
                        Product aux;
                        while (list->first != NULL) {
                            aux = list->first;
                            list->first = list->first->next;
                            free(aux);
                        }
                        free(list);
                    }



