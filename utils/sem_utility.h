#ifndef SEM_UTILITY_H

#define SEM_UTILITY_H

#define LOCK -1
#define UNLOCK 1
#define WAITZERO 0

/* fa la semget, ma gestisce l'errore di default (se errorHandler = NULL)
// altrimenti si può passare una funzione per gestire l'errore */
int useSem(int key, void (*errorHandler)(int, char*) , char* errCtx);

/* crea ed inizializza un solo semaforo */
int createSem(int key, int initValue, void (*errorHandler)(int, char*) , char* errCtx);

/* rimuove la struttura ipc, quindi rimuove il set di semafori */
void removeSem(int semid, void (*errorHandler)(int, char*) , char* errCtx);

/* esegue operazioni sul singolo semaforo con flag a 0
 LOCK decrementa
 UNLOCK incrementa
 WAITZERO aspetta lo 0 */
void mutex(int semid, int op, void (*errorHandler)(int, char*) , char* errCtx);

int createMultipleSem(int key, int nSem, int initValue, void (*errorHandler)(int, char*) , char* errCtx);

void mutexPro(int semid, int semIdx, int op, void (*errorHandler)(int, char*) , char* errCtx);

int getWaitingPxCount(int semid, int idx);

void getAllVAlues(int semid, int length);

int getOneValue(int semid, int idx);

#endif
