#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "../config1.h"
#include "../utils/errorHandler.h"
#include "../utils/sem_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"
#include "../utils/msg_utility.h"
#include "./dump.h"
#include "./nave.h"
#include "./porto.h"



void chargeProducts(Ship ship, int quantityToCharge, int* day, unsigned int* terminateValue){
    int availablePorts;
    int portID;
    int waitResponsesID;
    intList* typeToCharge;
    int so_porti;
    int so_days;
    PortOffer* port_offers;
    so_days = SO_("DAYS");
    so_porti = SO_("PORTI");
    port_offers = (PortOffer*)malloc(sizeof(PortOffer) * so_porti);
    typeToCharge = getTypeToCharge();


    logShip(ship->shipID, "controllo se ha senso continuare\n");
    if(typeToCharge->length == 0){
        if(*day < so_days -1){
            waitEndDay();
            free(port_offers);
            
            return;
        }else{
            printf("💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀\n");
            printf("Nave con id:%d NON HA PIÙ SENSO CONTINUARE\n", ship->shipID);
            printf("💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀\n");
            intFreeList(typeToCharge);
            free(port_offers);
            exitNave(ship);
        }
        
    }

    intFreeList(typeToCharge);
    initArrayOffers(port_offers);
    
    /* check merce scaduta*/
    printShip(ship);
    removeExpiredGoodsOnShip(ship);
    printShip(ship);


    if (quantityToCharge == 0) {
        free(port_offers);
        
        dischargeProducts(ship, day, terminateValue);
    }
    else {

        availablePorts = communicatePortsForCharge(ship, quantityToCharge, port_offers); 
        logShip(ship->shipID, "finito di chiamare i porti\n");
        if (availablePorts == 0) {
            
            replyToPortsForCharge(-1, port_offers);
            removeExpiredGoodsOnShip(ship);
            free(port_offers);
           
            chargeProducts(ship, chooseQuantityToCharge(ship), day, terminateValue);
            
            
        } else {
            /* ci sono porti che hanno merce da caricare*/
            
            portID = choosePortForCharge(port_offers, ship->shipID);
            
            replyToPortsForCharge(portID, port_offers);

            logShip(ship->shipID,"Sono partita...\n");
            travelCharge(ship, portID, day, port_offers);
            logShip(ship->shipID, "fatta travel");
            accessPortForCharge(ship, portID, port_offers);
            logShip(ship->shipID, "finita accessPortToCharge");
            removeExpiredGoodsOnShip(ship);
            free(port_offers);
            
        }
    }
    
}

void dischargeProducts(Ship ship, int* day, unsigned int* terminateValue) {

    int portID;
    int product_index;
    int quantoPossoScaricare;
    int so_porti;
    
    int* portResponses;
    Product prod = NULL;
    so_porti = SO_("PORTI");
    portResponses = (int*)malloc(sizeof(int) * so_porti);
    removeExpiredGoodsOnShip(ship);
    
    checkTerminateValue(ship, terminateValue);
    if (ship->weight== 0)
    {
        free(portResponses);
        return;
    }
    else
    {

        product_index = chooseProductToDelivery(ship);
        printf("[%d]Nave ho scelto per scaricare: %d\n", ship->shipID,product_index);
        prod = productAt(ship->loadship, product_index);
        
        /*
            verificare che prod != NULL, se è == NULL vuol dire che la lista è vuota
        */
        if (prod == NULL) {

            removeExpiredGoodsOnShip(ship);
            free(portResponses);
            return;
        }

        portID = communicatePortsForDischarge(ship, prod, &quantoPossoScaricare, portResponses);

        
        if (portID == -1) {

            addExpiredGood(prod->weight, prod->product_type, SHIP);
            printf("BUTTO VIA PRODOTTO a p_idx: %d\n", product_index);
            removeProduct(ship, product_index); 

            logShip( ship->shipID, " riprovo a scegliere il prodotto da scaricare\n");
            replyToPortsForDischarge(ship, -1, quantoPossoScaricare, portResponses, prod);
            free(portResponses);
            dischargeProducts(ship, day, terminateValue);            /* chiamo la dischargeProducts cercando un nuovo prodotto da consegnare */
        
        } else {
             
            replyToPortsForDischarge(ship, portID, quantoPossoScaricare, portResponses, prod);

            travelDischarge(ship, portID, day, prod, portResponses);
            accessPortForDischarge(ship, portID, prod, product_index, quantoPossoScaricare);
            free(portResponses);
            removeExpiredGoodsOnShip(ship);
            
        }
    }
}

void shipRoutine(Ship ship, unsigned int* terminateValue, int* day){

    checkTerminateValue(ship, terminateValue);
    chargeProducts(ship, chooseQuantityToCharge(ship), day, terminateValue);
    
}

int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/
    int endShmID;
    int dayShmID;
    int *day;
    unsigned int *terminateValue;
    Ship ship;
    signal(SIGCHLD, SIG_IGN);
    endShmID = useShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "Nave: use end shm");
    terminateValue = (unsigned int*)getShmAddress(endShmID, 0, errorHandler, "Nave: getShmAddress di endShm");

    dayShmID = useShm(DAYWORLDSHM, sizeof(int), errorHandler, "dayShmID nel main della nave");
    day = (int *)getShmAddress(dayShmID, 0, errorHandler, "dayShmID nel main della nave");

    ship = initShip(atoi(argv[1]));

    checkInConfig();
    logShip(ship->shipID, "config finita, aspetto ok partenza dal master");
    waitForStart();
    logShip(ship->shipID, "partita...");
    
       while (1) {      
        shipRoutine(ship, terminateValue, day);
    }
}

