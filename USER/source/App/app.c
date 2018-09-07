#include "app.h"
#include "radio/message.h"

//Queue_t MsgRxQue = NULL;

extern Queue_t MsgRxQue ;

void app_rec_msg(OB_Message_t * msg)
{
  
  /******/

   Ble_Message_Pro_t Msg;
   
   //IAR��С�˴洢����,����λ��ǰ����λ�ں󡣶�������Э������Ҫ�任Ϊ��λ��ǰ����λ�ں�
   /*����:IAR������ 0xfffe :0xfe  0xff �����ȷ���oxfe,�ٷ���0xff*/
   /*����BLE��ͨ��ʱ��Ҫ��С��ת�������ȷ���0xff,�ٷ���0xfe*/
   /*˫�ֽ����ݶ���Ҫ����С��ת��*/
   
   Msg.Header.Header = Msg_Header;
   Msg.Header.Header = htons(Msg.Header.Header);
   
  
   Msg.Header.Address = (unsigned short)(msg->src); 
   Msg.Header.Address = htons(Msg.Header.Address);
   
   
   Msg.Header.Opcode = 0x01;
   
   //���Ϊ248bytes
   if(msg->TMLen >= 248)Msg.Header.Length = 248;//248bytes
   else
   Msg.Header.Length = msg->TMLen;
   
   memcpy(&(Msg.Payload), msg->TMData, Msg.Header.Length);
   memset(&(Msg.Payload[Msg.Header.Length]), 0x00, 248-Msg.Header.Length);
   
   
   
   Msg.Checksum = msg_checksum((Ble_Message_Pro_t *)&Msg);
   Msg.Checksum = htons(Msg.Checksum);
   
   //Э��ṹ����
   Msg.Payload[Msg.Header.Length+1] = (unsigned char)(Msg.Checksum>>8);
   Msg.Payload[Msg.Header.Length+2] = (unsigned char)(Msg.Checksum);
  
   /******/
         
  QueuePush(MsgRxQue, &Msg);
  
}