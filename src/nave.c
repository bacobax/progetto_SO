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
        callPorts(ship, quantityToCharge); /* mando msg a tutti i porti perchè voglio caricare*/
    
        availablePorts = portResponses(ship, port_offers);

        printf("NAVE: Aviable ports = %d\n", availablePorts);
        if (availablePorts == 0) {
            /* non ci sono porti disponibili per la quantità
               di merce che voglio caricare, riprovo a chiamare i porti decrementando la quantità*/
            chargeProducts(ship, quantityToCharge - 1); 
        
        } else {
            /* ci sono porti che hanno merce da caricare*/
            
            portID = choosePort(port_offers);

            replyToPorts(ship, portID);

            
            travel(ship, portID);
            
            accessPortForCharge(ship, portID, port_offers[portID], quantityToCharge);

            
            
        }
    }
    
}

void dischargeProducts(Ship ship){
    /* TO-DO */

    /* 1) msg di richiesta a tutti i porti per visualizzare la lista di domande */

    /* 2) guardo tra tutte le merci richieste quale posso consegnare (criterio della data di scadenza)
           - se tutte le domande dei porti sono 0, allora chargeProducts() 

           - se nessuna delle domande dei porti corrisponde a della merce che ho sulla nave, allora chargeProducts()

    */       

    /* 3) scelta la merce da consegnare e il porto in cui andare mando un msg di conferma.
          il porto provvederà a decrementare la merce scelta e a controllare che la sua domanda
          non sia scesa a 0, infine manda un msg alla nave per farla partire.
          Se il msg è negativo torno al punto 1).

        Scelta del tipo di merce:
            VETTORE DI VALORI: V[SO_MERCI], V[i] = 1/(media([domanda del tipo i - capienza tipo i]) * scadenza della merce i)
            Es:
                Merce tipo 2: [domanda del porto 0 della merce 2 - mia capienza merce 2, domanda del porto 1 della merce 2 - mia capienza merce 2, ...]
     */

     /* 4) la nave esegue la travel(), cerca di accedere alla banchina e dopodichè rimuove la merce
            dal suo carico.
     
     
     */

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
    sleep(3);
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


