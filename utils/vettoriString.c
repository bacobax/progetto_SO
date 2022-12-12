#include "vettoriString.h"
#include <stdlib.h>
#include <stdio.h>

stringList* stringInit() {
    stringList* ret;
    ret = malloc(sizeof(stringList));
    ret->first = NULL;
    ret->last = NULL;
    ret->length = 0;
    return ret;
}

stringList* stringInitFromArray(char** a, int length) {
    stringList* ret;
    int i;

    ret = stringInit();
    for (i = 0; i < length; i++) {
        stringPush(ret, a[i]);
    }


    return ret;
}

void stringPush(stringList* list, char* i) {
    stringNode* newNode;

    newNode = (stringNode*)malloc(sizeof(stringNode));
    newNode->el = i;
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
}

void stringStampaLista(stringList* lista) {
    stringNode* aux;

    printf("[ ");
    aux = lista->first;
    while (aux != NULL) {
        printf("%s , ", aux->el);
        aux = aux->next;
    }
    printf(" ]\n");

}


char* stringElementAt(stringList* l, int idx) {
    stringNode* aux;

    int el;
    int count;


    if (idx >= l->length || idx < 0) return NULL;

    aux = l->first;
    count = 0;
    while (aux != NULL) {
        if (count == idx) {
            return (aux->el);
        }
        aux = aux->next;
        count++;
    }
    return NULL;
}


char* stringFindFirst(stringList* l, int(*f)(char*)) {
    stringNode* aux;
    aux = l->first;
    while (aux != NULL) {
        if (f(aux->el)) {
            return (aux->el);
        }
        aux = aux->next;
    }
    return NULL;
}

stringList* stringFindAll(stringList* l, int(*f)(char*)) {
    stringNode* aux;
    stringList* lRet;

    aux = l->first;
    lRet = stringInit();

    while (aux != NULL) {
        if (f(aux->el)) {
            stringPush(lRet, aux->el);
        }
        aux = aux->next;
    }

    return lRet;
}

void stringFreeList(stringList* lista) {
    stringNode* aux;
    while (lista->first != NULL) {
        aux = lista->first;
        lista->first = lista->first->next;
        free(aux);
    }
    free(lista);
}

void stringRemove(stringList* lista, int idx) {
    stringNode* aux;
    stringNode* innerAux;
    int count;
    aux = lista->first;

    if (idx == 0) {
        innerAux = lista->first;
        lista->first = lista->first->next;
        free(innerAux);
        return;
    }
    count = 0;
    while (aux != NULL) {
        if (count == idx - 1) {
            innerAux = aux->next->next;
            free(aux->next);
            aux->next = innerAux;
            return;
        }

        aux = aux->next;
        count++;
    }
}

