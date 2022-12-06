#include "./vettoriInt.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init() {
    srand(time(NULL));
}

intList* distribute(int quantity, int parts) {

    init();

    //per ciascuna parte, tranne l'ultima vale:

    //massimo: la quantità che gli verrebbe assegnata se le quantità fossero distribuite in parti uguali
    //questo perchè nel peggiore dei casi (in cui a tutte le quantita venga assegnato il massimo) la quantità totale <= quantity
    int max_q = quantity / parts;

    //minimo: la metà della quantità che gli verrebbe distribuita se le quantità fossero distribuite in parti uguali
    int min_q = quantity / parts / 2;
    intList* l = intInit();
    for (int i = 0; i < parts - 1; i++) {
        int random_q = rand() % max_q + min_q;
        intPush(l, random_q);
    }

    //per l'ultima quantità viene assegnata la quantità restate non ancora assegnata
    //questo per essere sicuro che la somma delle quantità sia = quantity
    int last_q = quantity - sum(l);
    intPush(l, last_q);
    return l;
}


