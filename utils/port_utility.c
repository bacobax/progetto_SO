#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
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




Port initPort(int supplyDisponibility,int requestDisponibility, int pIndex) {

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
    int so_lato;
    int key;
    int reqShmID;
    int* reqs;
    int* magazine;
    int so_merci = SO_("MERCI");
    int so_days = SO_("DAYS");
    so_lato = SO_("LATO");
    signal(SIGCHLD, SIG_IGN);

    p = getPort(pIndex);

    /*
        Distribuisco randomicamente domanda e offerta
    */
    requests = toArray(distributeV1(requestDisponibility, so_merci), &length);
    supplies = toArray(distributeV1(supplyDisponibility, so_merci), &length);

    /*
        informo l'area di dump delle risorse nel porto,
        lo faccio prima di azzerare delle offerte, perchè almeno vengono contate anche le merci che il porto non
        può offrire perchè c'è già la domanda

    */
   
    p->requestsID = createShm(ftok("./utils/errorHandler.c", pIndex), so_merci * sizeof(int), errorHandler, "initPort");
    p->supplies.magazineID = createShm(ftok("./utils/supplies.c", pIndex), sizeof(int) * so_days * so_merci, errorHandler, "init port magazine");
    p->supplies.expirationTimesID = createShm(ftok("./utils/supplies.h", pIndex), sizeof(int) * so_days * so_merci, errorHandler, "init port expTimes");
    reqs = (int*)getShmAddress(p->requestsID, 0, errorHandler, "initPort");
    magazine = getMagazine(p);

    
    /*
        assegno la richiesta
    */
    copyArray(reqs, requests, length);
    /*
        assegno la domanda usando la funzione d'appoggio
    */
    fillMagazine(magazine, 0, supplies);

    free(requests);
    free(supplies);

    fillExpirationTime(&p->supplies);

    /*
        Questa è una parte importante, viso che avevo un certo grado di libertà su come affrontare il fatto
        che non ci potesse essere domanda e offerta dello stesso type di merce in un porto, ho deciso di azzerare le ton del type di
        merce che non viene offerta, e azzerare le ton del type di merce che non viene richiesta, tanto quella merce è solo destinata a scadere.
        Per decidere quale type di merce offrire e richiedere lascio anche questo al caso con questo ciclo for:
        scorre tutti i tipi di merce e decide di azzerare l'offerta o la domanda, 
        
    */

    for (i = 0; i < so_merci; i++) {
        int c = rand() % 2;
        if (c == 1) {
            reqs[i] = 0;
        }
    }

    /*
    Azzero tutti tipi delle risorse degli altri giorni
    */
    for (i = 1; i < so_days; i++) {
        for (j = 0; j < so_merci; j++) {
            setMagazineVal(magazine, i, j, -1);
        }
    }


    if (pIndex == 0) {
        p->x = 0;
        p->y = 0;
    }
    else if (pIndex == 1) {
        p->x = so_lato;
        p->y = 0;
    }
    else if (pIndex == 2) {
        p->x = so_lato;
        p->y = so_lato;
    }
    else if (pIndex == 3) {
        p->x = 0;
        p->y = so_lato;
    }
    else {
        p->x = generateCord();
        p->y = generateCord();
    }
    /*
    reservePrint(printPort, p, pIndex);
    
    */
    shmDetach(reqs, errorHandler, "initPort detach requests");
    shmDetach(magazine, errorHandler, "init port magazine");
    return p;
}



void printPort(Port p, int idx, FILE* stream) {

    int i;
    int* reqs;
    int* magazine;
    int so_merci = SO_("MERCI");
    
    fprintf(stream, "[%d]Risorse porto %d:\n", getpid(), idx);
    fprintf(stream, "DOMANDE:\n");
    reqs = getShmAddress(p->requestsID, 0, errorHandler, "print porto");
    for (i = 0; i < so_merci; i++) {
        fprintf(stream,"%d, \n", reqs[i]);
    }
    shmDetach(reqs, errorHandler, "print porto");
    magazine = getMagazine(p);
    printSupplies(p->supplies, stream, magazine);
    shmDetach(magazine, errorHandler, "printPort magazine");
    fprintf(stream,"coords:\n");
    fprintf(stream, "x: %f\n", p->x);
    fprintf(stream,"y: %f\n",  p->y);

    fprintf(stream ,"______________________________________________\n");

}




int filterIdxs(int request) {
    return request != 0 && request != -1 && request != -2;
}


void refill(long type, char* text) {

    int so_merci = SO_("MERCI");    
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
    int* magazine;

    waitEndDaySemID = useSem(WAITENDDAYKEY, errorHandler, "refill->waitEndDaySem");
    

    srand((int)time(NULL) % getpid());

    correctType = (int)(type - 1);

    p = getPort(correctType);

    sscanf(text, "%d|%d", &day, &quantity);

    
    /*
        distribuisco le quantità da aggiungere
    */
    quanties = toArray(distributeV1(quantity, so_merci), &length);

    /*
        semaforo per modificare il magazzino dei porti
    */
    portBufferSem = useSem(RESPORTSBUFFERS, errorHandler , "refill->useSem portBufferSem");


    mutexPro(portBufferSem, (int)correctType, LOCK, errorHandler, "refill->portBufferSem LOCK");
    /* fillMagazine(&p->supplies, 0, supplies); */

    magazine = getMagazine(p);
    
    fillMagazine(magazine, day, quanties);
    shmDetach(magazine, errorHandler, "refill magazine");
    mutexPro(portBufferSem, (int)correctType, UNLOCK, errorHandler, "refill->portBufferSem UNLOCK");
    /*
        reservePrint(printPort, p, correctType);

    */
    
    detachPort(p, correctType);

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


void mySettedPort(int supplyDisponibility, int requestDisponibility, int idx, void(*portCode)(int endShmId, int idx,int waitPortsDeathSemID, int waitShipsDeathSemID)) {
     
    void (*oldHandler)(int);
    int endShmId;
    int waitPortsDeathSemID;
    int waitShipsDeathSemID;
    Port p;
    waitPortsDeathSemID = useSem(WAITPORTSSEM, errorHandler, "waitPortsDeathSemID in portCode");
    waitShipsDeathSemID = useSem(WAITSHIPSSEM, errorHandler, "waitPortsDeathSemID in portCode");
  
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

    detachPort(p, idx);

    launchRefiller(idx);

    checkInConfig();

    
    /*
        da aggiungere le due useQueue per le code di scaricamento
    */

    portCode(endShmId, idx, waitPortsDeathSemID, waitShipsDeathSemID);


}

int checkRequests(Port p, int type, int quantity) {
    int diff;
    int* reqs;
    int n;
    reqs = getShmAddress(p->requestsID, 0, errorHandler, "chackrequets");
    n = reqs[type];
    if (n == 0) return -1;
    if (quantity >= reqs[type]) {
        reqs[type] = 0;
    }
    else {
        reqs[type] -= quantity;
    }
    shmDetach(reqs, errorHandler, "chackrequets");
    return n;
}

int filter(int el){
    return el!=-2 ;
}

int filterReq(int req){
    return req == 0;
}
intList* validSupplies(Port p){
    int i;
    int* magazine;
    intList* ret = intInit();
    int so_merci = SO_("MERCI");
    magazine = getMagazine(p);
    for (i = 0; i < so_merci; i++) {
        if (validSupply(p->supplies, i, magazine)) {
            intPush(ret, i);
        }
    }
    shmDetach(magazine, errorHandler, "validSupplies");
    return ret;
}

intList* suppliesTypes(Port p) {
    int so_merci = SO_("MERCI");
    intList* tipiMerceSenzaRichiesta;
    int* reqs;
    intList* tipiMerceOffertaMaggioreDiZero = validSupplies(p);
    intList* ret;
    reqs = getShmAddress(p->requestsID, 0, errorHandler, "requestsTypes");

    tipiMerceSenzaRichiesta = findIdxs(reqs, so_merci, filterReq);
    shmDetach(reqs, errorHandler, "requestsTypes");
    
    ret = intIntersect(tipiMerceSenzaRichiesta, tipiMerceOffertaMaggioreDiZero);
    intFreeList(tipiMerceSenzaRichiesta);
    intFreeList(tipiMerceOffertaMaggioreDiZero);
    return ret;
}

intList* requestsTypes(Port p) {
    intList* ret;
    int* reqs;
    int so_merci = SO_("MERCI");
    reqs = getShmAddress(p->requestsID, 0, errorHandler, "requestsTypes");
    ret = findIdxs(reqs, so_merci, filterIdxs);
    shmDetach(reqs, errorHandler, "requestsTypes");
    return ret;
}


intList* getAllOtherTypeRequests(int idx, Port portArr) {
    int i;
    intList* ret = intInit();
    intList* requestsType;
    int so_porti = SO_("PORTI");

    for (i = 0; i < so_porti; i++)
    {
        if (i != idx) {
            requestsType = requestsTypes(portArr + i);
            ret = intUnion(ret, requestsType);
            intFreeList(requestsType);
        }
    }
    
    return ret;
}


intList* getTypeToCharge() {
  
    Port port;
    int i;
    int j;
    intList *inter;
    intList *merciTotaliRichieste = intInit();
    intList* merciTotaliOfferte = intInit();
    intList* aux0;
    intList* aux1;
    int so_porti = SO_("PORTI");
    port = getPort(0);
    for(i=0;i<so_porti; i++){
        aux0 = requestsTypes(port + i);
        
        aux1 = suppliesTypes(port + i);
        
        merciTotaliRichieste = intUnion(merciTotaliRichieste, aux0);
        merciTotaliOfferte = intUnion(merciTotaliOfferte, aux1);
        intFreeList(aux0);
        intFreeList(aux1);
        
    }
    
    detachPort(port, 0);
    /*
        {tipi di merce offerti in gioco} = ∪ (i = 0 -> SO_PORTI)(tipi di merce offerta dal porto[i])
        {tipi di merce richiesti in gioco} = ∪ (i = 0 -> SO_PORTI)(tipi di merce richiesta dal porto[i])
        Merce che ha senso caricare = {tipi di merce offerti in gioco} ∩ {tipi di merce richiesti nel gioco}
    */
    inter = intIntersect(merciTotaliOfferte, merciTotaliRichieste);
    intFreeList(merciTotaliOfferte);
    intFreeList(merciTotaliRichieste);

    return inter;
}


double getValue(int quantity, int expTime, int type, Port p, int idx) {
    intList *tipiDiMerceRichiestiAltriPorti = getAllOtherTypeRequests(idx,p-idx);
    if (expTime <= 0 || contain(requestsTypes(p), type) || !contain(tipiDiMerceRichiestiAltriPorti, type)){
        return 0;
    }else {
        /*
            se:
            expTime > 0 &&
            le mie richieste non contengono il type di merce di questa offerta &&
            le richieste degli altri porti contengono il type di merce di questa offerta
        */
        if (PORTOSCEGLIEMASSIMO) {
            return quantity * (double)expTime;
        }else{
            return quantity / (double)expTime;

        }
    }
}


int findTypeAndExpTime(Port port, int* type, int* foundDay, int* expTime, int quantity, int idx) {
    int i;
    int j;
    /*
    il valore dev'essere di type double perchè sennò le volte che 0 < quantity/expTime < 1,
    cioè quando il tempo di vita rimanente è maggiore della quantità, value diventa 0
    */
    double value = 0;
    int ton;
    double currentValue;
    int currentScadenza;
    int res;
    int* magazine;
    int so_days;
    int so_merci;
    magazine = getMagazine(port);
    *type = -1;
    *expTime = -1;
    *foundDay = -1;
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");
/*
    Ad ogni lotto offerto del magazzino attribuisco un valore il cui modo per calcolarlo varia a seconda del pattern:
    1) il porto offre il prodotto con peso maggiore e tempo di vita rimanente maggiore => (peso del lotto * expTime)
    2) il porto offre il prodotto con peso maggiore e tempo di vita rimanente minore => (peso del lotto / expTime)
    Il valore è = 0 se la merce è scaduta o se gli altri porti non contengono richiesta per quel tipo di merce

    Viene scelto il prodotto con valore maggiore

*/
    for (i = 0; i < so_days; i++) {
        for (j = 0; j < so_merci; j++) {
            
            ton = getMagazineVal(magazine,i,j);
            
            currentScadenza = getExpirationTime(port->supplies, j, i);
            /*
                assegno il valore
            */
            currentValue = getValue(ton, currentScadenza,j ,port,idx);
            if (ton >= quantity && currentValue > value) {
                value = currentValue;
                *type = j;
                *foundDay = i;
                *expTime = currentScadenza;
            }
        }
        
    }
    
    if (*type == -1 && *expTime == -1 && *foundDay == -1) {
        res = -1;
    }
    else {
        /*
                Operazione importante: decremento della quantità richiesta in anticipo, così nel mentre altre navi possono scegliere
                lo stesso type di merce con la quantità aggiornata
        */
        addMagazineVal(magazine, *foundDay, *type, quantity * -1);

        addNotExpiredGood(0 - quantity, *type, PORT, 0, idx);
        res = 1;
    }
    shmDetach(magazine, errorHandler, "trova type e expTime");
    return res;

}

int countPortsWhere(int(*f)(int,Port)){
    int count;
    int i;
    Port p;
    int so_porti;
    count = 0;
    so_porti = SO_("PORTI"); 

    p = getPort(0);
    for (i = 0; i < so_porti; i++){
        if(f(i,p+i)){
            count++;
        }
    }
    detachPort(p,0);
    return count;
}

int caughtBySwell(int idx, Port p){
    return p->weatherTarget;
}

void printPortsState(FILE *fp){
    int semPierID;
    int i;
    int c;
    int k;
    Supplies s;
    Port port;
    int so_porti = SO_("PORTI");
    int so_banchine = SO_("BANCHINE");
    semPierID = useSem(BANCHINESEMKY, errorHandler, "printStatoNavi");
    fprintf(fp, "Porti interessati da mareggiata: %d\n" , countPortsWhere(caughtBySwell));
    port = getPort(0);
    for (i = 0; i < so_porti; i++)
    {
        fprintf(fp, "Porto %d:\n", i);
        fprintf(fp, "Banchine occupate totali: %d\n", so_banchine - getOneValue(semPierID, i));
        fprintf(fp, "Merci ricevute: %d\n", port[i].deliveredGoods);
        fprintf(fp, "Merci spedite: %d\n", port[i].sentGoods);
        printPort(port+i, i, fp);
    }
    detachPort(port, 0);
}
Port getPort(int portID){
   int portShmid;
   Port port;
   
   portShmid = useShm(PSHMKEY, 0, errorHandler, "get port array");
   port = (Port)getShmAddress(portShmid, 0, errorHandler, "get port");

   return port + portID;
}

void detachPort(Port port,int portID) {
    shmDetach(port-portID, errorHandler, "detachPort");
}



void restorePromisedGoods(Port porto, int foundDay, int foundType, int quantity, int myPortIdx){
    int* magazine = getMagazine(porto);
    addNotExpiredGood(quantity, foundType, PORT, 0, myPortIdx);
    /*
        SE LA MERCE E' SCADUTA MENTRE IL PORTO ASPETTAVA DI SAPERE SE E' STATO SCELTO
        SE LA MERCE FOSSE SCADUTA PRIMA IL PROBLEMA NON ESISTEREBBE
    */
    if(getExpirationTime(porto->supplies,foundType, foundDay)== 0){
        addExpiredGood(quantity, foundType, PORT);
    }
    else {
        addMagazineVal(magazine, foundDay, foundType, quantity);
    
    }
    shmDetach(magazine, errorHandler, "restore promised goods");
}

int* getMagazine(Port port) {
    return (int*)getShmAddress(port->supplies.magazineID, 0, errorHandler, "get magazine");
}

