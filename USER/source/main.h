#ifndef _MAIN_H_
#define _MAIN_H_

#include "radio/hdpep.h"
#include "log/log.h"


typedef enum
{
    ExecFunc,
    ReexecFunc,
    WaitReply,
}AppExecSta_t;

typedef struct
{
    unsigned char NeedReply;
    unsigned short Opcode;   
    void ( *HdpepFunc)(void *);  
    void  * Parameter;  
}AppExe_t;


#define MAX_EXE_LIST_LEN 1



#endif