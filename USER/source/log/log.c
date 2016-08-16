#include "log/log.h"

//static UsartSta_t init(unsigned int baud); 
static UsartSta_t send(unsigned char * ch); 
static UsartSta_t receive(unsigned char * ch); 

static void rcc_log_init(void);
static void nvic_log_init(void);
static void gpio_log_init(void);
static void usart_log_init(void);

Log_t LogCreate(void)
{
    Log_t log = (Log_t)malloc(sizeof(LogStr_t));
    log->usart = UsartCreate();
    
    //log->usart->init = init;
    log->usart->send = send;
    log->usart->receive =receive;
    
    return log;
}
void DeleteCreate(Log_t log)
{
    if(NULL != log)
    {
        UsartDelete(log->usart);
        free(log);
        log = NULL;
    }
}


void log_init(void)
//static UsartSta_t init(unsigned int baud)
{
    rcc_log_init(); 
    nvic_log_init();
    gpio_log_init();
    usart_log_init();
    //return usart_ok;
}


PUTCHAR_PROTOTYPE
{
    USART_SendData(USART3, ch);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
    {}
    return ch;
}
static UsartSta_t send(unsigned char * ch)
{
    USART_SendData(USART3, *ch);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
    {}
  
    return usart_ok;
}
static UsartSta_t receive(unsigned char * ch)
{
    
    return usart_ok;
}

static void rcc_log_init(void)
{
    RCC_APB2PeriphClockCmd(USART3_GPIO_CLK , ENABLE);
    RCC_APB1PeriphClockCmd(USART3_CLK, ENABLE);
}

static void nvic_log_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
    /* Enable the USART3 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void gpio_log_init(void)
{          
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Configure USART3 Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = USART3_RxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART3_GPIO, &GPIO_InitStructure);    
    
    /* Configure USART3 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART3_TxPin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    
    GPIO_Init(USART3_GPIO, &GPIO_InitStructure);
}

static void usart_log_init(void)
{
    USART_InitTypeDef USART_InitStructure;
    
    USART_InitStructure.USART_BaudRate = 19200;               /*设置波特率为115200*/
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;/*设置数据位为8*/
    USART_InitStructure.USART_StopBits = USART_StopBits_1;     /*设置停止位为1位*/
    USART_InitStructure.USART_Parity = USART_Parity_No;        /*无奇偶校验*/
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;/*无硬件流控*/
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  /*发送和接收*/

    USART_Init(USART3, &USART_InitStructure);
    //USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    //USART_ITConfig(USART3, USART_IT_TXE, ENABLE);

    USART_Cmd(USART3, ENABLE); 
}

void USART3_IRQHandler()
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    { 
	while (USART_GetFlagStatus(USART3,USART_FLAG_TXE) == RESET);
    }
         
    if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {   
        USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
    }
}



