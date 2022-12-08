#ifndef SUPPORT_H
#define SUPPORT_H
#include "./vettoriInt.h"

//distribuisce casualmente un quantità "quantity" in "parts" parti
intList* distribute(int quantity, int parts);

//serve per stampare a schermo i dati di una struttura senza essere interrotti da una stampa di un altro processo
void reservePrint(void (*printer)(void* obj, int idx), void* obj, int idx);

#endif