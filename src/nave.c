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
    int idx;
    int res;
    Product p1, p2;
    Ship ship;
    p1.product_type = 0;
    p1.expirationTime = 3;
    p1.weight = 3;

    p2.product_type = 1;
    p2.expirationTime = 2;
    p2.weight = 2;

    /*
    res = useShm(SSHMKEY, SO_NAVI * sizeof(struct ship), NULL);
    printf("Nave shmid:%d\n", res);
    */

    printf("Nave: mio indice: %s\n" , argv[1]);
    idx = atoi(argv[1]);

    printf("Nave: sto per fare la initShip\n");
    
    ship = initShip(idx);
    printf("Nave: finito initschip\n");
    int charge = 1;

    checkInConfig();
    printf("Nave: config finita nave\n");
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


