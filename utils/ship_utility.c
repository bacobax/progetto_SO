#include <stdio.h>
#include <stdlib.h>
#include "../src/nave.h"
#include "../utils/loadShip.h"
#include "../config1.h"
#include <stdio.h>
#include <stdlib.h>

<<<<<<< HEAD
int checkCapacity(Ship* ship) {
    if (ship->load->weightLoad == 0) return 0;
    return ship->load->weightLoad;
}

int availableCapacity(Ship* ship) {
=======

int checkCapacity(Ship ship){
    if(ship->load->weightLoad == 0) return 0;
    return ship->load->weightLoad;
}

int availableCapacity(Ship ship){
>>>>>>> cc8ba5a8e07904a87c55ea99444c9ecb01531bcf
    int currentCapacity = checkCapacity(ship);
    if (currentCapacity < 0) return -1;
    return (SO_CAPACITY - currentCapacity);
}

<<<<<<< HEAD
double generateCord() { //! dai un occhiata a questa funzione, stai usando roba non importata e variabili non dichiarate
    double range = (SO_LATO - 0); // max-min 
    double div = RAND_MAX / range;
    return min + (rand() / div); //? min non vale 0?
=======
double generateCord(){
    double range = (SO_LATO - 0); /* max-min */ 
    double div = RAND_MAX / range;
    return 0 + (rand() / div);
>>>>>>> cc8ba5a8e07904a87c55ea99444c9ecb01531bcf
}

