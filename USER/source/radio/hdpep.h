#ifndef _HDPEP_H_
#define _HDPEP_H_

#include "radio/hrnp.h" 
#include "radio/message.h"

#define HDPEP_RCP 0x02  //little-endian mode
#define HDPEP_LP 0x08   //Bid-endian mode
#define HDPEP_TMP 0x09  //Bid-endian mode

#define MSH_END 0x03

#define REQUEST_MASK 0x0 
#define REPLY_MASK 0x8
#define NOTIFY_MASK 0x1
#define BRDCST_MASK 0xB

#define Hdpep_Sucess 0x00
#define Hdpep_failure 0x01

#define REQUEST(x) ((REQUEST_MASK << 12) | (x))
#define NOTIFY(x) ((NOTIFY_MASK << 12) | (x))
#define REPLY(x) ((REPLY_MASK << 12) | (x))

//IP；ID转换
#define ID2IP(x)  (0x0A000000 | ((x) & 0xFFFFFF))
#define IP2ID(x)  ((x) & 0xFFFFFF)


typedef enum
{
  Hdpep_Send,
  Hdpep_Resend,
  Hdpep_Reply,
}HdpepSta_t;

typedef struct
{
    unsigned short Opcode;   
    void ( *HdpepFunc)(void *);  
    void  * Parameter; 
}HdpepExe_t;

#pragma   pack(1)
typedef struct
{  
    unsigned short Opcode : 12;
    unsigned short Mask : 4;
}OpcodeStruct_t;

typedef struct
{
  unsigned short Opcode :8;
  unsigned short Rev : 6;
  unsigned short Option : 1;
  unsigned short Ack : 1;
}OpcodeTMS_t;

typedef union
{
    OpcodeStruct_t Struct;
    OpcodeTMS_t TMS;
    unsigned short Store;
}HdepeOpcode_t;

typedef struct
{
    unsigned char MshHdr;
    HdepeOpcode_t Opcode;
    unsigned short Length;  
}HdpepHeader_t;

typedef struct
{
    unsigned char Checksum;
    unsigned char MsgEnd;
}HdpepEnd_t;

//typedef struct
//{
//    unsigned short Opcode;   
//    void ( *HdpepFunc)(void *);         
//}HdpepExe_t;

#pragma   pack()

#define PowerUpCheck 0x00C6
#pragma   pack(1)
typedef struct
{
  HdpepHeader_t Header;
  HdpepEnd_t End;
}PowerUpCheck_req_t;

typedef struct
{
  HdpepHeader_t Header;
  unsigned char Result;
  HdpepEnd_t End;
}PowerUpCheck_reply_t;

#pragma   pack()



#define RadioidAndRadioipQuery  0x0452

#define Radio_ID 0x00
#define Radio_IP 0x01

#pragma  pack(1)
typedef struct
{
  HdpepHeader_t Header;
  unsigned char Traget;
  HdpepEnd_t End;
}RadioidAndRadioipQuery_req_t;

typedef struct
{
  HdpepHeader_t Header;
  unsigned char Result;
  unsigned char Traget;
  unsigned int Value;
  HdpepEnd_t End;
}RadioidAndRadioipQuery_reply_t;
#pragma  pack()


#define TextMessageNotification 0x00D0
//operation
#define Msg_No_Notified 0x00
#define Msg_Per_Equipment_Notified 0x01
#define Msg_Per_Equipment_Via_Serial_Notifiled 0x02
#define Msg_Per_Equipment_And_Serial_Notifiled 0x03

#pragma pack(1)
typedef struct
{
    HdpepHeader_t Header;
    unsigned char Operation;
    HdpepEnd_t End;
}TextMessageNotification_req_t;

typedef struct
{
    HdpepHeader_t Header;
    unsigned char Result;
    HdpepEnd_t End;
}TextMessageNotification_reply_t;
#pragma pack()


#define PrivateMessageTransmission 0x00A1
#define PrivateMessageAck 0x00A2

#define PrivateMsg  0xA1


#define ACK_Required 0
#define ACK_NotRequired 1

#define Enable_OptionField 1
#define Disable_OptionFiels 0

#define MAX_TXT_LEN 512
#pragma pack(1)

//typedef struct
//{
//    unsigned int dest;
//    unsigned int src;
//    unsigned char TMData[MAX_TXT_LEN];
//    unsigned short TMLen;
//}Message_t;

typedef struct
{
  HdpepHeader_t Header;
  unsigned int RequestID;
  unsigned int DestIP;
  unsigned int SrcIP;
  unsigned char TMData[MAX_TXT_LEN];
  HdpepEnd_t End;
  
}PrivateMessage_trans_t;

typedef struct
{
  HdpepHeader_t Header;
  unsigned int RequestID;
  unsigned int DestIP;
  unsigned int SrcIP;
  unsigned char Result;
  HdpepEnd_t End;
  
}PrivateMessage_ack_t;

#pragma pack()


#define GroupMessageTransmission 0x00B1
#define GroupMessageAck 0x00B2
#pragma pack(1)

typedef struct
{
  HdpepHeader_t Header;
  unsigned int RequestID;
  unsigned int GroupID;
  unsigned int SrcIP;
  unsigned char TMData[MAX_TXT_LEN];
  HdpepEnd_t End;
}GroupMessage_trans_t;

typedef struct
{
  HdpepHeader_t Header;
  unsigned int RequestID;
  unsigned int GroupID;
  unsigned char Result;
  HdpepEnd_t End;
}GroupMessage_ack_t;

#pragma pack()



void hdpep_init(void);
void hdpep_cfg(void);

void hdpep_exe( void * hdpep);

unsigned char hdpep_receive(void *hdpep);

void PowerUpCheck_req(void * p);
void PowerUpCheck_reply(void * hdpep);

void RadioidAndRadioipQuery_req(void *p);
void RadioidAndRadioipQuery_reply(void *hdpep);

void TextMessageNotification_req(void * p);
void TextMessageNotification_reply(void * hdpep);

void PrivateMessage_trans(void * p);
void PrivateMessage_ack(void * hdpep);
void PrivateMessage_rec(void * hdpep);

#define MAX_HDPEP_EXE_LIST 5
static const HdpepExe_t HdpepExeList[MAX_HDPEP_EXE_LIST] =
{
  {REPLY(PowerUpCheck), PowerUpCheck_reply},
  {REPLY(RadioidAndRadioipQuery), RadioidAndRadioipQuery_reply},
  {PrivateMessageAck, PrivateMessage_ack},
  {REPLY(TextMessageNotification),TextMessageNotification_reply},
  {PrivateMessageTransmission,PrivateMessage_rec},//注意，这里调用准备向蓝牙发送收到的短消息
};
#endif