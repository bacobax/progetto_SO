#ifndef DUMP_H
#define DUMP_H

struct good{                       /* struttura che contiene le 4 informazioni relative per ogni tipo di merce nella simulazione*/
    int goods_on_ship;             /* numero di beni TOTALI di quel tipo presenti sulla nave PRONTI PER ESSERE CONSEGNATI */
    int goods_on_port;             /* numero di beni TOTALI di quel tipo presenti nel porto PRONTI PER ESSERE CARICATI*/
    int delivered_goods;           /* numero di beni TOTALI di quel tipo consegnati nei porti */
    int expired_goods_on_ship;     /* numero di beni TOTALI di quel tipo scaduti nelle navi*/
    int expired_goods_on_port;     /* numero di beni TOTALI di quel tipo scaduti nei porti*/
};

/* Crea la shm per contenere le informazioni del dump */
void createDumpArea();

/*
    TO-DO

    implementare utili funzioni per interfacciarsi con la shm per il dump

    alcune funzioni utili:

    - printare lo stato della shm del dump
    - aggiornare la struttura dati contenuta in una posizione 'index' (preso come parametro) con
      un campo specifico di essa (esempio: voglio aggiornare il campo goods_on_ship della merce 1)
    - altri utilità se ci vengono in mente  
*/


#endif