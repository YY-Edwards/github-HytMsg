#include "bluetooth.h"

Queue_t BluetoothRxQue = NULL;

static void rcc_ble_init(void)
{
    RCC_APB2PeriphClockCmd(USART2_GPIO_CLK | BLE_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(USART2_CLK, ENABLE);
    
    
}

static void nvic_ble_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
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
    rcc_ble_init(); 
    nvic_ble_init();
    gpio_ble_init();
    usart_ble_init();
    
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
     
     if((Counter <= sizeof(Message_t)) && (Msg_Header == htons(msg->Header.Header)) )
     //if((Msg_Header == htons(msg->Header.Header)))
      {
            
            switch(msg->Header.Opcode)
            {
              
              case MSG_DATA://0x01
                  
                      unsigned short rxcheck = *(unsigned short *)((unsigned char *)msg +  sizeof(MessageHeader_t) + msg->Header.Length);
                        
                      //if(htons(rxcheck) == msg_checksum(msg)) У���в��� 
                      if(htons(rxcheck))
                      {
                          ble_rx_counter =0;
                          Counter = 0;
                          ble_send_ack(MSG_ACK);
                          return SUCCESS;//�ɹ�
                      }
                      else
                      {
                          ble_send_ack(MSG_NACK);
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
     //��С��bug
     ack.Header.Header = htons(Msg_Header);
     ack.Header.Address = 0; 
     ack.Header.Length = 0;
     ack.Header.Opcode = op;
     ack.Checksum = msg_checksum((Message_t *)&ack);
     
     ble_send((Message_t *)&ack);
}
