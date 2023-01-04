#ifndef SUPPLIES_H
#define SUPPLIES_H
#include "../config1.h"
#include "./vettoriInt.h"
/*
    libreria creata con lo scopo di effetture alcune operazioni sulle offerte di un porto cercandola di trattare un po' come una 'scatola nera'
*/



typedef struct supplies {
    int magazine[SO_DAYS][SO_MERCI];
    int expirationTimes[SO_DAYS*SO_MERCI];
}Supplies;

/*
    riempie di date di scadenza casuali il vettore di date di scadenza
*/
void fillExpirationTime(Supplies* S);

/*
    riempie di risorse il magazzino alla riga della matrice corrispondente all'indice day
*/
void fillMagazine(Supplies* S, int day, int* supplies);

/*
    dato il tipo della merce (indice colonna) e il giorno in cui è stata distribuita (indice riga) viene restituita la propria data di scadenza
*/
int getExpirationTime(Supplies S, int tipoMerce, int giornoDistribuzione);

/*
    stampa del magazzino delle scadenze
*/
void printSupplies(Supplies S);

/*
    decrementa i tepi di vita delle merci distribuite in un giorno < di {{day}}
*/
void decrementExpTimes(Supplies* S, int day);

/*
    rimuove le risorse scadute
*/
void removeExpiredGoods(Supplies* S);

/*
    algoritmo che trova la migliore coppia di coordinate della matrice (tipo merce, giorno di distribuzione della merce) che corrisponde
    alla migliore quantità disponibile da offrire di fronte alla richiesta di merce pari a {{quantity}}
*/
int trovaTipoEScadenza(Supplies* S, int* tipo, int* dayTrovato, int* scadenza, int quantity);


#endif
