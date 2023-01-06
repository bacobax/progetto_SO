#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"
#include "../utils/msg_utility.h"
#include "./dump.h"
#include "./nave.h"
#include "./porto.h"
/*
    TODO: aggiungere criterio alla nave per scegliere il porto in cui caricare: il tipo di offerta che deve accettare dev'essere tra i tipi di merce totali richiesti
    TODO: non decrementare più di 1 quantity, ma settarla direttamente a min{max delle offerte, aviable cap}
*/

void exitNave(){
    int waitShipSemID = useSem(WAITSHIPSSEM, errorHandler, "nave waitShipSemID");   
    mutex(waitShipSemID, LOCK, errorHandler, "nave mutex LOCK waitShipSemID");
    exit(0);
}

void chargeProducts(Ship ship, int quantityToCharge, int* day){
    int availablePorts;
    int portID;
    PortOffer port_offers[SO_PORTI];
    int waitToTravelSemID;
    int waitResponsesID;
    int waitEndDaySemID;
    int waitEndDayShipSemID;
    intList *tipiDaCaricare;

    tipiDaCaricare = haSensoContinuare();
    printf("TIPI DA CARICARE: \n");
    intStampaLista(tipiDaCaricare);

    waitEndDaySemID = useSem(WAITENDDAYKEY, errorHandler, "waitEndDay in chargeproducts");
    waitEndDayShipSemID = useSem(WAITENDDAYSHIPSEM, errorHandler, "waitEndDayShipSemID in chargeProducts");
    printf("CONTROLLO SE HA SENSO CONTINUARE: day: %d\n", *day);

    if(tipiDaCaricare->length == 0){
        if(*day < SO_DAYS -1){
            printf("SBATTO SU WAITZERO\n");
            mutex(waitEndDayShipSemID, 1, errorHandler, "+1 waitEndDayShipSemID");
            mutex(waitEndDaySemID, WAITZERO, errorHandler, "WAITZERO su waitEndDaySemID");
            mutex(waitEndDayShipSemID, -1, errorHandler, "-1 waitEndDayShipSemID");
            
            printf("E' PASSATO IL GIORNO\n");
            chargeProducts(ship, quantityToCharge, day);
            return;
        }else{
            printf("💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀\n");
            printf("Nave con id:%d NON HA PIÙ SENSO CONTINUARE\n", ship->shipID);
            printf("💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀💀\n");
            intFreeList(tipiDaCaricare);
            exitNave();
        }
        
    }

    intFreeList(tipiDaCaricare);


    initArrayOffers(port_offers);

    if (quantityToCharge == 0) {
        dischargeProducts(ship);
    }
    else {
        /*
        waitResponsesID= useSem(WAITFIRSTRESPONSES, NULL);
        
        */
        
        availablePorts = communicatePortsForCharge(ship, quantityToCharge, port_offers); /* mando msg a tutti i porti perchè voglio caricare*/
        printf("[%d]Nave: finito di chiamare i porti\n", getpid());
        /*
        mutexPro(waitResponsesID, ship->shipID, WAITZERO, errorHandler);
        
        printf("[%d]Nave: finito aspettare le risposte dai porti\n", getpid());
        */
        waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler, "chargeProducts->waitToTravelSemID");

        /*availablePorts = portResponsesForCharge(ship, port_offers);*/
        printf("NAVE con id:%d: Aviable ports = %d\n",ship->shipID, availablePorts);
        if (availablePorts == 0) {
            /* non ci sono porti disponibili per la quantità
               di merce che voglio caricare, riprovo a chiamare i porti decrementando la quantità*/
            replyToPortsForCharge(ship, -1);
            chargeProducts(ship, chooseQuantityToCharge(ship), day); 

            mutexPro(waitToTravelSemID, ship->shipID, SO_PORTI, errorHandler, "chargeProducts->waitToTravelSemID +SO_PORTI");

        
        } else {
            /* ci sono porti che hanno merce da caricare*/
            
            portID = choosePortForCharge(port_offers, ship->shipID);

            ship->promisedProduct.expirationTime = port_offers[portID].expirationTime;
            ship->promisedProduct.product_type = port_offers[portID].product_type;
            ship->promisedProduct.weight = quantityToCharge;

            replyToPortsForCharge(ship, portID);
            
            printf("Nave con id:%d: Aspetto a partire...\n", ship->shipID);
            mutexPro(waitToTravelSemID, ship->shipID, WAITZERO, errorHandler, "chargeProducts->waitToTravelSemID WAITZERO");
            mutexPro(waitToTravelSemID, ship->shipID, SO_PORTI, errorHandler, "chargeProducts->waitToTravelSemID +SO_PORTI");
            
        
            printf("Nave con id:%d: sono partita...\n", ship->shipID);
            travel(ship, portID);
            
            accessPortForCharge(ship, portID);
    
        }
    }
    
}

void dischargeProducts(Ship ship, int* day) {

    int portID;
    int product_index;
    int waitToTravelSemID;
    int quantoPossoScaricare;
    /*
        Se non può scaricare quello che ha 
    */
    if (ship->weight == 0) {

        chargeProducts(ship, chooseQuantityToCharge(ship), day);

    } else {

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
        printShip(ship);
        product_index = chooseProductToDelivery(ship);
        printf("PROD IDX: %d", product_index);
        printf("\n\nNave con id:%d: la mia merce scade tra:%d\n\n", ship->shipID, ship->products[product_index].expirationTime);
        portID = communicatePortsForDischarge(ship, ship->products[product_index], &quantoPossoScaricare);  

    
        
        if(portID == -1){

            addExpiredGood(ship->products[product_index].weight, ship->products[product_index].product_type, SHIP);
            removeProduct(ship, product_index); /* vecchio prodotto da scaricare rimosso (tanto le domande dei porti sono tutte a 0) */

            printf("Nave con id:%d: riprovo a scegliere il prodotto da scaricare\n", ship->shipID);
            
            dischargeProducts(ship, day);            /* chiamo la dischargeProducts cercando un nuovo prodotto da consegnare */
        
        } else {
            waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler,  "dischargeProducts->waitToTravelSemID");

            /* 3) Una volta arrivato al porto accedo alla prima banchina disponibile e rimuovo la merce che intendo
            consegnare dal carico della nave */
            
            replyToPortsForDischarge(ship, portID);
            mutexPro(waitToTravelSemID, ship->shipID, WAITZERO, errorHandler, "dischargeProducts->waitToTravelSemID WAITZERO");
            mutexPro(waitToTravelSemID, ship->shipID, SO_PORTI, errorHandler, "dischargeProducts->waitToTravelSemID +SO_PORTI");


            travel(ship, portID);
            printf("\n\nNave con id:%d: la mia merce scade tra:%d E STO PER FARE accessPortForDischarge\n\n", ship->shipID, ship->products[product_index].expirationTime);
            accessPortForDischarge(ship, portID, product_index, quantoPossoScaricare);
        }      


    /*

        LASCIO ANCORA QUESTO PERCHÈ FORSE PUÒ SERVIRE

        Scelta del tipo di merce:
            VETTORE DI VALORI: V[SO_MERCI], V[i] = 1/(media([domanda del tipo i - capienza tipo i]) * scadenza della merce i)
            Es:
                Merce tipo 2: [domanda del porto 0 della merce 2 - mia capienza merce 2, domanda del porto 1 della merce 2 - mia capienza merce 2, ...]
     
    */
    }
}

void shipRoutine(Ship ship, int* terminateValue, int restTime, int* day){
    if (*terminateValue == 1){
        printf("Nave con id:%d il programma è terminato\n", ship->shipID);
        exitNave();
    }
    chargeProducts(ship, chooseQuantityToCharge(ship), day);
    sleep(restTime);
    if (*terminateValue == 1){
        printf("Nave con id:%d il programma è terminato\n", ship->shipID);
        exitNave();
    }
    dischargeProducts(ship, day);
    sleep(restTime);   
}

int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/
    int endShmID;
    int dayShmID;
    int *day;
    Ship ship;
    int *terminateValue;
    int restTime;
    endShmID = useShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "Nave: use end shm");
    terminateValue = (int*)getShmAddress(endShmID, 0, errorHandler, "Nave: getShmAddress di endShm");

    dayShmID = useShm(DAYWORLDSHM, sizeof(int), errorHandler, "dayShmID nel main della nave");
    day = (int *)getShmAddress(dayShmID, 0, errorHandler, "dayShmID nel main della nave");

    ship = initShip(atoi(argv[1]));
    restTime = 1;

    checkInConfig();
    printf("Nave con id:%d: config finita, aspetto ok partenza dal master...\n", ship->shipID);
    waitForStart();
    printf("Nave con id:%d partita\n", ship->shipID);

    
    while (1) {      
        shipRoutine(ship, terminateValue, restTime, day);
    }
}

