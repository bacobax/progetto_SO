#include "vettoriInt.h"
#include <stdlib.h>
#include <stdio.h>

intList* intInit() {
    intList* ret = malloc(sizeof(intList));
    ret->first = NULL;
    ret->last = NULL;
    ret->length = 0;
    return ret;
}

intList* intInitFromArray(int* a, int length) {
    intList* ret = intInit();

    for (int i = 0; i < length; i++) {
        intPush(ret, a[i]);
    }


    return ret;
}

void intPush(intList* list, int i) {
    intNode* newNode = (intNode*)malloc(sizeof(intNode));
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
    printf("[ ");
    intNode* aux = lista->first;
    while (aux != NULL) {
        printf("%d , ", aux->numero);
        aux = aux->next;
    }
    printf(" ]\n");

}


int* intElementAt(intList* l, int idx) {
    int el;

    if (idx >= l->length || idx < 0) return NULL;

    intNode* aux = l->first;

    int count = 0;
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
    intNode* aux = l->first;
    int idx = 0;
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

    if (idx == 0) {
        innerAux = lista->first;
        lista->first = lista->first->next;
        free(innerAux);
        return;
    }
    int count = 0;
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
