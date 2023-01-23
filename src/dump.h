#ifndef DUMP_H
#define DUMP_H
#define DUMPSEMKEY 392
#define LOGFILESEMKEY 393
#define TXFILESEMKEY 394
#include "../utils/msg_utility.h"
/*
      Giornaliero: 
        - Merce affondata 
*/


typedef struct goodTypeInfo {                       /* struttura che contiene le 4 informazioni relative per ogni tipo di merce nella simulazione*/
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

/* Crea la shm per contenere le informazioni del dump */
void createDumpArea();


void addExpiredGood(int quantity, int type, ctx where);
void addNotExpiredGood(int quantity, int type, ctx where, int refilling, int idx);
void addDeliveredGood(int quantity, int type, int portIdx);
/*
    TO-DO

    implementare utili funzioni per interfacciarsi con la shm per il dump

    alcune funzioni utili:

    - printare lo stato della shm del dump
    - aggiornare la struttura dati contenuta in una posizione 'index' (preso come parametro) con
      un campo specifico di essa (esempio: voglio aggiornare il campo goods_on_ship della merce 1)
    - altri utilità se ci vengono in mente  
*/

void printDump(int mod, int day, int last);
void removeDumpArea();

void lockAllGoodsDump();
void unlockAllGoodsDump();
void printTransaction(int idxNave, int idxPorto, int carico, int ton, int tipoMerce);
void logShip(int shipID, char* msg);
#endif

