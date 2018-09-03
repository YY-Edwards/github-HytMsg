#include "message.h"
#include "app.h"
#include "core_cm3.h"

Queue_t MsgRxQue = NULL;

void msg_init(void)
{

    hrnp_init(); 
    
#ifdef TRUNKING_MODE
    
    hdtap_init();
    
#else
    
    hdpep_init();
    
    
#endif
    
   
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
extern bool Radio_Reject_Msg_flag;
extern bool msg_allowed_send_flag;
void msg_send( Message_t * msg)
{
  
    if(msg_allowed_send_flag == true)
    {
      msg_allowed_send_flag = false;
    }
    else
    {
      printf("[Radio is busying.Please hold on!] \r\n");
      //return;
    }
  
#ifdef TRUNKING_MODE
  
  MessageSendingRequest(msg);

#else
  
  if(msg->Header.Address == 0xFFBA)GroupMessage_transfer(msg);
    else//peer to peer
      PrivateMessage_trans(msg);
    
#endif
    
    if(Radio_Reject_Msg_flag == true)
    {
        printf("[send:need to reconnect OB.] \r\n");
        u8 connect_flag = FAILURE;
        u8 reconnect_count = 0;
        do
        {
          delaynms(300);
          
          connect_flag = hrnp_connect();
          reconnect_count++;
        
        }while((connect_flag == FAILURE)&& (reconnect_count<5));
        if(connect_flag == SUCCESS)
        {
          printf("[reset OB.] \r\n");
          Radio_Reject_Msg_flag = false;//clear flag
          delaynms(300);
          //__set_FAULTMASK(1);//关闭所有中断         
          NVIC_SystemReset();//复位
//          #ifdef trunking_mode   
//            hdtap_init();          
//          #else          
//            hdpep_init();                        
//          #endif
        }
    
    }
    
    
    
}

unsigned char  msg_receive(Message_t * msg)
{  
  unsigned char temp_data[MAX_PAYLOAD_SIZE]={0};
    
    Hrnp_t hrnp;
    
    if(SUCCESS == hrnp_receive(&hrnp))
    {
      #ifdef  TRUNKING_MODE    
        //save reg hdtap
        if(hrnp.Header.Length >= sizeof(HrnpHeader_t) + sizeof(HdtapHeader_t) +sizeof(HdtapEnd_t))
        {   
            memcpy(temp_data, hrnp.Payload.Data, hrnp.Header.Length - sizeof(HrnpHeader_t));           
            hdtap_exe(temp_data);
        }
    #else
        //save reg hdpep
        if(hrnp.Header.Length >= sizeof(HrnpHeader_t) + sizeof(HdpepHeader_t) +sizeof(HdpepEnd_t))
        {
            HdpepHeader_t * ptr = (HdpepHeader_t *)hrnp.Payload.Data;//hdpep-header结构指向
            if(HDPEP_RCP != ptr->MshHdr)//big-endian mode,根据hdpep协议文档，只有RCP协议是小端模式
            {
                //Hdpep 协议结构
                ptr->Opcode.Store = htons(ptr->Opcode.Store);
                ptr->Length = htons(ptr->Length );
            }    
            memcpy(temp_data, hrnp.Payload.Data, hrnp.Header.Length - sizeof(HrnpHeader_t));           
            hdpep_exe(temp_data);
        }
          
    #endif
        
        else if(hrnp.Header.Opcode == HRNP_REJECT)//需要重连
        {   
          
          printf("[recv:need to reset radio.] \r\n");
           delaynms(300);      
          NVIC_SystemReset();//复位

        }
    }
    

//unsigned char hdData[MAX_PAYLOAD_SIZE]; 
//#ifdef  TRUNKING_MODE
//  
//    if(SUCCESS == hdtap_receive(hdData))hdtap_exe(hdData); 
//    
//#else
//    
// 
//    if(SUCCESS == hdpep_receive(hdData))hdpep_exe(hdData); 
//    
//    
//#endif

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
  unsigned char temp[255]={0};
  memcpy(temp, (void *)msg, sizeof(Message_t));
  return CRC16_2(&temp[2], msg->Header.Length + 4) & 0xFFFF;
}
