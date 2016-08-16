#ifndef _PHYSICAL_H_
#define _PHYSICAL_H_

#include "string.h"
#include "stdio.h"

#include "lib/myqueue.h"
#include "stm32f10x_rcc.h"

#define SUCCESS 1
#define FAILURE 0


#if !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL) &&  !defined (USE_STM3210C_EVAL) &&  !defined (USE_STM32100B_EVAL)
 #define USE_STM3210B_EVAL
#endif

#define USART1_GPIO              GPIOA
#define USART1_CLK               RCC_APB2Periph_USART1
#define USART1_GPIO_CLK          RCC_APB2Periph_GPIOA
#define USART1_RxPin             GPIO_Pin_10
#define USART1_TxPin             GPIO_Pin_9
#define USART1_IRQn              USART1_IRQn
#define USART1_IRQHandler        USART1_IRQHandler


#define MAX_TX_DEEP 512
#define MAX_RX_DEEP 512

extern Queue_t UsartTxQue;
extern Queue_t UsartRxQue;

extern unsigned short DelayNmsCounter;

void physical_init( void );
void usart_send(void * pbuf, unsigned int length);
unsigned char usart_receive( unsigned char * dat );

unsigned short htons(unsigned short in);
unsigned int htonl(unsigned int in);
unsigned int Utf8ToUnicode(unsigned char * unicode, unsigned char * utf8, unsigned int utf8_bytescount);
void delaynms(unsigned short n);
#endif