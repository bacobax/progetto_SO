#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"
#include "../utils/msg_utility.h"
#include "./dump.h"
#include "./nave.h"
#include "./porto.h"

void discharge(){
 return;
}

void chargeProducts(int quantityToCharge){

    int availablePorts;
    int portID;
    struct port_offer port_offers[SO_PORTI];
    initArray(port_offers);

    if(quantityToCharge == 0){

        discharge();

    } else {

        callPorts(quantityToCharge); /* funzione che manda un msg a tutti i porti per iniziare una fase di carico PUNTO 2 DELL'ALGORITMO*/

        availablePorts = portResponses(port_offers); /* PUNTO 3 DELL'ALGORITMO*/

        if(availablePorts == 0){

            charge(quantityToCharge - 100); /* 100 è un valore d'esempio, bisogna decidere poi effettivamente quanto decrementare*/

        } else {

            /* Ci sono porti che hanno merce da caricare*/

            portID = choosePort(port_offers); /* PUNTO 4 DELL'ALGORITMO*/

            replyToPorts(portID); /* PUNTO 5 DELL'ALGORITMO*/

            /*PUNTO 6 DELL'ALGORITMO*/
            travel(portID); 
            accessPort(portID, port_offers[portID]);
        }
    }

}



int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/

    int quantityToCharge;
    int charge = 1;

    ship = initShip(atoi(argv[1])); /* inizializzo struttura dati della nave ed eventuali handler per segnali*/

    waitForStart();

    while (1) { 

        if(charge == 1){

            quantityToCharge = chooseQuantityToCharge(); /* funzione che ritorna la quantità che la nave vuole caricare PUNTO 1 DELL'ALGORITMO*/
            chargeProducts(quantityToCharge);
            charge = 0;

        } else {
            discharge();
            charge = 1;
        }       
    }

    exit(EXIT_FAILURE); /* non deve mai raggiungere questa parte di codice*/
}


