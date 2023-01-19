#ifndef SUPPORT_H
#define SUPPORT_H
#define NANOS_MULT 1000000000
#include "./vettoriInt.h"
#include "../src/porto.h"

/* distribuisce casualmente un quantità "quantity" in "parts" parti */
intList* distribute(int quantity, int parts);

/* serve per stampare a schermo i dati di una struttura senza essere interrotti da una stampa di un altro processo */
void reservePrint(void (*printer)(void* obj, int idx), void* obj, int idx);

/* handler del segnale USR1 che quitta */
void quitSignalHandler(int sig);

double generateCord(); /* genere una coordinata double */

intList* distributeV1(int quantity, int parts);

/* aspetta che il master metta a zero il semaforo con la key MASTKEY */
void waitForStart();

int random_int(int min, int max);

void copyArray(int a[], int* a1, int length);


int nanosecsleep(long nanosec);

/*
    decrementa il semaforo per cui il master fa la waitzero (vedi spiegazione in master.h=>waitConfigs()), servirà sia alle navi sia ai porti
*/
void checkInConfig();

void clearSigMask();


int getPortQueueRequest(int key);

int getPortQueueCharge(int id);

int getPortQueueDischarge(int id);

int getShipQueue(int id);

double mod(double z);


int choose(int n, int k);

double mediaTempoViaggioFraPorti();
double numeroDiCarichiOttimale();
int SO_(char* name);

#endif
