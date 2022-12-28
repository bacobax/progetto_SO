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
    
    initArrayOffers(port_offers);

    if(quantityToCharge == 0){
        dischargeProducts(ship);
    } else {
        callPortsForCharge(ship, quantityToCharge); /* mando msg a tutti i porti perchè voglio caricare*/
    
        availablePorts = portResponsesForCharge(ship, port_offers);

        printf("NAVE: Aviable ports = %d\n", availablePorts);
        if (availablePorts == 0) {
            /* non ci sono porti disponibili per la quantità
               di merce che voglio caricare, riprovo a chiamare i porti decrementando la quantità*/
            chargeProducts(ship, quantityToCharge - 1); 
        
        } else {
            /* ci sono porti che hanno merce da caricare*/
            
            portID = choosePortForCharge(port_offers);

            replyToPortsForCharge(ship, portID);

            
            travel(ship, portID);
            
            accessPortForCharge(ship, portID, port_offers[portID], quantityToCharge);

            
            
        }
    }
    
}

void dischargeProducts(Ship ship) {

    int portID;
    int product_index;

    if(ship->weight == 0){

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

        product_index = chooseProductToDelivery(ship);

        callPortsForDischarge(ship, ship->products[product_index]);  

    /* 2 - Nave) Per ogni porto che mi risponde posso trovarmi in uno dei seguenti casi:
            
            - Conferma positiva dal porto, ho consegnato (DOMANDA merce - quantità che volevo consegnare) quindi
              nel migliore dei casi ho azzerato la domanda per quel tipo di merce di quel determinato porto.

            - Conferma negativa dal porto, la domanda di quel tipo di merce che volevo consegnare è scesa a 0.

          Se ho trovato almeno un porto con conferma positiva faccio la travel() e mi dirigo da lui, il primo che
          mi risponde, vado al punto 3).

          Se tutti i porti mi hanno inviato una conferma negativa allora la domanda di quel tipo di merce
          in tutti i porti è pari a 0, torno al punto 1) scegliendo un tipo di merce diverso.
          
          Se tornando ripetutamente al punto 1) e arrivo ad esaurire tutte le merci perchè o sono scadute o 
          nessun porto ha DOMANDA relativa al mio carico, allora faccio chargeProducts()       

        
        2 - Porto) Il porto non fa niente

    */

        portID = portResponsesForDischarge();

        if(portID == -1){
            removeProduct(ship, product_index); /* vecchio prodotto da scaricare rimosso (tanto le domande dei porti sono tutte a 0) */
            
            dischargeProducts(ship);            /* chiamo la dischargeProducts cercando un nuovo prodotto da consegnare */
        
        } else {

            /* 3) Una volta arrivato al porto accedo alla prima banchina disponibile e rimuovo la merce che intendo
            consegnare dal carico della nave */
            
            travel(ship, portID);
            accessPortForDischarge(ship, portID, product_index);
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
    Product p1, p2;
    p1.product_type = 0;
    p1.expirationTime = 1;
    p1.weight = 3;
    p2.product_type = 1;
    p2.expirationTime = 1;
    p2.weight = 2;
    
    Ship ship;
    
    ship = initShip(atoi(argv[1]));
    int charge = 1;

    checkInConfig();
    printf("Nave con id:%d: config finita, aspetto ok partenza dal master...\n", ship->shipID);
    waitForStart();
    printf("Nave con id:%d partita\n", ship->shipID);

    /* while(1){
    res = addProduct(ship, p1);
    res = addProduct(ship, p2);
    printShip(ship);
    sleep(2);
        }
    */    
    sleep(1.5);
    chargeProducts(ship, 5);
    printShip(ship);
        

        while (1) {
            sleep(1);
        }

    exit(EXIT_FAILURE);

    /*
    while (1) { 
        if(charge == 1){
            chargeProducts(ship);
            charge = 0;
        } else {
            dischargeProducts(ship);
            charge = 1;
        }
    } 
    */
    exit(EXIT_FAILURE); /* non deve mai raggiungere questa parte di codice*/
}


