#include "../utils/shm_utility.h"
#include "../config1.h"
#include "./dump.h"

createDumpArea(){
    int shmid = createShm(DUMPSHMKEY, SO_MERCI * sizeof(struct good), errorHandler);
}