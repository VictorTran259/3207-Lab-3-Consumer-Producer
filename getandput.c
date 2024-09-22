#include "queue.h"

//Enqueues a value that is passed to the function to the queue that is passed to the function.
void put(struct Queue* q, int value) {
    enQueue(q, value);
}

//Stores the first value in the queue into a temp variable to be returned and then dequeues it from the queue.
int get(struct Queue* q) {
    int temp = getFrontVal(q);
    deQueue(q);
    return temp;
}