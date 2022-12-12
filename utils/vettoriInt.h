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

int* intFindFirst(intList* l, int(*f)(int el, int idx));

/* ritorna la lista filtrata */
intList* intFindAll(intList* l, int(*f)(int el, int idx));

void intFreeList(intList* lista);

void intRemove(intList* lista, int idx);


/*ritorna una nuova lista la cui costruzione dipende dalla prima

    es:
        - Per ritornare una lista i cui numeri sono il doppio della prima:

            map({1,2,3} , (int el,int idx)=>{
                return el * 2;
            }) == {2,4,6}




*/
intList* map(intList* l, int(*f)(int el, int idx));

int min(intList* l);
int max(intList* l);

int sum(intList* l);

/* trasforma in un normale array di interi la lista dinamica */
int* toArray(intList* l, int* length);

#endif
