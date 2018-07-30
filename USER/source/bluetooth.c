#include "bluetooth.h"

Queue_t BluetoothRxQue = NULL;
u8 USART2_RX_BUFF[USART2_BUFF_LEN]={0};
RingQueue_t ble_msg_queue_ptr = NULL;

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
  USART_InitTypeDef USART_InitStructure;
  DMA_InitTypeDef  DMA_InitStructure; 
  NVIC_InitTypeDef NVIC_InitStructure;
 
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
  
  //GPIO_ResetBits(BLE_GPIO, BLE_ResetPin);//��λBLE:0
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
  
  if(ble_msg_queue_ptr!=NULL)
  {
     free(ble_msg_queue_ptr);
     ble_msg_queue_ptr =NULL;
  }
  ble_msg_queue_ptr = malloc(sizeof(ring_queue_t));
  if(ble_msg_queue_ptr ==NULL)
  {
    printf("malloc ble_msg_queue_ptr failure\r\n");
    return ;
  }
  init_queue(ble_msg_queue_ptr);
#else
  
    rcc_ble_init(); 
    nvic_ble_init();
    gpio_ble_init();
    usart_ble_init();
    
    BluetoothRxQue = QueueCreate(80, 1); //��������ʱ����ע��
    
 #endif
    
    printf("\r\n  BLE_Interface Configuration Completed  \r\n");
    
}



 extern unsigned char  ble_rx_counter;
u8 g_usart_recv_buf[200]={0};//ȫ�ֱ���
Message_t g_rx_usart2_msg;
unsigned char ble_receive(Message_t * msg)
{
  
#if 1
  bool parse_ret =false;
  static PARSERSTATE m_parser_state = FIND_START_HEADER_H;
  static int usart2_recv_msg_len = 0;
  static int usart2_recv_msg_idx = 0;
  int recv_len =0; 
  memset(g_usart_recv_buf, 0x00, sizeof(g_usart_recv_buf));
  bool ret = take_from_queue(ble_msg_queue_ptr, &g_usart_recv_buf[0], &recv_len, true);
  if(ret != true)return FAILURE;
  int index =0;
  int bytecount =0;
  u8 ch =0;
  while(0 < recv_len--)
  {
    bytecount++;
    ch = g_usart_recv_buf[index++];
    switch(m_parser_state)
    {
      case FIND_START_HEADER_H:
            if(ch==0xFF){
               g_rx_usart2_msg.Header.Header = ch;
               m_parser_state = FIND_START_HEADER_L;
            }
        break;
      case FIND_START_HEADER_L:
            g_rx_usart2_msg.Header.Header = ((g_rx_usart2_msg.Header.Header<<8)&0xff00) | ch;
            if(g_rx_usart2_msg.Header.Header == Msg_Header)
               m_parser_state = HIGH_ADDR;
            else
              m_parser_state = FIND_START_HEADER_H;
        
        break;
      case HIGH_ADDR:
          g_rx_usart2_msg.Header.Address = ch;    
          m_parser_state = LOW_ADDR;
        break;
      case LOW_ADDR:
          g_rx_usart2_msg.Header.Address = ((g_rx_usart2_msg.Header.Address<<8)&0xff00) | ch;   
          m_parser_state = COMMAND;
        
        break;
      case COMMAND:
          g_rx_usart2_msg.Header.Opcode = ch;    
          m_parser_state = LENGTH;
        
        break;
      case LENGTH:
          g_rx_usart2_msg.Header.Length = ch; 
          usart2_recv_msg_len = ch;
          if(usart2_recv_msg_len > 60)
          {
            usart2_recv_msg_len = 60;
            ble_send_ack(MSG_NACK);                       
            printf("Usart2 recv data msg length is error  \r\n");
          }
          usart2_recv_msg_len +=2;//��Ҫ�����������crcУ������
          usart2_recv_msg_idx = 0;
          m_parser_state = READ_DATA;//get rest of msg
        
        break;
      case READ_DATA:
          g_rx_usart2_msg.Payload[usart2_recv_msg_idx++] = ch;
          if((--usart2_recv_msg_len) == 0)
          {
            //get crc value
            g_rx_usart2_msg.Checksum = 
              (((g_rx_usart2_msg.Payload[usart2_recv_msg_idx-2]<<8) &0xff00) | g_rx_usart2_msg.Payload[usart2_recv_msg_idx-1]);      
            
            usart2_recv_msg_len = g_rx_usart2_msg.Header.Length + 6+ 2;//ble��usart2֮��ͬ��Э����ܳ��ȡ�
            
            //if(msg_checksum(&g_rx_usart2_msg) ==  g_rx_usart2_msg.Checksum)//У��ͨ��
            if(msg_checksum(&g_rx_usart2_msg)) 
            {
              if(g_rx_usart2_msg.Header.Opcode == MSG_DATA)
              {
                  ble_send_ack(MSG_ACK);
                  memcpy(msg, (void *)&g_rx_usart2_msg, usart2_recv_msg_len);
                  parse_ret = SUCCESS;
              }
              else if(g_rx_usart2_msg.Header.Opcode == MSG_ACK)
              {
                printf("Usart2 send msg okay.  \r\n");
              }
              else if(g_rx_usart2_msg.Header.Opcode == MSG_ALIVE)
              {
                printf("Usart2 should not recv this msg!  \r\n");
              }
              else if(g_rx_usart2_msg.Header.Opcode == MSG_NACK)
              {
                printf("should check the send msg!  \r\n");
              }
             
            }
            else
            {
              
              ble_send_ack(MSG_NACK);
                          
              printf("Usart2 recv data CRC is error  \r\n");
            }
            m_parser_state = FIND_START_HEADER_H;
            bytecount =0;
          }
        
        break;
       
    default:
        m_parser_state = FIND_START_HEADER_H;//reset parser
      break;
    
    
    }//end switch
  }//end while
  
  return parse_ret;
  
  
  
  
#else
  
  
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
   
#endif 
   
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
