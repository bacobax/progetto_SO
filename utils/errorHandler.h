#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
/* ad ogni nome contenuto nell'enum corrisponde un messaggio di errore*/
typedef enum errors_ {SERRGET,SERRCTL,SERROP,MERRGET,MERRCTL,MERRSND,MERRRCV,SHMERRAT,SHMERRCTL,SHMERRGET,SHMERRDT} IPCerror;

/* funzione che prendendo un parametro di nome err (corrispondente ad uno dei nomi contenuti nell'enum) lancia un errore tramite throwError*/
void errorHandler(int err, char* errCtx);

void removeErrorHandler();
void initErrorHandler();

/* funzione che segnala un errore stampando su file più dettagli riguardanti il problema avvenuto*/
void throwError(char *myerr, char *errCtx);

#endif
