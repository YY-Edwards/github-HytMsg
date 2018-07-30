#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#include "stm32f10x.h"
#include "stm32_eval.h"

#include "lib/myqueue.h"
#include "radio/message.h"
#define USART2_BUFF_LEN 128
#define USART2_GPIO              GPIOA
#define USART2_CLK               RCC_APB1Periph_USART2
#define USART2_GPIO_CLK          RCC_APB2Periph_GPIOA
#define USART2_RxPin             GPIO_Pin_3
#define USART2_TxPin             GPIO_Pin_2
#define USART2_IRQn              USART2_IRQn
#define USART2_IRQHandler        USART2_IRQHandler

#define BLE_ResetPin             GPIO_Pin_14
#define BLE_WkupPin              GPIO_Pin_9
#define BLE_GPIO                 GPIOB
#define BLE_GPIO_CLK             RCC_APB2Periph_GPIOB


extern Queue_t BluetoothRxQue;

void ble_init(void);
void ble_send(Message_t * msg);
unsigned char ble_receive(Message_t * msg);

void ble_send_ack(unsigned op);

typedef enum 
{
  FIND_START_HEADER_H =7,
  FIND_START_HEADER_L =1,
  HIGH_ADDR =2,
  LOW_ADDR =3,
  COMMAND =4,
  LENGTH = 5,
  READ_DATA =6//°üº¬2bytesµÄCRC
}PARSERSTATE;



#endif