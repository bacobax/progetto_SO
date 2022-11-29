#include "./msg_utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define SO_MERCI 3

#define TIPO0 0
#define TIPO1 1
#define TIPO2 2

#define SO_PORTI 4

void msgRecvHandler(long type, char text[MEXBSIZE]) {
    printf("Messaggio di tipo %ld ricevuto: %s\n", type, text);
}

typedef struct trade_ {
    int tipo;
    int quantita;
}Trade;

typedef struct porto_ {
    double x;
    double y;
    int merci[SO_MERCI];
    Trade* domanda;
    Trade* offerta;
} Porto;

int scegliPortoPerOffrire(int tipo, int quantita, Porto* porti) {
    int idxPorto;
    Porto* portiConDomandaMinoreOUguale;
    int numPortiConDomandaMinoreOUguale;
    int i = 0;
    for (; i < SO_PORTI; i++) {
        if (porti[i].domanda->tipo == tipo && porti[i].domanda->quantita <= quantita) {
            i++;
        }
    }
    //TODO: Continuare: i conta il numero di porti con quantità richiesta <= della disponibilità della nave di quel tipo di risorsa

}

int main(int argc, char const* argv[])
{
    int mercip0[] = { 4, 3, 3 };
    int mercip1[] = { 5, 2, 9 };
    int mercip2[] = { 7, 1,19 };
    int mercip3[] = { 9,11,13 };


    Porto porti[SO_PORTI] = {
        {1,2, mercip0, {TIPO0,3}, {TIPO1,  2}},
        {2,1, mercip1, {TIPO0,4}, {TIPO2,  4}},
        {3,4, mercip2, {TIPO0,7}, {TIPO2, 12}},
        {5,2, mercip3, {TIPO0,9}, {TIPO0,  5}},
    };






    return 0;
}
