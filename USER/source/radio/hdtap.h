#ifndef _HDTAP_H_
#define _HDTAP_H_

#include "radio/hrnp.h" 
#include "radio/message.h"
#include "radio/hdpep.h"

#define TRUNKING_MODE


#define HDTAP 0x02  //little-endian mode

#define Private_Call    0x00
#define Group_Call      0x01

//注意测试结构体内的内存分配

#pragma   pack(1)

typedef struct
{  
    unsigned short Opcode : 12;
    unsigned short Mask : 4;
    
}OpcodeDTBSStruct_t;

typedef struct
{
  unsigned short H_Opcode : 8;
  unsigned short L_Opcode : 8;

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
  //unsigned char         TMData[Datalen];//
  unsigned char         *TMData;//可以吗？
  
  HdtapEnd_t              End;
  
}TrunkingMessage_req_t;


#pragma   pack()


void MessageSendingRequest(void * p);



#endif