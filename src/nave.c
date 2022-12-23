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


void chargeProducts(Ship ship){
    /* TO-DO */
}

void dischargeProducts(Ship ship){
    /* TO-DO */
}

int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/

    int res;
    Product p1, p2;
    Ship ship;
    p1.product_type = 0;
    p1.expirationTime = 3;
    p1.weight = 300;

    p2.product_type = 1;
    p2.expirationTime = 2;
    p2.weight = 150;

    ship = initShip(atoi(argv[1]));  
    int charge = 1;

    waitForStart();

    res = addProduct(ship, p1);

    printShip(ship);

    res = addProduct(ship, p2);

    printShip(ship);

    sleep(5);

    exit(EXIT_SUCCESS);

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


