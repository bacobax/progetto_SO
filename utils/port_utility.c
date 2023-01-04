#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <time.h>
#include "../src/porto.h"
#include "../src/nave.h"
#include "../src/dump.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include "./msg_utility.h"
#include "./support.h"
#include "./vettoriInt.h"

void refillerQuitHandler(int sig) {
    printf("refiller: ricevuto segnale di terminazione\n");
    exit(EXIT_SUCCESS);
}


Port initPort(int supplyDisponibility,int requestDisponibility, int pIndex) {

    int portShmId;
    int length;
    Port p;
    /*
        array nel quale verrà spartita casualmente la domanda in SO_MERCI parti
    */
    int* requests;
    /*
        array nel quale verrà spartita casualmente l'offerta in SO_MERCI parti
    */
    int* supplies;
    int i;
    int j;


    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler, "initPort");

    p = ((Port)getShmAddress(portShmId, 0, errorHandler, "initPort")) + pIndex;

    /*
        Distribuisco randomicamente domanda e offerta
    */
    requests = toArray(distribute(requestDisponibility, SO_MERCI), &length);
    supplies = toArray(distribute(supplyDisponibility, SO_MERCI), &length);

    /*
        informo l'area di dump delle risorse nel porto,
        lo faccio prima di azzerare delle offerte, perchè almeno vengono contate anche le merci che il porto non
        può offrire perchè c'è già la domanda

    */

    
    /*
        assegno la richiesta
    */
    copyArray(p->requests, requests, length);
    /*
        assegno la domanda usando la funzione d'appoggio
    */
    fillMagazine(&p->supplies, 0, supplies);

    free(requests);
    free(supplies);

    fillExpirationTime(&p->supplies);

    /*
        Questa è una parte importante, viso che avevo un certo grado di libertà su come affrontare il fatto
        che non ci potesse essere domanda e offerta dello stesso tipo di merce in un porto, ho deciso di azzerare le ton del tipo di
        merce che non viene offerta, e azzerare le ton del tipo di merce che non viene richiesta, tanto quella merce è solo destinata a scadere.
        Per decidere quale tipo di merce offrire e richiedere lascio anche questo al caso con questo ciclo for:
        scorre tutti i tipi di merce e decide di azzerare l'offerta o la domanda, 
        
    */

    for (i = 0; i < SO_MERCI; i++) {
        int c = rand() % 2;
        if (c == 1) {
            p->requests[i] = 0;
        }
        else if (c == 0) {
            addExpiredGood(p->supplies.magazine[0][i], i, PORT);
            p->supplies.magazine[0][i] = 0;
        }
        else {
            printf("Errore nella rand");
            exit(EXIT_FAILURE);
        }
    }

    /*
    Azzero tutti tipi delle risorse degli altri giorni
    */
    for (i = 1; i < SO_DAYS; i++) {
        for (j = 0; j < SO_MERCI; j++) {
            p->supplies.magazine[i][j] = -1;
        }
    }


    if (pIndex == 0) {
        p->x = 0;
        p->y = 0;
    }
    else if (pIndex == 1) {
        p->x = SO_LATO;
        p->y = 0;
    }
    else if (pIndex == 2) {
        p->x = SO_LATO;
        p->y = SO_LATO;
    }
    else if (pIndex == 3) {
        p->x = 0;
        p->y = SO_LATO;
    }
    else {
        p->x = (double)rand() / (double)(RAND_MAX / (SO_LATO));
        p->y = (double)rand() / (double)(RAND_MAX / (SO_LATO));
    }

    reservePrint(printPorto, p, pIndex);


    return p;
}



void printPorto(void* p, int idx) {

    int i;
    printf("[%d]Porto %d:\n", getpid(),idx);
    printf("DOMANDE:\n");
    for (i = 0; i < SO_MERCI; i++) {
        printf("%d, \n", ((Port)p)->requests[i]);
    }

    printSupplies(((Port)p)->supplies);

    printf("coords:\n");
    printf("x: %f\n", ((Port)p)->x);
    printf("y: %f\n", ((Port)p)->y);

    printf("______________________________________________\n");

}




int filterIdxs(int request) {
    return request != 0 && request != -1;
}


/* TODO: risolvere bug che non fa il parse del day del messaggio */
void refill(long type, char* text) {

    
    /*
        correctType varrà l'indice del  porto al quale il type del messaggio riferito farà riferimento,
        correcType = type - 1 perchè quando il master invia una certa quantità ad un certo porto, e vuole usare il type per riferirsi all'indice del porto,
        non può usare type = 0 perchè quel valore è riservato
    */
    int correctType;
    /*
        id della lista di semafori per cui semaforo[i] serve per bloccare scritture contemporanee nelle offerte del porto i
        (quindi se per esempio una nave deve prelevare deve assicurarsi che in quell'istante non ci sia un'operazione di riempimento delle risorse)
    */
    int portBufferSem;

    /*
        il refiller è solo un listener, non può sapere in che giorno ha appena ricevuto le risorse da distribuire,
        quindi il master, ogni giorno invia le risorse al refiller di tutti i porti specificando nel messaggio anche il giorno
        nel quale le sta inviando
    */
    int day;

    /*
        quantity è la quantità giornaliera che il master invia ogni giorno al refiller,
        ricavata dal messaggio
    */
    int quantity;

    /*
        ID della shm dei porti
    */
    int portShmID;
    Port p;

    /*
        array lungo SO_MERCI nel quale la quantità giornaliera sarà casualmente spartita tra le merci
    */
    int* quanties;
    /*
        length è stato dichiarato più per formalismo, perchè la funzione toArray restituisce anche la lunghezza dell'array
        anche se sappiamo che length = SO_MERCI
    */
    int length;
    /*
        lista dinamica che conterrà gli indici delle posizioni dell'array delle offerte nelle quali l'offerta per quel tipo di merce è a 0
        e ovviamente se è a 0 è perchè c'è già la domanda per quel tipo di merce
    */
    intList* listOfIdxs;

    int tipoMerceDaAzzerare;
    
    int i;
    /*
        semaforo che verrà decrementato così che il master passi la waitzero del semaforo waitEndDay
    */
    int waitEndDaySemID;

    waitEndDaySemID = useSem(WAITENDDAYKEY, errorHandler, "refill->waitEndDaySem");
    

    srand((int)time(NULL) % getpid());

    correctType = (int)(type - 1);
    
    portShmID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler, "refill");
    p = (Port)getShmAddress(portShmID, 0, errorHandler, "refill") + correctType;


    /*
        contiene gli indici delle domande != 0, così da sapere quali indici dell'offerta azzerare
    */
    listOfIdxs = findIdxs(p->requests, SO_MERCI, filterIdxs);
    /*
        printf("Ricevuto il messaggio %s\n", text);
    */


    sscanf(text, "%d|%d", &day, &quantity);

    
    /*
        distribuisco le quantità da aggiungere
    */
    quanties = toArray(distribute(quantity, SO_MERCI), &length);

    /*
        semaforo per modificare il magazzino dei porti
    */
    portBufferSem = useSem(RESPORTSBUFFERS, errorHandler , "refill->useSem portBufferSem");


    mutexPro(portBufferSem, (int)correctType, LOCK, errorHandler, "refill->portBufferSem LOCK");
    /* fillMagazine(&p->supplies, 0, supplies); */

    fillMagazine(&p->supplies, day, quanties);


    /*
        Azzero le offerte del tipo di merce per cui c'è già la domanda
    */

    for (i = 0; i < listOfIdxs->length; i++) {
        tipoMerceDaAzzerare = *(intElementAt(listOfIdxs, i));
        /*
            scelgo di marcare direttamente come merce scaduta la merce dell'offerta che
            non potrà mai essere offerta per il fatto che c'è già la domanda per quel tipo di merce,

        */
        addExpiredGood(p->supplies.magazine[day][tipoMerceDaAzzerare], tipoMerceDaAzzerare, PORT);
        
        p->supplies.magazine[day][tipoMerceDaAzzerare] = 0;
    }

    
    mutexPro(portBufferSem, (int)correctType, UNLOCK, errorHandler, "refill->portBufferSem UNLOCK");
    /*
        reservePrint(printPorto, p, correctType);

    */
    
    shmDetach(p-correctType, errorHandler, "refill");

    /*
        decremento il semaforo per cui il master sta aspettando lo 0 per iniziare il nuovo giorno   
    */
    mutex(waitEndDaySemID, -1, errorHandler, "refill->waitEndDaySem LOCK");
}

void refillerCode(int idx) {
    /*
        questo è una sorta di listener, che ascolta sempre in attesa di un messaggio per l'idx passatogli come argomento
        (indice del porto proprietario del refiller)
    */
    int refillerID;
    if (signal(SIGUSR1, refillerQuitHandler) == SIG_ERR) {
        perror("Refiller: non riesco a settare il signal handler\n");
        exit(EXIT_FAILURE);
    }

    refillerID = useQueue(REFILLERQUEUE, errorHandler, "useQueue in refillerCode");

    while (1) {
        /*
            idx+1 perchè nella coda di messaggi ci si riferisce all'indice di ogni porto incrementato di 1
            questo perchè type = 0 è riservato
        */
        msgRecv(refillerID, (long)(idx + 1), errorHandler, refill, ASYNC, "refillerCode");
    }
}

void launchRefiller(int idx) {
    int pid = fork();

    if (pid == -1) {
        perror("Error launching the refiller");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        refillerCode(idx);
        exit(EXIT_FAILURE);
    }
}


void mySettedPort(int supplyDisponibility, int requestDisponibility, int idx, void(*codicePorto)(Port porto, int myQueueID, int shipsQueueID, int idx)) {
     
    void (*oldHandler)(int);
    int i;
    Port p;
    int msgQueueID;
    int shipQueueID;
    /*
        questo perchè per qualche motivo srand(time(NULL)) non generava unici seed tra un processo unico e l'altro
        fonte della soluzione: https://stackoverflow.com/questions/35641747/why-does-each-child-process-generate-the-same-random-number-when-using-rand
        da quel che ho capito il bug era dovuto al fatto che le fork dei vari figli sono avvenute nello stesso secondo
    */
    srand((int)time(NULL) % getpid());

    /*
        setto il segnale che gestisce il segnle di terminazione che invia il master
    */
    oldHandler = signal(SIGUSR1, quitSignalHandler);
    if (oldHandler == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    

    p = initPort(supplyDisponibility,requestDisponibility, idx);



    launchRefiller(idx);

    checkInConfig();
    printf("P: finito configurazione\n");

    msgQueueID = 1;

    shipQueueID = 1;
    
    /*
        da aggiungere le due useQueue per le code di scaricamento
    */

    codicePorto(p, msgQueueID,shipQueueID, idx);


}

void dischargerCode(void (*recvHandler)(long, char*), int idx) {
     int requestPortQueueID;
     mex* res;
    requestPortQueueID = useQueue(PQUERECHKEY, errorHandler, "dischargerCode");
    while (1) {

        /*
            E' importante che sia sincrona la gestione del messaggio ricevuto
            perchè prima di poterne ricevere un altro il porto deve poter aver aggiornato le sue disponibilità
        */

        /*
            prendo il primo messaggio che arriva
        */
        res = msgRecv(requestPortQueueID, idx + 1, errorHandler, recvHandler, ASYNC, "dischargerCode");
         

         
    }
}

void chargerCode(void (*recvHandler)(long, char*), int idx) {
     int requestPortQueueID;
     mex* res;
    requestPortQueueID = useQueue(PQUEREDCHKEY, errorHandler, "dischargerCode");
    while (1) {

        /*
            E' importante che sia sincrona la gestione del messaggio ricevuto
            perchè prima di poterne ricevere un altro il porto deve poter aver aggiornato le sue disponibilità
        */

        /*
            prendo il primo messaggio che arriva
        */
        res = msgRecv(requestPortQueueID, idx + 1, errorHandler, recvHandler, ASYNC,"chargerCode");
        
    }
}

/* per operazioni di carico della nave*/
void launchDischarger(void (*recvHandler)(long, char*), int idx) {
    int pid;
    pid = fork();
    if (pid == -1) {
        perror("Errore nel lanciare il discharger");
        exit(1);
    }
    if (pid == 0) {
        dischargerCode(recvHandler, idx);
        exit(EXIT_FAILURE);
    }
    
}


/* per operazioni di scarico della nave*/
void launchCharger(void (*recvHandler)(long, char*), int idx) {
    int pid;
    pid = fork();
    if (pid == -1) {
        perror("Errore nel lanciare il charger");
        exit(1);
    }
    if (pid == 0) {
        chargerCode(recvHandler, idx);
        exit(EXIT_FAILURE);
    }
}
int checkRequests(Port p, int type, int quantity) {
    int diff;
    int n = p->requests[type];
    if (n == 0) return -1;
    if (quantity >= p->requests[type]) {
        p->requests[type] = 0;
    }
    else {
        p->requests[type] -= quantity;
    }
    return n;
}

int allRequestsZero(){
    int portShmid;
    Port portArr;
    int i;
    int j;
    int cond = 1;
    portShmid = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler,"allRequestsZero");
    portArr = getShmAddress(portShmid,0,errorHandler,"allRequestsZero");

    for(i=0; i<SO_PORTI && cond; i++){
        for(j=0;j<SO_MERCI && cond; j++){
            if(portArr[i].requests[j] > 0){
                cond = 0;
            }
        }
    }

    shmDetach(portArr,errorHandler,"allRequestsZero");
    return cond;
}
int filter(int el){
    return el!=0;
}



intList* tipiDiMerceOfferti(Port p) {
    intList* ret;
    intList* aux;
    int i;
    int j;
    ret = intInit();

    for(i=0; i<SO_DAYS; i++){
        aux = findIdxs(p->supplies.magazine[i], SO_MERCI,filterIdxs);
        ret = intUnion(ret, aux);
    }
    return ret;

}

intList* tipiDiMerceRichiesti(Port p){
    return findIdxs(p->requests, SO_MERCI,filterIdxs);
}

intList* getAllTypeRequests(Port portArr) {
    int i;
    intList* ret = intInit();
    intList* tipiRichiesti;
    for (i = 0; i < SO_PORTI; i++) {
        tipiRichiesti = tipiDiMerceRichiesti(portArr + i);
        ret = intUnion(ret, tipiRichiesti);
        intFreeList(tipiRichiesti);
    }
    return ret;
}


int haSensoContinuare() {
    int portShmid;
    Port portArr;
    int i;
    int j;
    int cond = 1;
    intList* merciTotaliRichieste = intInit();
    intList* merciTotaliOfferte = intInit();
    intList* aux0;
    intList* aux1;

    portShmid = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler,"allRequestsZero");
    portArr = getShmAddress(portShmid,0,errorHandler,"allRequestsZero");

    for(i=0;i<SO_PORTI; i++){
        aux0 = tipiDiMerceRichiesti(portArr + i);
        aux1 = tipiDiMerceOfferti(portArr + i);
        merciTotaliRichieste = intUnion(merciTotaliRichieste,aux0);
        merciTotaliOfferte = intUnion(merciTotaliOfferte, aux1);
        intFreeList(aux0);
        intFreeList(aux1);
    }
    printf("MERCI TOTALI RICHIESTE: \n");
    intStampaLista(merciTotaliRichieste);
    
    printf("MERCI TOTALI OFFERTE: \n");
    intStampaLista(merciTotaliOfferte);

    shmDetach(portArr,errorHandler,"allRequestsZero");
    cond = intIntersect(merciTotaliOfferte, merciTotaliRichieste)->length!=0;
    intFreeList(merciTotaliOfferte);
    intFreeList(merciTotaliRichieste);

    return cond;
}


