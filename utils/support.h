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

int random_int(int min, int max); /* ritorna un intero generato casualmente*/

/*
  copia il contenuto di un array in un altro array
  assumendo ovviamente che a.length >= a1.length
*/
void copyArray(int a[], int* a1, int length);

/*
  funzione che effettua una nanosleep in base 
  ad un numero di nanosecondi passati per parametro
*/
int nanosecsleep(long nanosec);

/*
    decrementa il semaforo per cui il master fa la waitzero (vedi spiegazione in master.h=>waitConfigs()), servirà sia alle navi sia ai porti
*/
void checkInConfig();

/*

*/
void clearSigMask();


/* ritorna il valorea assoluto di un numero double*/
double mod(double z);



double mediaTempoViaggioFraPorti();

double numeroDiCarichiOttimale();

/* ritorna il valore intero letto dal file configs, corrispondente
   alla stringa SO_name dove name può essere FILL, DAYS, NAVI ecc...
*/
int SO_(char* name);

#endif
