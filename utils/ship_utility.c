#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include "../src/nave.h"
#include "../src/porto.h"
#include "../config1.h"
#include "./support.h"
#include "../src/dump.h"
#include "./errorHandler.h"
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
        port_offers[i].distributionDay = -1;
        port_offers[i].weight = -1;
    }
}


Ship initShip(int shipID)
{
    Ship ship;
    int shipShmId;
    /*
        if (signal(SIGUSR1, quitSignalHandlerShip) == SIG_ERR)
        {  imposto l'handler per la signal SIGUSR1 
            perror("Error trying to set a signal handler for SIGUSR1");
            exit(EXIT_FAILURE);
        }
    */
    

    /* inizializziamo la nave in shm*/
   
    shipShmId = useShm(SSHMKEY, sizeof(struct ship) * SO_NAVI, errorHandler, "initShip");
    ship = ((struct ship*)getShmAddress(shipShmId, 0, errorHandler, "initShip")) + shipID;
    ship->pid = getpid();
    printf("PID assegnato:%d alla nave con id:%d\n", ship->pid, shipID);
    ship->shipID = shipID;
    ship->x = generateCord();
    ship->y = generateCord();
    ship->weight = 0;
    initArrayProducts(ship->products); /* inizializzo l'array con tutti i valori a -1*/
    ship->inSea = 1;

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

    printf("[%d]: Nave id:%d\n", ship->pid, ship->shipID);

    printf("coords: [x:%f, y:%f]\n", (ship->x), (ship->y));

    printf("ton trasporati:%d\n", ship->weight);

    printf("valore di storm: %d\n", ship->storm);

    printf("promised product type:%d, exp.time:%d, weight;%d", ship->promisedProduct.product_type, ship->promisedProduct.expirationTime, ship->promisedProduct.weight);

    printf("carico trasportato:\n");
    printLoadShip(ship->products);

    printf("______________________________________________\n");

    mutex(resSemID, UNLOCK, errorHandler, "printShip UNLOCK");
}

int addProduct(Ship ship, Product p, Port port){
    int i;
    int aviableCap;
    int res = -1;
    Product *products = ship->products;
    aviableCap = availableCapacity(ship);
    printf("AGGIUNGO IL PRODOTTO CHE PESA %d, aviable cap: %d\n", p.weight, aviableCap);


    if (aviableCap >= p.weight)
    {
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
                port->sentGoods += p.weight;
                addNotExpiredGood(products[i].weight, products[i].product_type, SHIP, 0, ship->shipID);
                    
                return 0;
                
            }
        }
    }
    else
    {
        /*
        Non capiterà mai
        */
        printf("🤡Nave con id:%d: non c'è abbastanza capienza per un prodotto che pesa %d, capienza: %d\n", ship->shipID, p.weight, aviableCap);

        res = -1;
    }

    return res;
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
    int res = -1;
    Product* products = ship->products;
    
    if(product_index<0 || product_index>SO_CAPACITY) return -1;

    for(i=0; i<SO_CAPACITY; i++){
        if(i == product_index){
            ship->weight = ship->weight - products[i].weight;
            products[i].product_type = -1;
            products[i].expirationTime = -1;
            products[i].weight = -1;
            res = 0;
        }
    }
    return res;
}

void exitNave(){
    int waitShipSemID = useSem(WAITSHIPSSEM, errorHandler, "nave waitShipSemID");   
    mutex(waitShipSemID, LOCK, errorHandler, "nave mutex LOCK waitShipSemID");
    exit(0);
}

int chooseQuantityToCharge(Ship ship){
    intList *tipiDaCaricare;
    int shmPort;
    Port portArr;
    int i;
    int j;
    int k;
    int cap;
    int max;

    tipiDaCaricare = haSensoContinuare();
    printf("CHOOSE QUANTITY\n");
    max = 0;
    shmPort = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler, "chooseQuantityToCharge->useShm");
    portArr =  (Port)getShmAddress(shmPort, 0, errorHandler, "chooseQuantityToCharge->getShmAddress");
    
    for(i=0; i<SO_PORTI; i++){
        
        for(j=0; j<SO_DAYS; j++){
            for(k=0; k<SO_MERCI; k++){
                if(portArr[i].supplies.magazine[j][k] > max && contain(getAllOtherTypeRequests(portArr, i), k) && portArr[i].requests[k]==0){
                    max = portArr[i].supplies.magazine[j][k];
                }
            }
        }
    }
    shmDetach(portArr, errorHandler, "chooseQuantityToCharge->shmDetach");
    
    intFreeList(tipiDaCaricare);

    if (max < availableCapacity(ship))
    {
        cap = max;
    }
    else
    {
        cap = availableCapacity(ship);
    }
    printf("BEST QUANTITY: %d\n", cap);
    return cap;
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
    int index = -2;      
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

int communicatePortsForCharge(Ship ship, int quantityToCharge, PortOffer* port_offers){
    int aviablePorts = 0;
    int i;
    char text[MEXBSIZE];
    int requestPortQueueID;
    int shipQueueID;
    mex* response;

    requestPortQueueID = useQueue(PQUERECHKEY, errorHandler , "communicatePortsForCharge");
    shipQueueID = useQueue(ftok("./src/nave.c", ship->shipID), errorHandler, "communicatePortsForCharge");

    sprintf(text, "%d %d", quantityToCharge , ship->shipID); 

    for (i = 0; i < SO_PORTI; i++) {
        
        printf("NAVE con id:%d: invio domanda al porto %d per caricare\n", ship->shipID ,i);
        msgSend(requestPortQueueID, text, i+1, errorHandler, 0,"callPortsForCharge");

        response = msgRecv(shipQueueID, i+1, errorHandler, NULL, SYNC, "msg recv communicatePortsForCharge");
        printf("Nave con id:%d risposta dal porto %d: %s\n", ship->shipID, i, response->mtext);

        if(strlen(response->mtext) > 1){ 
            sscanf(response->mtext, "%d %d %d", &port_offers[i].product_type, &port_offers[i].expirationTime, &port_offers[i].distributionDay);
            aviablePorts++;
        }
    }

    return aviablePorts;
}

int choosePortForCharge(PortOffer* port_offers, int idx){
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
    printf("Nave con id:%d: ho scelto il porto %d\n", idx , portID);
    return portID;
}

void replyToPortsForCharge(Ship ship, int portID){
    int i;
    int queueID;
    char text[MEXBSIZE];

    printf("Nave con id:%d invio conferme ai porti di chi è stato scelto\n", ship->shipID);
    for(i=0; i<SO_PORTI; i++){
        queueID = useQueue(ftok("./src/porto.c" , i), errorHandler, "replyToPortsForCharge"); 
        if(i == portID){

            printf("Nave con id:%d ho scelto il porto:%d\n", ship->shipID, i);
            sprintf(text, "1"); /*ok*/
            msgSend(queueID, text, ship->shipID + 1, errorHandler,0 ,"replyToPortsForCharge");
        }
        else {
            printf("Nave con id:%d NON ho scelto il porto:%d\n", ship->shipID, i);
            
            sprintf(text, "0"); /*negative*/
            msgSend(queueID, text, (ship->shipID + 1), errorHandler,0 ,"replyToPortsForCharge");
        }
    }

}

int communicatePortsForDischarge(Ship ship, Product p, int* quantoPossoScaricare) {
    int i;
    int shipQueueID;
    char text[MEXBSIZE];
    int portQueueID;
    mex* response;
    int arrayResponses[SO_PORTI];
    int validityArray[SO_PORTI];
    int cond = 0;
    int startIdx;
    int max;
    int portID = -1;
    
    sprintf(text, "%d %d %d", p.product_type, p.weight, ship->shipID);

    for (i = 0; i < SO_PORTI; i++) {
        validityArray[i] = 0;
    }
    initArrayResponses(arrayResponses);
    
    shipQueueID = useQueue(ftok("./src/nave.c", ship->shipID), errorHandler, "communicatePortsForDischarge"); /* coda di messaggi delle navi per le risposte di scaricamento*/
    portQueueID = useQueue(PQUEREDCHKEY, errorHandler, "callPortsForDischarge");

    for (i = 0; i < SO_PORTI; i++) {

        printf("NAVE con id:%d: invio domanda al porto %d per scaricare\n", ship->shipID, i);
        msgSend(portQueueID, text, i+1, errorHandler, 0, "callPortsForDischarge");
  
        response = msgRecv(shipQueueID, i+1, errorHandler, NULL, SYNC, "msg recv in communicatePortsForDischarge");
        printf("Nave con id:%d risposta del porto %d: %s\n", ship->shipID,i, response->mtext);
        
        if (strcmp(response->mtext, "NOPE") != 0) {
            printf("Nave con id:%d: ho trovato porto %d in cui fare scarico\n", ship->shipID, i);
            arrayResponses[i] = atoi(response->mtext);
            validityArray[i] = 1;
        }
    }

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
    
    printf("Nave con id:%d: ho scelto il porto %d per scaricare\n", ship->shipID, portID);
    
    return portID;

}

void replyToPortsForDischarge(Ship ship, int portID){
    int i;
    char mex[MEXBSIZE];
    int queueID;

    for(i=0; i<SO_PORTI; i++){
        queueID = useQueue(ftok("./src/porto.h", i), errorHandler, "replyToPortsForDischarge");
        if(i == portID){
            printf("Nave con id %d: mando msg CONFERMA al porto %d per scaricare\n",ship->shipID, i);
            sprintf(mex, "1");
            msgSend(queueID, mex, ship->shipID + 1, errorHandler,0 ,"replyToPortsForDischarge");
        }
        else {
            printf("Nave con id %d: mando msg CONFERMA NEGATIVA al porto %d per scaricare\n", ship->shipID, i);
            sprintf(mex, "0");
            msgSend(queueID, mex, ship->shipID + 1, errorHandler,0 ,"replyToPortsForDischarge");
        }
    }
    printf("Nave con id:%d TUTTE LE CONFERME SONO STATE MANDATE\n", ship->shipID);
    
}

void accessPortForCharge(Ship ship, int portID){
    int portShmID;
    int pierSemID;
    int shipSemID;
    int stormSwellShmID;
    int* victimIdx;
    Port port;
    Product p;
    p.expirationTime = ship->promisedProduct.expirationTime;
    p.weight = ship->promisedProduct.weight;
    p.product_type = ship->promisedProduct.product_type;
    
    portShmID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler, "accessPortForCharge");
    pierSemID = useSem(BANCHINESEMKY, errorHandler, "accessPortForCharge->semaforo banchine");
    shipSemID = useSem(SEMSHIPKEY, errorHandler, "accessPortForCharge->semaforo rw navi");
    ship->inSea = 0;

    mutexPro(pierSemID, portID, LOCK, errorHandler, "accessPortForCharge->semBanchine LOCK");

    /* in questo momento la nave è attraccata alla banchina del porto*/

    /* il porto ha già decrementato */

    /* nanosecsleep(p.weight / SO_LOADSPEED);  da cambiare nanosecsleep perchè il parametro da mandare deve essere di tipo double*/
    mutexPro(shipSemID, ship->shipID, LOCK, errorHandler, "accessPortForCharge-> shipSemID LOCK");

    printf("Nave con id:%d: sono attracata alla banchina del porto per aggiungere la merce\n", ship->shipID);
    /* TO-DO GESTIRE PROBEMA MERCE SCADUTA UNA VOLTA ARRIVATO AL PORTO*/
    if(ship->promisedProduct.expirationTime != 0){
        
        /* PRIMA NANOSEECSLEEP E POI CONTROLLO SE SONO VITTIMA, RICORDARSI DI RESETTARE IL VALORE DALLA NAVE*/
        
        port = ((Port)getShmAddress(portShmID,0,errorHandler,"accessPortForCharge getportAddress")) + portID;
       
        nanosecsleep((p.weight / SO_LOADSPEED)*NANOS_MULT);
        if (port->swell) {
            port->weatherTarget = 1;
            printf("⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️\nDormo %d ore in più perchè c'è la SWELL\n⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️\n" , SO_SWELL_DURATION);
            nanosecsleep((double)(NANOS_MULT* 0.04166667) * SO_SWELL_DURATION);
            port->swell = 0;
        }
        
        addProduct(ship, p, port);
        shmDetach(port - portID, errorHandler, "shmDetach Porto");
        
    }
    else {
        printf("\nOOPS! Nave con id:%d la merce che volevo caricare è scaduta!!!\n", ship->shipID);
        addNotExpiredGood(ship->promisedProduct.weight,ship->promisedProduct.product_type,SHIP, 0, ship->shipID);
        addExpiredGood(ship->promisedProduct.weight, ship->promisedProduct.product_type, SHIP);
    }
    
    ship->promisedProduct.product_type = -1;
    ship->promisedProduct.expirationTime = -1;
    ship->promisedProduct.weight = -1;
    ship->promisedProduct.distributionDay = -1;
      
    mutexPro(shipSemID, ship->shipID, UNLOCK, errorHandler, "accessPortForCharge->shipSemID UNLOCK");
    ship->inSea = 1;
    mutexPro(pierSemID, portID, UNLOCK, errorHandler, "accessPortForCharge->pierSemID UNLOCK");
    if (ship->dead) {
            printf("🌊🌊🌊🌊🌊🌊\nNave %d, sono stata uccisa\n🌊🌊🌊🌊🌊🌊\n", ship->shipID);
            exitNave();
    }
    

    
}

void accessPortForDischarge(Ship ship, int portID, int product_index, int quantoPossoScaricare){
    int portShmID;
    int pierSemID;
    int shipSemID;
    int portBufferSemID;
    int stormSwellShmID;
    int* victimIdx;
    Product p;
    Port port;
    p.product_type = ship->products[product_index].product_type;
    p.expirationTime = ship->products[product_index].expirationTime;
    p.weight = ship->products[product_index].weight;

    portShmID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler,"accessPortForDischarge");
    pierSemID = useSem(BANCHINESEMKY, errorHandler,"accessPortForCharge->semaforo banchine");
    shipSemID = useSem(SEMSHIPKEY, errorHandler,"accessPortForCharge->semaforo rw navi");

    
    ship->inSea = 0;
    
    mutexPro(pierSemID, portID, LOCK, errorHandler, "accessPortForDisCharge->pierSemID LOCK");
   
    mutexPro(shipSemID, ship->shipID, LOCK, errorHandler,  "accessPortForDisCharge->shipSemid LOCK");

    if (ship->products[product_index].expirationTime > 0) {
        port = ((Port)getShmAddress(portShmID,0,errorHandler,"accessPortForDisCharge getportAddress")) + portID;
        
        nanosecsleep((p.weight / SO_LOADSPEED) * NANOS_MULT);
        if (port->swell) {
            port->weatherTarget = 1;
            printf("Porto %d colpito da una mareggiata, devo aspettare %d ore in più\n" , portID, SO_SWELL_DURATION);
            nanosecsleep((double)(NANOS_MULT  * 0.04166667)*SO_SWELL_DURATION);
            port->swell = 0;
        }

        
        if (ship->products[product_index].expirationTime > 0) {
            if (quantoPossoScaricare >= p.weight) {

                addDeliveredGood(p.weight, p.product_type, portID);
                removeProduct(ship, product_index);
                        
                        
            }else {
                addDeliveredGood(quantoPossoScaricare, ship->products[product_index].product_type, portID);
                ship->products[product_index].weight -= quantoPossoScaricare;
                ship->weight -= quantoPossoScaricare;
            }
        }else {
            printf("\nOOPS! Nave con id:%d la merce che volevi scaricare è scaduta mentre la stavi scaricando!!!\n", ship->shipID);
            
        }

        shmDetach(port-portID, errorHandler, "accessPortDischarge shmDetach");
    } else {
        
        printf("\nOOPS! Nave con id:%d la merce che volevi scaricare è scaduta!!!\n", ship->shipID);
    }

    
    mutexPro(shipSemID, ship->shipID, UNLOCK, errorHandler, "accessPortForCharge->shipSemID UNLOCK");
    ship->inSea = 1;

    mutexPro(pierSemID, portID, UNLOCK, errorHandler, "accessPortForCharge->pierSemID UNLOCK");
    if (ship->dead) {
            printf("🌊🌊🌊🌊🌊🌊\nNave %d, sono stata uccisa\n🌊🌊🌊🌊🌊🌊\n", ship->shipID);
            exitNave();
    }
}

void travel(Ship ship, int portID, int* day)
{

    Port p;
    double dt_x, dt_y, spazio, nanosleep_arg;
    long tempo;
    double tempoInSecondi;
    int tempoRimanente;
    int portShmId;
    int portBufferSem;
    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler, "travel"); /* prendo l'id della shm del porto */
    portBufferSem = useSem(RESPORTSBUFFERS, errorHandler , "refill->useSem portBufferSem");

    p = ((Port)getShmAddress(portShmId, 0, errorHandler, "travel")) + portID;  /* prelevo la struttura del porto alla portID-esima posizione nella shm 

     imposto la formula per il calcolo della distanza */

    dt_x = p->x - ship->x;
    dt_y = p->y - ship->y;

    spazio = sqrt(pow(dt_x, 2) + pow(dt_y, 2));
    
    /* spazio/SO_SPEED è misurato in giorni (secondi), quindi spazio/SO_SPEED*1000000000 sono il numero di nanosecondi per cui fare la sleep */
    tempoInSecondi = spazio / SO_SPEED;

    tempo = (long)((spazio / SO_SPEED) * NANOS_MULT);

    tempoRimanente = SO_DAYS - 1 - (*day);

    printf("Nave con id:%d: viaggio per %f secondi...\n", ship->shipID, tempoInSecondi);
    if (tempoInSecondi > tempoRimanente)
    {
        printf("Nave con id:%d non avrei abbastanza giorni per raggiungere il porto: %d, termino...\n", ship->shipID, portID);
        if(ship->promisedProduct.expirationTime != -1){
            addNotExpiredGood(ship->promisedProduct.weight, ship->promisedProduct.product_type, SHIP, 0, ship->shipID);
            if(ship->promisedProduct.expirationTime == 0){
                addExpiredGood(ship->promisedProduct.weight, ship->promisedProduct.product_type, SHIP);
            }
        }
        
        
        exitNave();
    }
    printf("NAVE, STO PER FARE LA NANOSECSLEEP\n");
    ship->inSea = 1;
    nanosecsleep(tempo);

    if (ship->storm == 1) {
        ship->weatherTarget = 1;
        printf("🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️\nNave con id:%d ho beccato una tempesta\n🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️\n", ship->shipID);
        nanosecsleep((double)(NANOS_MULT * 0.04166667) * SO_STORM_DURATION);
        ship->storm = 0;
    }

    if (ship->dead) {
        if (ship->promisedProduct.expirationTime != -1) {
            if (ship->promisedProduct.expirationTime == 0) {
                addNotExpiredGood(ship->promisedProduct.weight, ship->promisedProduct.product_type, PORT, 0, portID);
                addExpiredGood(ship->promisedProduct.weight, ship->promisedProduct.product_type, PORT);
            }
            else {
                addNotExpiredGood(ship->promisedProduct.weight, ship->promisedProduct.product_type, PORT, 0, portID);


                mutexPro(portBufferSem, portID, LOCK, errorHandler, "refill->portBufferSem LOCK");
                p->supplies.magazine[ship->promisedProduct.distributionDay][ship->promisedProduct.product_type] += ship->promisedProduct.weight;
                mutexPro(portBufferSem, portID, UNLOCK, errorHandler, "refill->portBufferSem UNLOCK");
            }
        }
        printf("🌊🌊🌊🌊🌊🌊\nNave %d, sono stata uccisa\n🌊🌊🌊🌊🌊🌊\n", ship->shipID);
        exitNave();
    }
    printf("Nave con id:%d: viaggio finito...\n", ship->shipID);
    


    /* Dopo aver fatto la nanosleep la nave si trova esattamente sulle coordinate del porto
       quindi aggiorniamo le sue coordinate */
    
   
    ship->x = p->x;
    ship->y = p->y;
    shmDetach(p - portID, errorHandler, "travel");

}

void updateExpTimeShip(Ship ship){
    int i;
    Product* products = ship->products;

   

    for(i=0; i<SO_CAPACITY; i++){
        if(products[i].product_type == -1) break;
        
        products[i].expirationTime = products[i].expirationTime -1;

        if (products[i].expirationTime == 0) {
            
            addExpiredGood(products[i].weight, products[i].product_type, SHIP);
            removeProduct(ship, i);
            printf("IN EXP TIMES:\n");
            printShip(ship);
        }
    }

    if(ship->promisedProduct.expirationTime > 0){
        ship->promisedProduct.expirationTime -= 1;
    }
}

int countShipWhere(Ship arrShip , int(*f)(int,Ship)){
    int count;
    int i;
    count = 0;
    for (i = 0; i < SO_NAVI; i++){
        if(f(i,arrShip+i)){
            count++;
        }
    }
    return count;
}

int weigthMaggioreDiZero(int idx, Ship ship){
    return !ship->dead && ship->inSea && ship->weight > 0;
}

int weigthMinoreDiZero(int idx, Ship ship){
    return !ship->dead && ship->inSea && ship->weight <= 0;
}

int isTargeted(int idx, Ship ship){
    return ship->weatherTarget == 1;
}

int isDead(int idx, Ship ship){
    return ship->dead;
}

int caughtByStorm(int idx, Ship ship){
    return ship->weatherTarget;
}

void printStatoNavi(FILE* fp){
    int shmShip;
    Ship ships; 
    int i;
    int j;
    int semPierID;
    int shipsInPorts = 0;
    semPierID = useSem(BANCHINESEMKY, errorHandler, "printStatoNavi");

    shmShip = useShm(SSHMKEY, sizeof(struct ship) * SO_NAVI, errorHandler, "printStatoNavi");
    ships = (Ship)getShmAddress(shmShip, 0, errorHandler, "printStatoNavi");

    /*
    for(i=0; i<SO_NAVI; i++){
        if(ship[i].inSea){
            if(ship[i].weight>0){            nave in mare con carico a bordo
                shipsInSeaWithProducts++;
            } else {                         nave in mare senza carico a bordo
                shipsInSeaWithoutProducts++;
            }
        }
    } 
    */

    for (i = 0; i < SO_PORTI; i++)
    { /* navi in porto che fanno operazioni di carico/scarico*/

        shipsInPorts += SO_BANCHINE - getOneValue(semPierID, i);
    }

    fprintf(fp, "Numero di nave in mare senza carico:%d\n", countShipWhere(ships, weigthMinoreDiZero));
    fprintf(fp, "Numero di navi in mare con carico a bordo:%d\n", countShipWhere(ships, weigthMaggioreDiZero));
    fprintf(fp, "Numero di navi in porto che stanno facendo operazioni di carico/scarico:%d\n", shipsInPorts);
    fprintf(fp, "Numero di navi rallentate a causa delle tempeste: %d\n", countShipWhere(ships, caughtByStorm));
    fprintf(fp, "Numero di navi affondate:%d\n", countShipWhere(ships, isDead));


    shmDetach(ships, errorHandler, "printStatoNave");
}


void waitEndDay(){
    int waitEndDaySemID;
    int waitEndDayShipSemID;
    waitEndDaySemID = useSem(WAITENDDAYKEY, errorHandler, "waitEndDay in chargeproducts");
    waitEndDayShipSemID = useSem(WAITENDDAYSHIPSEM, errorHandler, "waitEndDayShipSemID in chargeProducts");
    mutex(waitEndDayShipSemID, 1, errorHandler, "+1 waitEndDayShipSemID");
    mutex(waitEndDaySemID, WAITZERO, errorHandler, "WAITZERO su waitEndDaySemID");
    mutex(waitEndDayShipSemID, -1, errorHandler, "-1 waitEndDayShipSemID");
}

void waitToTravel(Ship ship){
    int waitToTravelSemID;
    waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler, "chargeProducts->waitToTravelSemID");
    mutexPro(waitToTravelSemID, ship->shipID, WAITZERO, errorHandler, "chargeProducts->waitToTravelSemID WAITZERO");
    mutexPro(waitToTravelSemID, ship->shipID, SO_PORTI, errorHandler, "chargeProducts->waitToTravelSemID +SO_PORTI");
}

void initPromisedProduct(Ship ship, PortOffer port_offer, int quantityToCharge){
    ship->promisedProduct.expirationTime = port_offer.expirationTime;
    ship->promisedProduct.product_type = port_offer.product_type;
    ship->promisedProduct.weight = quantityToCharge;
    ship->promisedProduct.distributionDay = port_offer.distributionDay;
}

