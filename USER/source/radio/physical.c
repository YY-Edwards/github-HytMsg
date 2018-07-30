#include "radio/physical.h"


 Queue_t UsartTxQue = NULL;
 Queue_t UsartRxQue = NULL;

unsigned short DelayNmsCounter = 0;

static void rcc_hrnp_init(void)
{
    RCC_APB2PeriphClockCmd(USART1_GPIO_CLK , ENABLE);
    RCC_APB2PeriphClockCmd(USART1_CLK, ENABLE); 
}

static void nvic_hrnp_init(void)
{
     NVIC_InitTypeDef NVIC_InitStructure;
    
    /* Configure the NVIC Preemption Priority Bits */  
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void gpio_hrnp_init(void)
{          
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Configure USART1 Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = USART1_RxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART1_GPIO, &GPIO_InitStructure);    
    
    /*串口1 TX管脚配置*/ 
    /* Configure USART1 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART1_TxPin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    
    GPIO_Init(USART1_GPIO, &GPIO_InitStructure);
}


static void usart_hrnp_init(void)
{
    
  //USART_OverSampling8Cmd
    USART_InitTypeDef USART_InitStructure;
    
    USART_Cmd(USART1, DISABLE); 
    
    USART_OverSampling8Cmd(USART1, ENABLE);  
    
    USART_InitStructure.USART_BaudRate = 9600;               /*设置波特率为115200*/
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;/*设置数据位为8*/
    USART_InitStructure.USART_StopBits = USART_StopBits_1;     /*设置停止位为1位*/
    USART_InitStructure.USART_Parity = USART_Parity_No;        /*无奇偶校验*/
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;/*无硬件流控*/
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  /*发送和接收*/

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    
//        /* Enable Parity Error interrupt */
//    USART_ITConfig(USART1, USART_IT_PE, ENABLE);
//    /* Enable Error interrupt */
//    USART_ITConfig(USART1, USART_IT_ERR, ENABLE);
//    /* Enable Overrun interrupt */
//    USART_ITConfig(USART1, USART_IT_ORE, ENABLE);
//    /* Enable Noise detected Flag interrupt */
//    USART_ITConfig(USART1, USART_IT_NE, ENABLE);
//    /* Enable Framing Error interrupt */
//    USART_ITConfig(USART1, USART_IT_FE, ENABLE);

    USART_Cmd(USART1, ENABLE); 
}

void physical_init( void )
{
    rcc_hrnp_init(); 
    nvic_hrnp_init();
    gpio_hrnp_init();
    usart_hrnp_init();
    
    if(UsartTxQue!= NULL)
    {
      QueueDelete(UsartTxQue);
      UsartTxQue = NULL;
    }
    UsartTxQue = QueueCreate(MAX_TX_DEEP, 1);
    
    if(UsartRxQue!= NULL)
    {
      QueueDelete(UsartRxQue);
      UsartRxQue = NULL;
    }
    UsartRxQue = QueueCreate(MAX_RX_DEEP, 1);   
}

void usart_send(void * pbuf, unsigned int length)
{
      
    if((NULL == pbuf)||(0 >= length))
    {
        return;
    }
    
    for(int i = 0 ; i < length; i++)
    {
        if(queue_ok != QueuePush(UsartTxQue, (unsigned char *)pbuf + i))
        {
          printf("[QueuePush UsartTxQue failure !]\n");
          break;
        }
    }
       
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    
    while(0 < UsartTxQue->count);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

unsigned char usart_receive( unsigned char * dat )
{ 
    if(queue_ok == QueuePull(UsartRxQue, dat))
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}




void delaynms(unsigned short n)
{
    DelayNmsCounter = n;  
    while(DelayNmsCounter);
}
              

