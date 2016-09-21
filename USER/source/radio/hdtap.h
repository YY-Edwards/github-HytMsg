#ifndef _HDTAP_H_
#define _HDTAP_H_

#include "radio/hrnp.h" 
#include "radio/message.h"
#include "radio/hdpep.h"

#define TRUNKING_MODE


#define HDTAP 0x02  //little-endian mode

#define Private_Call    0x00
#define Group_Call      0x01


#define REQUEST_MASK 0x0 
#define REPLY_MASK 0x8
#define NOTIFY_MASK 0x1
#define BRDCST_MASK 0xB


#define REQUEST(x) ((REQUEST_MASK << 12) | (x))
#define NOTIFY(x) ((NOTIFY_MASK << 12) | (x))
#define REPLY(x) ((REPLY_MASK << 12) | (x))

#define Hdtap_Sucess 0x00
#define Hdtap_failure 0x01


//注意测试结构体内的内存分配

#pragma   pack(1)

typedef struct
{  
    unsigned short Opcode : 12;
    unsigned short Mask : 4;
    
}OpcodeDTBSStruct_t;

typedef struct
{
  unsigned short L_Opcode : 8;
  unsigned short H_Opcode : 8;

}OpcodeDTBS_t;


typedef union
{
    OpcodeDTBSStruct_t Struct;
    OpcodeDTBS_t DTBS;
    unsigned short Store;
}HdtapOpcode_t;



typedef struct
{
    unsigned char MshHdr;
    HdtapOpcode_t Opcode;
    unsigned short Length;  
}HdtapHeader_t;


typedef struct
{
    unsigned char Checksum;
    unsigned char MsgEnd;
}HdtapEnd_t;


typedef struct
{
  HdtapHeader_t           Header;
  
  unsigned int          TargetID;
  unsigned char         CallType;
  unsigned char         Option;
  unsigned short        Datalen;
  unsigned char         TMData[512];//
  //unsigned char         *TMData;//可以吗？不可以
  
  HdtapEnd_t              End;
  
}TrunkingMessage_req_t;


#pragma   pack()


void MessageSendingRequest(void * p);


typedef struct
{
    HdtapHeader_t Header;
    unsigned char Result;
    HdtapEnd_t End;

}
MessageSending_reply_t;



void MessageSending_reply(void * hdtap);
void MessageSendingReq_rec(void * hdtap);



typedef struct
{
    unsigned short Opcode;   
    void ( *HdtapFunc)(void *);  
    void  * Parameter; 
}HdtapExe_t;

#define MessageSendingReq  0x0C08


#define MAX_HDTAP_EXE_LIST 5

static const HdtapExe_t HdtapExeList[MAX_HDTAP_EXE_LIST] =
{

  {REPLY(PowerUpCheck), PowerUpCheck_reply},
  
  {REPLY(RadioidAndRadioipQuery), RadioidAndRadioipQuery_reply},
  
  {REPLY(TextMessageNotification),TextMessageNotification_reply},
  
  
  {REPLY(MessageSendingReq), MessageSending_reply},
  
  {MessageSendingReq, MessageSendingReq_rec},//注意，这里调用准备向蓝牙发送收到的短消息
  
  
};










#endif