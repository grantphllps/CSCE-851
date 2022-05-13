#include <iostream>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

int bufferSize;
int nProducers;
int nConsumers;
int nItems;

char *stack;

sem_t *mutex = new sem_t;
sem_t *empty = new sem_t;
sem_t *full = new sem_t;

long id;

void *producer(void *threadid);
void *consumer(void *threadid);

volatile int* topOfStack = new int;
int* itemsLeftToProduce = new int;
int* itemsLeftToConsume = new int;

char buffer[10000]; //here is the buffer 'cuz im not fancey with pointers

int main(int argc, char** argv) {
    //Get the input parameters
    for (int i = 1; i < argc; i++) {
        if (argc != 9) {
            std::cout << "part1 requires exactly 8 inputs" << std::endl;
            break;
        }
        else if (strcmp(argv[i],"-b") == 0) {
            bufferSize = atoi(argv[++i]);
            //std::cout << "buffer is " << bufferSize << " bytes" << std::endl;
        }
        else if (strcmp(argv[i],"-p") == 0) {
            nProducers = atoi(argv[++i]);
            //std::cout << "number of producer threads is " << nProducers << " threads" << std::endl;
        }
        else if (strcmp(argv[i],"-c") == 0) {
            nConsumers = atoi(argv[++i]);
            //std::cout << "number of consumer threads is " << nConsumers << " threads" << std::endl;
        }
        else if (strcmp(argv[i],"-i") == 0) {
            //nItems = atoi(argv[++i]);
            //std::cout << "number of items is " << nItems << " items" << std::endl;
            *itemsLeftToProduce = atoi(argv[++i]);
            *itemsLeftToConsume = *itemsLeftToProduce;
        }
    }

    *topOfStack = -1;
    //printf("The stack is at %u\n",stack);

    //std::cout << "making semaphores" << std::endl;
    //create semaphores
    sem_init(mutex, 0, 1);
    //std::cout << "mutex" << std::endl;
    sem_init(empty, 0, bufferSize);
    //std::cout << "empty" << std::endl;
    sem_init(full, 0, 0);
    
    //std::cout << "making thread ids" << std::endl;
    //create threads
    pthread_t producers[nProducers];
    pthread_t consumers[nConsumers];
    
    //std::cout << "making producer threads" << std::endl;
    for (long i = 1; i < (nProducers + 1); i++) {
        pthread_create(&producers[i], NULL, producer, (void*) i); 
    }
    //std::cout << "producers done" << std::endl;

    for (long i = 1; i < (nConsumers + 1); i++) {
    //std::cout << "made consumer " << i << std::endl;
    	pthread_create(&consumers[i], NULL, consumer, (void*) i);
    }
    
    pthread_exit(NULL);

    return 0;
}

//consumer and producer functions

void *producer(void *threadid) {

    while(1) {
        
        sem_wait(empty); //Don't bother mutex unless there is an empty
        sem_wait(mutex); //Grap the mutex
        if (*itemsLeftToProduce <= 0) { //check if there are even any items to produce
            sem_post(mutex);            //if not, release mutex
            sem_post(empty);            //create an empty os other producer threads can exit
            pthread_exit(NULL);         //Yeet the thread
        }
        
        id = (long)threadid;
        *topOfStack = *topOfStack + 1;
        *itemsLeftToProduce = *itemsLeftToProduce - 1;
        // insert here
        char temp = 'X';
        buffer[*topOfStack] = temp;
        //
        printf("p:<%d>, item: %c, at %d\n\r",(int)id,temp,*topOfStack);
        sem_post(mutex);
        sem_post(full);
    }
}

void *consumer(void *threadid) {

    while(1) {
        
        sem_wait(full);
        sem_wait(mutex);
        if (*itemsLeftToConsume <= 0) {
            sem_post(mutex);
            sem_post(full);
            pthread_exit(NULL);
        }
        id = (long)threadid;
        // consume here
        char temp = buffer[*topOfStack];
        buffer[*topOfStack] = 'E';
        //
        printf("c:<%d>, item: %c, at %d\n\r",(int)id,temp,*topOfStack);

        *topOfStack = *topOfStack- 1;
        *itemsLeftToConsume = *itemsLeftToConsume - 1;
        if (*itemsLeftToConsume == 0) {
            sem_post(full);
        }
        sem_post(mutex);
        sem_post(empty);
    }
    std::cout << "killing consumer " << std::endl;
    pthread_exit(NULL);
}
