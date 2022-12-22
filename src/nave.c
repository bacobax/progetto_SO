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

void test0(int shipID){
    Ship ship;
    Product p1, p2;
    int res;

    createShmShips();
    
    ship = initShip(shipID); /* inizializzo struttura dati della nave ed eventuali handler per segnali*/
    
    printShip(ship);

    p1.product_type = 1;
    p1.expirationTime = 1;
    p1.weight = 1;

    p2.product_type = 2;
    p2.expirationTime = 2;
    p2.weight = 2;

    res = addProduct(ship, p1);
    res = addProduct(ship, p2);

    printShip(ship);

    res = findProduct(ship->products, p2);
    res = removeProduct(ship, res);

    printShip(ship);

    removeShmShips();

    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/

    /*test0(argv[1]);*/    

    /*waitForStart();

    while (1) { 

         operazioni da definire
    } */

    exit(EXIT_FAILURE); /* non deve mai raggiungere questa parte di codice*/
}


