#include "myqueue.h"

Queue_t QueueCreate(unsigned short deep, unsigned short elementsize )
{    
    if(( 0 == deep) || ( 0 == elementsize ))
    {
        return NULL;
    }
    
    Queue_t pQueue = (Queue_t)malloc(sizeof(QueueStr_t));
    if(NULL == pQueue)
    {
        return NULL;
    }
    
    pQueue->store = malloc(deep * elementsize);
    if(NULL == pQueue->store)
    {
        free(pQueue);
        return NULL;
    }
    
    pQueue->deep = deep;
    pQueue->elementsize = elementsize;
    pQueue->front = 0;
    pQueue->rear = 0;
    pQueue->count = 0;
    
    return pQueue;
}

QueueSta_t QueueClear(Queue_t queue)
{
  if(NULL == queue) return queue_null;
  

    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    return queue_ok;
}

QueueSta_t QueueDelete(Queue_t queue)
{
    if(NULL == queue) return queue_null;
    if(NULL == queue->store)
    {
       free(queue);
       queue = NULL;
       return queue_null;
    }
    else
    {
        free(queue->store);
        free(queue);
        queue = NULL;
        return queue_ok; 
    }
}

QueueSta_t QueuePush(Queue_t queue, void * element)
{
    if(NULL == element) return element_null;
    if(NULL == queue) return queue_null;
    if(NULL == queue->store) return queue_null;
    
    if(queue->count > queue->deep)return queue_full;
    
    unsigned short cur = 0;
    
    if(0 == queue->count)
    {
       cur = 0; 
       queue->rear = 0;
       queue->front = 0;
    }
    else
    {
       cur = (queue->rear + 1 >= queue->deep) ? 0 : (queue->rear + 1);
    }
    
    memcpy(queue->store + queue->elementsize * cur, element, queue->elementsize);
    
    queue->rear = cur;
    queue->count += 1;
    return queue_ok;
}

QueueSta_t QueuePull(Queue_t queue, void * element)
{
    if(NULL == element) return element_null;
    if(NULL == queue) return queue_null;
    if(NULL == queue->store) return queue_null;
    
    if(0 >= queue->count)return queue_empty;
    memcpy(element, queue->store + queue->elementsize * queue->front,  queue->elementsize);
    
    queue->front = (queue->front + 1 >= queue->deep) ? 0 : (queue->front + 1);
    queue->count -= 1;
    
    if(0 >= queue->count)
    {
       QueueClear(queue);
    }
    
    return queue_ok;
}
