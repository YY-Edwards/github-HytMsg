#include "usart.h"

static UsartSta_t init(unsigned int baud); 
static UsartSta_t send(unsigned char * ch); 
static UsartSta_t receive(unsigned char * ch); 

Usart_t UsartCreate(void)
{
    Usart_t usart = (Usart_t)malloc(sizeof(UsartStr_t));
    
    usart->txqueue = QueueCreate(DefaultQueueDeep, 1);
    usart->rxqueue = QueueCreate(DefaultQueueDeep, 1);
    
    usart->init = init;
    usart->send = send;
    usart->receive =receive;
 
    return usart;
}

void UsartDelete(Usart_t usart)
{
  if(NULL != usart)
  {
      free(usart);
      usart = NULL;
  }
}

static UsartSta_t init(unsigned int baud)
{
    return usart_ok;
}
static UsartSta_t send(unsigned char * ch)
{
    return usart_ok;
}
static UsartSta_t receive(unsigned char * ch)
{
    return usart_ok;
}



