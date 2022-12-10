#ifndef NAVE_H
#define NAVE_H


#include "../utils/loadShip.h"

#define SHIPSHMKEY 8080

typedef struct ship {
    double cords[2];
    int capacity;
    loadShip* load_as_list;
    Products* load_as_array; // questo qui ci servirà nella shm per il dump della nave
} Ship;


//a queste funzioni ti consiglio di passare una struttura Ship* come argomento

int checkCapacity(Ship* ship); // ritorna il numero di ton presenti sulla nave

int availableCapacity(Ship* ship); // ritorna il numero di ton disponibili sulla nave 

double generateCord(); // genere una coordinata double

Products* generateArrayOfProducts(loadShip* list); // genera un array di products tramite una lista loadShip

void copyArray(); // copia il contenuto di load_as_array in array_of_products IN DUBBIO!

#endif