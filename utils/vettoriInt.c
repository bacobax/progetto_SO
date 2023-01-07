#include "vettoriInt.h"
#include <stdlib.h>
#include <stdio.h>

intList* intInit() {
    intList* ret;
    ret = malloc(sizeof(intList));
    ret->first = NULL;
    ret->last = NULL;
    ret->length = 0;
    return ret;
}

intList* intInitFromArray(int* a, int length) {
    intList* ret;
    int i;

    ret = intInit();
    for (i = 0; i < length; i++) {
        intPush(ret, a[i]);
    }


    return ret;
}

void intPush(intList* list, int i) {
    intNode* newNode;
    newNode = (intNode*)malloc(sizeof(intNode));
    newNode->numero = i;
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

void intStampaLista(intList* lista) {
    intNode* aux;
    aux = lista->first;

    printf("[ ");
    while (aux != NULL) {
        printf("%d , ", aux->numero);
        aux = aux->next;
    }
    printf(" ]\n");

}


int* intElementAt(intList* l, int idx) {
    int el;
    intNode* aux;
    int count;
    if (idx >= l->length || idx < 0) return NULL;

    aux = l->first;

    count = 0;
    while (aux != NULL) {
        if (count == idx) {
            return &(aux->numero);
        }
        aux = aux->next;
        count++;
    }
    return NULL;
}


int* intFindFirst(intList* l, int(*f)(int el, int idx)) {
    intNode* aux;
    int idx;

    aux = l->first;
    idx = 0;
    while (aux != NULL) {
        if (f(aux->numero, idx)) {
            return &(aux->numero);
        }
        aux = aux->next;
        idx++;
    }
    return NULL;
}

intList* intFindAll(intList* l, int(*f)(int el, int idx)) {
    intNode* aux = l->first;
    intList* lRet = intInit();
    int idx = 0;
    while (aux != NULL) {
        if (f(aux->numero, idx)) {
            intPush(lRet, aux->numero);
        }
        aux = aux->next;
        idx++;
    }

    return lRet;
}

intList* map(intList* l, int(*f)(int el, int idx)) {
    intNode* aux = l->first;
    intList* lRet = intInit();
    int idx = 0;
    while (aux != NULL) {

        intPush(lRet, f(aux->numero, idx));

        aux = aux->next;
        idx++;
    }

    return lRet;
}

void intFreeList(intList* lista) {
    intNode* aux;
    while (lista->first != NULL) {
        aux = lista->first;
        lista->first = lista->first->next;
        free(aux);
    }
    free(lista);
}

void intRemove(intList* lista, int idx) {
    intNode* aux = lista->first;
    intNode* innerAux;
    int count;
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
int rMin(intNode* n) {
    if (n->next == NULL) {
        return n->numero;
    }
    else {
        int min = rMin(n->next);
        if (n->numero < min) {
            return n->numero;
        }
        else {
            return min;
        }
    }
}
int min(intList* l) {
    intNode* aux = l->first;
    return rMin(aux);
}

int rMax(intNode* n) {
    if (n->next == NULL) {
        return n->numero;
    }
    else {
        int max = rMax(n->next);
        if (n->numero > max) {
            return n->numero;
        }
        else {
            return max;
        }
    }
}

int max(intList* l) {
    intNode* aux = l->first;
    return rMax(aux);
}

int sumR(intNode* n) {
    if (n == NULL) {
        return 0;
    }
    else {
        return n->numero + sumR(n->next);
    }
}
int sum(intList* l) {
    return sumR(l->first);
}

intList* findIdxs(int* vect, int length, int(*filter)(int)) {
    intList* l;
    int i;
    l = intInit();
    
    for (i = 0; i < length; i++) {
        if (filter(vect[i])) {
            intPush(l, i);
        }
    }
    return l;
    
}

int* toArray(intList* l, int* length) {
    int* retArray;
    int i;
    intNode* aux;

    *length = l->length;

    retArray = (int*)calloc(l->length, sizeof(int));
    i = 0;
    for (aux = l->first; aux != NULL; aux = aux->next) {
        retArray[i] = aux->numero;
        i++;
    }
    intFreeList(l);
    return retArray;
}
int contain(intList* l, int n){
    intNode* aux;
    int cond = 0;

    for(aux = l->first; aux!=NULL && !cond; aux = aux->next){
        if(aux->numero == n){
            cond = 1;
        }
    }
    return cond;
}

intList* intIntersect(intList* l1, intList* l2){
    intList* min;
    intList* max;
    intList* ret;
    intNode* aux;
    ret = intInit();
    if(l1->length<l2->length){
        min = l1;
        max = l2;
    }else{
        min = l2;
        max = l1;
    }
    for(aux = min->first; aux!=NULL; aux = aux->next){
        if(contain(max,aux->numero)){
            intPush(ret,aux->numero);
        }
    }

    return ret;
}

intList* intUnion(intList* l1, intList* l2){
    
    intList* ret;
    intNode* aux;
    ret = intInit();
    
    for(aux = l1->first; aux!=NULL; aux = aux->next){
        if(!contain(ret,aux->numero)){
            intPush(ret,aux->numero);
        }
    }
    for(aux = l2->first; aux!=NULL; aux = aux->next){
        if(!contain(ret,aux->numero)){
            intPush(ret,aux->numero);
        }
    }

    return ret;
}
