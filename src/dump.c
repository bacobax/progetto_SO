#include "../utils/shm_utility.h"
#include "../config1.h"
#include "./dump.h"

void createDumpArea(){
    int shmid;
    shmid = createShm(DUMPSHMKEY, SO_MERCI * sizeof(struct good), errorHandler);
    
}