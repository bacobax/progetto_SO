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
#include "./nave.h"


void shipSignalHandler(int signal){
    if(signal == SIGUSR1){
        printf("Nave: ricevuto segnale di terminazione\n");
        exit(EXIT_SUCCESS);
    }  
}

Ship initShip() {

    if(signal(SIGUSR1, shipSignalHandler) == SIG_ERR){              /* imposto l'handler per la signal SIGUSR1 */
        perror("Error trying to set a signal handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }
  
    /* inizializziamo la nave */
    Ship ship = (struct ship*) malloc(sizeof(struct ship));
    ship->cords[0] = generateCord();
    ship->cords[1] = generateCord();
    ship->capacity = 0;
    /*
        load sarà NULL all'inizio
    */
    ship->load = initLoadShip();
    
    return ship;
}

int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/
    
    Ship ship = initShip(); /* inizializzo struttura dati della nave ed eventuali handler per segnali*/

    while(1){ /* ciclo di operazione che esegue la nave fino a quando non viene killata dal master*/

        int port_id = choosePort(/* PARAMETRI IN VIA DI DEFINIZIONE*/); /* la nave sceglie il porto dove andare a scambiare*/

        /*
            la funzione choosePort sarà da implementare costruendo un algoritmo che scelga un porto per
            andare a caricare e/o scaricare merci
        */

       travel(); // TO-DO  

       /*
            la funzione travel() utilizza la nanosleep() per spostare la nave alle coordinate del porto
            una volta terminato l'algoritmo choosePort
       */

       accessPort(); // TO-DO

       /*
            la funzione accessPort() tenta l'accesso alla banchina del porto per far si che la
            nave possa compiere le sue operazioni di carico/scarico
       */

    }

    exit(EXIT_FAILURE); /* non deve mai raggiungere questa parte di codice*/
}