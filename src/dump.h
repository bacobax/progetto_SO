#ifndef DUMP_H
#define DUMP_H
#define DUMPSEMKEY 392
#define LOGFILESEMKEY 393
#define TXFILESEMKEY 394
#include "../utils/msg_utility.h"

typedef struct goodTypeInfo {    /* struttura che contiene le 4 informazioni relative per ogni tipo di merce nella simulazione*/
  int goods_on_ship;             /* numero di beni TOTALI di quel tipo presenti sulla nave PRONTI PER ESSERE CONSEGNATI */
  int goods_on_port;             /* numero di beni TOTALI di quel tipo presenti nel porto PRONTI PER ESSERE CARICATI*/
  int delivered_goods;           /* numero di beni TOTALI di quel tipo consegnati nei porti */
  int expired_goods_on_ship;     /* numero di beni TOTALI di quel tipo scaduti nelle navi*/
  int expired_goods_on_port;     /* numero di beni TOTALI di quel tipo scaduti nei porti*/
} GoodTypeInfo;

typedef struct dumpArea {
  
  int typesInfoID;
  double expTimeVariance;
  double tempoScaricamentoTot;
} DumpArea;

typedef enum Context { PORT, SHIP } ctx;

/* 
  crea la shm per contenere le informazioni del dump 
*/
void createDumpArea();
/*
  le tre funzioni servono per aggiornare i campi del dump relativi alle quantità di merce scaduta, di merce non scaduta (ancora in vita),
  e di quella consegnata
*/
void addExpiredGood(int quantity, int type, ctx where);
void addNotExpiredGood(int quantity, int type, ctx where, int refilling, int idx);
void addDeliveredGood(int quantity, int type, int portIdx);
/*
  funzione per stampare il dump su file
*/
void printDump(int mod, int day, int last);
/*
  serve per eliminare la shared memory del dump
*/
void removeDumpArea();
/*
  le due funzioni servono per accedere in mutua esclusione
  ai campi della shared memory del dump
*/
void lockAllGoodsDump();
void unlockAllGoodsDump();
/*
 stampa un'azione di carico/scarico della nave su file, indicandone: la quantità,
 il tipo di merce e il porto interessato
*/
void printTransaction(int idxNave, int idxPorto, int carico, int ton, int tipoMerce, int fail);
/*
 stampa le azioni, della nave, descritte nella stringa msg su file
*/
void logShip(int shipID, char* msg);
#endif

