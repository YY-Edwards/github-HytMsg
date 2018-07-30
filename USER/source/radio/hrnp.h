#ifndef _HRNP_H_
#define _HRNP_H_

#include "radio/physical.h"

#define HRNP_HEADER 0x7E
#define HRNP_VER  0x04

#define DEFAULT_PN 0xFFFF

#define OB_ID_BASS 0x20
#define MASTER_ID  0x10

#define HRNP_CONNECT 0xFE
#define HRNP_ACCPET 0xFD
#define HRNP_REJECT 0xFC
#define HRNP_CLOSE 0xFB
#define HRNP_CLOSE_ACK 0xFA
#define HRNP_DATA 0x00
#define HRNP_DATA_ACK 0x10

#define MAX_PAYLOAD_SIZE 540 // text message has 256 uncode charestr 

typedef enum
{
    WaitToSend,
    WaitAck,
}HrnpSta_t;

#pragma   pack(1)
typedef struct
{
    unsigned char Header;
    unsigned char Version;
    unsigned char Block;
    unsigned char Opcode;
    unsigned char SourceID;
    unsigned char DestinationID;
    unsigned short PN;
    unsigned short Length;
    unsigned short CheckSum;  
}HrnpHeader_t;
#pragma   pack()

typedef union
{
    unsigned char Data[MAX_PAYLOAD_SIZE];
    //unsigned char VoicePacket[320];
}HrnpPayload_t;

#pragma   pack(1)
typedef struct
{
    HrnpHeader_t Header;
    HrnpPayload_t Payload;
}Hrnp_t;
#pragma   pack()

void hrnp_init(void);
unsigned char hrnp_data(unsigned char * dat, unsigned short length);
unsigned char hrnp_receive(Hrnp_t * hrnp);
unsigned char hrnp_connect(void);
unsigned char hrnp_close(void);

#endif