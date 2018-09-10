#ifndef _MESSAGE_H_
#define _MESSAGE_H_


//是否启用集群模式
//#define  TRUNKING_MODE

#ifdef  TRUNKING_MODE
  #include "radio/hdtap.h"

#else
  #include "radio/hdpep.h"

#endif

#include "log/log.h"
//#include "app.h"

#define CMD_DATA                0x01
#define CMD_ACK                 0x00
#define CMD_ALIVE               0x02
#define CMD_NACK                0xFF

#define CMD_NOTIFY_MSG_SEND_RESULT        0xB1

#define MAX_MSG_DATA_SIZE  250 //248+2,//62//多出的两个字节用来存储checksum数据
#define BLE_PRO_HEADER 0xFFFE


#pragma pack(1)

typedef struct
{
    unsigned short      Header;
    //unsigned short      Address;
    unsigned char       Length;
    unsigned char       Opcode;
    
}MessageHeader_t;//重新调整协议结构


typedef struct
{
    MessageHeader_t Header;
    unsigned char Payload[MAX_MSG_DATA_SIZE];
    unsigned short Checksum;
}Ble_Message_Pro_t;

typedef struct
{
    MessageHeader_t Header;
    unsigned short Checksum;
}Message_ack_t;



#pragma pack()


unsigned short msg_checksum(Ble_Message_Pro_t * msg);

void msg_init(void);
void msg_send( Ble_Message_Pro_t * msg);
unsigned char  msg_receive(Ble_Message_Pro_t * msg);
void msg_receive_event(void * msg);
#endif