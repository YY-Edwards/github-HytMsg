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
    
    GPIO_ResetBits(BLE_GPIO, BLE_ResetPin);//��λBLE:0
    delaynms(200);//��ʱ200ms
    GPIO_SetBits(BLE_GPIO, BLE_ResetPin);//����BLE�ĸ�λ��:1
    

    GPIO_ResetBits(BLE_GPIO, BLE_WkupPin);//����BLE:0
        
    
}

static void ble_usart_interface_init()
{
  
   GPIO_InitTypeDef GPIO_InitStructure;
   
  /*ʹ�ܴ���2,BLEģ��ʹ�õ�GPIOʱ��*/
   RCC_APB2PeriphClockCmd(USART2_GPIO_CLK | BLE_GPIO_CLK, ENABLE);

  /*ʹ�ܴ���2ʱ��*/
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
  
  GPIO_ResetBits(BLE_GPIO, BLE_ResetPin);//��λBLE:0
  delaynms(200);//��ʱ200ms
  GPIO_SetBits(BLE_GPIO, BLE_ResetPin);//����BLE�ĸ�λ��:1
  

  GPIO_ResetBits(BLE_GPIO, BLE_WkupPin);//����BLE:0
  
  /* USART2 configuration ------------------------------------------------------*/

  /* USART2 configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 9600;               /*���ò�����Ϊ9600*/
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;/*��������λΪ8*/
  USART_InitStructure.USART_StopBits = USART_StopBits_1;     /*����ֹͣλΪ1λ*/
  USART_InitStructure.USART_Parity = USART_Parity_No;        /*����żУ��*/
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;/*��Ӳ������*/
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  /*���ͺͽ���*/

  /*���ô���2 */
  USART_Init(USART2, &USART_InitStructure);
 
  /*ʹ�ܴ���2�Ľ����ж�*/
	
  
  USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
  USART_ITConfig(USART2, USART_IT_TC, DISABLE);
  USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);//�����жϣ�����δ֪���ȣ���ʹ�ÿ����ж����ж��Ƿ������ϣ� 
  USART_ClearFlag(USART2,USART_FLAG_IDLE); 				//��USART_FLAG_IDLE��־ 


  //����DMA��ʽ����  
  USART_DMACmd(USART2,USART_DMAReq_Rx ,ENABLE);
    
  //����DMA1(channel1~7)��ʱ��
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  /* DMA1 channel 6 configuration */ //USART2_RX  
  DMA_DeInit(DMA1_Channel6);  
  DMA_InitStructure.DMA_PeripheralBaseAddr      =(u32)(&USART2->DR);  			//���贮��2��ַ  
 
  DMA_InitStructure.DMA_MemoryBaseAddr          =(u32)USART2_RX_BUFF;
  DMA_InitStructure.DMA_DIR                     =DMA_DIR_PeripheralSRC;   	//������ΪĿ�ĵ�ַ   //DMA_DIR_PeripheralSRC;   //������ΪDMA��Դ��  
  DMA_InitStructure.DMA_BufferSize              =USART2_BUFF_LEN; 				//BufferSize;      //���仺������С 
  DMA_InitStructure.DMA_PeripheralInc           =DMA_PeripheralInc_Disable; 	//�������ģʽ��ֹ   DMA_PeripheralInc_Enable;            //�����ַ����  
  DMA_InitStructure.DMA_MemoryInc               =DMA_MemoryInc_Enable;   	//�ڴ��ַ����  
  DMA_InitStructure.DMA_PeripheralDataSize      =DMA_PeripheralDataSize_Byte; 	//���䷽ʽ���ֽ�   DMA_PeripheralDataSize_Word;    //�֣�32λ��  
  DMA_InitStructure.DMA_MemoryDataSize          =DMA_MemoryDataSize_Byte;  	//�ڴ�洢��ʽ���ֽ�  DMA_MemoryDataSize_Word;  
  DMA_InitStructure.DMA_Mode                    =DMA_Mode_Normal;  		//DMA_Mode_Normal ����ģʽ��ֻ����һ��;  DMA_Mode_Circular:ѭ��ģʽ����ͣ�Ĵ���;  
  DMA_InitStructure.DMA_Priority                =DMA_Priority_VeryHigh;  	//����1�Ľ�����Ϊ������ȼ�
  DMA_InitStructure.DMA_M2M                     =DMA_M2M_Disable;             	//DMA_M2M_Enable;      
  DMA_Init(DMA1_Channel6,&DMA_InitStructure); 

	//ʹ��ͨ��6  
  DMA_Cmd(DMA1_Channel6,ENABLE);  
  
  
  /* Enable the USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//�����ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* ʹ�ܴ���2 */
  USART_Cmd(USART2, ENABLE);
  USART_ClearFlag(USART2,USART_FLAG_TC); //���USART_FLAG_TC�������һ���ֽڲ��ܷ��������� 
  while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {}

}

static void usart_ble_init(void)
{
    USART_InitTypeDef USART_InitStructure;
    
    USART_InitStructure.USART_BaudRate = 9600;               /*���ò�����Ϊ115200*/
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;/*��������λΪ8*/
    USART_InitStructure.USART_StopBits = USART_StopBits_1;     /*����ֹͣλΪ1λ*/
    USART_InitStructure.USART_Parity = USART_Parity_No;        /*����żУ��*/
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;/*��Ӳ������*/
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  /*���ͺͽ���*/

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    //USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

    USART_Cmd(USART2, ENABLE); 
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {}
}

void ble_init(void)
{
  //����usart2DMA�����жϽ��������߼�
#if 1
  
  ble_usart_interface_init();//�������ڽӿڳ�ʼ��
  
#else
  
    rcc_ble_init(); 
    nvic_ble_init();
    gpio_ble_init();
    usart_ble_init();
    
#endif
  
    
    BluetoothRxQue = QueueCreate(80, 1); //��������ʱ����ע��
    
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
        delaynms(300);//һ֡���ݳ���68bytes����Լ��Ҫ����70ms
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
   
   
   /************���ԡ�***************/
   
   if( sizeof(MessageHeader_t) + 2 <= Counter)
   {
     //�յ������ݳ���Э��������ݰ����ȣ�68bytes��Ҳ�ǿ���ͨ����
     //if((Counter >= sizeof(MessageHeader_t) + msg->Header.Length + 2) && (Msg_Header == htons(msg->Header.Header)) )
     
     
     //��BLE����0xfffe����OB�����յ�0xff,Ȼ���յ�0xfe.
      /*IAR��С�˴洢����,��0xff ��ǰ��0xfe�ں󣬼��洢����Ϊ0xfeff,�����Ҫ��С��ת��*/
     //˫�ֽ����ݶ�Ӧ����ͬ����
     
     /*Ϊ����У���룬���յ�����ҪУ������ݲ�����С�˱任�����¸�ֵ����*/
     //��Ϊ����������С�˱仯���Ӱ��У�麯���Ľ��.
     //У�����ݰ�������ַ��������Ⱥ�����
     
     if((Counter <= sizeof(Message_t)) && (Msg_Header == htons(msg->Header.Header)))
      {
            
            switch(msg->Header.Opcode)
            {
              
              case MSG_DATA://0x01
                  
                      unsigned short local_rxcheck = msg_checksum(msg);//MODBUS����CRC
                      //unsigned short rxcheck = *(unsigned short *)((unsigned char *)msg +  sizeof(MessageHeader_t) + msg->Header.Length);
                       msg->Checksum =   *(unsigned short *)((unsigned char *)msg +  sizeof(MessageHeader_t) + msg->Header.Length);
                           
                      //if(htons(msg->Checksum) == local_rxcheck) //У���в��� 
                      if(htons(msg->Checksum))
                      {
                          ble_rx_counter =0;
                          Counter = 0;
                          ble_send_ack(MSG_ACK);
                          return SUCCESS;//�ɹ�
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
   
  /************���ԡ�***************/ 
   
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
     //��С��bug,��ʱ����ʱ��Ĭ��Э���ʽΪ���ģʽ��
     ack.Header.Header = htons(Msg_Header);
     ack.Header.Address = 0; 
     ack.Header.Length = 0;
     ack.Header.Opcode = op;
     ack.Checksum = msg_checksum((Message_t *)&ack);
     
     ble_send((Message_t *)&ack);
}
