//***I COPIED THIS CODE FROM GEEKSFORGEEKS***
//LINK: https://www.geeksforgeeks.org/queue-linked-list-implementation/?ref=lbp#

//***note to self (delete later): need to add locks to this implementation and make it bounded***
// A C program to demonstrate linked list based
// implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
 
// A utility function to create a new linked list node.
struct QNode* newNode(int k) {
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}
 
// A utility function to create an empty queue
struct Queue* createQueue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}
 
// The function to add a key k to q
void enQueue(struct Queue* q, int k) {
    // Create a new LL node
    struct QNode* temp = newNode(k);
 
    // If queue is empty, then new node is front and rear
    // both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        q->size++;

        //printf("Successfully enqueued %d\n", k);

        return;
    }
    
    if(getSize(q) == MAX_SIZE) {
        printf("Error: Failed to enqueue %d. Queue is full.\n", k);
    }
    else {
        // Add the new node at the end of queue and change rear
        q->rear->next = temp;
        q->rear = temp;

        // Increment the size variable in the queue struct by 1.
        q->size++;

        //printf("Successfully enqueued %d\n", k);
    }
}
 
// Function to remove a key from given queue q
void deQueue(struct Queue* q) {
    // If queue is empty, return NULL.
    if (q->front == NULL) {
        printf("Error: failed to dequeue. Queue is empty.\n");
        return;
    }
 
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;
 
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL) {
        q->rear = NULL;
    }

    //printf("Successfully dequeued %d\n", temp->key);

    // Decrement the size variable in the queue struct by 1.
    q->size--;
 
    free(temp);
}


// Retrieves the size of a queue when called.
int getSize(struct Queue* q) {
    return q->size;
}

// Retrieves the first value of the queue when called.
int getFrontVal(struct Queue* q) {
    if(q->front == NULL) {
        return 0;
    }
    else {
        return q->front->key;
    }
}

/*
// Driver code
int main() {
    struct Queue* q = createQueue();
    enQueue(q, 10);
    enQueue(q, 20);
    deQueue(q);
    deQueue(q);
    enQueue(q, 30);
    enQueue(q, 40);
    enQueue(q, 50);
    deQueue(q);
    printf("Queue Front : %d\n", ((q->front != NULL) ? (q->front)->key : -1));
    printf("Queue Rear : %d\n", ((q->rear != NULL) ? (q->rear)->key : -1));
    return 0;
}
*/
