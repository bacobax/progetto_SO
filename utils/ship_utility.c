#include "../src/nave.h"
#include "../utils/loadShip.h"
#include "../config1.h"


int checkCapacity(Ship* ship){
    if(ship->load_as_list->weightLoad == 0) return 0;
    return ship->load_as_list->weightLoad;
}

int availableCapacity(Ship* ship){
    int currentCapacity = checkCapacity(ship);
    if(currentCapacity < 0) return -1;
    return (SO_CAPACITY - currentCapacity);
}

double generateCord(){
    double range = (SO_LATO - 0); // max-min 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

Products* generateArrayOfProducts(loadShip* load_as_list){

    if(load_as_list->first == NULL){ // se la nave è ancora vuota ritorno un array
                                     // vuoto
        return NULL;
    } else {
            Product* l_aux = load_as_list->first;
            int listSize = load_as_list->length;

            Products* p = (Products*) malloc(listSize * sizeof(struct products));
            Products* p_aux = p;

            int offset = 0;

            while(l_aux != NULL){

                p->id = l_aux->id;
                p->weight = l_aux->weight;
                p->expirationTime = l_aux->expirationTime;

                offset += 1;
                p = p + offset; 

                l_aux = l_aux->next;
                return p_aux;
            }
    }
    
}

void copyArray(Products* load_as_array, Products* shared_load){
    
}


/*
 2 shm: 1 per il dump contenente il numero di navi e ogni posizione nell'array dump contiene
        le informazioni relative al dump di quella nave

        2 seconda shm per accedere alle merci dei porti BISOGNA SAPERE L'INDICI DEL PORTO CON CUI
        SCAMBIARE LE MERCI

*/
