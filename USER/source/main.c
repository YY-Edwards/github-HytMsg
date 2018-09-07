/*******************************************************************************
* File Name       main.c
* Anthor          Edwards     
* Version         V2.00
* Date            2016/08/22
* Description     --Ŀǰ�İ汾�ǱȽ��ȶ��İ汾�������κ��쳣��Ӳ�����󣬴�����    
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
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж������飬ͳһһ�������á�
    SysTick_Config(SYSTEM_CLOCK / 1000);  //1ms   
    //NVIC_SetPriority(SysTick_IRQn,0);//���ж����ȼ�����û��ߣ�����ʲô�顣    
    log_init();
    printf("log initialize finished\r\n");   
    msg_init();
    ble_init();  
    
    //TIM3_Int_Init(9999,7199);//timer3,��ʱ1s
    Ble_Message_Pro_t Message, * Msg = &Message;    
    
    for(;;)
    { 
      if(Msg_send_flag)//���ŷ��Ͷ�ʱ����
      {
        Msg_send_flag = 0;
        if(SUCCESS == ble_receive(Msg))//�������ģ���յ�����
        {
            printf("\r\n Ble_receive \r\n");
            msg_send(Msg);//��̨����
        }
      }
      
      if(SUCCESS == msg_receive(Msg))//�����̨�ж�������
      {
          ble_send(Msg);//��������
      }
        
      
       if(ble_alive_flag)//����ʹ��
      {              
        if(Ble_send_flag)//��ʱ���������Խ������ŵ����Ҫ���ϸ�������
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


//ͨ�ö�ʱ�� 3 �жϳ�ʼ��
//����ʱ��ѡ��Ϊ APB1 �� 2 ������ APB1 Ϊ 36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ�� 3!
//void TIM3_Int_Init(u16 arr,u16 psc)
//{
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //��ʱ�� TIM3 ʹ��
//  //��ʱ�� TIM3 ��ʼ��
//  TIM_TimeBaseStructure.TIM_Period = arr; //�����Զ���װ�ؼĴ������ڵ�ֵ
//  TIM_TimeBaseStructure.TIM_Prescaler =psc; //����ʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
//  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�
//  
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM ���ϼ���
//  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //�ڳ�ʼ�� TIM3
//  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //����������ж�
//  //�ж����ȼ� NVIC ����
//  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //TIM3 �ж�
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //��ռ���ȼ� 3 ��
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //�����ȼ� 1��
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ ͨ����ʹ��
//  NVIC_Init(&NVIC_InitStructure); //�ܳ�ʼ�� NVIC �Ĵ���
//  
//  TIM_Cmd(TIM3, ENABLE); //��ʹ�� TIM3
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