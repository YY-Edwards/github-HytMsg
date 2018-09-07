#ifndef _HDTAP_H_
#define _HDTAP_H_

#include "radio/hrnp.h" 
#include "radio/message.h"

#define HDTAP 0x02  //little-endian mode

#define Private_Call    0x00
#define Group_Call      0x01


#define REQUEST_MASK 0x0 
#define REPLY_MASK 0x8
#define NOTIFY_MASK 0x1
#define BRDCST_MASK 0xB

#define MSH_END 0x03


#define REQUEST(x) ((REQUEST_MASK << 12) | (x))
#define NOTIFY(x) ((NOTIFY_MASK << 12) | (x))
#define REPLY(x) ((REPLY_MASK << 12) | (x))
#define BRDCST(x) ((BRDCST_MASK << 12) | (x))

#define Hdtap_Sucess 0x00
#define Hdtap_failure 0x01

#define Hdtap_Msg_Payload_Len 514

typedef enum
{
  Hdtap_Send,
  Hdtap_Resend,
  Hdtap_Reply,
  
}HdtapSta_t;


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


#pragma   pack()




#define TrunkingPowerUpCheck 0x00C6

#pragma   pack(1)
typedef struct
{


    HdtapHeader_t Header;
    HdtapEnd_t End;


}TrunkingPowerUpCheck_req_t;


typedef struct
{
  HdtapHeader_t Header;
  unsigned char Result;
  HdtapEnd_t End;
  
}TrunkingPowerUpCheck_reply_t;
#pragma   pack()


#define RadioIDQuery  0x0C02

#pragma   pack(1)
typedef struct
{


    HdtapHeader_t Header;
    HdtapEnd_t End;


}RadioIDQuery_req_t;


typedef struct
{
  HdtapHeader_t          Header;
  unsigned char         Result;
  unsigned int            ID;
  HdtapEnd_t             End;
  
}RadioIDQuery_reply_t;
#pragma   pack()



#define RegisterServiceQuery 0x0C18

#pragma   pack(1)
typedef struct
{
    HdtapHeader_t Header;
    HdtapEnd_t End;
    
}RegisterStatusQuery_req_t;


typedef struct
{
  HdtapHeader_t        Header;
  unsigned char       Status;
  HdtapEnd_t           End;
  
}RegisterStatusQuery_reply_t;
#pragma   pack()

#define SystemModeOperation             0x0C10
#define SystemModeOperation_Read        0x00
#define SystemModeOperation_Set         0x01
#define SystemMode_Conventional         0x00
#define SystemMode_DigitalTrunking      0x03
#define SystemMode_AnalogTrunking       0x02


#pragma   pack(1)
typedef struct
{
    HdtapHeader_t Header;
    unsigned char  Option;
    unsigned char  Mode;
    HdtapEnd_t End;
    
}SystemModeOperation_req_t;


typedef struct
{
  HdtapHeader_t        Header;
  unsigned char  Result;
  unsigned char  Option;
  unsigned char  Mode;
  HdtapEnd_t           End;
  
}SystemModeOperation_reply_t;
#pragma   pack()












#define DigitalTrunkingBusinessService 0x1C06

//Target
#define Message_Receipt         0x01  //corresponding notice message:0xBC09

//Operation
#define NOT_Inform_Peripheral   0x00 
#define Inform_Peripheral       0x01


#pragma   pack(1)


#define MAX_SERVICE_CTR  10

typedef struct
{

  unsigned char       Target;
  unsigned char       Operation;


}ServiceData_t;

typedef struct
{


    HdtapHeader_t         Header;
    unsigned char        Number;
    ServiceData_t         ServiceData[MAX_SERVICE_CTR];
    HdtapEnd_t            End;


}DigitalTrunkingBusinessService_req_t;


typedef struct
{
  HdtapHeader_t          Header;
  unsigned char         Result;
  HdtapEnd_t             End;
  
}DigitalTrunkingBusinessService_reply_t;
#pragma   pack()


#define MessageSendingReq  0x0C08

#pragma   pack(1)


typedef struct
{
  HdtapHeader_t           Header;
  
  unsigned int          TargetID;
  unsigned char         CallType;
  unsigned char         Option;
  unsigned short        Datalen;
  unsigned char         TMData[Hdtap_Msg_Payload_Len];//集群数据负载
  //unsigned char         *TMData;//可以吗？不可以
  
  HdtapEnd_t              End;
  
}TrunkingMessage_req_t;



typedef struct
{
    HdtapHeader_t Header;
    unsigned char Result;
    HdtapEnd_t End;

}
MessageSending_reply_t;

#pragma   pack()



#define TrunkingMsg  0x09

#define MessageReceivingReport  0xBC09

//Oprion
#define Text_Message    0x00
//#define Status_Message  0x20


#pragma   pack(1)
typedef struct
{
  HdtapHeader_t           Header;
  
  unsigned char         CallType;
  unsigned char         Option;
  unsigned short        Datalen;
  unsigned int          GroupID;//This value is invalid for private message
  unsigned int          SourceID;
 
  unsigned char         MsgData[1000];

  
  HdtapEnd_t              End;
  
}MessageReceivingReport_brd_t;

#pragma   pack(1)


typedef enum 
{
  Success =0,
  Failure,
  Unregistered,
  Low_Battery,
  Disabled_ID,
  Disabled_Status_Code
}MsgSendReplyResult;

typedef struct
{
    unsigned short Opcode;   
    void ( *HdtapFunc)(void *);  
    void  * Parameter; 
}HdtapExe_t;



void hdtap_init(void);
void hdtap_cfg(void);
void hdtap_exe( void * hdtap);

void TrunkingPowerUpCheck_req(void * p);
void TrunkingPowerUpCheck_reply(void *hdtap);

void RadioIDQuery_req(void *p);
void RadioIDQuery_reply(void *hdtap);

void RegisterStatusQuery_req(void *p);
void RegisterStatusQuery_reply(void *hdtap);

void SystemModeOperation_req(void *p);
void SystemModeOperation_reply(void *hdtap);

void DigitalTrunkingBusinessService_req(void * p);
void DigitalTrunkingBusinessService_reply(void * hdtap);

void MessageSendingRequest(void * p);
void MessageSending_reply(void * hdtap);
void MessageSendingReq_rec(void * hdtap);

void MessageReceivingReport_rec(void * hdtap);


#define MAX_HDTAP_EXE_LIST 10

static const HdtapExe_t HdtapExeList[MAX_HDTAP_EXE_LIST] =
{

  {REPLY(TrunkingPowerUpCheck), TrunkingPowerUpCheck_reply},
  
  {REPLY(RadioIDQuery), RadioIDQuery_reply},
  
  {REPLY(DigitalTrunkingBusinessService),DigitalTrunkingBusinessService_reply},
  
  {REPLY(MessageSendingReq), MessageSending_reply},
  
  {BRDCST(MessageReceivingReport), MessageReceivingReport_rec},//注意，这里调用准备向蓝牙发送收到的短消息
  
  {REPLY(RegisterServiceQuery), RegisterStatusQuery_reply},
  
  {REPLY(SystemModeOperation),SystemModeOperation_reply},
  
  {MessageSendingReq, MessageSendingReq_rec},
  
  
};










#endif