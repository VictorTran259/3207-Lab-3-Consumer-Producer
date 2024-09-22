#define MAX_SIZE 50

// A linked list (LL) node to store a queue entry
struct QNode {
    int key;
    struct QNode* next;
};

// The queue, front stores the front node of LL and rear
// stores the last node of LL
struct Queue {
    struct QNode *front, *rear;
    int size;
};

struct QNode* newNode(int k);
struct Queue* createQueue();
void enQueue(struct Queue* q, int k);
void deQueue(struct Queue* q);
int getSize(struct Queue* q);
int getFrontVal(struct Queue* q);