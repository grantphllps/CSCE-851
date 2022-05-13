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
//sem_t *empty = new sem_t;
//sem_t *full = new sem_t;

volatile long id;

void *producer(void *threadid);
void *consumer(void *threadid);

char generate_random_alphabet();

void mon_insert(char alpha);
char mon_remove();

char buffer[10000]; //here is the buffer 'cuz im not fancey with pointers

typedef struct {
    int threadsWaiting = 0;       //how many processes are waiting on cv
    sem_t *semaphore = new sem_t; //queue of processes waiting on cv init 0
} cond;

int count(cond cv);
void wait(cond cv);
void signal(cond cv);

void insert_item(char alpha);
char remove_item();

cond empty, full;
volatile int itemsLeftToProduce;
volatile int itemsLeftToConsume;

volatile int topOfStack = -1;
volatile int itemsInTheBuffer = 0;

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
            itemsLeftToProduce = atoi(argv[++i]);
            itemsLeftToConsume = itemsLeftToProduce;
        }
    }

    //std::cout << "parsed Inputs :)\n";

    sem_init(full.semaphore,0,0);      //how many buffer spots are full
    sem_init(empty.semaphore,0,0);     //how many buffer spots are empty
    sem_init(mutex, 0, 1);

    //create threads
    pthread_t producers[nProducers];
    pthread_t consumers[nConsumers];

    //std::cout << "declared thread ids :)\n";
    
    //std::cout << "making producer threads" << std::endl;
    for (long i = 1; i < (nProducers + 1); i++) {
        pthread_create(&producers[i], NULL, producer, (void*) i);
        //std::cout << "made producer " << i << std::endl;
    }
    //std::cout << "producers done :)" << std::endl;

    for (long i = 1; i < (nConsumers + 1); i++) {
        //std::cout << "made consumer " << i << std::endl;
    	pthread_create(&consumers[i], NULL, consumer, (void*) i);
    }
    //std::cout << "consumers done :)" << std::endl;
    
    // for (int i = 0; i <= nProducers; i++)
    //     pthread_join(producers[i], NULL);

    // for (int i = 0; i <= nConsumers; i++)
    //     pthread_join(consumers[i], NULL);
    
    pthread_exit(NULL);

    return 0;
}

//consumer and producer functions

void *producer(void *threadid) {
    char alpha;
    while(1) {
        sem_wait(mutex);
        id = (long)threadid;
        //std::cout << "there are: " << itemsLeftToProduce << " items left to produce\n";
        //std::cout << "**producer thread " << id << " has the mutex**\n";
        if (itemsLeftToProduce <= 0) {
            //std::cout << "mutex released\n";
            //std::cout << "killing producer thread " << id << " =========>\n";
            sem_post(empty.semaphore);
            sem_post(mutex);
            pthread_exit(NULL);
        }
        id = (long)threadid;
        alpha = generate_random_alphabet();
        mon_insert(alpha);
        id = (long)threadid;
        printf("p:<%d>, item: %c, at %d\n\r",(int)id,alpha,topOfStack);
        //std::cout << "there are " << itemsInTheBuffer << " items in the buffer\n";
        signal(empty);
    }
}

void *consumer(void *threadid) {
    char result;
    while(1) {
        sem_wait(mutex);
        id = (long)threadid;
        //std::cout << "consumer " << id << " has the mutex\n";
        //std::cout << "there are: " << itemsLeftToConsume << " items left to consume\n";
        //std::cout << "**consumer thread " << id << " has the mutex**\n";
        if (itemsLeftToConsume <= 0) {
            //std::cout << "there are " << itemsLeftToConsume  << " items left to consume\n";
            //std::cout << "killing consumer thread " << id << " =========>\n";
            sem_post(mutex);
            pthread_exit(NULL);
        }
        id = (long)threadid;
        result = mon_remove();
        id = (long)threadid;
        printf("c:<%d>, item: %c, at %d\n\r",(int)id,result,(topOfStack+1));
        //std::cout << "there are " << itemsInTheBuffer << " items in the buffer\n";
        signal(full);
    }
}


char generate_random_alphabet() {
    char alphabet[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
    int temp = rand() % 52;
    //std::cout << temp;
    char tempC = alphabet[temp];
    //std::cout << tempC;
    return tempC;
}

int count(cond cv) {
    std::cout << "waiting on " << cv.threadsWaiting << "\n";
    return cv.threadsWaiting;
}

void wait(cond cv) {
    cv.threadsWaiting = cv.threadsWaiting + 1;
    //std::cout << "mutex released\n";
    //std::cout << "there are " << cv.threadsWaiting << " on some queue\n";
    sem_post(mutex);
    sem_wait(cv.semaphore);
    //sem_wait(mutex);
    //std::cout << "mutex taken\n";
}

void signal(cond cv) {
    if (cv.threadsWaiting > 0)
        cv.threadsWaiting--;
    sem_post(cv.semaphore);
    //std::cout << "there are now" << cv.threadsWaiting << " on some queue\n";
    sem_post(mutex);
}

void mon_insert(char alpha) {
    while((topOfStack+1) == (bufferSize)){
        wait(full);
        sem_wait(mutex);
        //std::cout << "blocked prodcuer has the the mutex\n";
        if (itemsLeftToProduce < 1) {
            //std::cout << "killing blocked producer thread =========>\n";
            sem_post(full.semaphore);
            sem_post(mutex);
            pthread_exit(NULL);
        }
        
    }
    topOfStack = topOfStack + 1;
    itemsInTheBuffer = itemsInTheBuffer + 1;
    itemsLeftToProduce = itemsLeftToProduce - 1;
    //std::cout << "After insert, top of stack is: " << topOfStack << "\n";
    insert_item(alpha);
}

char mon_remove() {
    char result;
    while(itemsInTheBuffer == 0) {
        wait(empty);
        //std::cout << "done waiting empty\n";
        sem_wait(mutex);
        //std::cout << "blocked consumer has the mutex\n";
        if (itemsLeftToConsume < 1) {
            //std::cout << "killing blocked consumer thread =========>\n";
            sem_post(empty.semaphore);
            sem_post(mutex);
            pthread_exit(NULL);
        }
        
    }
    result = remove_item();
    topOfStack = topOfStack - 1;
    itemsInTheBuffer = itemsInTheBuffer - 1;
    itemsLeftToConsume = itemsLeftToConsume - 1;
    
    //std::cout << "After consume, top of stack is: " << topOfStack << "\n";
    return result;
}

void insert_item(char alpha) {
    buffer[topOfStack] = alpha;
}

char remove_item() {
    char temp = buffer[topOfStack];
    return temp;
}