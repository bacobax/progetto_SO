#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"



int main(int argc, char const* argv[]) {

    int semid = useSem(MASTKEY, NULL);

    mutex(semid, WAITZERO, NULL);

    printf("Ciao, sono il porto con quantità %d\n", atoi(argv[1]));
    return 0;

}


