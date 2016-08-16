#ifndef _MYQUEUE_H_
#define _MYQUEUE_H_

#include "stdlib.h"
#include "string.h"

typedef void * Obj_t;

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
    unsigned char * store; 
    
    QueueSta_t ( *push)(Obj_t e, void * element); 
    QueueSta_t ( *pull)(Obj_t e,void * element); 
    QueueSta_t ( *clear)(Obj_t e); 
}QueueStr_t;

typedef QueueStr_t * Queue_t;

Queue_t QueueCreate(unsigned short deep, unsigned short elementsize );
void QueueDelete(Queue_t queue);
#endif