#include "./msg_utility.h"
#include "./vettoriInt.h"
#include "./support.h"
#include "./shm_utility.h"
#include "./sem_utility.h"
#include "../config1.h"
#include "../src/nave.h"  
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#define SO_MERCI 3

#define TIPO0 0
#define TIPO1 1
#define TIPO2 2

#define SO_PORTI 3

/* void msgRecvHandler(long type, char text[MEXBSIZE]) {
//     printf("Messaggio di tipo %ld ricevuto: %s\n", type, text);
// } */

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
    
    int idxPorto, maxDomanda, idx;
    Porto* portiConDomandaMinoreOUguale;
    intList* l;
    int i;

    l = intInit();

    for (i = 0; i < SO_PORTI; i++) {
        if (porti[i].domanda->tipo == tipo && porti[i].domanda->quantita <= quantita) {
            intPush(l, porti[i].domanda->quantita);
        }
    }
    if (l->length == 0) {
        return -1;
    }

    maxDomanda = max(l);


    for (i = 0; i < SO_PORTI; i++) {
        if (porti[i].domanda->tipo == tipo && porti[i].domanda->quantita == maxDomanda) {
            idx = i;
            break; /* facoltativo */
        }
    }
    return idx;

}

void test0() {

    /* Porto 0 */
    Porto p0;
    int mercip0[] = { 4, 3, 3 };

    Trade domandap0 = { TIPO0, 12 };
    Trade offertap0 = { TIPO1,  2 };

    p0.x = 1;
    p0.y = 2;
    p0.merci = mercip0;
    p0.domanda = &domandap0;
    p0.offerta = &offertap0;

    /* Porto 1 */
    
    Porto p1;

    int mercip1[] = { 5, 2, 9 };

    Trade domandap1 = { TIPO0, 10 };
    Trade offertap1 = { TIPO2,  7 };

    p1.x = 1;
    p1.y = 2;
    p1.merci = mercip1;
    p1.domanda = &domandap1;
    p1.offerta = &offertap1;

    /* Porto 2 */

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

    /* Porto porti[SO_PORTI] = {
    //     {1,2, mercip0, {TIPO0,3}, {TIPO1,  2}},
    //     {2,1, mercip1, {TIPO0,4}, {TIPO2,  4}},
    //     {3,4, mercip2, {TIPO0,7}, {TIPO2, 12}},
    //     {5,2, mercip3, {TIPO0,9}, {TIPO0,  5}},
    // }; */
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

    int i;
    for (i = 0; i < length; i++) {
        printf("%d,\n ", a[i]);
    }
    /* l = map(l, mapCriterio); */

    /* intStampaLista(l); */

    /* intFreeList(l); */


}

void testShm() {
    int shmid = createShm(IPC_PRIVATE, sizeof(int), NULL);
    int* shmAddr = (int*)getShmAddress(shmid, 0, NULL);
    *shmAddr = 5;
    printf("%d\n", *shmAddr);
    shmDetach(shmAddr, NULL);
    removeShm(shmid, NULL);
}


void testSemFunc() {
    int semid;
    semid = createMultipleSem(IPC_PRIVATE, 10, 0, NULL);
    printf("Creato il semaforo\n");

    mutexPro(semid, 8, LOCK, NULL);

}


/*
    si assume che i messaggi siano sempre scritti con questo 'pattern': giorno|quantita
*/
void mexParse(const char* mex, int* intDay, int* intQuantity) {
    int sizeDay;
    int sizeQuantity;
    int i;
    int c;
    int j;
    char* day;
    char* quantity;
    for (i = 0; *(mex + i); i++) {
        if(mex[i]=='|'){
            sizeDay = i;
            break;
        }
    }
    
    c = 0;
    
    for (i = i + 1; i < strlen(mex); i++) {
        c++;
    }
    sizeQuantity = c;
    

    day = malloc(sizeof(char) * sizeDay);
    quantity = malloc(sizeof(char) * sizeQuantity);

    for(i=0; i<sizeDay; i++){
        day[i] = mex[i];
    }
    i++;
    for (j = 0; j < sizeQuantity; j++) {
        quantity[j] = mex[i];
        i++;
    }
    
    *intDay = atoi(day);
    *intQuantity = atoi(quantity);
    free(day);
    free(quantity);
}
void testMexParse() {
    int day;
    int quantity;

    mexParse("2|12", &day, &quantity);
    printf("DAY: %d\nQUANITY: %d\n", day, quantity);
}

int* arr;

void testVariabileGlobale() {
    int i;
    
    for ( i = 0; i < 4; i++) {
        arr[i] = 2;
    }
    for ( i = 0; i < 4; i++) {
        printf("%d, ", arr[i]);
    }

    printf("\n");

}



void testFormuleMatrice() {
    int m[SO_DAYS][SO_PORTI];
    int i;
    int j;
    int idx;
    for (i = 0; i < SO_DAYS; i++) {
        for (j = 0; j < SO_MERCI; j++) {
            m[i][j] = 2;
        }
    }
    printf("indice del vettore: ");
    scanf("%d" , &idx);
    i = idx / SO_DAYS;
    j = idx % SO_MERCI;
    
    printf("vettore in posizione %d = matrice in posizione[%d][%d]\n", idx, i, j);


}

void testSemafori() {
    int semID;
    int key;
    int idx;
    int n;
    printf("Chiave del set e idx semaforo: ");

    
    scanf("%d %d", &key , &idx);
    printf("FACCIO LA SEM GET\n");
    semID = useSem(key, errorHandler);

    getAllVAlues(semID , 20);

    /*
    n = getWaitingPxCount(semID, idx);
    if(n == -1){
        perror("");
    }else{
        printf("\nNumero di px in attesa: %d\n", n);

    }*/
}
void testCode() {
    int queueID;
    int key;
    int res;
    mex* messaggioRicevuto;
    printf("Chiave della coda: ");
    scanf("%d" , &key);
    queueID = useQueue(key, errorHandler);
    printQueueState(queueID, errorHandler);

    do
    {
        printf("Vuoi leggere un messaggio? [0/1]\n");
        scanf("%d", &res);
        printf("\n");

        if (res) {
            messaggioRicevuto = msgRecv(queueID, 0, errorHandler, NULL, SYNC);
            printf("TIPO: %ld\n", messaggioRicevuto->mtype);
            printf("TESTO: '%s'\n", messaggioRicevuto->mtext);
        }

    } while (res == 1);
    
}

int main(int argc, char const* argv[])
{

    int s;

    /*test v. globale*/
    int i;
    arr = (int*)malloc(sizeof(int) * 4);

    for ( i = 0; i < 4; i++) {
        arr[i] = 0;
    }
    /*test v. globale*/
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
    case 2:
        testShm();
        break;
    case 3:
        
        break;
    case 4:
        testSemFunc();
        break;
    case 5:
        testMexParse();
        break;
    case 6:
        testVariabileGlobale();
         break;
    case 7:
        testFormuleMatrice();
        break;
    case 8:
        testSemafori();
        break;
    case 9:
        testCode();
    default:
        break;
    }

    return 0;
}
