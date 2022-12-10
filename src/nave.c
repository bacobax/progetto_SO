#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"
#include "./nave.h"


Ship* initShip(int sIndex) {
  
    int shipShmId = useShm(SHIPSHMKEY, SO_NAVI * sizeof(struct ship), errorHandler);

    Ship* ship = ((Ship*) getShmAddress(shipShmId, 0, errorHandler)) + sIndex;
    
    ship->cords[0] = generateCord(); //coordinata x
    ship->cords[1] = generateCord(); //coordinata y
    
    ship->capacity = 0;
    
    ship->load_as_list= initLoadShip();
    ship->load_as_array = generateArrayOfProducts(ship->load);

    return ship;
    
}

int main(int argc, char* argv[]) {


}