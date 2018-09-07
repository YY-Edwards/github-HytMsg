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
#include "radio/message.h"
#include "bluetooth.h"
#include "app.h"

void TIM3_Int_Init(u16 arr,u16 psc);

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
unsigned char  Msg_send_flag = 0;
unsigned char Ble_alive_counter = 0;


int main ( void )
{  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//配置中断优先组，统一一次性配置。
    SysTick_Config(SYSTEM_CLOCK / 1000);  //1ms   
    //NVIC_SetPriority(SysTick_IRQn,0);//讲中断优先级设置没最高，无论什么组。    
    log_init();
    printf("log initialize finished\r\n");   
    msg_init();
    ble_init();  
    
    //TIM3_Int_Init(9999,7199);//timer3,定时1s
    Ble_Message_Pro_t Message, * Msg = &Message;    
    
    for(;;)
    { 
      if(Msg_send_flag)//短信发送定时触发
      {
        Msg_send_flag = 0;
        if(SUCCESS == ble_receive(Msg))//如果蓝牙模块收到数据
        {
            printf("\r\n Ble_receive \r\n");
            msg_send(Msg);//手台发送
        }
      }
      
      if(SUCCESS == msg_receive(Msg))//如果手台有短信数据
      {
          ble_send(Msg);//蓝牙发送
      }
        
      
       if(ble_alive_flag)//心跳使能
      {              
        if(Ble_send_flag)//定时触发，可以将心跳放到这里，要求不严格的情况下
        {
            Ble_send_flag = 0;
            ble_send_ack(MSG_ALIVE);
            Ble_alive_counter++;
            if(ble_alive_flag)printf("\r\n B_alive:%d \r\n", Ble_alive_counter);
          
        }
      }
      memset(Msg, 0x00, sizeof(Ble_Message_Pro_t));//clear buff

    } 
} /* End of main */          


//通用定时器 3 中断初始化
//这里时钟选择为 APB1 的 2 倍，而 APB1 为 36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器 3!
//void TIM3_Int_Init(u16 arr,u16 psc)
//{
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //①时钟 TIM3 使能
//  //定时器 TIM3 初始化
//  TIM_TimeBaseStructure.TIM_Period = arr; //设置自动重装载寄存器周期的值
//  TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置时钟频率除数的预分频值
//  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割
//  
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM 向上计数
//  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //②初始化 TIM3
//  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //③允许更新中断
//  //中断优先级 NVIC 设置
//  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //TIM3 中断
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //先占优先级 3 级
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //从优先级 1级
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ 通道被使能
//  NVIC_Init(&NVIC_InitStructure); //④初始化 NVIC 寄存器
//  
//  TIM_Cmd(TIM3, ENABLE); //⑤使能 TIM3
//  
//}








/***************************** End of File ************************************/

//
//PUTCHAR_PROTOTYPE
//{
//    
//
//    return ch;
//} /* End of PUTCHAR_PROTOTYPE */