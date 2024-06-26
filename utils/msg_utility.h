

#ifndef MSG_UTILITY

#define MSG_UTILITY

#define MEXBSIZE 1024

typedef struct _mex {
    long mtype;
    char mtext[MEXBSIZE];
} mex;

#define SYNC 0
#define ASYNC 1




#define EX_KEY 132

/* semplificazione della msgsnd con una gestione degli errori di default
   se si setta l'handler a NULL allora è usato quello di default */
void msgSend(int msgqID, char text[MEXBSIZE], long type, void (*errorHandler)(int err, char* errCtx),int ipcNoWait ,char* errCtx);


/* semplificazione della msgrcv con una gestione degli errori di default
se si setta l'handler a NULL allora è usato quello di default
il callback da passare è quello che si vuole che venga eseguito una volta ricevuto il messaggio
questa funzione crea un processo figlio che si occupa di eseguire il callback
mod può essere = SYNC oppure = ASYNC
se è = SYNC, l'handler non è eseguito e ritorna il puntatore al messaggio (dev'essere liberato)
se è = ASYNC, questa funzione crea un processo che getisce indipendentemente la ricezione del messaggio
se è != ASYNC e != SYNC, da errore */
mex* msgRecv(int msgqID, long type, void (*errorHandler)(int err, char* errCtx), void (*callback)(long type, char text[MEXBSIZE]), int mod, char* errCtx);

mex* msgRecvPro(int msgqID, long type, void (*errorHandler)(int err, char* errCtx), void (*callback)(long type, char text[MEXBSIZE], int arg), int mod, int arg, char* errCtx);


/* in caso di successo restituisce l'id della queue
restituisce EEXIST se esiste già
se si presentano altri errori viene eseguito l'handler
se si setta l'handler a NULL allora è usato quello di default */
int createQueue(int key, void (*errorHandler)(int err, char* errCtx), char* errCtx);


/* ritorna l'id di una queue esistente */
int useQueue(int key, void (*errorHandler)(int err, char* errCtx), char* errCtx);

/* ritorna il numero di messaggi */
int getMexCount(int id, void (*errorHandler)(int err, char* errCtx), char* errCtx);

void printQueueState(int id, void (*errorHandler)(int err, char* errCtx), char* errCtx);

void removeQueue(int id, void (*errorHandler)(int err, char* errCtx), char* errCtx);

#endif
