/**
  ******************************************************************************
  * @file    USART/Interrupt/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
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

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    static unsigned int counter = 0;
    counter++;
    if(counter == 5000)//1000ms
    {
      counter = 0;
      if(ble_alive_flag)ble_send_ack(MSG_ALIVE);
      
    }
    
    
    
    DelayNmsCounter--;
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles USART1 global interrupt request.
  * @param  None
  * @retval None
  */

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

unsigned char test[1000];
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
       test[index] = ch;
       index = (index + 1 >= 1000 ) ?0:( index + 1);
       
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
    /* error interrupt port */
    if((USART_GetITStatus(USART2, USART_IT_ORE) != RESET)
      ||(USART_GetITStatus(USART2, USART_IT_NE) != RESET)
      ||(USART_GetITStatus(USART2, USART_IT_FE) != RESET)
      ||(USART_GetITStatus(USART2, USART_IT_PE) != RESET))
    {
        USART_ReceiveData(USART2);
    }
    
    /* receive interrupt port */
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
      unsigned char ch = USART_ReceiveData(USART2);       
      if(QueuePush(BluetoothRxQue, &ch)== queue_ok)
      {
        
        ble_rx_counter++;
        
      }
    }
    
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

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
