#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/shm_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"
#include "./nave.h"

Ship* initShip(int sIndex) {
  
    // per prima cosa inizializziamo la nave in locale
    Ship* ship = (Ship*) malloc(sizeof(struct ship));
    ship->cords[0] = generateCord();
    ship->cords[1] = generateCord();
    ship->capacity = 0;
    /*
        Poichè è la initShip sia load_as_list che load_as_array 
        saranno puntatori a NULL
    */
    ship->load_as_list = initLoadShip();
    ship->load_as_array = generateArrayOfProducts(ship->load_as_list);

    // adesso colleghiamo la nave alla shm per avere un riferimento
    // alla zona di dump

    int shipShmId = useShm(SHIPSHMKEY, SO_NAVI * sizeof(struct products), errorHandler);
    
    Products* array_of_products = ((Products*) getShmAddress(shipShmId, 0, errorHandler)) + sIndex; //puntatore all'array of products dell'sIndex-esima nave

    /*
        Anche qui non ha senso fare copyArray perchè al momento dell'initShip
        load_as_array e array_of_products saranno NULL

    copyArray(load_as_array, array_of_products); // copiamo in shm load_as_array così che possiamo
                                                 // avere le informazioni della nave del suo carico
    */

    /*
        Assegno questi valori nella shm per il dump come esempio
        per indicare che la nave di numero sIndex è ancora vuota e
        non ha alcun tipo di merce sopra.

        Ovviamente questa cosa si può cambiare e trovare una convenzione
        per mostrare che la nave è vuota/appena stata creata
    */
    array_of_products->id = -1;
    array_of_products->weight = -1;
    array_of_products->expirationTime = -1;

    return ship;
}

int main(int argc, char* argv[]) {
    //TODO: devi aggiungere l'handler del segnale USR1 che il master manda per killare tutti i figli tranne se stesso
    //* vedi dentro support.c l'handler


}