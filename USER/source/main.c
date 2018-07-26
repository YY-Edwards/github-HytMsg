/*******************************************************************************
* File Name       main.c
* Anthor          Edwards     
* Version         V2.00
* Date            2016/08/22
* Description     --目前的版本是比较稳定的版本，暂无任何异常或硬件错误，待测试    
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <wchar.h>

#include "stm32f10x.h"

#include "main.h"
#include "radio/message.h"
#include "app.h"


/*******************************************************************************
* Function Nmae     main
* Date              2013/7/30
* Anthor            WXJ
* Input Parameter   None
* Output Parameter  None
* Description       --
* File              main.c
*******************************************************************************/

#define SYSTEM_CLOCK 72000000


unsigned char ble_alive_flag = 0;
unsigned char  ble_rx_counter = 0;
unsigned char  Ble_send_flag = 0;
unsigned char Ble_alive_counter = 0;


int main ( void )
{  
    SysTick_Config(SYSTEM_CLOCK / 1000);  //1ms   
  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//配置中断优先组，统一一次性配置。
    
    log_init();
    printf("log initialize finished\r\n");
    
    
    msg_init();
    ble_init();  

    Message_t Message, * Msg = &Message;
    
    for(;;)
    {        
        if(SUCCESS == ble_receive(Msg))
        {
            printf("\r\n Ble_receive \r\n");
            msg_send(Msg);
        }
        
        if(SUCCESS == msg_receive(Msg))
        {
            ble_send(Msg);
        }
        
      if(Ble_send_flag)
      {
          Ble_send_flag = 0;
          Ble_alive_counter++;
          if(ble_alive_flag)printf("\r\n B_alive:%d \r\n", Ble_alive_counter);
        
      }
      memset(Msg, 0x00, sizeof(Message_t));//clear buff

    } 
    while(1);
      
    return 0;
} /* End of main */             
/***************************** End of File ************************************/

//
//PUTCHAR_PROTOTYPE
//{
//    
//
//    return ch;
//} /* End of PUTCHAR_PROTOTYPE */