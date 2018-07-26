#ifndef _MYQUEUE_H_
#define _MYQUEUE_H_
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

#define QUEUEDEEP 10
#define DATADEEP  128
typedef enum
{
    queue_ok,
    queue_null,
    element_null,
    queue_full,
    queue_empty,
}QueueSta_t;

#pragma   pack(1)
typedef struct
{
  char  data[DATADEEP];
  int   len ;
}mydata_t;

typedef struct
{
    mydata_t queue_array[QUEUEDEEP];
    unsigned short head;
    unsigned short tail;
    
} ring_queue_t;

void init_queue(ring_queue_t ring_queue);
bool take_from_queue(ring_queue_t ring_queue, void *buf, int *len, bool erase);
bool push_to_queue(ring_queue_t ring_queue, void *buf, int len);
void clear_queue(ring_queue_t ring_queue);
bool queue_is_empty(ring_queue_t ring_queue);

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

#pragma   pack()
typedef QueueStr_t * Queue_t;


Queue_t QueueCreate(unsigned short deep, unsigned short elementsize );
QueueSta_t QueueDelete(Queue_t queue);
QueueSta_t QueuePush(Queue_t queue, void * element);
QueueSta_t QueuePull(Queue_t queue, void * element);
#endif