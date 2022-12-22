
#include "../config1.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <unistd.h>

#include "./support.h"
#include "./sem_utility.h"
#include "./vettoriInt.h"


void ErrorHandler(int err) {
    perror("reservePrint->useSem");
}

void quitSignalHandler(int sig) {
    printf("Porto: ricevuto segnale di terminazione\n");
    exit(EXIT_SUCCESS);
}

int random_int(int min, int max) {
    return min + rand() % (max+1 - min);
}

intList* distribute(int quantity, int parts) {



    /*per ciascuna parte, tranne l'ultima vale:

    //massimo: la quantità che gli verrebbe assegnata se le quantità fossero distribuite in parti uguali
    //questo perchè nel peggiore dei casi (in cui a tutte le quantita venga assegnato il massimo) la quantità totale <= quantity */
    int max_q;
    int min_q;
    intList* l;
    int random_q;
    int last_q;
    int i;


    max_q = quantity / parts;

    /* minimo: la metà della quantità che gli verrebbe distribuita se le quantità fossero distribuite in parti uguali */
    min_q = quantity / parts / 2;
    l = intInit();
    for (i = 0; i < parts - 1; i++) {
        random_q = random_int(min_q, max_q);
        intPush(l, random_q);
    }

    /* per l'ultima quantità viene assegnata la quantità restate non ancora assegnata
    questo per essere sicuro che la somma delle quantità sia = quantity */
    last_q = quantity - sum(l);
    intPush(l, last_q);
    return l;
}


void reservePrint(void (*printer)(void* obj, int idx), void* object, int idx) {
    int semid;
    semid = useSem(RESPRINTKEY, ErrorHandler);

    mutex(semid, LOCK, NULL);

    printer(object, idx);

    mutex(semid, UNLOCK, NULL);
}



void waitForStart() {
    int semid;
    semid = useSem(MASTKEY, NULL);
    mutex(semid, WAITZERO, NULL);
}

/*copia il contenuto di un array in un altro array
  assumendo ovviamente che a.length >= a1.length */
void copyArray(int a[], int* a1, int length) {
    int i;
    for (i = 0; i < length; i++) {
        a[i] = a1[i];
    }
}

int nanosecsleep(long nanosec)
{
   struct timespec rem;
   struct timespec req;
   req.tv_sec = (long)(nanosec / NANOS_MULT);

   
    /* %1000000000 perchè se per esempio miliseconds fosse = 1000000001 allora il numero di nanosecondi è = 1 */
   

   req.tv_nsec = nanosec % NANOS_MULT;
   

   return nanosleep(&req , &rem);
}

void checkInConfig() {
    int waitConfigSemID = useSem(WAITCONFIGKEY, errorHandler);
    mutex(waitConfigSemID, LOCK, errorHandler);
}
