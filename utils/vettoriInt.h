#ifndef VETTORI_INT_H

#define VETTORI_INT_H
#define length(array) (sizeof(array)/sizeof(array[0]))
/* Addizione */
typedef struct intNode_ {
    int numero;
    struct intNode_* next;
} intNode;

typedef struct intList_ {
    intNode* first;
    intNode* last;
    int length;
}intList;

void intPush(intList* list, int i);

void intStampaLista(intList* lista);

intList* intInit();
intList* intInitFromArray(int* a, int length);
/* ritorna il puntatore all'elemento trovato, se l'idx è out of bound ritorna NULL */
int* intElementAt(intList* l, int idx);

/*
    ritorna il primo elemento trovato che soddisfa il criterio inposto dalla funzione f
*/
int* intFindFirst(intList* l, int(*f)(int el, int idx));

/* ritorna la lista filtrata con tutti gli elementi che soddisfano il criterio imposto da f*/
intList* intFindAll(intList* l, int(*f)(int el, int idx));

void intFreeList(intList* lista);

void intRemove(intList* lista, int idx);


/*ritorna una nuova lista la cui costruzione dipende dalla prima

    es:
        - Per ritornare una lista i cui numeri sono il doppio della prima:

            map({1,2,3} , function(int el,int idx){
                return el * 2;
            }) == {2,4,6}




*/
intList* map(intList* l, int(*f)(int el, int idx));


/*
    Ritorna il minimo della lista
*/
int min(intList* l);
/*
    Ritorna il massimo della lista
*/
int max(intList* l);

/*
    Somma tutti gli elementi della lista
*/
int sum(intList* l);

/* trasforma in un normale array di interi la lista dinamica */
int* toArray(intList* l, int* length);


/*ritorna una lista di indici tali che per cisacuno di essi l'elemento in posizione di quell'indice rispetta la condizione 'booleana' imposta da filter()*/
intList* findIdxs(int* vect, int length, int(*filter)(int));

intList* intIntersect(intList* l1, intList* l2);
intList* intUnion(intList* l1, intList* l2);

int contain(intList* l1, int n);

#endif
