#ifndef _LOG_H_
#define _LOG_H_

#include "stdio.h"

#include "usart.h"

#include "stm32f10x.h"
#include "stm32_eval.h"


typedef struct
{
    Usart_t usart;   
}LogStr_t;

typedef LogStr_t * Log_t;

Log_t LogCreate(void);
void DeleteCreate(Log_t log);


#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#define USART3_GPIO              GPIOB
#define USART3_CLK               RCC_APB1Periph_USART3
#define USART3_GPIO_CLK          RCC_APB2Periph_GPIOB
#define USART3_RxPin             GPIO_Pin_11
#define USART3_TxPin             GPIO_Pin_10
#define USART3_IRQn              USART3_IRQn
#define USART3_IRQHandler        USART3_IRQHandler

void log_init(void);

#endif