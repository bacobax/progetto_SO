#include "./msg_utility.h"
#include "./vettoriInt.h"
#include "./support.h"
#include "./errorHandler.h"
#include "./shm_utility.h"
#include "./sem_utility.h"
#include "./loadShip.h"
#include "../config1.h"
#include "../src/nave.h"
#include "../src/porto.h"  
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/shm.h>
#define SO_MERCI 3

#define TIPO0 0
#define TIPO1 1
#define TIPO2 2

#define SO_PORTI 3


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

    for (i = 0; i < SO_("PORTI"); i++) {
        if (porti[i].domanda->tipo == tipo && porti[i].domanda->quantita <= quantita) {
            intPush(l, porti[i].domanda->quantita);
        }
    }
    if (l->length == 0) {
        return -1;
    }

    maxDomanda = max(l);


    for (i = 0; i < SO_("PORTI"); i++) {
        if (porti[i].domanda->tipo == tipo && porti[i].domanda->quantita == maxDomanda) {
            idx = i;
            break; /* facoltativo */
        }
    }
    return idx;

}
/*

void test0() {

    /* Porto 0 
    Porto p0;
    int mercip0[] = { 4, 3, 3 };

    Trade domandap0 = { TIPO0, 12 };
    Trade offertap0 = { TIPO1,  2 };

    p0.x = 1;
    p0.y = 2;
    p0.merci = mercip0;
    p0.domanda = &domandap0;
    p0.offerta = &offertap0;

    /* Porto 1 
    
    Porto p1;

    int mercip1[] = { 5, 2, 9 };

    Trade domandap1 = { TIPO0, 10 };
    Trade offertap1 = { TIPO2,  7 };

    p1.x = 1;
    p1.y = 2;
    p1.merci = mercip1;
    p1.domanda = &domandap1;
    p1.offerta = &offertap1;

    /* Porto 2 

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
    // }; 
    int idx = scegliPortoPerOffrire(TIPO0, 8, porti);
    printf("Il miglior porto è il porto %d\n", idx);

}*/

int mapCriterio(int el, int idx) {
    return el * 2;
}

void test1() {

    int q;
    int n;
    srand(time(NULL));
    printf("Inserire quantità: ");
    scanf("%d", &q);
    printf("Inserire parti: ");
    scanf("%d", &n);
    
    intList* l = distributeV1(q, n);

    intStampaLista(l);
    
    /* l = map(l, mapCriterio); */

    /* intStampaLista(l); */

    /* intFreeList(l); */


}

void testShm() {
    int shmid = createShm(IPC_PRIVATE, sizeof(int), NULL, "dfs");
    int* shmAddr = (int*)getShmAddress(shmid, 0, NULL, "fd");
    *shmAddr = 5;
    printf("%d\n", *shmAddr);
    shmDetach(shmAddr, NULL, "dsf");
    removeShm(shmid, NULL, "dfs");
}


void testSemFunc() {
    int semid;
    semid = createMultipleSem(IPC_PRIVATE, 10, 0, NULL, "testSemFunc");
    printf("Creato il semaforo\n");

    mutexPro(semid, 8, LOCK, NULL, "testSemFunc");

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
    semID = useSem(key, errorHandler, "testSemafori");

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
    int ftokId;
    char text[1024];
    mex* messaggioRicevuto;
    printf("Vuoi usare ftok? ");
    scanf("%d", &res);
    if (res) {
        printf("Inserire nome file:\n");
        scanf("%s", text);
        printf("Nome file: %s\n", text);
        printf("Inserire id: \n");
        scanf("%d", &ftokId);
        key = ftok(text, ftokId);
        printf("\nkey = %d\n", key);
    }
    else {
        printf("Chiave della coda: ");
        scanf("%d" , &key);
    }

    
    queueID = useQueue(key, errorHandler, "testCode");
    printQueueState(queueID, errorHandler ,"testCode printQueueState");

    do
    {
        printf("Vuoi leggere un messaggio? [0/1]\n");
        scanf("%d", &res);
        printf("\n");

        if (res) {
            messaggioRicevuto = msgRecv(queueID, 0, errorHandler, NULL, SYNC, "testCode");
            printf("TIPO: %ld\n", messaggioRicevuto->mtype);
            printf("TESTO: '%s'\n", messaggioRicevuto->mtext);
        }

    } while (res == 1);
    
}
/*
void printLoadShipTest(loadShip list){
    Product node;
    node = list->first;
    printf("Lenght della lista:%d && Carico trasportato:\n", list->length);
    int i = 0;
    while(node!=NULL){
        if(node->next == NULL){
            printf("Nodo:%d - address:%d il mio next e'->:NULL\n", i, node);
        } else {
            printf("Nodo:%d - address:%d il mio next e'->:%d\n", i, node, node->next);
        }
        printf("product_type:%d weight:%d expirationTime:%d distributionDay:%d portId:%d\n\n", node->product_type, node->weight, node->expirationTime, node->distributionDay, node->portID);
        node = node->next;
        i++;
    }
    printf("--------------------------------------------------\n");
}

void printShipTest(Ship ship){
    printf("Nave id:%d e weight:%d\n", ship->shipID, ship->weight);
    printLoadShipTest(ship->loadship);

}*/
/*
Ship initShipTest(){
    Ship ship;
    ship = (Ship)malloc(sizeof(struct ship));
    ship->loadship = initLoadShip();
    printf("ship inizializzata...\n");
    /*printShipTest(ship);
    return ship;
}*/

void testLoadShip(){
    /*
Ship ship = initShipTest();

    Product prod1 = (Product) malloc(sizeof(struct productNode_));
    prod1->product_type = 1;
    prod1->weight = 150;
    prod1->next = NULL;

    Product prod2 = (Product) malloc(sizeof(struct productNode_));
    prod2->product_type = 2;
    prod2->weight = 50;
    prod2->next = NULL;

    Product prod3 = (Product) malloc(sizeof(struct productNode_));
    prod3->product_type = 3;
    prod3->weight = 100;
    prod3->next = NULL;


    addProduct(ship, prod1, NULL);
    printShipTest(ship);
    
    addProduct(ship, prod2, NULL);
    printShipTest(ship);
    
    
    addProduct(ship, prod3, NULL);
    printShipTest(ship);

    */
    
    
    /*removeProduct(ship, 2);            LA RIMOZIONE DI UN PRODOTTO FUNZIONA*/
    
    /*
    removeProduct(ship, 2);              ANCHE IN QUESTO CASO FUNZIONA
    printShipTest(ship);
    removeProduct(ship, 0);
    printShipTest(ship);

    */

   /* QUESTO PRIMA DAVA CORE DUMP POI L'HO RISOLTO CON IL CONTROLLO DELL'INDEX
   removeProduct(ship, 0);
   printShipTest(ship);

     prod3 = (Product) malloc(sizeof(struct productNode_));
    prod3->product_type = 3;
    prod3->weight = 100;
    prod3->next = NULL;

   addProduct(ship, prod3, NULL);
   printShipTest(ship);
   
*/
   /*
   removeProduct(ship, 0);              RIMOZIONE DI ELEMENTI UNO DOPO L'ALTRO FUNZIONA
   removeProduct(ship, 1);
   removeProduct(ship, 0);
   printShipTest(ship);
   */
   
   /*
   removeProduct(ship, 1);            
   removeProduct(ship, 2);
   removeProduct(ship, 0);
   printShipTest(ship);
    */

   /*
   addProduct(ship, prod2, NULL);
    printShipTest(ship);*/
    
    /*
    addProduct(ship, prod3, NULL);
    printShipTest(ship);

    removeProduct(ship, 3);
    printShipTest(ship);
    removeProduct(ship, 5);
    printShipTest(ship);
    */
}

void constants() {
    int res = SO_("FILL");
    printf("Variabile: %d\n", res);
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
        
        break;
    case 1:
        test1();
        break;
    case 2:
        testShm();
        break;
    case 3:
        testLoadShip();
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
        break;
    case 10:
        constants();
        break;
    default:
        break;
    }

    return 0;
}
