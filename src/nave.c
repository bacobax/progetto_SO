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


int main(int argc, char* argv[]) { /* mi aspetto che nell'argv avrò l'identificativo della nave (es: nave 0, nave 1, nave 2, ecc..)*/

    Ship ship;
    ship = initShip(atoi(argv[1])); /* inizializzo struttura dati della nave ed eventuali handler per segnali*/

    waitForStart();

    while (1) { 

        /* operazioni da definire*/
    }

    exit(EXIT_FAILURE); /* non deve mai raggiungere questa parte di codice*/
}


