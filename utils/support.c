
#include "../config1.h"
#include "../src/porto.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <math.h>


#include "./support.h"
#include "./errorHandler.h"
#include "./sem_utility.h"
#include "./msg_utility.h"
#include "./shm_utility.h"
#include "./vettoriInt.h"


void quitSignalHandler(int sig) {
    printf("Porto: ricevuto segnale di terminazione non faccio null\n");
    
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

    
    max_q = quantity / (parts-1);

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
    semid = useSem(RESPRINTKEY, errorHandler, "reservePrint");

    mutex(semid, LOCK, errorHandler, "reservePrint LOCK");

    printer(object, idx);

    mutex(semid, UNLOCK, errorHandler, "reservePrint UNLOCK");
}



void waitForStart() {
    int semid;
    semid = useSem(MASTKEY, errorHandler,"waitForStart");
    mutex(semid, WAITZERO, errorHandler, "waitForStart");
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

   
   /*
% 1000000000 perchè se per esempio miliseconds fosse = 1000000001 allora il numero di nanosecondi è = 1
   */ 


   req.tv_nsec = nanosec % NANOS_MULT;
   
   return nanosleep(&req , &rem);
}


void checkInConfig() {
    int waitConfigSemID = useSem(WAITCONFIGKEY, errorHandler, "checkInConfig");
    mutex(waitConfigSemID, LOCK, errorHandler, "checkInConfig");
}

void clearSigMask(){
    sigset_t ss;
    sigemptyset(&ss);
    sigprocmask(SIG_SETMASK, &ss, NULL);

}

double generateCord()
{
    double range, div;

    range = (SO_LATO); /* max-min */
    div = RAND_MAX / range;
    return (rand() / div);
}

Port getPortsArray(){
   int portShmid;
   Port portArr;
   portShmid = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler,"get ports array");
   portArr = getShmAddress(portShmid,0,errorHandler,"get ports array"); 
   return portArr;
}




int getPortQueueRequest(int key){
    int queueID;
    queueID = useQueue(key, errorHandler , "communicate ports");
    return queueID;
}

int getPortQueueCharge(int id){
    int queueID;
    queueID = useQueue(ftok("./src/porto.c" , id), errorHandler, "reply to ports");
    return queueID;
}

int getPortQueueDischarge(int id){
    int queueID;
    queueID = useQueue(ftok("./src/porto.h", id), errorHandler, "reply to ports");
    return queueID;
}

int getShipQueue(int id){
    int queueID;
    queueID = useQueue(ftok("./src/nave.c", id), errorHandler, "communicate ports");
    return queueID;
}
/*in secondi*/
double getTempoDiViaggio(double x, double y, double x1, double y1) {
    double spazio;

    spazio = sqrt(pow(x - x1, 2) + pow(y - y1, 2));

    return spazio / SO_SPEED;

}

double mod(double r) {
    return ((r < 0) ? r * (-1) : r);
}
int fact(int n) {
    if (n == 0) {
        return 1;
    }
    else {
        return fact(n - 1) * n;
    }
}
int choose(int n, int k) {
    return fact(n) / (fact(k) * fact(n - k));
}

double mediaTempoViaggioFraPorti() {
    Port portArr;
    long c;
    int i;
    int j;
    char text[64]
    double sum = 0;
    c = 0;
    portArr = getPortsArray();
    for (i = 0; i < SO_PORTI-1; i++) {
        for (j = i + 1; j < SO_PORTI; j++) {
                    sum += getTempoDiViaggio(portArr[i].x, portArr[i].y, portArr[j].x, portArr[j].y);
            c++;
        }
    }
   
    throwError("")
    shmDetach(portArr, errorHandler, "mediaDistanzaFraPorti");
    return sum / c;
}
