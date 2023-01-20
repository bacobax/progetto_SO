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
#include "./vettoriInt.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include "./loadShip.h"
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>



int availableCapacity(Ship ship)
{
    int so_capacity = SO_("CAPACITY");
    return ( so_capacity - ship->weight);
}


void initValidityArray(int* a){
    int i;
    int so_porti = SO_("PORTI");
    for (i = 0; i < so_porti; i++) {
        a[i] = 0;
    }
}

void initArrayResponses(int* a){
    int i;
    int so_porti = SO_("PORTI");
    for(i=0; i<so_porti; i++){
        a[i] = -1;
    }
}
/*
void initArrayProducts(Product products){
    int i;
    for(i=0; i<SO_CAPACITY; i++){
        products[i].product_type = -1;
        products[i].expirationTime = -1;
        products[i].weight = -1;
    }
}*/

void initArrayOffers(PortOffer* port_offers){
    int i;
    int so_porti = SO_("PORTI");
    for(i=0; i<so_porti; i++){
        port_offers[i].expirationTime = -1;
        port_offers[i].product_type = -1;
        port_offers[i].distributionDay = -1;
        port_offers[i].weight = -1;
    }
}

int checkIndexes(int* validityArray){
    int startIdx;
    int i;
    int cond = 0;
    int so_porti = SO_("PORTI");

    for (i = 0; i < so_porti && !cond; i++) {
        if (validityArray[i]) {
            startIdx = i;
            cond = 1;
        }
    }
    if (!cond) {
        startIdx = -1;
    }
    return startIdx;
}

int chooseBestPort(int* validityArray, int* arrayResponses, int startIdx, int* quantoPossoScaricare){
    int i;
    int max;
    int portID = startIdx;

    max = arrayResponses[startIdx];
    int so_porti = SO_("PORTI"); 

    for (i = startIdx; i < so_porti; i++) {
        if(validityArray[i] && arrayResponses[i] > max){
          max = arrayResponses[i];
          portID = i;  
        } 
    }
    *quantoPossoScaricare = max;
    return portID;
}


Ship initShip(int shipID)
{
    Ship ship;
    int shipShmId;
    
    int so_navi = SO_("NAVI");

    /* inizializziamo la nave in shm*/
    
    shipShmId = useShm(SSHMKEY, sizeof(struct ship) * so_navi, errorHandler, "initShip");
    ship = ((struct ship*)getShmAddress(shipShmId, 0, errorHandler, "initShip")) + shipID;
    ship->pid = getpid();
    printf("PID assegnato:%d alla nave con id:%d\n", ship->pid, shipID);
    ship->shipID = shipID;
    ship->x = generateCord();
    ship->y = generateCord();
    ship->weight = 0;
    ship->loadship = initLoadShip();
    ship->inSea = 1;

    return ship;
}
/*
void printLoadShip(Product* products){
    int i;
    for (i = 0; i < SO_CAPACITY; i++) {
        if (products[i].product_type != -1) {
            printf("\nProduct type:%d, Expiration time: %d, Weight: %d", products[i].product_type, products[i].expirationTime, products[i].weight);
            
        }
    }
    printf("\n");
}
*/

void printShip(Ship ship)
{
    FILE *fp;
    int resSemID = useSem(RESPRINTKEY, errorHandler, "printShip");

    mutex(resSemID, LOCK, errorHandler, "printShip LOCK");
    fp = fopen("./logs/logNavi.log", "a+");

    fprintf(fp,"[%d]: Nave\n", ship->shipID);

    fprintf(fp,"coords: [x:%f, y:%f]\n", (ship->x), (ship->y));

    fprintf(fp,"ton trasportati:%d\n", ship->weight);

    fprintf(fp,"valore di storm: %d\n", ship->storm);
    /*

    printf("promised product type:%d, exp.time:%d, weight;%d", ship->promisedProduct.product_type, ship->promisedProduct.expirationTime, ship->promisedProduct.weight);
    */

    fprintf(fp,"carico trasportato:\n");
    printLoadShip(ship->loadship, fp);

    fprintf(fp, "______________________________________________\n");

    if(weigthSum(ship->loadship) != ship->weight){
        fprintf(fp, "❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌\n");
    } 

    fclose(fp);

    mutex(resSemID, UNLOCK, errorHandler, "printShip UNLOCK");
}
/*
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
        
        printf("🤡Nave con id:%d: non c'è abbastanza capienza per un prodotto che pesa %d, capienza: %d\n", ship->shipID, p.weight, aviableCap);
    throwError("Add product failed" , "addProduct");
        res = -1;
    }

    return res;
}*/
/*
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
    
    if(product_index<0 || product_index>=SO_CAPACITY) return -1;

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
*/


void exitNave(Ship s){
    
    FILE* fp = fopen("./logs/exitShipLog.log", "a+");
    int waitShipSemID = useSem(WAITSHIPSSEM, errorHandler, "nave waitShipSemID");   
    fprintf(fp,"[%d]Nave: faccio la lock\n", getpid());
    
    mutex(waitShipSemID, LOCK, errorHandler, "nave mutex LOCK waitShipSemID");
    fprintf(fp,"[%d]Nave: uscita\n", getpid());

    fclose(fp);
    
    freeLoadShip(s->loadship);
    s->dead = 1;
    exit(0);
}

int chooseQuantityToCharge(Ship ship){
    intList *tipiDaCaricare;
    int shmPort;
    Port port;
    int i;
    int j;
    int k;
    int cap;
    int max;
    char text[64];
    int so_porti;
    int so_days;
    int so_merci;
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");
    so_porti = SO_("PORTI");
    tipiDaCaricare = haSensoContinuare();
    logShip(ship->shipID, "sto per fare CHOOSE QUANTITY\n");
    max = 0;
    
    for(i=0; i<so_porti; i++){
        port = getPort(i);
        for (j = 0; j < so_days; j++)
        {
            for(k=0; k<so_merci; k++){
                if(port->supplies.magazine[j][k] > max && contain(getAllOtherTypeRequests(i), k) && port->requests[k]==0){
                    max = port->supplies.magazine[j][k];
                }
            }
        }
        detachPort(port ,i, "chooseQuantityToCharge");
    }
    
    intFreeList(tipiDaCaricare);

    if (max < availableCapacity(ship))
    {
        logShip(ship->shipID, "cap = max\n");
        cap = max;
    }
    else
    {
        logShip(ship->shipID, "cap = aviableCap\n");
        printShip(ship);
        cap = availableCapacity(ship);
    }
    sprintf(text, "BEST QUANTITY: %d" , cap );
    logShip(ship->shipID , text);
    return cap;
}
/*
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
}*/

int chooseProductToDelivery(Ship ship) {
    int i;
    int index = -2;      
    int expTime = SO_("MAX_VITA") + 1;                    
    Product aux;

    i = 0;
    for (aux = ship->loadship->first; aux != NULL; aux = aux->next) {
       
        if(aux->expirationTime > 0 && aux->expirationTime < expTime){
            index = i;        
        }
        i++;
    }
    return index;
}

int communicatePortsForChargeV1(int quantityToCharge, PortOffer* port_offers) {
    int i;
    Port p;
    int controlPortsDisponibilitySemID;
    int tipoTrovato;
    int dayTrovato;
    int dataScadenzaTrovata;
    int aviablePorts;
    int res;
    int so_porti = SO_("PORTI");

    controlPortsDisponibilitySemID = useSem(PSEMVERIFYKEY, errorHandler, "RecvDischargerHandler->controlPortsDisponibilitySemID");
    aviablePorts = 0;

    for (i = 0; i < so_porti; i++) {
        p = getPort(i);
        mutexPro(controlPortsDisponibilitySemID, i, LOCK, errorHandler, "RecvDischargerHandler->controlPortsDisponibilitySemID LOCK");
        res = trovaTipoEScadenza(&p->supplies, &tipoTrovato, &dayTrovato, &dataScadenzaTrovata, quantityToCharge, i);
        mutexPro(controlPortsDisponibilitySemID, i, UNLOCK, errorHandler, "RecvDischargerHandler->controlPortsDisponibilitySemID UNLOCK");

        if (res != -1) {
            port_offers[i].distributionDay = dayTrovato;
            port_offers[i].expirationTime  = dataScadenzaTrovata;
            port_offers[i].portID          = i;
            port_offers[i].product_type    = tipoTrovato;
            port_offers[i].weight          = quantityToCharge;
            aviablePorts++;
        }
        
        detachPort(p, i, "communicatePortsForChargeV1");
    }
    return aviablePorts;
}
/*
int communicatePortsForCharge(Ship ship, int quantityToCharge, PortOffer* port_offers) {
    int aviablePorts = 0;
    int i;
    char text[MEXBSIZE];
    int requestPortQueueID;
    int shipQueueID;
    mex* response;
    int so_porti = SO_("PORTI");

    requestPortQueueID = getPortQueueRequest(PQUERECHKEY);
    shipQueueID = getShipQueue(ship->shipID);

    sprintf(text, "%d %d", quantityToCharge , ship->shipID); 

    for (i = 0; i < so_porti; i++) {
        printf("[%d]Nave: invio domanda al porto %d per caricare\n", ship->shipID, i);
        logShip(ship->shipID, " invio domanda al porto %d per caricare");
        msgSend(requestPortQueueID, text, i+1, errorHandler, 0,"callPortsForCharge");

        response = msgRecv(shipQueueID, i+1, errorHandler, NULL, SYNC, "msg recv communicatePortsForCharge");
        printf("[%d]Nave risposta dal porto %d: %s\n", ship->shipID, i, response->mtext);
        logShip(ship->shipID,"RICEVUTA RISPOSTA PORTO\n");
        if (strlen(response->mtext) > 1)
        {
            sscanf(response->mtext, "%d %d %d", &port_offers[i].product_type, &port_offers[i].expirationTime, &port_offers[i].distributionDay);
            aviablePorts++;
        }
    }

    return aviablePorts;
}
*/
int choosePortForCharge(PortOffer* port_offers, int idx){
    int i;
    int portID = 0;
    int expTime = 0;
    int so_porti = SO_("PORTI");

    for(i=0; i<so_porti; i++){

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
    logShip(idx, " ho scelto il porto");
    return portID;
}



void replyToPortsForChargeV1(int portID, PortOffer* port_offers) {
    int i;
    Port p;
    int so_porti = SO_("PORTI");

    for (i = 0; i < so_porti; i++) {
        if (i != portID && port_offers[i].product_type != -1) {
            p = getPort(i);
            
            restorePromisedGoods(p, port_offers[i].distributionDay, port_offers[i].product_type, port_offers[i].weight, i);
            detachPort(p, i, "replyToPortsForChargeV1");
        }
        
    }
    
}
/*
void replyToPortsForCharge(Ship ship, int portID) {
    int i;
    int queueID;
    char text[MEXBSIZE];
    int so_porti = SO_("PORTI");

    logShip(ship->shipID, " invio conferme ai porti di chi è stato scelto");
    for(i=0; i<so_porti; i++){
        queueID =  getPortQueueCharge(i);
        if(i == portID){

            logShip(ship->shipID, " ho scelto il porto");
            sprintf(text, "1"); /*ok
            msgSend(queueID, text, ship->shipID + 1, errorHandler,0 ,"replyToPortsForCharge");
        }
        else {
            logShip(ship->shipID, " NON ho scelto il porto");
            sprintf(text, "0"); /*negative
            msgSend(queueID, text, (ship->shipID + 1), errorHandler,0 ,"replyToPortsForCharge");
        }
    }

}
*/


int communicatePortsForDischargeV1(Ship ship, Product p, int* quantoPossoScaricare, int* arrayResponses) {
    int i;
    int verifyRequestSemID;
    Port port;
    int res;
    int* validityArray;
    int startIdx;
    int so_porti;
    int portID = -1;
    so_porti = SO_("PORTI");
    validityArray = (int*)malloc(so_porti * sizeof(int));
    initValidityArray(validityArray);
    initArrayResponses(arrayResponses);
    
    verifyRequestSemID = useSem(P2SEMVERIFYKEY, errorHandler, "recvChargerHandler->verifyRequestSemID");
    
    for (i = 0; i < so_porti; i++) {
        port = getPort(i);
        mutexPro(verifyRequestSemID, i, LOCK, errorHandler, "recvChargerHandler->verifyRequestSemID LOCK");
        res = checkRequests(port, p->product_type, p->weight);
        mutexPro(verifyRequestSemID, i, UNLOCK, errorHandler, "recvChargerHandler->verifyRequestSemID UNLOCK");

        if (res!= -1) {
            printf("Nave con id:%d: ho trovato porto %d in cui fare scarico\n", ship->shipID, i);
            arrayResponses[i] = res;
            validityArray[i] = 1;
        }
        detachPort(port,i, "communicatePortsForDischargeV1");
    }

    startIdx = checkIndexes(validityArray);
    if(startIdx == -1) return -1;

    portID = chooseBestPort(validityArray, arrayResponses, startIdx, quantoPossoScaricare);
       
    printf("[%d]Nave: ho scelto il porto %d per scaricare\n", ship->shipID, portID);
    
    return portID;

}
/*
int communicatePortsForDischarge(Ship ship, Product p, int* quantoPossoScaricare) {
    int i;
    int shipQueueID;
    char text[MEXBSIZE];
    int portQueueID;
    mex* response;
    int arrayResponses[SO_PORTI];
    int validityArray[SO_PORTI];
    int startIdx;
    int portID = -1;
    
    sprintf(text, "%d %d %d", p.product_type, p.weight, ship->shipID);

    initValidityArray(validityArray);
    initArrayResponses(arrayResponses);
    
    shipQueueID = getShipQueue(ship->shipID); /* coda di messaggi delle navi per le risposte di scaricamento
    portQueueID = getPortQueueRequest(PQUEREDCHKEY);

    for (i = 0; i < SO_PORTI; i++) {

        logShip(ship->shipID, " invio domanda al porto per scaricare");
        msgSend(portQueueID, text, i+1, errorHandler, 0, "callPortsForDischarge");
  
        response = msgRecv(shipQueueID, i+1, errorHandler, NULL, SYNC, "msg recv in communicatePortsForDischarge");
        logShip(ship->shipID, " risposta del porto");
        
        if (strcmp(response->mtext, "NOPE") != 0) {
            printf("Nave con id:%d: ho trovato porto %d in cui fare scarico\n", ship->shipID, i);
            arrayResponses[i] = atoi(response->mtext);
            validityArray[i] = 1;
        }
    }

    startIdx = checkIndexes(validityArray);
    if(startIdx == -1) return -1;

    portID = chooseBestPort(validityArray, arrayResponses, startIdx, quantoPossoScaricare);
       
    logShip(ship->shipID, " ho scelto il porto per scaricare");
    
    return portID;

}

*/

void restorePortRequest(Port p, int type, int originalPortRequest, int pWeight){
    /*
        se quello che ho chiesto era >= della sua richiesta => porto aveva decrementato di res;
        quello che ho chiesto era < della sua richiesta => porto aveva decrementato di prod.weight;
        se prod.weight >= res vuol dire che quello che ho chiesto supera la richiesta
        altrimenti è il contrario
    */
    if (pWeight >= originalPortRequest) {
        p->requests[type] += originalPortRequest;
    }
    else {
        p->requests[type] += pWeight;
    }
}

void replyToPortsForDischargeV1(Ship ship, int portID, int quantoPossoScaricare, int* portResponses, Product prod) {
    int i;
    Port porto;
    int so_porti = SO_("PORTI");

    for (i = 0; i < so_porti; i++) {
        if (i != portID && portResponses[i] != -1) {
            porto = getPort(i);
            
            restorePortRequest(porto, prod->product_type, portResponses[i], prod->weight);
            
            detachPort(porto, i, "replyToPortsForDischargeV1");
        }
    }

}
/*
void replyToPortsForDischarge(Ship ship, int portID) {
    int i;
    char mex[MEXBSIZE];
    int queueID;
    int so_porti = SO_("PORTI");

    for(i=0; i<so_porti; i++){
        queueID = getPortQueueDischarge(i);
        if(i == portID){
            logShip(ship->shipID, " mando msg CONFERMA al porto per scaricare");
            sprintf(mex, "1");
            msgSend(queueID, mex, ship->shipID + 1, errorHandler,0 ,"replyToPortsForDischarge");
        }
        else {
            logShip(ship->shipID, " mando msg CONFERMA NEGATIVA al porto per scaricare");
            sprintf(mex, "0");
            msgSend(queueID, mex, ship->shipID + 1, errorHandler,0 ,"replyToPortsForDischarge");
        }
    }
    logShip(ship->shipID, " TUTTE LE CONFERME SONO STATE MANDATE");
    
}
*/


void accessPortForChargeV1(Ship ship, int portID, PortOffer* port_offers) {
    int pierSemID;
    int shipSemID;
    int stormSwellShmID;
    int* victimIdx;
    Port port;
    int so_loadspeed = SO_("LOADSPEED");
    int so_swell_duration = SO_("SWELL_DURATION");

    Product p = initProduct(port_offers[portID].weight,port_offers[portID].product_type,port_offers[portID].expirationTime,port_offers[portID].portID,port_offers[portID].distributionDay);

    
    pierSemID = getPierSem();
    shipSemID = getShipSem();

    ship->inSea = 0;
    mutexPro(pierSemID, portID, LOCK, errorHandler, "accessPortForCharge->semBanchine LOCK");

    /* in questo momento la nave è attraccata alla banchina del porto*/

    /* il porto ha già decrementato */

    mutexPro(shipSemID, ship->shipID, LOCK, errorHandler, "accessPortForCharge-> shipSemID LOCK");

    if(!isScadutaProduct(p)){
                
        port = getPort(portID);
       
        nanosecsleep((p->weight / so_loadspeed) * NANOS_MULT);

        if (port->swell) {
            port->weatherTarget = 1;
            printf("⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️\n[%d]Nave: rallentata %d ore in più perchè c'è mareggiata\n⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️\n" , ship->shipID,SO_("SWELL_DURATION"));
            nanosecsleep((double)(NANOS_MULT* 0.04166667) * so_swell_duration);
            port->swell = 0;
        }
        logShip(ship->shipID, "prima add");
        printShip(ship);
        addProduct(ship, p,port);
        

        printTransaction(ship->shipID, portID, 1, p->weight, p->product_type);
        
        detachPort(port , portID, "accessPortForChargeV1");
        
    }
    else {
        printf("\nOOPS! [%d]Nave: la merce che volevo caricare è scaduta!!!\n", ship->shipID);
        logShip( ship->shipID, "OOPS la merce che volevo caricare è scaduta!!!");
        addNotExpiredGood(p->weight,p->product_type,SHIP, 0, ship->shipID);
        addExpiredGood(p->weight, p->product_type, SHIP);
    }
    
    mutexPro(shipSemID, ship->shipID, UNLOCK, errorHandler, "accessPortForCharge->shipSemID UNLOCK");
    ship->inSea = 1;
    mutexPro(pierSemID, portID, UNLOCK, errorHandler, "accessPortForCharge->pierSemID UNLOCK");
    
    checkShipDead(ship);
    
}
/*
void accessPortForCharge(Ship ship, int portID) {
    int pierSemID;
    int shipSemID;
    int stormSwellShmID;
    int* victimIdx;
    Port port;
    Product p;

    p.expirationTime = ship->promisedProduct.expirationTime;
    p.weight = ship->promisedProduct.weight;
    p.product_type = ship->promisedProduct.product_type;
    
    pierSemID = getPierSem();
    shipSemID = getShipSem();

    ship->inSea = 0;

    mutexPro(pierSemID, portID, LOCK, errorHandler, "accessPortForCharge->semBanchine LOCK");

    /* in questo momento la nave è attraccata alla banchina del porto*/

    /* il porto ha già decrementato 
    logShip( ship->shipID, "[%d]FACCIO LOCK DI SHIP SHMID+ DURANTE UNA CHARGE");
    mutexPro(shipSemID, ship->shipID, LOCK, errorHandler, "accessPortForCharge-> shipSemID LOCK");

    logShip(ship->shipID, " sono attracata alla banchina del porto per aggiungere la merce");
    if(ship->promisedProduct.expirationTime > 0){
                
        port = getPort(portID);
       
        nanosecsleep((p.weight / SO_LOADSPEED)*NANOS_MULT);

        if (port->swell) {
            port->weatherTarget = 1;
            logShip(ship->shipID, "⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️\nrallentata %d ore in più perchè c'è mareggiata\n⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️⚓️\n");
            nanosecsleep((double)(NANOS_MULT* 0.04166667) * SO_SWELL_DURATION);
            port->swell = 0;
        }
        
        addProduct(ship, p, port);
        logShip(ship->shipID, "merce caricata con successo");
        printTransaction(ship->shipID, portID, 1, p.weight, p.product_type);
        
        detachPort(port), "shmDetach Porto");
        
    }
    else {
        logShip( ship->shipID, "\nOOPS! [%d]Nave: la merce che volevo caricare è scaduta!!!\n");
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
    
    checkShipDead(ship);
}
*/


void accessPortForDischargeV1(Ship ship, int portID, Product p, int product_index, int quantoPossoScaricare) {
    int pierSemID;
    int shipSemID;
    int portBufferSemID;
    int stormSwellShmID;
    int* victimIdx;
    Port port;
    int i;

    pierSemID = getPierSem();
    shipSemID = getShipSem();

    ship->inSea = 0;

    port = getPort(portID);
    
    mutexPro(pierSemID, portID, LOCK, errorHandler, "accessPortForDisCharge->pierSemID LOCK");
    mutexPro(shipSemID, ship->shipID, LOCK, errorHandler,  "accessPortForDisCharge->shipSemid LOCK");
    i = 0;
    
    while (product_index != -1) {
        logShip(ship->shipID, "prima di deliverProduct");
        printShip(ship);
        
        product_index = deliverProduct(ship, port, product_index, p, portID, i==0, quantoPossoScaricare);
        
        logShip(ship->shipID, "dopo delierProdut");
        printShip(ship);
        
        if(product_index != -1){
            p = productAt(ship->loadship, product_index);
            
        }else{
            p = NULL;
        }
        i++;
    }
    
    detachPort(port,portID, "accessPortForDischargeV1");

    mutexPro(shipSemID, ship->shipID, UNLOCK, errorHandler, "accessPortForCharge->shipSemID UNLOCK");
    ship->inSea = 1;

    mutexPro(pierSemID, portID, UNLOCK, errorHandler, "accessPortForCharge->pierSemID UNLOCK");
    checkShipDead(ship);
    
}
/*
void accessPortForDischarge(Ship ship, int portID, int product_index, int quantoPossoScaricare) {
    int pierSemID;
    int shipSemID;
    int portBufferSemID;
    int stormSwellShmID;
    int* victimIdx;
    Product p;
    Port port;
    int i;

    
    p->product_type = ship->products[product_index].product_type;
    p->expirationTime = ship->products[product_index].expirationTime;
    p->weight = ship->products[product_index].weight;
    

    pierSemID = getPierSem();
    shipSemID = getShipSem();

    ship->inSea = 0;

    port = getPort(portID);
    
    mutexPro(pierSemID, portID, LOCK, errorHandler, "accessPortForDisCharge->pierSemID LOCK");
    logShip(ship->shipID, "Faccio lock di shipSemID discharge\n");
    mutexPro(shipSemID, ship->shipID, LOCK, errorHandler,  "accessPortForDisCharge->shipSemid LOCK");
    logShip(ship->shipID, "Passata lock shipSemID discharge\n");
    i=0;
    while(product_index >=0){
        product_index = deliverProduct(ship, port, product_index, p, portID, i==0);
        
        if(product_index >= 0){
            p.product_type = ship->products[product_index].product_type;
            p.expirationTime = ship->products[product_index].expirationTime;
            p.weight = ship->products[product_index].weight;
        }


    }
    
    detachPort(port), "accessPortDischarge shmDetach");

    mutexPro(shipSemID, ship->shipID, UNLOCK, errorHandler, "accessPortForCharge->shipSemID UNLOCK");
    ship->inSea = 1;

    mutexPro(pierSemID, portID, UNLOCK, errorHandler, "accessPortForCharge->pierSemID UNLOCK");
    checkShipDead(ship);
}

*/

int isScadutaOffer(PortOffer offer) {
    Port p;
    int res;
    p = getPort(offer.portID);
    res = getExpirationTime(p->supplies, offer.product_type, offer.distributionDay) == 0;
    detachPort(p, offer.portID, "isScadutaOffer");
    return res;
}

int isScadutaProduct(Product prod){
    Port p;
    int res;
    printf("sono dentro isScadutaProduct, sto per gare getPort l'id vale:%d\n", prod->portID);
    p = getPort(prod->portID);
    res = (getExpirationTime(p->supplies, prod->product_type, prod->distributionDay) == 0);
    detachPort( p, prod->portID, "isScadutaProduct");
    return res;
}

void travelCharge(Ship ship, int portID, int* day, PortOffer* port_offers) {
     Port p;
    double dt_x, dt_y, spazio, nanosleep_arg;
    long tempo;
    double tempoInSecondi;
    int tempoRimanente;
    int portBufferSem;
    int so_speed;
    int so_days;
    int so_storm_duration = SO_("STORM_DURATION");
    so_speed = SO_("SPEED");
    so_days = SO_("DAYS");
    portBufferSem = useSem(RESPORTSBUFFERS, errorHandler, "refill->useSem portBufferSem");

    p = getPort(portID);  /* prelevo la struttura del porto alla portID-esima posizione nella shm*/ 

    /* imposto la formula per il calcolo della distanza */

    dt_x = p->x - ship->x;
    dt_y = p->y - ship->y;
    spazio = sqrt(pow(dt_x, 2) + pow(dt_y, 2));
    tempo = (long)((spazio / so_speed) * NANOS_MULT);

    
    /* spazio/SO_SPEED è misurato in giorni (secondi), quindi spazio/SO_SPEED*1000000000 sono il numero di nanosecondi per cui fare la sleep */
    tempoInSecondi = spazio / so_speed;
    tempoRimanente = so_days - 1 - (*day);

    printf("[%d]Nave: viaggio per %f secondi...\n", ship->shipID, tempoInSecondi);
    if (tempoInSecondi > tempoRimanente)
    {
        printf("[%d]Nave: non avrei abbastanza giorni per raggiungere il porto: %d, termino...\n", ship->shipID, portID);
        restorePromisedGoods(p, port_offers[portID].distributionDay, port_offers[portID].product_type, port_offers[portID].weight, portID);
        exitNave(ship);
    }

    ship->inSea = 1;
    nanosecsleep(tempo);

    if (ship->storm == 1) {
        ship->weatherTarget = 1;
        printf("🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️\n[%d]Nave: ho beccato una tempesta\n🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️\n", ship->shipID);
        nanosecsleep((double)(NANOS_MULT * 0.04166667) * so_storm_duration);
        ship->storm = 0;
    }

    if (ship->dead) {
        restorePromisedGoods(p, port_offers[portID].distributionDay, port_offers[portID].product_type, port_offers[portID].weight, portID);
        printf("🌊🌊🌊🌊🌊🌊\n[%d]Nave: sono stata uccisa\n🌊🌊🌊🌊🌊🌊\n", ship->shipID);
        exitNave(ship);
    }
    printf("[%d]Nave: viaggio finito...\n", ship->shipID);
    

    /* Dopo aver fatto la nanosleep la nave si trova esattamente sulle coordinate del porto
       quindi aggiorniamo le sue coordinate */
    
   
    ship->x = p->x;
    ship->y = p->y;
    detachPort(p, portID, "travelCharge");

}

void travelDischarge(Ship ship, int portID, int* day, Product prod, int* portResponses) {
    Port p;
    double dt_x, dt_y, spazio, nanosleep_arg;
    long tempo;
    double tempoInSecondi;
    int tempoRimanente;
    int portBufferSem;
    int so_speed;
    int so_days;
    int so_storm_duration = SO_("STORM_DURATION");
    so_speed = SO_("SPEED");
    so_days = SO_("DAYS");
    portBufferSem = useSem(RESPORTSBUFFERS, errorHandler , "refill->useSem portBufferSem");

    p = getPort(portID);  /* prelevo la struttura del porto alla portID-esima posizione nella shm*/ 

    /* imposto la formula per il calcolo della distanza */

    dt_x = p->x - ship->x;
    dt_y = p->y - ship->y;
    spazio = sqrt(pow(dt_x, 2) + pow(dt_y, 2));
    tempo = (long)((spazio / so_speed) * NANOS_MULT);

    
    /* spazio/SO_SPEED è misurato in giorni (secondi), quindi spazio/SO_SPEED*1000000000 sono il numero di nanosecondi per cui fare la sleep */
    tempoInSecondi = spazio / so_speed;
    tempoRimanente = so_days - 1 - (*day);

    logShip(ship->shipID, " viaggio per %f secondi...\n");
    if (tempoInSecondi > tempoRimanente)
    {
        printf("[%d]Nave: non avrei abbastanza giorni per raggiungere il porto: %d, termino...\n", ship->shipID, portID);
        logShip(ship->shipID, " non avrei abbastanza giorni per raggiungere il porto: %d, termino...\n");
        
        
        restorePortRequest(p, prod->product_type, portResponses[prod->portID] , prod->weight);
        detachPort(p, portID, "travelDischarge");
        exitNave(ship);
    }

    ship->inSea = 1;
    nanosecsleep(tempo);

    if (ship->storm == 1) {
        ship->weatherTarget = 1;
        logShip(ship->shipID, "🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️\n[%d]Nave: ho beccato una tempesta\n🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️🌪️\n");
        nanosecsleep((double)(NANOS_MULT * 0.04166667) * so_storm_duration);
        ship->storm = 0;
    }

    if (ship->dead) {
      
        restorePortRequest(p, prod->product_type, portResponses[prod->portID] , prod->weight);
        
        printf("🌊🌊🌊🌊🌊🌊\n[%d]Nave: sono stata uccisa\n🌊🌊🌊🌊🌊🌊\n", ship->shipID);

        detachPort(p, portID, "travelDischarge");
       
        exitNave(ship);
    }
    logShip(ship->shipID,"viaggio finito...\n");
    

    /* Dopo aver fatto la nanosleep la nave si trova esattamente sulle coordinate del porto
       quindi aggiorniamo le sue coordinate */
    
   
    ship->x = p->x;
    ship->y = p->y;
    detachPort(p, portID, "travelDischarge");
}


int countShipWhere(Ship arrShip , int(*f)(int,Ship)){
    int count;
    int i;
    count = 0;

    int so_navi = SO_("NAVI");
    for (i = 0; i < so_navi; i++){
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
    int so_porti = SO_("PORTI");
    int so_navi = SO_("NAVI");
    int so_banchine = SO_("BANCHINE");
    semPierID = useSem(BANCHINESEMKY, errorHandler, "printStatoNavi");

    shmShip = useShm(SSHMKEY, sizeof(struct ship) * so_navi, errorHandler, "printStatoNavi");
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

    for (i = 0; i < so_porti; i++)
    { /* navi in porto che fanno operazioni di carico/scarico*/

        shipsInPorts += so_banchine - getOneValue(semPierID, i);
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
    int so_porti = SO_("PORTI");
    waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler, "chargeProducts->waitToTravelSemID");
    mutexPro(waitToTravelSemID, ship->shipID, WAITZERO, errorHandler, "chargeProducts->waitToTravelSemID WAITZERO");
    sleep(0.2);
    mutexPro(waitToTravelSemID, ship->shipID, so_porti, errorHandler, "chargeProducts->waitToTravelSemID +SO_PORTI");
}
/*
void initPromisedProduct(Ship ship, PortOffer port_offer, int quantityToCharge){
    ship->promisedProduct.expirationTime = port_offer.expirationTime;
    ship->promisedProduct.product_type = port_offer.product_type;
    ship->promisedProduct.weight = quantityToCharge;
    ship->promisedProduct.distributionDay = port_offer.distributionDay;
}*/

void checkTerminateValue(Ship ship, unsigned int* terminateValue){
 if (*terminateValue == 1){
        logShip(ship->shipID, "il programma è terminato\n");
        exitNave(ship);
    }   
}

int getPierSem(){
    int semID;
    semID = useSem(BANCHINESEMKY, errorHandler, "get pier sem");
    return semID;
}

int getShipSem(){
    int semID;
    semID = useSem(SEMSHIPKEY, errorHandler, "get ship sem");
    return semID;
}

void checkShipDead(Ship ship){
    if (ship->dead) {
        printf("🌊🌊🌊🌊🌊🌊\n[%d]Nave: sono stata uccisa\n🌊🌊🌊🌊🌊🌊\n", ship->shipID);
        /* SEGNARE TUTTA LA MERCE COME AFFONDATA*/
        exitNave(ship);
    }
}
int f(int el){
    return el > 0;
}
int chooseNewProductIndex(Ship s, Port p){
    int i;
    Product aux;
    int so_merci = SO_("MERCI");
    i=0;
    for (aux = s->loadship->first; aux!=NULL; aux= aux->next){
        if(contain(findIdxs(p->requests,so_merci,f),aux->product_type)){
            return i;
        }
        i++;
    }
    return -1;
}

int deliverProduct(Ship ship, Port port, int product_index, Product p, int portID, int firstProd, int quantoPossoScaricare){

    int scarico;
    int new_index;
    int verifyRequestSemID;
    int so_loadspeed = SO_("LOADSPEED");
    int so_swell_duration = SO_("SWELL_DURATION");

    verifyRequestSemID = useSem(P2SEMVERIFYKEY, errorHandler, "recvChargerHandler->verifyRequestSemID");

    /* TODO: SEMAFORO PROTEZIONE RICHIESTE LOCK*/
    if (!isScadutaProduct(p)) {

        if(!firstProd){
            mutexPro(verifyRequestSemID, portID, LOCK, errorHandler, "recvChargerHandler->verifyRequestSemID LOCK");

            scarico = checkRequests(port, p->product_type, p->weight);
            mutexPro(verifyRequestSemID, portID, UNLOCK, errorHandler, "recvChargerHandler->verifyRequestSemID UNLOCK");

        }else{
            scarico = quantoPossoScaricare;
        }

        /*UNLOCK*/
    
        if(scarico!=-1){
        
           
                    
            nanosecsleep((p->weight / so_loadspeed) * NANOS_MULT);
            
            if (port->swell) {
                port->weatherTarget = 1;
                printf("Porto %d colpito da una mareggiata, devo aspettare %d ore in più\n" , portID, so_swell_duration);
                nanosecsleep((double)(NANOS_MULT  * 0.04166667)* so_swell_duration);
                port->swell = 0;
            }
            printf("Scarico: %d, p.weight: %d\n", scarico, p->weight);
            if (scarico >= p->weight) {
                
                addDeliveredGood(p->weight, p->product_type, portID);
                
                printTransaction(ship->shipID, portID, 0, p->weight, p->product_type);
                
                removeProduct(ship, product_index);
                

            }else {
                
                addDeliveredGood(scarico, p->product_type, portID);
                printTransaction(ship->shipID, portID, 0, scarico, p->product_type);
                
                p->weight -= scarico;
                ship->weight-= scarico;
            }
    
        } else {
            logShip(ship->shipID ,"OOPS! il porto non ha richiesta per questo prodotto");
        }
    }
    else {
        addExpiredGood(p->weight, p->product_type, SHIP);
        removeProduct(ship, product_index);

        logShip(ship->shipID ,"OOPS! la merce che volevi scaricare è scaduta!!!");

    }
    new_index = chooseNewProductIndex(ship,port);
    printf("new product index: %d\n", product_index);
    return new_index;
}

void removeExpiredGoodsOnShip(Ship ship){
    Product aux;
    int index = 0;
    for(aux = ship->loadship->first; aux != NULL; aux = aux->next){
        if(isScadutaProduct(aux)){
            addExpiredGood(aux->weight, aux->product_type, SHIP);
            removeProduct(ship, index);
        }
        index++;
    }
}
