#include "app.h"
#include "radio/message.h"

//Queue_t MsgRxQue = NULL;

extern Queue_t MsgRxQue ;

void app_rec_msg(OB_Message_t * msg)
{
  
  /******/

   Ble_Message_Pro_t Msg;
   
   //IAR是小端存储数据,即低位在前，高位在后。而与蓝牙协议中需要变换为高位在前，低位在后。
   /*例如:IAR中数据 0xfffe :0xfe  0xff 。则先发送oxfe,再发送0xff*/
   /*则与BLE的通信时需要大小端转换，即先发送0xff,再发送0xfe*/
   /*双字节数据都需要做大小端转换*/
   
   Msg.Header.Header = Msg_Header;
   Msg.Header.Header = htons(Msg.Header.Header);
   
  
   Msg.Header.Address = (unsigned short)(msg->src); 
   Msg.Header.Address = htons(Msg.Header.Address);
   
   
   Msg.Header.Opcode = 0x01;
   
   //最大为248bytes
   if(msg->TMLen >= 248)Msg.Header.Length = 248;//248bytes
   else
   Msg.Header.Length = msg->TMLen;
   
   memcpy(&(Msg.Payload), msg->TMData, Msg.Header.Length);
   memset(&(Msg.Payload[Msg.Header.Length]), 0x00, 248-Msg.Header.Length);
   
   
   
   Msg.Checksum = msg_checksum((Ble_Message_Pro_t *)&Msg);
   Msg.Checksum = htons(Msg.Checksum);
   
   //协议结构处理
   Msg.Payload[Msg.Header.Length+1] = (unsigned char)(Msg.Checksum>>8);
   Msg.Payload[Msg.Header.Length+2] = (unsigned char)(Msg.Checksum);
  
   /******/
         
  QueuePush(MsgRxQue, &Msg);
  
}