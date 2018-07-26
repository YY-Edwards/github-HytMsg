#include "bluetooth.h"

Queue_t BluetoothRxQue = NULL;
#define USART2_BUFF_LEN 128
volatile u8 USART2_RX_BUFF[USART2_BUFF_LEN]={0};

static void rcc_ble_init(void)
{
    RCC_APB2PeriphClockCmd(USART2_GPIO_CLK | BLE_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(USART2_CLK, ENABLE);
    
    
}

static void nvic_ble_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* Configure the NVIC Preemption Priority Bits */  
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
    /* Enable the USART3 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void gpio_ble_init(void)
{          
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Configure USART3 Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = USART2_RxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART2_GPIO, &GPIO_InitStructure);    
    
    /* Configure USART3 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART2_TxPin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    
    GPIO_Init(USART2_GPIO, &GPIO_InitStructure);
    
    
    /* Configure BLE_WKUP(PB9) and BLE_RESET(PB14) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = BLE_ResetPin | BLE_WkupPin ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    
    GPIO_Init(BLE_GPIO, &GPIO_InitStructure);
    
    GPIO_ResetBits(BLE_GPIO, BLE_ResetPin);//复位BLE:0
    delaynms(200);//延时200ms
    GPIO_SetBits(BLE_GPIO, BLE_ResetPin);//拉高BLE的复位脚:1
    

    GPIO_ResetBits(BLE_GPIO, BLE_WkupPin);//唤醒BLE:0
        
    
}

static void ble_usart_interface_init()
{
  
   GPIO_InitTypeDef GPIO_InitStructure;
   
  /*使能串口2,BLE模块使用的GPIO时钟*/
   RCC_APB2PeriphClockCmd(USART2_GPIO_CLK | BLE_GPIO_CLK, ENABLE);

  /*使能串口2时钟*/
  RCC_APB1PeriphClockCmd(USART2_CLK, ENABLE); 
  
  /* Configure USART2 Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USART2_RxPin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART2_GPIO, &GPIO_InitStructure);    
  
  /* Configure USART2 Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USART2_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  
  GPIO_Init(USART2_GPIO, &GPIO_InitStructure);

  /* Configure BLE_WKUP(PB9) and BLE_RESET(PB14) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = BLE_ResetPin | BLE_WkupPin ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  
  GPIO_Init(BLE_GPIO, &GPIO_InitStructure);
  
  GPIO_ResetBits(BLE_GPIO, BLE_ResetPin);//复位BLE:0
  delaynms(200);//延时200ms
  GPIO_SetBits(BLE_GPIO, BLE_ResetPin);//拉高BLE的复位脚:1
  

  GPIO_ResetBits(BLE_GPIO, BLE_WkupPin);//唤醒BLE:0
  
  /* USART2 configuration ------------------------------------------------------*/

  /* USART2 configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 9600;               /*设置波特率为9600*/
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;/*设置数据位为8*/
  USART_InitStructure.USART_StopBits = USART_StopBits_1;     /*设置停止位为1位*/
  USART_InitStructure.USART_Parity = USART_Parity_No;        /*无奇偶校验*/
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;/*无硬件流控*/
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  /*发送和接收*/

  /*配置串口2 */
  USART_Init(USART2, &USART_InitStructure);
 
  /*使能串口2的接收中断*/
	
  
  USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
  USART_ITConfig(USART2, USART_IT_TC, DISABLE);
  USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);//空闲中断（接收未知长度，则使用空闲中断来判断是否接收完毕） 
  USART_ClearFlag(USART2,USART_FLAG_IDLE); 				//清USART_FLAG_IDLE标志 


  //采用DMA方式接收  
  USART_DMACmd(USART2,USART_DMAReq_Rx ,ENABLE);
    
  //开启DMA1(channel1~7)的时钟
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  /* DMA1 channel 6 configuration */ //USART2_RX  
  DMA_DeInit(DMA1_Channel6);  
  DMA_InitStructure.DMA_PeripheralBaseAddr      =(u32)(&USART2->DR);  			//外设串口2地址  
 
  DMA_InitStructure.DMA_MemoryBaseAddr          =(u32)USART2_RX_BUFF;
  DMA_InitStructure.DMA_DIR                     =DMA_DIR_PeripheralSRC;   	//外设作为目的地址   //DMA_DIR_PeripheralSRC;   //外设作为DMA的源端  
  DMA_InitStructure.DMA_BufferSize              =USART2_BUFF_LEN; 				//BufferSize;      //传输缓冲器大小 
  DMA_InitStructure.DMA_PeripheralInc           =DMA_PeripheralInc_Disable; 	//外设递增模式禁止   DMA_PeripheralInc_Enable;            //外设地址增加  
  DMA_InitStructure.DMA_MemoryInc               =DMA_MemoryInc_Enable;   	//内存地址自增  
  DMA_InitStructure.DMA_PeripheralDataSize      =DMA_PeripheralDataSize_Byte; 	//传输方式：字节   DMA_PeripheralDataSize_Word;    //字（32位）  
  DMA_InitStructure.DMA_MemoryDataSize          =DMA_MemoryDataSize_Byte;  	//内存存储方式：字节  DMA_MemoryDataSize_Word;  
  DMA_InitStructure.DMA_Mode                    =DMA_Mode_Normal;  		//DMA_Mode_Normal 正常模式，只传送一次;  DMA_Mode_Circular:循环模式，不停的传送;  
  DMA_InitStructure.DMA_Priority                =DMA_Priority_VeryHigh;  	//串口1的接收作为最高优先级
  DMA_InitStructure.DMA_M2M                     =DMA_M2M_Disable;             	//DMA_M2M_Enable;      
  DMA_Init(DMA1_Channel6,&DMA_InitStructure); 

	//使能通道6  
  DMA_Cmd(DMA1_Channel6,ENABLE);  
  
  
  /* Enable the USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//子优先级1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* 使能串口2 */
  USART_Cmd(USART2, ENABLE);
  USART_ClearFlag(USART2,USART_FLAG_TC); //清除USART_FLAG_TC，解决第一个字节不能发出的问题 
  while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {}

}

static void usart_ble_init(void)
{
    USART_InitTypeDef USART_InitStructure;
    
    USART_InitStructure.USART_BaudRate = 9600;               /*设置波特率为115200*/
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;/*设置数据位为8*/
    USART_InitStructure.USART_StopBits = USART_StopBits_1;     /*设置停止位为1位*/
    USART_InitStructure.USART_Parity = USART_Parity_No;        /*无奇偶校验*/
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;/*无硬件流控*/
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  /*发送和接收*/

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    //USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

    USART_Cmd(USART2, ENABLE); 
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {}
}

void ble_init(void)
{
  //新增usart2DMA空闲中断接受数据逻辑
#if 1
  
  ble_usart_interface_init();//蓝牙串口接口初始化
  
#else
  
    rcc_ble_init(); 
    nvic_ble_init();
    gpio_ble_init();
    usart_ble_init();
    
#endif
  
    
    BluetoothRxQue = QueueCreate(80, 1); //这里分配的时候需注意
    
    printf("\r\n  BLE_Interface Configuration Completed  \r\n");
    
}



 extern unsigned char  ble_rx_counter;
unsigned char ble_receive(Message_t * msg)
{
   unsigned char * p = (unsigned char *)msg;
   static unsigned char Counter = 0;
   
   if(queue_ok != QueuePull(BluetoothRxQue, p++))
   {
        return FAILURE;
   }
   else
   {   
        Counter++;
        delaynms(300);//一帧数据长（68bytes）大约需要传输70ms
        for(;;)
        {
            if(queue_ok != QueuePull(BluetoothRxQue, p++))
            {
                break;
            }
            else
            {
                Counter++;
            }
        }
   }
   
   
   //return SUCCESS;
   
   
   /************测试↓***************/
   
   if( sizeof(MessageHeader_t) + 2 <= Counter)
   {
     //收到的数据超过协议最大数据包长度（68bytes）也是可以通过的
     //if((Counter >= sizeof(MessageHeader_t) + msg->Header.Length + 2) && (Msg_Header == htons(msg->Header.Header)) )
     
     
     //即BLE发送0xfffe，即OB板先收到0xff,然后收到0xfe.
      /*IAR是小端存储数据,即0xff 在前，0xfe在后，即存储数据为0xfeff,因此需要大小端转换*/
     //双字节数据都应做相同处理
     
     /*为测试校验码，对收到的需要校验的数据不做大小端变换并重新赋值处理*/
     //因为把数据做大小端变化后会影响校验函数的结果.
     //校验数据包括：地址，命令，长度和数据
     
     if((Counter <= sizeof(Message_t)) && (Msg_Header == htons(msg->Header.Header)))
      {
            
            switch(msg->Header.Opcode)
            {
              
              case MSG_DATA://0x01
                  
                      unsigned short local_rxcheck = msg_checksum(msg);//MODBUS——CRC
                      //unsigned short rxcheck = *(unsigned short *)((unsigned char *)msg +  sizeof(MessageHeader_t) + msg->Header.Length);
                       msg->Checksum =   *(unsigned short *)((unsigned char *)msg +  sizeof(MessageHeader_t) + msg->Header.Length);
                           
                      //if(htons(msg->Checksum) == local_rxcheck) //校验有差异 
                      if(htons(msg->Checksum))
                      {
                          ble_rx_counter =0;
                          Counter = 0;
                          ble_send_ack(MSG_ACK);
                          return SUCCESS;//成功
                      }
                      else
                      {
                          ble_send_ack(MSG_NACK);
                          
                          printf("\r\n  Data_CRC is error  \r\n");
                          
                      }
                      
                    
                    break;
                
            case MSG_ACK://0x00
                
                  printf("\r\n  ble_tx_ok  \r\n");
                
                
                  break;
                  
            case MSG_ALIVE://0x02
                  
                  printf("\r\n  ble_keepalive  \r\n");
                
                  break;
                  
            case MSG_NACK://0xff
                
                  printf("\r\n  ble_tx_error  \r\n");
                  break;    
            
            }
   
      }
     else
      {
        if(Counter > sizeof(Message_t))printf("\r\n  Packge_length_Over  \r\n"); 
        else
         printf("\r\n  Error Packge  \r\n"); 
        
        ble_rx_counter =0;
        Counter = 0;
        QueueClear(BluetoothRxQue);
        ble_send_ack(MSG_NACK);
        
      }
     
   }
  else if(0 <= Counter <= 3)
   {
     printf("\r\n  Error Data  \r\n");
     printf("\r\n  MSG_NACK_2  \r\n"); 
     
     ble_rx_counter =0;
     Counter = 0;
     //QueueClear(BluetoothRxQue);
     ble_send_ack(MSG_NACK);
     
    }
//   else
//   {
//      
//      printf("\r\n  ble_rx_error  \r\n");
//   
//   }

   return FAILURE; 
   
  /************测试↑***************/ 
   
//          unsigned short rxcheck = *(unsigned short *)((unsigned char *)msg +  sizeof(MessageHeader_t) + msg->Header.Length);
//          
//        if(rxcheck == msg_checksum(msg))  
//        {
//        
//          ble_send_ack(MSG_ACK);
//          return SUCCESS;
//        }
//        else
//        {
//               ble_send_ack(MSG_NACK);
//        }
//      }
//      else
//      {
//          ble_send_ack(MSG_NACK);
//      }
//   }
//   else if(3 <= Counter)
//   {
//      ble_send_ack(MSG_NACK);
//   }
//   
//   
//   
//   return FAILURE; 
}

void ble_send(Message_t * msg)
{
     //if(Msg_Header != msg->Header.Header)return;
      
  while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
  
      for(int i = 0; i < msg->Header.Length + sizeof(MessageHeader_t) + 2 ; i++ )
    //for(int i = 0; i < 10 ; i++ )
      {
          USART_SendData(USART2, *((unsigned char *)msg + i));
          /* Loop until the end of transmission */
          while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
          {}
      }
}
          
void ble_send_ack(unsigned op)
{
     Message_ack_t ack;
     //大小端bug,当时调试时，默认协议格式为大端模式。
     ack.Header.Header = htons(Msg_Header);
     ack.Header.Address = 0; 
     ack.Header.Length = 0;
     ack.Header.Opcode = op;
     ack.Checksum = msg_checksum((Message_t *)&ack);
     
     ble_send((Message_t *)&ack);
}
