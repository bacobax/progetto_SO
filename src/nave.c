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
    PortOffer port_offers[SO_PORTI];
    int waitResponsesID;
    int waitToTravelSemID;
    intList *tipiDaCaricare;

    tipiDaCaricare = haSensoContinuare();
    printf("[%d]Nave tipi da caricare: \n", ship->shipID);
    intStampaLista(tipiDaCaricare);

    
    printf("[%d]Nave, controllo se ha senso continuare-day: %d\n", ship->shipID,*day);
    if(tipiDaCaricare->length == 0){
        if(*day < SO_DAYS -1){
            waitEndDay();
            
            return;
        }else{
            printf("💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀\n");
            printf("Nave con id:%d NON HA PIÙ SENSO CONTINUARE\n", ship->shipID);
            printf("💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀\n");
            intFreeList(tipiDaCaricare);
            exitNave(ship);
        }
        
    }

    intFreeList(tipiDaCaricare);
    initArrayOffers(port_offers);
    

    if (quantityToCharge == 0) {
        dischargeProducts(ship, day, terminateValue);
    }
    else {

        availablePorts = communicatePortsForChargeV1(quantityToCharge, port_offers); /* mando msg a tutti i porti perchè voglio caricare*/
        logShip(ship->shipID, "finito di chiamare i porti\n");
        
        if (availablePorts == 0) {
            
            replyToPortsForChargeV1(-1, port_offers);
           
            chargeProducts(ship, chooseQuantityToCharge(ship), day, terminateValue);
            
            
        } else {
            /* ci sono porti che hanno merce da caricare*/
            
            portID = choosePortForCharge(port_offers, ship->shipID);
            
            replyToPortsForChargeV1(portID, port_offers);
            
            
            printf("[%d]Nave: Sono partita...\n", ship->shipID);
            travelCharge(ship, portID, day, port_offers);
            
            accessPortForChargeV1(ship, portID, port_offers);
            /*
                TODO: checkMerceScaduta();
            */
        }
    }
    
}

void dischargeProducts(Ship ship, int* day, unsigned int* terminateValue) {

    int portID;
    int product_index;
    int waitToTravelSemID;
    int quantoPossoScaricare;
    int portResponses[SO_PORTI];
    Product prod = NULL;
    /*
        Se non può scaricare quello che ha 
    */
    checkTerminateValue(ship, terminateValue);
    if (ship->loadship->weightLoad == 0)
    {
        return;
    }
    else
    {

        /*

            1 - Nave) Mando un msg a tutti i porti indicano il tipo di merce che voglio scaricare e la quantità che possiedo
                  (scelgo la merce con tempo di scadenza minore di tutte le altre merci che posseggo).

           2 - Porto) il porto riceve il messaggio è valuta guardando il suo magazzino se la merce si può consegnare
                      oppure no perchè la domanda è arrivata a 0.

                        - Se si può consegnare decrementa la domanda e manda una conferma positiva al porto
                        - Altrimenti manda una conferma negativa

                        OVVIAMENTE POICHÈ LA PRIORITÀ È CONSEGNARE LE MERCI IN "FIN DI VITA", ALLA PRIMA NAVE
                        CHE MANDA UN MESSAGGIO POSITIVO DI CONFERMA IL PORTO SI DISINTERESSA DELLE RICHIESTE SUCCESSIVE
                        SE LA SUA DOMANDA È ARRIVATA A 0.
                        ALTRIMENTI CONTINUA FINO A QUANDO LA DOMANDA PER QUEL TIPO DI MERCE SCENDE A 0

                        POLITICA FIFO


        */

        product_index = chooseProductToDelivery(ship);
        printf("[%d]Nave ho scelto per scaricare: %d", ship->shipID,product_index);

        prod = productAt(ship->loadship, product_index);

        portID = communicatePortsForDischargeV1(ship, prod, &quantoPossoScaricare, portResponses);

        
        if (portID == -1) {

            addExpiredGood(prod->weight, prod->product_type, SHIP);
            removeProduct(ship->loadship, product_index); 

            logShip(" riprovo a scegliere il prodotto da scaricare\n", ship->shipID);
            replyToPortsForDischargeV1(ship, -1, quantoPossoScaricare, portResponses, prod);
            dischargeProducts(ship, day, terminateValue);            /* chiamo la dischargeProducts cercando un nuovo prodotto da consegnare */
        
        } else {
            


            /* 3) Una volta arrivato al porto accedo alla prima banchina disponibile e rimuovo la merce che intendo
            consegnare dal carico della nave */
            
            replyToPortsForDischargeV1(ship, portID, quantoPossoScaricare, portResponses, prod);
            //TODO: SONO ARRIVATO QUI

            travel(ship, portID, day);
            accessPortForDischargeV1(ship, portID, product_index, quantoPossoScaricare);
            
        }
    }
}

void shipRoutine(Ship ship, unsigned int* terminateValue, double restTime, int* day){

    checkTerminateValue(ship, terminateValue);
    chargeProducts(ship, chooseQuantityToCharge(ship), day, terminateValue);
    nanosecsleep((long)(restTime * NANOS_MULT));
    
    /*
    checkTerminateValue(ship, terminateValue);
    dischargeProducts(ship, day);
    nanosecsleep((long)(restTime * NANOS_MULT)); 
    */
    
    
    
    
}

int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/
    int endShmID;
    int dayShmID;
    int *day;
    unsigned int *terminateValue;
    double restTime = RESTTIMESHIP;
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
    ship->nChargesOptimal = (int)numeroDiCarichiOttimale();
    logShip(ship->shipID, "partita...");
    
       while (1) {      
        shipRoutine(ship, terminateValue, restTime, day);
    }
}

