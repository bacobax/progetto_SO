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


void accessPort(int portID) {
    int portSHMID;
    Port port;
    int banchineSem;
    portSHMID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler);

    port = (Port)getShmAddress(portSHMID, 0, errorHandler) + portID;

    banchineSem = useSem(BANCHINESEMKY, errorHandler);

    mutexPro(banchineSem, portID, LOCK, errorHandler);

    /*
    !SEZIONE CRITICA
    */
    
    mutexPro(banchineSem, portID, UNLOCK, errorHandler);

}

void charge(Ship ship){

    int quantityToCharge = chooseQuantityToCharge(); /* funzione che ritorna la quantità che la nave vuole caricare */

    

}

void discharge(){

}

int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/

    Ship ship;
    int ship_index, portID;
    int charge = 1;

    ship = initShip(); /* inizializzo struttura dati della nave ed eventuali handler per segnali*/
    ship_index = atoi(argv[1]);

    waitForStart();

    while (1) { 

        if(charge == 1){
            charge(ship);
            charge = 0;
        } else {
            discharge();
            charge = 1;
        }       
    }

    exit(EXIT_FAILURE); /* non deve mai raggiungere questa parte di codice*/
}


