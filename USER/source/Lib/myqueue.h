#ifndef _MYQUEUE_H_
#define _MYQUEUE_H_

#include "stdlib.h"
#include "string.h"

typedef enum
{
    queue_ok,
    queue_null,
    element_null,
    queue_full,
    queue_empty,
}QueueSta_t;

typedef struct
{
    unsigned short deep;  
    unsigned short elementsize;
    unsigned short front;
    unsigned short rear;
    unsigned short count;
    unsigned short index;
    unsigned char * store;
} QueueStr_t;

typedef QueueStr_t * Queue_t;


Queue_t QueueCreate(unsigned short deep, unsigned short elementsize );
QueueSta_t QueueDelete(Queue_t queue);
QueueSta_t QueuePush(Queue_t queue, void * element);
QueueSta_t QueuePull(Queue_t queue, void * element);
#endif