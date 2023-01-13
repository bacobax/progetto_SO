#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <time.h>
#include "../src/porto.h"
#include "../src/nave.h"
#include "../src/dump.h"
#include "./errorHandler.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include "./msg_utility.h"
#include "./support.h"
#include "./vettoriInt.h"

void refillerQuitHandler(int sig) {
    
    kill(0, SIGUSR1);
    
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

    signal(SIGCHLD, SIG_IGN);
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
        p->x = generateCord();
        p->y = generateCord();
    }
    /*
    reservePrint(printPorto, p, pIndex);
    
    */


    return p;
}



void printPorto(Port p, int idx, FILE* stream) {

    int i;
    fprintf(stream, "[%d]Risorse porto %d:\n", getpid(),idx);
    fprintf(stream,"DOMANDE:\n");
    for (i = 0; i < SO_MERCI; i++) {
        fprintf(stream,"%d, \n", p->requests[i]);
    }

    printSupplies(p->supplies, stream);

    fprintf(stream,"coords:\n");
    fprintf(stream, "x: %f\n", p->x);
    fprintf(stream,"y: %f\n",  p->y);

    fprintf(stream ,"______________________________________________\n");

}




int filterIdxs(int request) {
    return request != 0 && request != -1 && request != -2;
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
    signal(SIGCHLD, SIG_IGN);
    
    refillerID = useQueue(REFILLERQUEUE, errorHandler, "useQueue in refillerCode");

    clearSigMask();
    while (1)
    {
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
        throwError("Error launching the refiller","launchRefiller");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        refillerCode(idx);
        exit(EXIT_FAILURE);
    }
}


void mySettedPort(int supplyDisponibility, int requestDisponibility, int idx, void(*codicePorto)(int endShmId, int idx,int aspettoMortePortiSemID, int aspettoMorteNaviSemID)) {
     
    void (*oldHandler)(int);
    int endShmId;
    int aspettoMortePortiSemID;
    int aspettoMorteNaviSemID;
    Port p;
    aspettoMortePortiSemID = useSem(WAITPORTSSEM, errorHandler, "aspettoMortePortiSemID in codicePorto");
    aspettoMorteNaviSemID = useSem(WAITSHIPSSEM, errorHandler, "aspettoMortePortiSemID in codicePorto");
  
    /*
        questo perchè per qualche motivo srand(time(NULL)) non generava unici seed tra un processo unico e l'altro
        fonte della soluzione: https://stackoverflow.com/questions/35641747/why-does-each-child-process-generate-the-same-random-number-when-using-rand
        da quel che ho capito il bug era dovuto al fatto che le fork dei vari figli sono avvenute nello stesso secondo
    */
    srand((int)time(NULL) % getpid());

    /*
        setto il segnale che gestisce il segnle di terminazione che invia il master
    */
    
    endShmId = useShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "mySettedPort");

    p = initPort(supplyDisponibility,requestDisponibility, idx);



    launchRefiller(idx);

    checkInConfig();
    printf("P: finito configurazione\n");

    
    
    /*
        da aggiungere le due useQueue per le code di scaricamento
    */

    codicePorto(endShmId, idx, aspettoMortePortiSemID, aspettoMorteNaviSemID);


}

void dischargerCode(void (*recvHandler)(long, char*), int idx) {
    int requestPortQueueID;
    mex* res;
    signal(SIGCHLD, SIG_IGN);
    
    requestPortQueueID = useQueue(PQUERECHKEY, errorHandler, "dischargerCode");

    clearSigMask();

    while (1) {

        /*
            E' importante che sia sincrona la gestione del messaggio ricevuto
            perchè prima di poterne ricevere un altro il porto deve poter aver aggiornato le sue disponibilità
        */

        /*
            prendo il primo messaggio che arriva
        */
        res = msgRecv(requestPortQueueID, idx + 1, errorHandler, recvHandler, ASYNC, "dischargerCode");
        sleep(0.2);

         
    }
}

void chargerCode(void (*recvHandler)(long, char*), int idx) {
    int requestPortQueueID;
    mex* res;
    signal(SIGCHLD, SIG_IGN);
    
    requestPortQueueID = useQueue(PQUEREDCHKEY, errorHandler, "dischargerCode");
    clearSigMask();
   
    while (1) {

        /*
            E' importante che sia sincrona la gestione del messaggio ricevuto
            perchè prima di poterne ricevere un altro il porto deve poter aver aggiornato le sue disponibilità
        */

        /*
            prendo il primo messaggio che arriva
        */
        res = msgRecv(requestPortQueueID, idx + 1, errorHandler, recvHandler, ASYNC,"chargerCode");
        sleep(0.2);

    }
}

/* per operazioni di carico della nave*/
void launchDischarger(void (*recvHandler)(long, char*), int idx) {
    int pid;
    pid = fork();
    if (pid == -1) {
        throwError("Errore nel lanciare il discharger","launchDischarger");
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
        throwError("Errore nel lanciare il charger","launchCharger");
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
    return el!=-2 ;
}

int filterReq(int req){
    return req == 0;
}
intList* validSupplies(Port p){
    int i;
    intList* ret = intInit();
    for (i = 0; i < SO_MERCI; i++) {
        if (validSupply(p->supplies, i)) {
            intPush(ret, i);
        }
    }
    return ret;
}

intList* tipiDiMerceOfferti(Port p) {
    intList* tipiMerceSenzaRichiesta = findIdxs(p->requests, SO_MERCI, filterReq);
    intList* tipiMerceOffertaMaggioreDiZero = validSupplies(p);
    intList* ret;
    ret = intIntersect(tipiMerceSenzaRichiesta, tipiMerceOffertaMaggioreDiZero);
    intFreeList(tipiMerceSenzaRichiesta);
    intFreeList(tipiMerceOffertaMaggioreDiZero);
    return ret;
}

intList* tipiDiMerceRichiesti(Port p){
    return findIdxs(p->requests, SO_MERCI,filterIdxs);
}


intList* getAllOtherTypeRequests(Port portArr, int idx) {
    int i;
    intList* ret = intInit();
    intList* tipiRichiesti;
    for (i = 0; i < SO_PORTI; i++) {
        if(i!=idx){
            tipiRichiesti = tipiDiMerceRichiesti(portArr + i);
            ret = intUnion(ret, tipiRichiesti);
            intFreeList(tipiRichiesti);
        }
        
    }
    return ret;
}


intList* haSensoContinuare() {
    Port portArr;
    int i;
    int j;
    intList *inter;
    intList *merciTotaliRichieste = intInit();
    intList* merciTotaliOfferte = intInit();
    intList* aux0;
    intList* aux1;

    portArr = getPortsArray();

    for(i=0;i<SO_PORTI; i++){
        aux0 = tipiDiMerceRichiesti(portArr + i);
        aux1 = tipiDiMerceOfferti(portArr + i);
        merciTotaliRichieste = intUnion(merciTotaliRichieste,aux0);
        merciTotaliOfferte = intUnion(merciTotaliOfferte, aux1);
        intFreeList(aux0);
        intFreeList(aux1);
    }
    
    
    shmDetach(portArr,errorHandler,"allRequestsZero");
    inter = intIntersect(merciTotaliOfferte, merciTotaliRichieste);
    intFreeList(merciTotaliOfferte);
    intFreeList(merciTotaliRichieste);

    return inter;
}


double getValue(int quantity, int scadenza, int tipo, Port arrPorts, int idx) {
    intList *tipiDiMerceRichiestiAltriPorti = getAllOtherTypeRequests(arrPorts, idx);
    if (scadenza == 0 || contain(tipiDiMerceRichiesti(arrPorts + idx) , tipo) || !contain(tipiDiMerceRichiestiAltriPorti, tipo))
    {
        return 0;
    }
    else /*scadenza > 0 && le mie richieste non contengono il tipo di merce di questa offerta && le richieste degli altri porti contengono il tipo di merce di questa offerta*/
    {
        return quantity / (double)scadenza;
    }
}


int trovaTipoEScadenza(Supplies* S, int* tipo, int* dayTrovato, int* scadenza, int quantity, Port arrPorts, int idx) {
    int i;
    int j;
    /*
    il valore dev'essere di tipo double perchè sennò le volte che 0 < quantity/scadenza < 1,
    cioè quando il tempo di vita rimanente è maggiore della quantità, value diventa 0
    */
    double value = 0;
    int ton;
    double currentValue;
    int currentScadenza;
    int res;

    *tipo = -1;
    *scadenza = -1;
    *dayTrovato = -1;
    for (i = 0; i < SO_DAYS; i++) {
        for (j = 0; j < SO_MERCI; j++) {
            
            ton = S->magazine[i][j];
            
            currentScadenza = getExpirationTime(*S, j, i);
            currentValue = getValue(ton, currentScadenza,j ,arrPorts,idx);
            if (ton >= quantity && currentValue > value) {
                value = currentValue;
                *tipo = j;
                *dayTrovato = i;
                *scadenza = currentScadenza;
            }
        }
        
    }
    
    if (*tipo == -1 && *scadenza == -1 && *dayTrovato == -1) {
        res = -1;
    }
    else {
        /*
                Operazione importante: decremento della quantità richiesta in anticipo, così nel mentre altre navi possono scegliere
                lo stesso tipo di merce con la quantità aggiornata
            */
            S->magazine[*dayTrovato][*tipo] -= quantity;
            printf("PORTO: tolgo %d\n" ,quantity);

            addNotExpiredGood(0 - quantity, *tipo, PORT, 0, idx);
        res = 1;
    }
    return res;

}

int countPortsWhere(Port arrPort , int(*f)(int,Port)){
    int count;
    int i;
    count = 0;
    for (i = 0; i < SO_PORTI; i++){
        if(f(i,arrPort+i)){
            count++;
        }
    }
    return count;
}

int caughtBySwell(int idx, Port p){
    return p->weatherTarget;
}

void printStatoPorti(FILE *fp, Port portArr){
    int semPierID;
    int i;
    int c;
    int k;
    Supplies s;
    semPierID = useSem(BANCHINESEMKY, errorHandler, "printStatoNavi");
    fprintf(fp, "Porti interessati da mareggiata: %d\n" , countPortsWhere(portArr, caughtBySwell));
    for (i = 0; i < SO_PORTI; i++)
    {
        fprintf(fp, "Porto %d:\n", i);
        fprintf(fp, "Banchine occupate totali: %d\n", SO_BANCHINE - getOneValue(semPierID, i));
        fprintf(fp, "Merci ricevute: %d\n", portArr[i].deliveredGoods);
        fprintf(fp, "Merci spedite: %d\n", portArr[i].sentGoods);
        printPorto(portArr + i, i, fp);
    }
}


void restorePromisedGoods(Port porto, int dayTrovato, int tipoTrovato, int quantity, int myPortIdx){
    printf("Porto %d, non sono stato scelto anche se avevo trovato della rob\n", myPortIdx);
    porto->supplies.magazine[dayTrovato][tipoTrovato] += quantity;
    printf("PORTO %d: riaggiungo %d\n" , myPortIdx, quantity);
    
    addNotExpiredGood(quantity, tipoTrovato, PORT, 0, myPortIdx);
    /*
        SE LA MERCE E' SCADUTA MENTRE IL PORTO ASPETTAVA DI SAPERE SE E' STATO SCELTO
        SE LA MERCE FOSSE SCADUTA PRIMA IL PROBLEMA NON ESISTEREBBE
    */
    if(getExpirationTime(porto->supplies,tipoTrovato, dayTrovato)== 0){
        addExpiredGood(quantity, tipoTrovato, PORT);
    }
}
