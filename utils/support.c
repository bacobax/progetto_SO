
#include "../config1.h"
#include "../src/porto.h"
#include <stdio.h>
#include <signal.h>
#include <string.h>
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

intList* distributeV1(int quantity, int parts) {
    int media;
    int scarto;
    intList* l;
    int i;
    media = quantity / parts;
    scarto = media / 5; /*per avere un coefficente di variazione massimo del 20% (1/5 della media)*/

    l = intInit();
    for (i = 0; i < parts - 1; i++) {
        intPush(l, random_int(media - scarto, media + scarto));
    }
    /*la quantità restante viene messa nell'ultimo elemento*/
    intPush(l, quantity - sum(l));
    return l;


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
    int so_lato = SO_("LATO");
    range = so_lato; /* max-min */
    div = RAND_MAX / range;
    return (rand() / div);
}



/*
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
}*/
/*in secondi*/
double getTempoDiViaggio(double x, double y, double x1, double y1) {
    double spazio;
    int so_speed = SO_("SPEED");
    spazio = sqrt(pow(x - x1, 2) + pow(y - y1, 2));

    return spazio / so_speed;

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
    long c;
    int i;
    int j;
    double sum = 0;
    int so_porti = SO_("PORTI");
    
    Port p = getPort(0);
    c = 0;
    for (i = 0; i < so_porti-1; i++) {
        for (j = i + 1; j < so_porti; j++) {
            
            sum += getTempoDiViaggio(p[i].x, p[i].y, p[j].x, p[j].y);
            c++;
        }
    }
    detachPort(p, 0);
    return sum / c;
}

double numeroDiCarichiOttimale() {
    double probabilitaDiCambiarePorto;
    double tempoDiViaggioMedio;
    double tempoDiScaricoMedio;
    double tempoDiViaggioEffettivo;
    double res;
    int so_porti;
    int so_days;
    int so_merci;
    int so_loadspeed;
    int so_fill;
    so_porti = SO_("PORTI");
    so_days = SO_("DAYS");
    so_merci = SO_("MERCI");
    so_loadspeed = SO_("LOADSPEED");
    so_fill = SO_("FILL");
    tempoDiViaggioMedio = mediaTempoViaggioFraPorti();
    probabilitaDiCambiarePorto =(double)(so_porti - 1) / so_porti;
    tempoDiViaggioEffettivo = (double)probabilitaDiCambiarePorto * tempoDiViaggioMedio;
    tempoDiScaricoMedio = ((double)so_fill / so_loadspeed) / (so_porti * so_days * so_merci);
    /*
        k = tempoDiViaggioMedio + tempoDiScaricoMedio ~= tempo di una charge/dscharge
        n*k< SO_DAYS-1 - n*k <==> 2nk < SO_DAYS - 1 <==> n < (SO_DAYS - 1)/k*2 <==> n < (SO_DAYS - 1)/((tempoDiViaggioMedio + tempoDiScaricoMedio)*2) 
    
    */
    res = (double)(so_days - 1) / ((tempoDiViaggioEffettivo + tempoDiScaricoMedio) * 2);
 

    return res;
}

int SO_(char* name) {
    FILE* fp;
    char buff[1024];
    char* token;
    fp = fopen("./configs", "r");
    if (fp == NULL) {
        throwError("fopen", "SO_");
        fclose(fp);
        exit(1);
    }

    while (fgets(buff, 1024, fp) != NULL) {
        
        
        token = strtok(buff, "=");
        if (strcmp(token, name) == 0) {
            token = strtok(NULL, "=");
            fclose(fp);
            return atoi(token);
        }
    }
    if (fclose(fp) == -1) {
        fclose(fp);
        throwError("fclose", "SO_");
        exit(1);
    }
    throwError("No variables with this name", "SO_");
    fclose(fp);
    exit(1);
    return -1;
}
