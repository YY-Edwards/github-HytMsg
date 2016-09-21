#include "radio/message.h"
#include "app.h"


Queue_t MsgRxQue = NULL;

void msg_init(void)
{
    hdpep_init();
    MsgRxQue = QueueCreate(10, sizeof(Message_t));
}
void msg_receive_event(void * msg)
{
  //直接就转发了，没有做数据判断
////    MessageHeader_t * p = (MessageHeader_t *)msg;
//    if(Msg_Header != p->Header)return;
//    if(sizeof(Message_t) <= sizeof(MessageHeader_t) + p->Length + 2 )return;
//    
//    
//    unsigned short rxcheck = *(unsigned short*)((unsigned char *)msg + sizeof(MessageHeader_t) + p->Length);
//    if( rxcheck == msg_checksum(msg))
    {
        Message_t Msg;
        
        memcpy(&Msg, msg, sizeof(Message_t));
        
        //memcpy(&Msg, msg, sizeof(MessageHeader_t) + p->Length + 2);
        QueuePush(MsgRxQue, &Msg);
    }
}

void msg_send( Message_t * msg)
{
    
#ifdef TRUNKING_MODE
  
  MessageSendingRequest(msg);

#else
  
  if(msg->Header.Address == 0xFFBA)GroupMessage_trans(msg);
    else//peer to peer
      PrivateMessage_trans(msg);
    
  
#endif
    
}

unsigned char  msg_receive(Message_t * msg)
{  
    unsigned char hdData[MAX_PAYLOAD_SIZE];
    
    
#ifdef  TRUNKING_MODE
  
    if(SUCCESS == hdtap_receive(hdData))hdtap_exe(hdData); 
    
#else
    
 
    if(SUCCESS == hdpep_receive(hdData))hdpep_exe(hdData); 
    
    
#endif

    if(queue_ok != QueuePull(MsgRxQue, msg))//get data for Que.
    {
        return FAILURE;
    }
    else
    {
        return SUCCESS;
    }
}




unsigned int CRC16_2(unsigned char *buf, int len)
{  
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++)
  {
  crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

  for (int i = 8; i != 0; i--) {    // Loop over each bit
    if ((crc & 0x0001) != 0) {      // If the LSB is set
      crc >>= 1;                    // Shift right and XOR 0xA001
      crc ^= 0xA001;
    }
    else                            // Else LSB is not set
      crc >>= 1;                    // Just shift right
    }
  }

  return crc;
}


unsigned short msg_checksum(Message_t * msg)
{
    return CRC16_2((unsigned char *)&msg->Header.Address, msg->Header.Length + 4) & 0xFFFF;
}
