/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"

    
    
#include "radio/physical.h"
#include "bluetooth.h"
/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
//  */

/** @addtogroup USART_Interrupt
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 extern unsigned char ble_alive_flag;   
 extern unsigned char  ble_rx_counter;
 extern unsigned char  Ble_send_flag;
 extern unsigned char  Msg_send_flag;
 extern  u8 USART2_RX_BUFF[USART2_BUFF_LEN];
 extern RingQueue_t ble_msg_queue_ptr;
//
//static uint16_t USART1_TXCounter;
//extern uint8_t USART1_TXBuffer[];
//extern uint8_t USART1_RXBuffer[];
//extern uint16_t USART1_RXCounter;
//extern bool USART1_RXFlag;
//
//static uint16_t USART2_TXCounter;
//extern uint8_t USART2_TXBuffer[];
//extern uint8_t USART2_RXBuffer[];
//extern uint16_t USART2_RXCounter;
//extern bool USART2_RXFlag;
//extern char RecFlag;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/


 
void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}
 
void PendSV_Handler(void)
{
}
 
void SysTick_Handler(void)
{
    static unsigned int counter = 0;
    static unsigned int ble_counter = 0;
    counter++;
    ble_counter++;
    if(ble_counter == 1500)//1.5s
    {
      ble_counter= 0;
      Msg_send_flag = 1;
    }
    if(counter == 15000)//15s
    {
      counter = 0;
      Ble_send_flag = 1;
    }
      
    DelayNmsCounter--;
  
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

char BeepSwitch = 0;
unsigned long int Delay8ms= 0;
unsigned long int Delay10ms= 0;
unsigned long int Delay300ms = 0;
unsigned long int Delay100ms = 0;
unsigned long int Delay2s = 0;
unsigned long int Delay400ms = 0;
static unsigned short LED_BLINK = 0;
char Check2sDelay( void )
{
    if(Delay2s >= 5000)return 1;
    else return 0;
}
void Clear2sDelay(void )
{
    Delay2s=0;
}

char Check100msDelay( void )
{
    if(Delay100ms >= 500)return 1;
    else return 0;
}
void Clear100msDelay(void )
{
    Delay100ms=0;
}

char Check8msDelay( void )
{
    if(Delay8ms >= 50)return 1;
    else return 0;
}
void Clear8msDelay(void )
{
    Delay8ms=0;
}
char Check10msDelay( void )
{
    if(Delay10ms >= 50)
     return 1;
}
void Clear10msDelay( void )
{
    Delay10ms=0;
}

char Check300msDelay( void )
{
    if(Delay300ms >= 1500)return 1;
    else return 0;
}
void Clear300msDelay(void )
{
   Delay300ms=0;
}


char Check400msDelay( void )
{
    if(Delay400ms >= 2000)return 1;
    else return 0;
}
void Clear400msDelay(void )
{
   Delay400ms=0;
}

//unsigned char test[1000];
unsigned short index;


void USART1_IRQHandler(void)
{
    /* error interrupt port */
    if((USART_GetITStatus(USART1, USART_IT_ORE) != RESET)
      ||(USART_GetITStatus(USART1, USART_IT_NE) != RESET)
      ||(USART_GetITStatus(USART1, USART_IT_FE) != RESET)
      ||(USART_GetITStatus(USART1, USART_IT_PE) != RESET))
    {
        USART_ReceiveData(USART1);
    }
    
    /* receive interrupt port */
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        
       unsigned char ch = USART_ReceiveData(USART1);  
       QueuePush(UsartRxQue, &ch);  
       //test[index] = ch;
       //index = (index + 1 >= 1000 ) ?0:( index + 1);
       
    }
    
    /* send interrupt port */
    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {      
        unsigned char ch = 0;  
        if(queue_ok == QueuePull(UsartTxQue, &ch))
        {
            USART_SendData(USART1, ch);
        }
        else
        { 
          USART_ITConfig(USART1, USART_IT_TXE, DISABLE); 
        }     
    }
}

/**
  * @brief  This function handles USART2 global interrupt request.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
  volatile u16 DMARxCounter = 0;
  u16 temp = 0;
    /* error interrupt port */
    if((USART_GetITStatus(USART2, USART_IT_ORE) != RESET)
      ||(USART_GetITStatus(USART2, USART_IT_NE) != RESET)
      ||(USART_GetITStatus(USART2, USART_IT_FE) != RESET)
      ||(USART_GetITStatus(USART2, USART_IT_PE) != RESET))
    {
        USART_ReceiveData(USART2);
    }
    
 #if 1
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)//空闲接收串口数据
    {
        DMA_Cmd(DMA1_Channel6,DISABLE);						//关闭DMA1通道6（USART2_RX） 
        temp = USART2->SR;	   //貌似根据手册说，先读SR，再读DR就可以清除IDLE位。。。。
        temp = USART2->DR;	        
        DMARxCounter = sizeof(USART2_RX_BUFF) - DMA_GetCurrDataCounter(DMA1_Channel6);	//缓冲器数量够大//用缓冲器的设定值-当前指针数值（寄存器内容在每次DMA传输后递减）=接收的数据长度。 
        printf("-------Usart2 rx counter: %d\n\r",DMARxCounter);      
        //DMA_GetInputBytes(USART2_RX_BUFF, DMARxCounter);
        
        bool ret = push_to_queue(ble_msg_queue_ptr, USART2_RX_BUFF, DMARxCounter);
        if(ret == false)
        {
          printf("-------Usart2 rx queue full !\n\r");
        }
        
        memset(USART2_RX_BUFF, 0x00, sizeof(USART2_RX_BUFF));
        //设置传输数据长度  
        DMA_SetCurrDataCounter(DMA1_Channel6, sizeof(USART2_RX_BUFF));//即是通道可容纳的最大数据量。           
        //重新打开DMA1_6（USART2_RX）  
        DMA_Cmd(DMA1_Channel6,ENABLE);
        
        DMARxCounter =0;  
    }
  
#else
    
    
    /* receive interrupt port */
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
      unsigned char ch = USART_ReceiveData(USART2);       
      if(QueuePush(BluetoothRxQue, &ch)== queue_ok)
      {
        
        ble_rx_counter++;
        
      }
    }
    
#endif
    /* send interrupt port */
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {      
        USART_ITConfig(USART2, USART_IT_TXE, DISABLE);             
    }
}


void BeepTurnOn( void )
{
    BeepSwitch = 1;    
    //GPIO_SetBits(BEEP_PORT, BEEP_Pin);
}
void BeepTurnOff( void )
{
    BeepSwitch = 0;    
    //GPIO_ResetBits(BEEP_PORT, BEEP_Pin);
}

void LedBlinkOn( unsigned short led )
{
    LED_BLINK |= led;
}
void LedBlinkOff( unsigned short led )
{
    LED_BLINK &= ~led;
}

void TIM2_IRQHandler(void)
{
//  static unsigned long int Counter = 0; 
//  static char str[8]={0,0,0,'.',0,0,0,0};
//  static char on = 0;
//  if(TIM_GetFlagStatus(TIM2, TIM_FLAG_Update) != RESET)
//  {
//      
//      TIM_ClearFlag(TIM2,TIM_FLAG_Update); 
//      Counter++;
//      
//      if(BeepSwitch)
//      {
//          if(GPIO_ReadOutputDataBit(BEEP_PORT, BEEP_Pin))GPIO_ResetBits(BEEP_PORT, BEEP_Pin);
//          else GPIO_SetBits(BEEP_PORT, BEEP_Pin);   
//      }   
//      
//      if((Counter % 1500 == 0) && (LED_BLINK))
//      {
//          if(on)
//          {
//              LedTurnOn(LED_BLINK);   
//              on = 0;
//          }
//          else
//          {
//              LedTurnOff(LED_BLINK);
//              on = 1;
//          }         
//      } 
//      
//      
//      if(Counter % 20 == 0)DsProcess();
//      
//      Delay8ms++;
//      Delay10ms++;
//      Delay300ms++;
//      Delay100ms++;
//      Delay2s++;
//      Delay400ms++;
//  }
//    if(Counter%5000 == 0)
//    {
//        str[0]=((Counter % 50000000) /5000000) +0x30;
//        str[2]=((Counter % 5000000) /500000) +0x30;
//        str[4]=((Counter % 500000) /50000) +0x30;
//        str[6]=((Counter % 50000) /5000) +0x30;
//        DispDS2(str);
//        DispDS1(&str[4]);
//    }
    
    
}



/**
  * @brief  This function handles TIM3 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
//  Ble_Message_Pro_t Message, * ble_Msg_ptr = &Message;  
//  
//  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
//  {
//    if(SUCCESS == ble_receive(ble_Msg_ptr))
//    {
//        printf("\r\n Ble_receive \r\n");
//        msg_send(ble_Msg_ptr);
//    }
//     memset(ble_Msg_ptr, 0x00, sizeof(Ble_Message_Pro_t));
//    
//   TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
//  }
 }

/******************************************************************************/


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

