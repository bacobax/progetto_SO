#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include "../src/nave.h"
#include "../src/porto.h"
#include "../config1.h"
#include "./support.h"
#include "../src/dump.h"
#include "./msg_utility.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>


void quitSignalHandlerShip(int sig){
    printf("PID[%d] Nave termino!\n", getpid());
    exit(EXIT_SUCCESS);
}

int availableCapacity(Ship ship)
{
    return (SO_CAPACITY - ship->weight);
}

double generateCord()
{
    double range, div;

    range = (SO_LATO); /* max-min */
    div = RAND_MAX / range;
    return (rand() / div);
}

void initArrayResponses(int* a){
    int i;
    for(i=0; i<SO_PORTI; i++){
        a[i] = -1;
    }
}

void initArrayProducts(Product* products){
    int i;
    for(i=0; i<SO_CAPACITY; i++){
        products[i].product_type = -1;
        products[i].expirationTime = -1;
        products[i].weight = -1;
    }
}

void initArrayOffers(PortOffer* port_offers){
    int i;
    for(i=0; i<SO_PORTI; i++){
        port_offers[i].expirationTime = -1;
        port_offers[i].product_type = -1;
    }
}

Ship initShip(int shipID)
{
    Ship ship;
    int shipShmId;

    if (signal(SIGUSR1, quitSignalHandlerShip) == SIG_ERR)
    { /* imposto l'handler per la signal SIGUSR1 */
        perror("Error trying to set a signal handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }

    /* inizializziamo la nave in shm*/

    shipShmId = useShm(SSHMKEY, sizeof(struct ship) * SO_NAVI, errorHandler, "initShip");
    ship = ((struct ship*) getShmAddress(shipShmId, 0, errorHandler, "initShip")) + shipID;
    ship->shipID = shipID;
    ship->x = generateCord();
    ship->y = generateCord();
    ship->weight = 0;
    initArrayProducts(ship->products); /* inizializzo l'array con tutti i valori a -1*/

    return ship;
}

void printLoadShip(Product* products){
    int i;
    for (i = 0; i < SO_CAPACITY; i++) {
        if (products[i].product_type != -1) {
        printf("\nProduct type:%d, Expiration time: %d, Weight: %d", products[i].product_type, products[i].expirationTime, products[i].weight);
            
        }
    }
    printf("\n");
}

void printShip(Ship ship)
{

    int resSemID = useSem(RESPRINTKEY, errorHandler, "printShip");

    mutex(resSemID, LOCK, errorHandler, "printShip LOCK");

    printf("[%d]: Nave\n", ship->shipID);

    printf("coords: [x:%f, y:%f]\n", (ship->x), (ship->y));

    printf("ton trasporati:%d\n", ship->weight);

    printf("carico trasportato:\n");
    printLoadShip(ship->products);

    printf("______________________________________________\n");

    mutex(resSemID, UNLOCK, errorHandler, "printShip UNLOCK");
}

int addProduct(Ship ship, Product p){
    int i;
    int aviableCap;
    Product *products = ship->products;
    aviableCap = availableCapacity(ship);
    if (aviableCap >= p.weight)
    {
        printf("Nave: c'è abbastanza capienza per un prodotto che pesa %d, capienza: %d\n" , p.weight, aviableCap);
        for (i = 0; i < SO_CAPACITY; i++)
        {
            /*
                inserisco il prodotto nella prima posizione dell'array che trovo in cui
                product_type = 0
                per non inizializzare tutto l'array a -1 suggerisco di far partire tutti i prodotti
                con un product_type da 1.
            */
            if (products[i].product_type == -1)
            {
                products[i].product_type = p.product_type;
                products[i].expirationTime = p.expirationTime;
                products[i].weight = p.weight;
                ship->weight = ship->weight + p.weight;

                addNotExpiredGood(products[i].weight, products[i].product_type, SHIP);

                break;
            }
        }
    }
    else
    {
        printf("[%d]Nave: non c'è abbastanza capienza per un prodotto che pesa %d, capienza: %d\n" , getpid(),p.weight, aviableCap);

        return -1;
    }
}

int compareProducts(Product p1, Product p2){
    if((p1.product_type == p2.product_type) && (p1.expirationTime == p2.expirationTime) && (p1.weight == p2.weight))
        return 0;
    else
        return -1;    
}

int findProduct(Product* products, Product p){
    int i;
    for(i=0; i<SO_CAPACITY; i++){
        if(compareProducts(products[i], p) == 0){
            return i;
        }
    }
    return -1;
}

int removeProduct(Ship ship, int product_index){
    int i;
    Product* products = ship->products;
    
    if(product_index<0 || product_index>SO_CAPACITY) return -1;

    for(i=0; i<SO_CAPACITY; i++){
        if(i == product_index){
            ship->weight = ship->weight - products[i].weight;
            products[i].product_type = -1;
            products[i].expirationTime = -1;
            products[i].weight = -1;
            return 0;
        }
    }
}

int chooseQuantityToCharge(Ship ship){
    return availableCapacity(ship);
    /* il massimo che posso caricare
       valutare se conviene caricare di meno per essere più leggeri e viaggiare più
       velocemente */
}

int firstValidExpTime(Product* p, int* idx) {
    int i;
    *idx = -1;
    for (i = 0; i < SO_CAPACITY; i++) {
        if (p[i].expirationTime != -1) {
            *idx = i;
            return p[i].expirationTime;
        }
    }
    return -1;
}

int chooseProductToDelivery(Ship ship) {
    int i;
    int index;      
    int expTime;                    /* do per scontato che ci sia almeno 1 tipo di merce sulla nave in questo caso nella posizione 0*/
    int semID;
    Product* products = ship->products;

    expTime = firstValidExpTime(ship->products, &index);
    for(i=1; i<SO_CAPACITY; i++){
        /*
        if(products[i].product_type == -1) break;
        */
        if(products[i].expirationTime > 0 && products[i].expirationTime < expTime){
            index = i;        
        }
    }
    return index;
}

void callPortsForCharge(Ship ship, int quantityToCharge){
    int i;
    char text[MEXBSIZE];
    int requestPortQueueID;
    int waitResponseSemID;
    
    requestPortQueueID = useQueue(PQUEREQCHKEY, errorHandler , "callPortsForCharge");
    waitResponseSemID = useSem(WAITFIRSTRESPONSES, errorHandler, "callPortsForCharge");
    
    sprintf(text, "%d %d", quantityToCharge , ship->shipID); 

    for (i = 0; i < SO_PORTI; i++) {
        /*
            queueID = useQueue(PQUEUEKEY + i, errorHandler);

        */
         printf("[%d]NAVE: invio domanda al porto %d\n",getpid() ,i);
         msgSend(requestPortQueueID, text, i+1, errorHandler, 0,"callPortsForCharge");
         mutexPro(waitResponseSemID, ship->shipID, WAITZERO, errorHandler, "callPortsForCharge WAITZERO");
         mutexPro(waitResponseSemID, ship->shipID, UNLOCK, errorHandler, "callPortsForCharge +1");

         /*
                poichè non ci possono essere type uguali a 0 aggiungo
                all'id della nave +1
         */
    }
    

}

int portResponsesForCharge(Ship ship, PortOffer* port_offers){
    int i;
    int queueID;
    int ports = 0;
    mex* response;
    FILE* fp[SO_PORTI];
    char readerFileName[1024];
    char textResponse[1024];
    queueID = useQueue(ftok("./src/nave.c", ship->shipID), errorHandler, "portResponsesForCharge");
    /*
    for (i = 0; i < SO_PORTI; i++) {
        sprintf(readerFileName, "./utils/bin/queuereader %d %d", queueID, i + 1);
        fp[i] = popen("./utils/bin/queuereader", "r");
        if (fp[i] == NULL) {
            perror("Errore nella popen");
            exit(EXIT_FAILURE);
        }
    }
    for (i = 0; i < SO_PORTI; i++) {
        fscanf(fp[i], "%s", textResponse);
        if(strlen(response->mtext) > 1){
            
            sscanf(response->mtext, "%d %d", &port_offers[i].product_type, &port_offers[i].expirationTime);
            ports++;
        }
    }
    */
    for (i = 0; i < SO_PORTI; i++) {
        printf("[%d]Nave: aspetto su coda %d messaggio con type %d\n", getpid(), queueID, i + 1);
        response = msgRecv(queueID, i + 1, errorHandler, NULL, SYNC, "portResponsesForCharge");

        
        printf("🤡[%d]Nave: Strlen del messaggio ricevuto : %d\n" ,getpid() ,strlen(response->mtext) );
        if(strlen(response->mtext) > 1){
            
            sscanf(response->mtext, "%d %d", &port_offers[i].product_type, &port_offers[i].expirationTime);
            ports++;
        }
    }
    
    

    return ports;
}

int choosePortForCharge(PortOffer* port_offers){
    int i;
    int portID = 0;
    int expTime = 0;
    for(i=0; i<SO_PORTI; i++){

        if(expTime == 0 && port_offers[i].expirationTime != -1){
            
            expTime = port_offers[i].expirationTime;
            portID = i;

        } else {
            
            if(port_offers[i].expirationTime != -1 && port_offers[i].expirationTime < expTime){
                expTime = port_offers[i].expirationTime;
                portID = i;
            }
        }
    }
    printf("[%d]Nave: ho scelto il porto %d\n", getpid() , portID);
    return portID;
}

void replyToPortsForCharge(Ship ship, int portID){
    int i;
    int queueID;
    char text[MEXBSIZE];
    printf("[%d]Nave invio conferme ai porti di chi è stato scelto\n",getpid());
    for(i=0; i<SO_PORTI; i++){
        queueID = useQueue(ftok("./src/porto.c" , i), errorHandler, "replyToPortsForCharge");
        
        if(i == portID){
            printf("[%d]Nave ho scelto il porto:%d\n", getpid(), i);
            sprintf(text, "1"); /*ok*/
            msgSend(queueID, text, (ship->shipID + 1), errorHandler,0 ,"replyToPortsForCharge");
        }
        else {
            printf("[%d]Nave NON ho scelto il porto:%d\n", getpid(), i);
            
            sprintf(text, "0"); /*negative*/
            msgSend(queueID, text, (ship->shipID + 1), errorHandler,0 ,"replyToPortsForCharge");
        }
    }

}

void callPortsForDischarge(Ship ship, Product p) {
    int i;
    int queueID;
    char text[MEXBSIZE];
    int requesetPortQueueID;
    int waitResponseSemID = useSem(WAITFIRSTRESPONSES, errorHandler, "callPortsForDischarge");
    sprintf(text, "%d %d %d", p.product_type, p.weight, ship->shipID);
    requesetPortQueueID = useQueue(PQUEREDCHKEY, errorHandler, "callPortsForDischarge");


    for (i = 0; i < SO_PORTI; i++) {
        
        printf("[%d]NAVE: invio domanda al porto %d per scaricare\n", getpid(), i);
        msgSend(requesetPortQueueID, text, i+1, errorHandler,0 ,"callPortsForDischarge");
        mutexPro(waitResponseSemID, ship->shipID, WAITZERO, errorHandler, "callPortsForDischarge WAITZERO");
        mutexPro(waitResponseSemID, ship->shipID, UNLOCK, errorHandler, "callPortsForDischarge LOCK");
    }
}

int portResponsesForDischarge(Ship ship, int* quantoPossoScaricare){
    int i;
    int max;
    int queueID;
    int portID = -1;
    mex* response;
    int startIdx = -1;
    int arrayResponses[SO_PORTI];
    int validityArray[SO_PORTI];
    int cond = 0;
    FILE* fp[SO_PORTI];
    char readerFileName[1024];
    char textResponse[1024] = "";
    
    for (i = 0; i < SO_PORTI; i++) {
        validityArray[i] = 0;
    }
    initArrayResponses(arrayResponses);

    queueID = useQueue(ftok("./src/nave.c", ship->shipID), errorHandler, "portResponsesForDischarge"); /* coda di messaggi delle navi per le risposte di scaricamento*/


    for (i = 0; i < SO_PORTI; i++) {
        sprintf(readerFileName, "./utils/bin/queuereader %d %d", queueID, i + 1);
        fp[i] = popen("./utils/bin/queuereader", "r");
        if (fp[i] == NULL) {
            perror("Errore nella popen");
            exit(EXIT_FAILURE);
        }
    }
    for (i = 0; i < SO_PORTI; i++) {
        //fscanf(fp[i], "%s", textResponse);
        while (fgets(textResponse, 1024, fp[i]) != NULL);
        if (ferror(fp[i])) {
            perror("Errore nella lettura della pipe");
            exit(1);
        }
        printf("RESPONSE: %s\n", textResponse);
        if (strcmp(textResponse, "NOPE") != 0) {
            printf("[%d]Nave: ho trovato porto %d in cui fare scarico\n", getpid(), i);
            arrayResponses[i] = atoi(response->mtext);
            validityArray[i] = 1;
        }
    }
    for (i = 0; i < SO_PORTI; i++) {
        pclose(fp[i]);
    }
    /*
    for (i = 0; i < SO_PORTI; i++) {
        response = msgRecv(queueID, i+1, errorHandler, NULL, SYNC, "portResponsesForDischarge");

        printf("🤡[%d]Nave: messaggio ricevuto per scaricare: %s\n" ,getpid() , response->mtext);
        if(strcmp(response->mtext, "NOPE") != 0){
            printf("[%d]Nave: ho trovato porto %d in cui fare scarico\n", getpid(), i);
            arrayResponses[i] = atoi(response->mtext);
            validityArray[i] = 1;
        }
    }*/
    for (i = 0; i < SO_PORTI && !cond; i++) {
        if (validityArray[i]) {
            startIdx = i;
            cond = 1;
        }
    }
    if (!cond) {
        return -1;
    }
    max = arrayResponses[startIdx];
    portID = startIdx;
    for (i = startIdx; i < SO_PORTI; i++) {
        if(validityArray[i] && arrayResponses[i] > max){
          max = arrayResponses[i];
          portID = i;  
        } 
    }
    *quantoPossoScaricare = max;
    printf("[%d]Nave: ho scelto il porto %d per scaricare\n", getpid(), portID);
    return portID;
}

void replyToPortsForDischarge(Ship ship, int portID){
    int i;
    char mex[MEXBSIZE];
    int queueID;

    for(i=0; i<SO_PORTI; i++){
        queueID = useQueue(ftok("./src/porto.h", i), errorHandler, "replyToPortsForDischarge");
        if(i == portID){
            printf("[%d]Nave: mando msg CONFERMA al porto %d per scaricare\n", getpid(), portID);
            sprintf(mex, "1");
            msgSend(queueID, mex, ship->shipID + 1, errorHandler,0 ,"replyToPortsForDischarge");
        } else {
            sprintf(mex, "0");
            msgSend(queueID, mex, ship->shipID + 1, errorHandler,0 ,"replyToPortsForDischarge");
        }
    }
}

void accessPortForCharge(Ship ship, int portID, PortOffer offer_choosen, int weight){
    int portShmID;

    
    int pierSemID;
    int shipSemID;
    Port port;
    Product p;
    p.product_type = offer_choosen.product_type;
    p.expirationTime = offer_choosen.expirationTime;
    p.weight = weight;

    portShmID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler, "accessPortForCharge");
    pierSemID = useSem(BANCHINESEMKY, errorHandler, "accessPortForCharge->semaforo banchine");
    shipSemID = useSem(SEMSHIPKEY, errorHandler, "accessPortForCharge->semaforo rw navi");

    port = ((Port)getShmAddress(portShmID, 0, errorHandler, "accessPortForCharge")) + portID;

    mutexPro(pierSemID, portID, LOCK, errorHandler, "accessPortForCharge->semBanchine LOCK");

    /* in questo momento la nave è attraccata alla banchina del porto*/

    /* il porto ha già decrementato */

    /* nanosecsleep(p.weight / SO_LOADSPEED);  da cambiare nanosecsleep perchè il parametro da mandare deve essere di tipo double*/

    sleep(p.weight / SO_LOADSPEED);

    mutexPro(shipSemID, ship->shipID, LOCK, errorHandler, "accessPortForCharge-> shipSemID LOCK");

    printf("[%d]Nave: sono attracata alla banchina del porto per aggiungere la merce\n", getpid());

    addProduct(ship, p);

    mutexPro(shipSemID, ship->shipID, UNLOCK, errorHandler, "accessPortForCharge->shipSemID UNLOCK");

    printShip(ship);

    mutexPro(pierSemID, portID, UNLOCK, errorHandler, "accessPortForCharge->pierSemID UNLOCK");
}

void accessPortForDischarge(Ship ship, int portID, int product_index, int quantoPossoScaricare){
    int portShmID;
    int pierSemID;
    int shipSemID;
    int portBufferSemID;
    Port port;

    portShmID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler,"accessPortForDischarge");
    pierSemID = useSem(BANCHINESEMKY, errorHandler,"accessPortForCharge->semaforo banchine");
    shipSemID = useSem(SEMSHIPKEY, errorHandler,"accessPortForCharge->semaforo rw navi");

    port = ((Port) getShmAddress(portShmID, 0, errorHandler, "accessPortForCharge")) + portID;

    mutexPro(pierSemID, portID, LOCK, errorHandler , "accessPortForCharge->pierSemID LOCK");
    /*nanosecsleep(ship->products[product_index].weight / SO_LOADSPEED);*/
    sleep(1);
    mutexPro(shipSemID, ship->shipID, LOCK, errorHandler,  "accessPortForCharge->shipSemid LOCK");

    if(ship->products[product_index].expirationTime != -1){
        if (quantoPossoScaricare >= ship->products[product_index].weight) {
            addDeliveredGood(ship->products[product_index].weight, ship->products[product_index].product_type);
            
            removeProduct(ship, product_index);
        }
        else {
            addDeliveredGood(quantoPossoScaricare, ship->products[product_index].product_type);
            ship->products[product_index].weight -= quantoPossoScaricare;
        }
    } else {
        printf("\nOOPS! [%d]Nave con id:%d la merce che volevi scaricare è scaduta!!!\n", getpid(), ship->shipID);
    }

   

    mutexPro(shipSemID, ship->shipID, UNLOCK, errorHandler, "accessPortForCharge->shipSemID UNLOCK");

    mutexPro(pierSemID, portID, UNLOCK, errorHandler, "accessPortForCharge->pierSemID UNLOCK");

}

void travel(Ship ship, int portID)
{

    Port p;
    double dt_x, dt_y, spazio, nanosleep_arg;
    long tempo;

    int portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler, "travel");  /* prendo l'id della shm del porto */

    p = ((Port)getShmAddress(portShmId, 0, errorHandler, "travel")) + portID;  /* prelevo la struttura del porto alla portID-esima posizione nella shm 

     imposto la formula per il calcolo della distanza */

    dt_x = p->x - ship->x;
    dt_y = p->y - ship->y;

    spazio = sqrt(pow(dt_x, 2) + pow(dt_y, 2));
    
    /* spazio/SO_SPEED è misurato in giorni (secondi), quindi spazio/SO_SPEED*1000000000 sono il numero di nanosecondi per cui fare la sleep */
    tempo = (long)((spazio / SO_SPEED) * NANOS_MULT);
    printf("[%d]Nave: viaggio per %ld secondi...\n", getpid(), tempo);

    
    /*nanosecsleep(tempo); */
    sleep(0.5);

    printf("[%d]Nave: viaggio finito...\n", getpid());
    


    /* Dopo aver fatto la nanosleep la nave si trova esattamente sulle coordinate del porto
       quindi aggiorniamo le sue coordinate */
    
   
    ship->x = p->x;
    ship->y = p->y;
    
}

void updateExpTimeShip(Ship ship){
    int i;
    Product* products = ship->products;

    for(i=0; i<SO_CAPACITY; i++){
        if(products[i].product_type == -1) break;
        
        products[i].expirationTime = products[i].expirationTime -1;

        if(products[i].expirationTime == 0){
            addExpiredGood(products[i].weight, products[i].product_type, SHIP);
            removeProduct(ship, i);
            
            printShip(ship);
        }
    }
}

