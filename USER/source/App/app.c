#include "app.h"
#include "radio/message.h"

//Queue_t MsgRxQue = NULL;

extern Queue_t MsgRxQue ;

void app_rec_msg(OB_Message_t * msg)
{
  
  /******/

   Message_t Msg;
   
   //IAR是小端存储数据
   Msg.Header.Header = Msg_Header;
   Msg.Header.Header = htons(Msg.Header.Header);
   
   //此处地址问题待测试
   Msg.Header.Address = (unsigned short)(msg->src); 
   Msg.Header.Address = htons(Msg.Header.Address);
   
   
   Msg.Header.Opcode = 0x01;
   
   //最大为60bytes
   if(msg->TMLen >= 60)Msg.Header.Length = 0x3c;
   else
   Msg.Header.Length = msg->TMLen;
   
   memcpy(&(Msg.Payload), msg->TMData, Msg.Header.Length);
   memset(&(Msg.Payload[Msg.Header.Length]), 0x00, 60-Msg.Header.Length);
   
   
   
   Msg.Checksum = msg_checksum((Message_t *)&Msg);
   //Msg.Checksum = htons(Msg.Checksum);
   
   //协议结构处理
   Msg.Payload[Msg.Header.Length+1] = (unsigned char)(Msg.Checksum>>8);
   Msg.Payload[Msg.Header.Length+2] = (unsigned char)(Msg.Checksum);
  
   /******/
         
  QueuePush(MsgRxQue, &Msg);
  
  

}