#include "../src/nave.h"
#include "../utils/loadShip.h"
#include "../config1.h"

Ship* initShip(){
    Ship* ship = (Ship*) malloc(sizeof(Ship));
    ship->cords[0] = generateCord(); //coordinata x
    ship->cords[1] = generateCord(); //coordinata y
    ship->capacity = 0;
    ship->load = initLoadShip();
}

int checkCapacity(loadShip* load){
    if(load->weightLoad == 0) return 0;
    return load->weightLoad;
}

int availableCapacity(loadShip* load){
    int currentCapacity = checkCapacity(loadShip* load)
    if(currentCapacity < 0) return -1;
    return (SO_CAPACITY - currentCapacity);
}

double generateCord(){
    // TO-DO implementare metodo che genere le coordinate
}