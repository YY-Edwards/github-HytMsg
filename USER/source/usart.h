#ifndef _USART_H_
#define _USART_H_

#include <string.h>
#include <stdlib.h>

#include "lib/queue.h"

#define DefaultQueueDeep 256

typedef enum
{
    usart_ok,
}UsartSta_t;

typedef struct
{
    Queue_t txqueue;   
    Queue_t rxqueue;

    UsartSta_t ( *init)(unsigned int baud); 
    UsartSta_t ( *send)(unsigned char * ch); 
    UsartSta_t ( *receive)(unsigned char * ch);     
}UsartStr_t;

typedef UsartStr_t * Usart_t;

Usart_t UsartCreate(void);
void UsartDelete(Usart_t usart);

#endif