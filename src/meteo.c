#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../config1.h"
#include "../utils/errorHandler.h"
#include "../utils/support.h"
#include "../utils/shm_utility.h"
#include "../utils/sem_utility.h"

int main(int argc, char* argv[]) {
    int day;
    int* terminateValue;
    int endShmID;
    int aspettoMortePortiSemID;
    char str[128];
    
    endShmID = useShm(ENDPROGRAMSHM, sizeof(unsigned int), errorHandler, "getEndDayShmID meteo");
    terminateValue = (int*) getShmAddress(endShmID, 0, errorHandler, "meteo terminateValue");

    
    aspettoMortePortiSemID = useSem(WAITPORTSSEM, errorHandler, "aspettoMortePortiSemID in codicePorto");
    

    checkInConfig();
    printf("Meteo chekInConfig finita...\n");
    waitForStart();
    printf("Meteo partito...\n");
    
    while (fgets(str, 128, stdin) != NULL) {
        day = atoi(str);
        printf("Giorno %d\n", day);

        

    }
    mutex(aspettoMortePortiSemID, LOCK, errorHandler, "LOCK su aspettoMortePortiSemID");
    printf("Meteo: termino\n");
    exit(EXIT_SUCCESS);
        

}