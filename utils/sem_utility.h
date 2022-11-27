#ifndef SEM_UTILITY_H

#define SEM_UTILITY_H

#define ERRGET 0
#define ERRCTL 1
#define ERROP  2

#define LOCK -1
#define UNLOCK 1
#define WAITZERO 0

// fa la semget, ma gestisce l'errore di default (se errorHandler = NULL)
// altrimenti si può passare una funzione per gestire l'errore 
int useSem(int key, void (*errorHandler)(int));

// crea ed inizializza un solo semaforo
int createSem(int key, int initValue, void (*errorHandler)(int err));

//esegue operazioni sul singolo semaforo con flag a 0 
// LOCK decrementa
// UNLOCK incrementa
// WAITZERO aspetta lo 0
void mutex(int semid, int op, void (*errorHandler)(int err));
#endif