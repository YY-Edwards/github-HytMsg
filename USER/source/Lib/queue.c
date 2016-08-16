#include "queue.h"



static QueueSta_t push(Obj_t e, void * element);
static QueueSta_t pull(Obj_t e, void * element);
static QueueSta_t clear(Obj_t e);

Queue_t QueueCreate(unsigned short deep, unsigned short elementsize )
{    
    if(( 0 == deep) || ( 0 == elementsize ))
    {
        return NULL;
    }
    
    Queue_t queue = (Queue_t)malloc(sizeof(QueueStr_t));
    if(NULL == queue)
    {
        return NULL;
    }
    
    queue->store = malloc(deep * elementsize);
    if(NULL == queue->store)
    {
        free(queue);
        return NULL;
    }
     
    queue->deep = deep;
    queue->elementsize = elementsize;
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    
    queue->push = push;
    queue->pull = pull;
    queue->clear = clear;
    
    return queue;
}

void QueueDelete(Queue_t queue)
{
    if(NULL == queue) return;
    if(NULL == queue->store)
    {
       free(queue);
    }
    else
    {
        free(queue->store);
        free(queue);
    }
}



static QueueSta_t push(Obj_t e, void * element)
{
    if(NULL == e) return queue_null;  
    if(NULL == element) return element_null;
    
    Queue_t this = (Queue_t)e;
    
    if(NULL == this->store) return queue_null;
    if(this->count > this->deep)return queue_full;
    
    unsigned short cur = 0;
    
    if(0 == this->count)
    {
       cur = 0; 
       this->rear = 0;
       this->front = 0;
    }
    else
    {
       cur = (this->rear + 1 >= this->deep) ? 0 : (this->rear + 1);
    }
    
    memcpy(this->store + this->elementsize * cur, element, this->elementsize);
    
    this->rear = cur;
    this->count += 1;
    return queue_ok;
}

static QueueSta_t pull(Obj_t e, void * element)
{
    if(NULL == e) return queue_null;  
    if(NULL == element) return element_null;
     Queue_t this = (Queue_t)e;
    
    if(NULL == this->store) return queue_null;
    
    if(0 >= this->count)return queue_empty;
    memcpy(element, this->store + this->elementsize * this->front,  this->elementsize);
    
    this->front = (this->front + 1 >= this->deep) ? 0 : (this->front + 1);
    this->count -= 1;
    
    if(0 >= this->count)
    {
       clear(this);
    }
    
    return queue_ok;
}

static QueueSta_t clear(Obj_t e)
{
     if(NULL == e) return queue_null; 
     Queue_t this = (Queue_t)e;
    this->front = 0;
    this->rear = 0;
    this->count = 0;
    return queue_ok;
}

