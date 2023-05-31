#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

typedef struct QNode {
    void *data;
    struct QNode *next;
} QNode;

typedef struct queue {
    QNode *head;
    QNode *tail;
    pthread_mutex_t mutex_lock;
    pthread_cond_t cond;
} queue;

typedef struct activeObject {
    pthread_t pid;
    queue *queue;
    void *(*func)(void *);
} activeObject;

activeObject *AO1, *AO2, *AO3, *AO4;

bool isPrime(unsigned int num)
{
    if (num < 2)
        return false;
    
    for (unsigned int i = 2; i * i <= num; i++)
    {
        if (num % i == 0)
            return false;
    }
    
    return true;
}


void enqueue(queue *q, void *item)
{
    QNode *new_node = malloc(sizeof(QNode));
    if (new_node == NULL)
    {
        fprintf(stderr, "Allocate Memmory failed.\n");
        exit(EXIT_FAILURE);
    }

    new_node->data = item;
    new_node->next = NULL;

    pthread_mutex_lock(&(q->mutex_lock));
    
    if (q->head == NULL)
    {
        q->head = new_node;
        q->tail = new_node;
    }
    else
    {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    
    pthread_cond_signal(&(q->cond));
    pthread_mutex_unlock(&(q->mutex_lock));
}


void *dequeue(queue *q)
{
    pthread_mutex_lock(&(q->mutex_lock));
    
    while (q->head == NULL)
    {
        pthread_cond_wait(&(q->cond), &(q->mutex_lock));
    }
    
    QNode *head = q->head;
    void *item = head->data;
    q->head = head->next;
    
    if (q->head == NULL)
    {
        q->tail = NULL;
    }
    
    pthread_mutex_unlock(&(q->mutex_lock));
    
    free(head);
    return item;
}


void *start(void *arg){
    activeObject *self = (activeObject *)arg;
    void *task;
    while ((task = dequeue(self->queue)) != NULL)
    {
        self->func(task);
    }
    return NULL;
}

activeObject *createActiveObject(void *(*func)(void *))
{
    activeObject *aobj = malloc(sizeof(activeObject));
    if (aobj == NULL)
    {
        fprintf(stderr, "Allocate Memmory failed.\n");
        exit(EXIT_FAILURE);
    }
    aobj->queue = malloc(sizeof(queue));
    if (aobj->queue == NULL)
    {
        fprintf(stderr, "Allocate Memmory failed.\n");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&(aobj->queue->mutex_lock), NULL);
    pthread_cond_init(&(aobj->queue->cond), NULL);
    aobj->queue->head = NULL;
    aobj->queue->tail = NULL;
    aobj->func = func;
    pthread_create(&(aobj->pid), NULL, start, aobj);
    return aobj;
}

void freeQueue(queue *q)
{
    pthread_mutex_lock(&(q->mutex_lock));
    QNode *current = q->head;
    while (current != NULL)
    {
        QNode *to_free = current;
        current = current->next;
        free(to_free);
    }
    pthread_mutex_unlock(&(q->mutex_lock));
    pthread_mutex_destroy(&(q->mutex_lock));
    pthread_cond_destroy(&(q->cond));
    free(q);
}

void stop(activeObject *aoObj)
{
    enqueue(aoObj->queue, NULL);
    pthread_join(aoObj->pid, NULL);
    freeQueue(aoObj->queue);
    free(aoObj);
}

// void stopAll(activeObject *ao1, activeObject *ao2, activeObject *ao3, activeObject *ao4)
// {
//     stop(ao1);
//     stop(ao2);
//     stop(ao3);
//     stop(ao4);
// }

queue *getQueue(activeObject *obj)
{
    return obj->queue;
}

void *nextQueue1(void *args)
{
     int *number = ( int *)args;

    printf("%d\n%s\n", *number ,isPrime(*number) ? "true" : "false");

    enqueue(getQueue(AO2), number);

    return NULL;
}

void *nextQueue2(void *args)
{
     int *number = ( int *)args;

    *number += 11;

    if(isPrime(*number)){
        printf("%d\n",*number);
        printf("true\n");
    }else{
         printf("false\n");
    }
    // printf("%d\n%s\n", *number, isPrime(*number) ? "true" : "false");

    enqueue(getQueue(AO3), number);

    return NULL;

}

void *nextQueue3(void *args)
{
     int *number = ( int *)args;

    *number -= 13;
    if(isPrime(*number)){
        printf("%d\n",*number);
        printf("true\n");
    }else{
         printf("false\n");
    }
    // printf("%u\n%s\n", *number, isPrime(*number) ? "true" : "false");

    enqueue(getQueue(AO4), number);

    return NULL;

}


void *nextQueue4(void *args)
{
     int *number = ( int *)args;

    *number += 2;
        printf("%d\n",*number);
    
    // printf("%u\n", *number);

    free(number);

    return NULL;

}

// void initializeActiveObjects(activeObject **ao1, activeObject **ao2, activeObject **ao3, activeObject **ao4)
// {
//     *ao1 = createActiveObject(nextQueue1);
//     *ao2 = createActiveObject(nextQueue2);
//     *ao3 = createActiveObject(nextQueue3);
//     *ao4 = createActiveObject(nextQueue4);
// }

void updateFirstQueue(activeObject *aobj, int size)
{
    for (int i = 0; i < size; i++)
    {
        unsigned int *new_number = malloc(sizeof(unsigned int));
        *new_number = rand() % 900000 + 100000;
        enqueue(getQueue(aobj), new_number);
        printf("\n");

        if (i < size - 1) {
            sleep(1);
        }
    }
}

int main(int argc, char **argv)
{
    if(argc > 2)
    {
        printf("ilegall arguments!!\n");
        printf("usage:- ./st_pipelie N");
        return 0;
    }

    if (argc < 2)
    {
        printf("Please add positive number(N).\n");
        return 0;
    }

    int iter = atoi(argv[1]);
    
    int seed = argc == 3 ? atoi(argv[2]) : time(NULL);

    srand(seed);

     AO1 = createActiveObject(nextQueue1);
    AO2 = createActiveObject(nextQueue2);
    AO3 = createActiveObject(nextQueue3);
    AO4 = createActiveObject(nextQueue4);
    // initializeActiveObjects(&AO1,&AO2,&AO3, &AO4);

    updateFirstQueue(AO1,iter);

    // stopAll(AO1,AO2,AO3,AO4);

    stop(AO1);
    stop(AO2);
    stop(AO3);
    stop(AO4);

    return 0;
}
