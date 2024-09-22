#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "queue.h"
#include "getandput.h"

void *producer(void *arg);
void *distributor(void *arg);
void *consumer(void *arg);

pthread_mutex_t mutex;
pthread_cond_t empty, fill;

//Struct that contains information that can be initiallized and sent to producer and consumer threads.
struct args {
    struct Queue *queue;
    int productType;
    int *totalConsumeCount;
    int *consumeSequence;
    int *ptrToPipe;
};

//Struct that contains information that can be initiallized and sent to distributor thread.
struct distributorStruct {
    struct Queue *queue1;
    struct Queue *queue2;
    int signalCount;
    int *ptrToPipe;
};

//Main function.
int main(int argc, char **argv) {
    //Remove any prior instances of "log.txt" before continuting on in the program.
    remove("log.txt");

    //Create the pipe
    int fd[2];

    //Throw an error if the pipe failed to create.
    if(pipe(fd) == -1){
        printf("Error: failed to create pipe.\n");
        exit(-1);
    }

    //Create the buffers
    struct Queue *buf1 = createQueue();
    struct Queue *buf2 = createQueue();

    //Initialize all of the counts to 0.
    int totalConsumeCount = 0;
    int consumeSequence1 = 0;
    int consumeSequence2 = 0;

    //Initialize the two structs that contain the arguments to be passed to the producers and consumers.
    struct args args1;
    struct args args2;

    //Initialize the struct that contains the arguments to be passed to the distributor.
    struct distributorStruct distributorStruct;

    //Initialize the data in each args struct to its corresponding value.
    args1.productType = 1;
    args2.productType = 2;

    args1.consumeSequence = &consumeSequence1;
    args2.consumeSequence = &consumeSequence2;

    args1.totalConsumeCount = &totalConsumeCount;
    args2.totalConsumeCount = &totalConsumeCount;

    args1.queue = buf1;
    args2.queue = buf2;

    args1.ptrToPipe = fd;
    args2.ptrToPipe = fd;

    //Initialize the data in the 
    distributorStruct.queue1 = buf1;
    distributorStruct.queue2 = buf2;
    distributorStruct.signalCount = 0;
    distributorStruct.ptrToPipe = fd;

    //Arrays containing the disributor and consumer threads.
    pthread_t producer_tid[2];
    pthread_t distributor_tid[1];
    pthread_t consumer_tid[4];

    //Initialize mutex and condition variables.
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&empty, NULL);
    pthread_cond_init(&fill, NULL);

    //Create producer thread 1.
    if(pthread_create(&producer_tid[0], NULL, producer, &args1) != 0) {
        printf("Error: failed to create producer thread.\n");
        exit(-1);
    }
    //Create producer thread 2.
    if(pthread_create(&producer_tid[1], NULL, producer, &args2) != 0) {
        printf("Error: failed to create producer thread.\n");
        exit(-1);
    }

    //Create distributor thread.
    if(pthread_create(&distributor_tid[0], NULL, distributor, &distributorStruct) != 0) {
        printf("Error: failed to create distributor thread.\n");
        exit(-1);
    }

    //Create consumer thread 1.
    if(pthread_create(&consumer_tid[0], NULL, consumer, &args1) != 0) {
        printf("Error: failed to create consumer thread 1.\n");
        exit(-1);
    }
    //Create consumer thread 2.
    if(pthread_create(&consumer_tid[1], NULL, consumer, &args1) != 0) {
        printf("Error: failed to create consumer thread 2.\n");
        exit(-1);
    }

    //Create consumer thread 3.
    if(pthread_create(&consumer_tid[2], NULL, consumer, &args2) != 0) {
        printf("Error: failed to create consumer thread 3.\n");
        exit(-1);
    }
    //Create consumer thread 4.
    if(pthread_create(&consumer_tid[3], NULL, consumer, &args2) != 0) {
        printf("Error: failed to create consumer thread 4.\n");
        exit(-1);
    }

    //Create the process for producer 1.
    int pid1 = fork();

    //Throw an error and exit the program if fork to create process for producer 1 failed.
    if(pid1 == -1) {
        printf("Error: failed to fork.\n");
        exit(-1);
    }

    //In the child process. Call join on the thread for producer 1.
    if(pid1 == 0) {
        if(pthread_join(producer_tid[0], NULL) != 0) {
            printf("Error: failed to join producer thread 1\n");
            exit(-1);
        }
    }
    //In the parent process.
    else {
        //Create the process for producer 2.
        int pid2 = fork();

        //Throw an error and exit the program if fork to create process for producer 2 failed.
        if(pid2 == -1) {
            printf("Error: failed to fork.\n");
            exit(-1);
        }

        //In the child process. Call join on the thread for producer 1.
        if(pid2 == 0) {
            if(pthread_join(producer_tid[1], NULL) != 0) {
                printf("Error: failed to join producer thread 2\n");
                exit(-1);
            }
        }
        //In the parent process which will now act as the distributor process. Call join on all of the consumer threads and the distributor thread.
        else {
            if(pthread_join(distributor_tid[0], NULL) != 0) {
                printf("Error: failed to join distributor thread\n");
                exit(-1);
            }

            if(pthread_join(consumer_tid[0], NULL) != 0) {
                printf("Error: failed to join consumer thread 1\n");
                exit(-1);
            }
            if(pthread_join(consumer_tid[1], NULL) != 0) {
                printf("Error: failed to join consumer thread 2\n");
                exit(-1);
            }
            if(pthread_join(consumer_tid[2], NULL) != 0) {
                printf("Error: failed to join consumer thread 3\n");
                exit(-1);
            }
            if(pthread_join(consumer_tid[3], NULL) != 0) {
                printf("Error: failed to join consumer thread 4\n");
                exit(-1);
            }
        }
    }

    //Close the pipe after it's done being used.
    close(fd[0]);
    close(fd[1]);

    //Destroy the mutex and condition variables after the program finishes its job and before the program exits.
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&empty);
    pthread_cond_destroy(&fill);
}

//Producer function to be used by threads.
//***BASED ON CODE FROM THREE EASY PIECES CHAPTER 30 PAGE 13***
void *producer(void *arg) {
    //Cast the arg structure from void* back to args* so the producers have access to the data in the struct.
    struct args *args0 = (struct args*)arg;

    //Producers will each produce 150 values plus the -1 that signals that the producer is done producing.
    for(int i = 1; i <= 151; i++) {
        //Lock over the critical section.
        pthread_mutex_lock(&mutex);

        int productID;
        int productType = args0->productType;

        //The producer will send -1 to the pipe signalling that the producer is done producing if i = 151, the last value and also the product type.
        if(i == 151) {
            productID = -1;
            if(write(args0->ptrToPipe[1], &productID, sizeof(int)) == -1) {
                printf("failed to write product ID %d to pipe.\n", productID);
            }
            if(write(args0->ptrToPipe[1], &productType, sizeof(int)) == -1) {
                printf("failed to write product type %d to pipe.\n", productType);
            }
        }
        //The producer will send the product id to the pipe for all other values and also the product type.
        else {
            productID = i;
            if(write(args0->ptrToPipe[1], &productID, sizeof(int)) == -1) {
                printf("failed to write product ID %d to pipe.\n", productID);
            }
            if(write(args0->ptrToPipe[1], &productType, sizeof(int)) == -1) {
                printf("failed to write product type %d to pipe.\n", productType);
            }
        }

        //End of critical section.
        pthread_mutex_unlock(&mutex);

        //Sleep for 0.2 seconds.
        usleep(200000);
    }
    pthread_exit(NULL);
}

//Distributor function to be used by the distributor thread.
void *distributor(void *arg) {
    //Cast the arg structure from void* back to args* so the consumers have access to the data in the struct.
    struct distributorStruct *args1 = (struct distributorStruct*)arg;

    //Will continue distributing until producers send -1 into the pipe.
    //Will enqueue -1 into the queue to tell the consumers to stop.
    while(args1->signalCount != 2) {
        //Beginning of critical section.
        pthread_mutex_lock(&mutex);

        int productID;
        int productType;

        //Read the product id from the pipe which will be the first value in the 2 value sequence.
        //Throw an error if the read failed for some reason.
        if(read(args1->ptrToPipe[0], &productID, sizeof(int)) == -1) {
            printf("failed to read the product id\n");
        }
        //Read the product type from the pipe which will be the second value in the 2 value sequence.
        //Throw an error if the read failed for some reason.
        if(read(args1->ptrToPipe[0], &productType, sizeof(int)) == -1) {
            printf("failed to read the product type\n");
        }

        //args1->signalCount will equal 2 when both producers have sent -1 into the pipe.
        //When the distributor encounters a -1 in the pipe, increment the signal count.
        if(productID == -1 && args1->signalCount != 2) {
            args1->signalCount += 1;
        }

        //Enqueues the product id into queue 1 if it's of product type 1.
        if(productType == 1) {
            while(getSize(args1->queue1) == MAX_SIZE) {
                pthread_cond_wait(&empty, &mutex);
            }
            put(args1->queue1, productID);
            pthread_cond_signal(&fill);
        }
        //Enqueues the product id into queue 2 if it's of product type 2.
        if(productType == 2) {
            while(getSize(args1->queue1) == MAX_SIZE) {
                pthread_cond_wait(&empty, &mutex);
            }
            put(args1->queue2, productID);
            pthread_cond_signal(&fill);
        }

        //End of critical section.
        pthread_mutex_unlock(&mutex);
        //The distributor has to sleep for a little bit or the program bugs out.
        usleep(100000);
    }
    pthread_exit(NULL);
}

//Consumer function to be used by threads.
//***BASED ON CODE FROM THREE EASY PIECES CHAPTER 30 PAGE 13***
void *consumer(void *arg) {
    //Cast the arg structure from void* back to args* so the consumers have access to the data in the struct.
    struct args *args0 = (struct args*)arg;

    //Consumers will keep going until the producer sends a -1 into the queue signalling that it's done producing.
    while(getFrontVal(args0->queue) != -1) {
        //Lock over the critical section.
        pthread_mutex_lock(&mutex);

        //Consumer waits until the queue isn't empty before continuing.
        while(getSize(args0->queue) == 0) {
            pthread_cond_wait(&fill, &mutex);
        }
        
        //Buffer that holds the value that was removed from the queue.
        int temp = get(args0->queue);

        //Increment the consumer sequence and total consumer count by 1.
        *(args0->consumeSequence) += 1;
        *(args0->totalConsumeCount) += 1;

        //Print the information to standard output.
        printf("Product ID: %5d | Product Type: %5d | Thread ID: %5lu | Prod SEQ #: %5d | Consume SEQ #: %5d | Total Consume Count: %5d\n", 
        temp, args0->productType, pthread_self(), temp , *(args0->consumeSequence), *(args0->totalConsumeCount));

        //Code I borrowed from my project 2 that redirects the standard output to a file which will be the "log.txt" file in this case.
        //Buffers to hold the original file descriptors so the file descriptors can be reset later.
        int originalSTDOUT = dup(STDOUT_FILENO);
        int originalSTDIN = dup(STDIN_FILENO);

        //Initialize the output file variable.
        int outputfile;

        //Open the output file. File is created if it doesn't exist and is opened in append mode so no previous information is lost.
        outputfile = open("log.txt", O_WRONLY | O_APPEND | O_CREAT,  0777);

        //Throw an error if "log.txt" failed to open.
        if(outputfile == -1) {
            printf("failed to open file.\n");
        }
        else {
            //Change STDOUT to the output file.
            dup2(outputfile, STDOUT_FILENO);
            close(outputfile);
        }

        //Print the information to the "log.txt" file.
        printf("Product ID: %5d | Product Type: %5d | Thread ID: %5lu | Prod SEQ #: %5d | Consume SEQ #: %5d | Total Consume Count: %5d\n", 
        temp, args0->productType, pthread_self(), temp , *(args0->consumeSequence), *(args0->totalConsumeCount));

        //Reset the file descriptors to their original values.
        dup2(originalSTDOUT, STDOUT_FILENO);
        dup2(originalSTDIN, STDIN_FILENO);
        close(originalSTDIN);
        close(originalSTDOUT);
        
        //End of critical section.
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);

        //The program bugs out if the consumers don't sleep so this needs to be here and the sleep time needs to be set to at least 400000 (0.4 seconds).
        //Generally, this value probably has to be a little over double the value of the sleep time for the producer process in order to consistently work.
        usleep(450000);
    }
    pthread_exit(NULL);
}