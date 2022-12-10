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

/*
    APPENA IL PROFESSORE CHIARISCE IL MIO DUBBIO SUL DUMP DELLA NAVE
    INIZIO A COSTRUIRE LA STRUTTURA SHM
*/

void shipSignalHandler(int signal){
    if(signal == SIGUSR1){
        printf("Nave: ricevuto segnale di terminazione\n");
        exit(EXIT_SUCCESS);
    }  
}

Ship* initShip() {
  
    // inizializziamo la nave
    Ship* ship = (Ship*) malloc(sizeof(struct ship));
    ship->cords[0] = generateCord();
    ship->cords[1] = generateCord();
    ship->capacity = 0;
    /*
        load sarà NULL all'inizio
    */
    ship->load = initLoadShip();
    
    return ship;
}

int main(int argc, char* argv[]) {
    

}