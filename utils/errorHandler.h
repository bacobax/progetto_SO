#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
/*funzione che gestisce gli errori*/
typedef enum errors_ {SERRGET,SERRCTL,SERROP,MERRGET,MERRCTL,MERRSND,MERRRCV,SHMERRAT,SHMERRCTL,SHMERRGET,SHMERRDT} IPCerror;
void errorHandler(int err, char* errCtx);
void removeErrorHandler();
void initErrorHandler();

#endif
