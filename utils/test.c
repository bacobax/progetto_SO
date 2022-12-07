#include "./msg_utility.h"
#include "./vettoriInt.h"
#include "./support.h"  
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define SO_MERCI 3

#define TIPO0 0
#define TIPO1 1
#define TIPO2 2

#define SO_PORTI 3

// void msgRecvHandler(long type, char text[MEXBSIZE]) {
//     printf("Messaggio di tipo %ld ricevuto: %s\n", type, text);
// }

typedef struct trade_ {
    int tipo;
    int quantita;
}Trade;

typedef struct porto_ {
    double x;
    double y;
    int* merci;
    Trade* domanda;
    Trade* offerta;
} Porto;

int scegliPortoPerOffrire(int tipo, int quantita, Porto* porti) {
    int idxPorto;
    Porto* portiConDomandaMinoreOUguale;


    intList* l = intInit();

    for (int i = 0; i < SO_PORTI; i++) {
        if (porti[i].domanda->tipo == tipo && porti[i].domanda->quantita <= quantita) {
            intPush(l, porti[i].domanda->quantita);
        }
    }
    if (l->length == 0) {
        return -1;
    }

    int maxDomanda = max(l);


    int idx;
    for (int i = 0; i < SO_PORTI; i++) {
        if (porti[i].domanda->tipo == tipo && porti[i].domanda->quantita == maxDomanda) {
            idx = i;
            break; //* facoltativo
        }
    }
    return idx;

}

void test0() {

    //* Porto 0
    Porto p0;
    int mercip0[] = { 4, 3, 3 };

    Trade domandap0 = { TIPO0, 12 };
    Trade offertap0 = { TIPO1,  2 };

    p0.x = 1;
    p0.y = 2;
    p0.merci = mercip0;
    p0.domanda = &domandap0;
    p0.offerta = &offertap0;

    //* Porto 1

    Porto p1;
    int mercip1[] = { 5, 2, 9 };

    Trade domandap1 = { TIPO0, 10 };
    Trade offertap1 = { TIPO2,  7 };

    p1.x = 1;
    p1.y = 2;
    p1.merci = mercip1;
    p1.domanda = &domandap1;
    p1.offerta = &offertap1;

    //* Porto 2

    Porto p2;
    int mercip2[] = { 7, 1,19 };

    Trade domandap2 = { TIPO0, 9 };
    Trade offertap2 = { TIPO2,  15 };

    p2.x = 1;
    p2.y = 2;
    p2.merci = mercip2;
    p2.domanda = &domandap2;
    p2.offerta = &offertap2;

    Porto porti[SO_PORTI] = { p0,p1,p2 };

    // Porto porti[SO_PORTI] = {
    //     {1,2, mercip0, {TIPO0,3}, {TIPO1,  2}},
    //     {2,1, mercip1, {TIPO0,4}, {TIPO2,  4}},
    //     {3,4, mercip2, {TIPO0,7}, {TIPO2, 12}},
    //     {5,2, mercip3, {TIPO0,9}, {TIPO0,  5}},
    // };
    int idx = scegliPortoPerOffrire(TIPO0, 8, porti);
    printf("Il miglior porto è il porto %d\n", idx);

}

int mapCriterio(int el, int idx) {
    return el * 2;
}

void test1() {


    intList* l = distribute(30, 3);

    int length;
    int* a = toArray(l, &length);

    for (int i = 0; i < length; i++) {
        printf("%d,\n ", a[i]);
    }
    // l = map(l, mapCriterio);

    // intStampaLista(l);

    // intFreeList(l);


}

int main(int argc, char const* argv[])
{

    int s;
    printf("Scegli il test da fare: ");
    scanf("%d", &s);
    switch (s)
    {
    case 0:
        test0();
        break;
    case 1:
        test1();
        break;
    default:
        break;
    }






    return 0;
}