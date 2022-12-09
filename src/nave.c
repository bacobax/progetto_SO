#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"
#include "./nave.h"
Ship* initShip() {
    /*
        La initShip falla come ho fatto la initPorto:

        Port initPort(int sIndex , ...) {

            int shipShmId = useShm(SSHMKEY, SO_NAVI * sizeof(struct ship), errorHandler);

            Ship* p = ((Ship*)getShmAddress(portShmId, 0, errorHandler)) + sIndex; //* per prendere la sIndex-esima nave dell'array in shm

    */

    return NULL;
}
int main(int argc, char* argv[]) {


}