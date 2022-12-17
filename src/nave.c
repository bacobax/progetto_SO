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

int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/

    Ship ship;
    int ship_index, portID;
    
    ship = initShip(); /* inizializzo struttura dati della nave ed eventuali handler per segnali*/
    ship_index = atoi(argv[1]);

    waitForStart();

    while (1) { /* ciclo di operazione che esegue la nave fino a quando non viene killata dal master*/
        
        portID = choosePort(/* PARAMETRI IN VIA DI DEFINIZIONE*/); /* la nave sceglie il porto dove andare a scambiare*/

        /*
            la funzione choosePort sarà da implementare costruendo un algoritmo che scelga un porto per
            andare a caricare e/o scaricare merci
        */

        travel(ship, portID); /*  TO-DO */ 

        /*
             la funzione travel() utilizza la nanosleep() per spostare la nave alle coordinate del porto
             una volta terminato l'algoritmo choosePort
        */

        accessPort(portID); /* TO-DO */

        /*
             la funzione accessPort() tenta l'accesso alla banchina del porto per far si che la
             nave possa compiere le sue operazioni di carico/scarico
        */

    }

    exit(EXIT_FAILURE); /* non deve mai raggiungere questa parte di codice*/
}


