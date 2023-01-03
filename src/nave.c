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


void chargeProducts(Ship ship, int quantityToCharge){
    int availablePorts;
    int portID;
    PortOffer port_offers[SO_PORTI];
    int waitToTravelSemID;
    int waitResponsesID;
    
    initArrayOffers(port_offers);

    if(quantityToCharge == 0){
        dischargeProducts(ship);
    }
    else {
        /*
        waitResponsesID= useSem(WAITFIRSTRESPONSES, NULL);
        
        */
        
        callPortsForCharge(ship, quantityToCharge); /* mando msg a tutti i porti perchè voglio caricare*/
        printf("[%d]Nave: finito di chiamare i porti\n", getpid());
        /*
        mutexPro(waitResponsesID, ship->shipID, WAITZERO, errorHandler);
        
        printf("[%d]Nave: finito aspettare le risposte dai porti\n", getpid());
        */

        availablePorts = portResponsesForCharge(ship, port_offers);
        printf("[%d]NAVE: Aviable ports = %d\n",getpid(), availablePorts);
        if (availablePorts == 0) {
            /* non ci sono porti disponibili per la quantità
               di merce che voglio caricare, riprovo a chiamare i porti decrementando la quantità*/
            chargeProducts(ship, quantityToCharge - 1); 
        
        } else {
            /* ci sono porti che hanno merce da caricare*/
            waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler, "chargeProducts->waitToTravelSemID");
            
            portID = choosePortForCharge(port_offers);

            replyToPortsForCharge(ship, portID);
            
            printf("[%d]Nave: Aspetto a partire...\n", getpid());
            mutexPro(waitToTravelSemID, ship->shipID, WAITZERO, errorHandler, "chargeProducts->waitToTravelSemID WAITZERO");
            mutexPro(waitToTravelSemID, ship->shipID, SO_PORTI, errorHandler, "chargeProducts->waitToTravelSemID +SO_PORTI");
            
            
            
            printf("[%d]Nave: sono partita...\n", getpid());
            travel(ship, portID);
            
            accessPortForCharge(ship, portID, port_offers[portID], quantityToCharge);

            
            
        }
    }
    
}

void dischargeProducts(Ship ship) {

    int portID;
    int product_index;
    int waitToTravelSemID;
    int quantoPossoScaricare;
    
    if (ship->weight == 0) {

        chargeProducts(ship, chooseQuantityToCharge(ship));

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
        printf("\n\n[%d]Nave: la mia merce scade tra:%d\n\n", getpid(), ship->products[product_index]);
        portID = communicatePortsForDischarge(ship, ship->products[product_index], &quantoPossoScaricare);  

    /* 2 - Nave) Per ogni porto che mi risponde posso trovarmi in uno dei seguenti casi:
            
            - Conferma positiva dal porto, ho consegnato (DOMANDA merce - quantità che volevo consegnare) quindi
              nel migliore dei casi ho azzerato la domanda per quel tipo di merce di quel determinato porto.

            - Conferma negativa dal porto, la domanda di quel tipo di merce che volevo consegnare è scesa a 0.

          Se ho trovato almeno un porto con conferma positiva faccio la travel() e mi dirigo da lui, il primo che
          mi risponde, vado al punto 3).

          Se tutti i porti mi hanno inviato una conferma negativa allora la domanda di quel tipo di merce
          in tutti i porti è pari a 0, torno al punto 1) scegliendo un tipo di merce diverso.
          
          Se tornando ripetutamente al punto 1) arrivo ad esaurire tutte le merci perchè o sono scadute o 
          nessun porto ha DOMANDA relativa al mio carico, allora faccio chargeProducts()       

        
        2 - Porto) Il porto non fa niente

    */
    /*
        for(int i=0; i<SO_PORTI; i++){
            1)nave scrive su coda del porto i (ftok(porto.h , i)) 
            2)Dentro il messaggio: 0/1|tipo di merce|peso merce TYPE: shipID +1
                Porto:
                sscanf("%d|%s", &scarico, &payload);
                if(scarico){
                    sscanf("%d|%d" , &type, &quantity);
                    1)Porto scrive su coda della nave shipID (ftok(nave.c, shipID))
                    2)Dentro il messaggio: richiesta di merce del tipo type
                }else{
                    ...
                }
            3)Nave riceve messaggio del porto
        }

    */
    /*portID = portResponsesForDischarge(ship, &quantoPossoScaricare);*/
        
        printf("PORT ID SCELTO: %d\n", portID);
        if(portID == -1){
            addExpiredGood(ship->products[product_index].weight, ship->products[product_index].product_type, SHIP);
            removeProduct(ship, product_index); /* vecchio prodotto da scaricare rimosso (tanto le domande dei porti sono tutte a 0) */

            printf("Riprovo a scegliere il prodotto da scaricare\n");
            dischargeProducts(ship);            /* chiamo la dischargeProducts cercando un nuovo prodotto da consegnare */
        
        } else {
            waitToTravelSemID = useSem(WAITTOTRAVELKEY, errorHandler,  "dischargeProducts->waitToTravelSemID");

            /* 3) Una volta arrivato al porto accedo alla prima banchina disponibile e rimuovo la merce che intendo
            consegnare dal carico della nave */
            
            replyToPortsForDischarge(ship, portID);
            mutexPro(waitToTravelSemID, ship->shipID, WAITZERO, errorHandler, "dischargeProducts->waitToTravelSemID WAITZERO");
            mutexPro(waitToTravelSemID, ship->shipID, SO_PORTI, errorHandler, "dischargeProducts->waitToTravelSemID +SO_PORTI");


            travel(ship, portID);
            printf("\n\n[%d]Nave: la mia merce scade tra:%d E STO PER FARE accessPortForDischarge\n\n", getpid(), ship->products[product_index]);
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



int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/
    int res;
    Product p1, p2, p3, p4;
    p1.product_type = 0;
    p1.expirationTime = 4;
    p1.weight = 3;
    p2.product_type = 1;
    p2.expirationTime = 2;
    p2.weight = 7;
    p3.product_type = 2;
    p3.expirationTime = 3;
    p3.weight = 10;
    p4.product_type = 4;
    p4.expirationTime = 2;
    p4.weight = 8;

    Ship ship;
    
    ship = initShip(atoi(argv[1]));
    int charge = 1;

    checkInConfig();
    printf("[%d]Nave con id:%d: config finita, aspetto ok partenza dal master...\n", getpid(),ship->shipID);
    waitForStart();
    printf("[%d]Nave con id:%d partita\n", getpid(),ship->shipID);

    /* while(1){
    res = addProduct(ship, p2);
    printShip(ship);
    sleep(2);
        }
    */
    res = addProduct(ship, p1);
    res = addProduct(ship, p2);
    res = addProduct(ship, p3);
    res = addProduct(ship, p4);
    

    sleep(1.5);
    dischargeProducts(ship);
    printf("FINITO SCARICO\n");
    printShip(ship);
        

        while (1) {
            sleep(1);
        }

    exit(EXIT_FAILURE);

    
    // while (1) { 
    //     if(charge == 1){
    //         chargeProducts(ship, 10);
    //         charge = 0;
    //     } else {
    //         dischargeProducts(ship);
    //         charge = 1;
    //     }
    //     nanosecsleep(NANOS_MULT);
    // }
    
}


