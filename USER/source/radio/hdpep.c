#include "radio/hdpep.h"
#include "App/app.h"


Queue_t HdpepExecQue = NULL;
unsigned int SrcIP;
extern unsigned char ble_alive_flag;

void hdpep_init(void)
{
    hrnp_init(); 
    hdpep_cfg();
}

void hdpep_cfg(void)
{    
    HdpepExecQue = QueueCreate(5, sizeof(HdpepExe_t));
    if(NULL == HdpepExecQue)return;
    
    HdpepExe_t exe;       
    exe.Opcode = REQUEST(PowerUpCheck);
    exe.HdpepFunc = PowerUpCheck_req;
    exe.Parameter = NULL;
    QueuePush(HdpepExecQue, &exe);
    
    exe.Opcode = NOTIFY(RadioidAndRadioipQuery);
    exe.HdpepFunc = RadioidAndRadioipQuery_req;
    exe.Parameter = NULL;
    QueuePush(HdpepExecQue, &exe);
    
    exe.Opcode = NOTIFY(TextMessageNotification);
    exe.HdpepFunc = TextMessageNotification_req;
    exe.Parameter = NULL;
    QueuePush(HdpepExecQue, &exe);
     
    HdpepSta_t sta = Hdpep_Send;
    unsigned char hdpep[MAX_PAYLOAD_SIZE];
    
    unsigned short timeout = 0;
    unsigned short resendtimes = 0;
    
    for(;;)
    {
        switch(sta)
        {
          case Hdpep_Send:
            if(NULL != exe.Parameter)
            {
                free(exe.Parameter);
                exe.Parameter = NULL;
            }
            if(queue_ok == QueuePull(HdpepExecQue, &exe))
            {
                exe.HdpepFunc(exe.Parameter);
                timeout = 0;
                sta = Hdpep_Reply;
            }
            else
            {
                QueueDelete(HdpepExecQue);
                return;
            }
            break;
          case Hdpep_Resend:
                exe.HdpepFunc(exe.Parameter);
                timeout = 0;
                sta = Hdpep_Reply;           
            break;
          case Hdpep_Reply:       
            delaynms(100);  
            if((SUCCESS == hdpep_receive(hdpep)) && (((HdpepHeader_t *)hdpep)->Opcode.Struct.Opcode == (exe.Opcode & 0x0FFF)))//receive
            {
                hdpep_exe(hdpep);   
                sta = Hdpep_Send;
                break;
            }
            if((timeout += 1) >= 5)//timeout resend
            {
                timeout = 0;
                sta = Hdpep_Resend;  
                if((resendtimes += 1) >= 3)//resend times
                {
                     resendtimes = 0;
                      //sta = ExecFunc;
                }
            }  
            
            break;
          default:
              sta = Hdpep_Send;
              break;
        }
    } 
}

void hdpep_exe( void * hdpep)
{
    HdpepHeader_t * ptr = (HdpepHeader_t *)hdpep;
    
    printf("receive hdpep:[ 0x%04X ]\r\n", ptr->Opcode.Store);
    
    for(int i = 0; i < MAX_HDPEP_EXE_LIST; i++)
    {
        if((ptr->Opcode.Store & 0xFFF) == (HdpepExeList[i].Opcode & 0xFFF) )
        {
            HdpepExeList[i].HdpepFunc(ptr);
            break;
        }
    }
}


unsigned char hdpep_checksum(void * hdpep,  unsigned int PayloadLen)
{
    /* hrcc Checksum = ~ ( Opcode + # of bytes + payload ) + 0x33 */
    unsigned short  Sumofdata   = 0;
    unsigned char *index = 0;
    
    index =((unsigned char *)hdpep + 1);
 
    if(NULL == hdpep)return 0;
    
    for(int i = 0; i < (4 + PayloadLen); i++)
    {
      Sumofdata += *((unsigned char *)index + i);
    }
    
    Sumofdata = ~ Sumofdata +0x33;
    return (unsigned char)Sumofdata;
}


unsigned char hdpep_receive(void *hdpep)
{
    Hrnp_t hrnp;
    if(SUCCESS == hrnp_receive(&hrnp))
    {
        //save reg hdpep
        if(hrnp.Header.Length >= sizeof(HrnpHeader_t) + sizeof(HdpepHeader_t) +sizeof(HdpepEnd_t))
        {
            HdpepHeader_t * ptr = (HdpepHeader_t *)hrnp.Payload.Data;//hdpep-header结构指向
            if(HDPEP_RCP != ptr->MshHdr)//big-endian mode
            {
                //Hdpep 协议结构
                ptr->Opcode.Store = htons(ptr->Opcode.Store);
                ptr->Length = htons(ptr->Length );
            }
          
            //把剩下的数据部分贴在hedep-header结构的后面
            memcpy(hdpep, hrnp.Payload.Data, hrnp.Header.Length - sizeof(HrnpHeader_t));
            return SUCCESS;
        }
    }
    
    return FAILURE;
}

void PowerUpCheck_req(void * p)
{
    PowerUpCheck_req_t  PowerUpCheck_req,  * req = &PowerUpCheck_req;
    
    req->Header.MshHdr = HDPEP_RCP;
    req->Header.Opcode.Struct.Mask = REQUEST_MASK;
    req->Header.Opcode.Struct.Opcode = PowerUpCheck;
    req->Header.Length = 0;
    
    req->End.Checksum = hdpep_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdpepHeader_t) + sizeof(HdpepEnd_t));
}

void PowerUpCheck_reply(void * hdpep)
{
    PowerUpCheck_reply_t * reply = (PowerUpCheck_reply_t *)hdpep;
    if(Hdpep_Sucess == reply->Result)
    {
        printf("PowerUpCheck Success\r\n");
    }
    else
    {
       printf("PowerUpCheck Failure\r\n");
    }
}



void RadioidAndRadioipQuery_req(void *p)
{
    RadioidAndRadioipQuery_req_t  RadioidAndRadioipQuery_req,  * req = &RadioidAndRadioipQuery_req;
    
    req->Header.MshHdr = HDPEP_RCP;
    req->Header.Opcode.Struct.Mask = REQUEST_MASK;
    req->Header.Opcode.Struct.Opcode = RadioidAndRadioipQuery;
    req->Header.Length = 1;
    
    req->Traget = Radio_IP;
   
    req->End.Checksum = hdpep_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdpepHeader_t) + sizeof(HdpepEnd_t) + 1);
}

void RadioidAndRadioipQuery_reply(void *hdpep)
{
    RadioidAndRadioipQuery_reply_t * reply = (RadioidAndRadioipQuery_reply_t *)hdpep;
    if(Hdpep_Sucess == reply->Result)
    {
        if(Radio_IP == reply->Traget)
        {
          SrcIP = reply->Value;
          
          printf("Query Radio Ip Success : 0x%08X\r\n", reply->Value);
        }
        
        
    }
    else
    {
       printf("Msg Notify CFG Failure\r\n");
    }
}


void TextMessageNotification_req(void * p)
{
    TextMessageNotification_req_t  TextMessageNotification_req,  * req = &TextMessageNotification_req;
    
    req->Header.MshHdr = HDPEP_RCP;
    req->Header.Opcode.Struct.Mask = NOTIFY_MASK;
    req->Header.Opcode.Struct.Opcode = TextMessageNotification;
    req->Header.Length = 1;
    
    req->Operation = Msg_Per_Equipment_And_Serial_Notifiled;
    
    req->End.Checksum = hdpep_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdpepHeader_t) + sizeof(HdpepEnd_t) + 1);
}

void TextMessageNotification_reply(void * hdpep)
{
    TextMessageNotification_reply_t * reply = (TextMessageNotification_reply_t *)hdpep;
    if(Hdpep_Sucess == reply->Result)
    {
        printf("Msg Notify CFG Success\r\n");
        ble_alive_flag =1;
    }
    else
    {
       printf("Msg Notify CFG Failure\r\n");
    }
}

void PrivateMessage_trans(void * p)
{       
    static unsigned int req_id = 1;
    
    Message_t * msg = (Message_t *)p;
    PrivateMessage_trans_t PrivateMessage_trans, *req = &PrivateMessage_trans;
    
    req->Header.MshHdr = HDPEP_TMP;//0x09
    
    HdepeOpcode_t op;
    op.TMS.Ack = ACK_NotRequired;
    op.TMS.Option = Disable_OptionFiels;
    op.TMS.Opcode = PrivateMessageTransmission;      
    req->Header.Opcode.Store = htons(op.Store);//0x00A1
    
    if(msg->Header.Length >= 510)req->Header.Length = 510;//如果ble发过来的数据超过510bytes,则留两个字节给END
    else
      req->Header.Length = htons(msg->Header.Length + 12);//RequestID[4]+DestID[4]+SrcIP[4]+TMData[msg->Header.Length]
    
    //req->Header.Length = htons(msg->Header.Length + sizeof(MessageHeader_t) + 2);//msg lengtg + info

    req->RequestID = htonl(req_id++);
    //req->DestIP = htonl(ID2IP(msg->Header.Address));
    req->DestIP = htonl(ID2IP(2));
    req->SrcIP = htonl(ID2IP(1));//need to change;设置为自动获取
    
    memcpy(req->TMData, msg->Payload, msg->Header.Length);
    
    unsigned char i =0;
    unsigned char temp =0;
    
    for(i=0 ; i<msg->Header.Length; )
          {
            //将radio发过来的短信数据每两个字节进行大小端转换为正常的unicode码
            temp = req->TMData[i];
            req->TMData[i] = req->TMData[i+1];
            req->TMData[i+1] = temp;
            i+=2;
          
          }
    
    memset(&(req->TMData[msg->Header.Length]), 0x00, 512-msg->Header.Length);//TMData未用的部分清零。
    //memcpy(req->TMData, msg, msg->Header.Length + sizeof(MessageHeader_t) + 2);
  
    //注意地址
    req->End.Checksum =  hdpep_checksum(req, htons(req->Header.Length));
    req->End.MsgEnd = MSH_END;
      
    //协议结构处理,将checksum 和msgend 贴在TMData的末尾以便传输。
      
      req->TMData[msg->Header.Length] =  req->End.Checksum; 
      req->TMData[msg->Header.Length+1] =  req->End.MsgEnd; 
    
      
//    HdpepEnd_t * end = (HdpepEnd_t *)((unsigned char *)req + sizeof(HdpepHeader_t) + msg->Header.Length);
//    
//    end->Checksum = hdpep_checksum(req, htons(req->Header.Length));
//    end->MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdpepHeader_t) + sizeof(HdpepEnd_t) + msg->Header.Length + 12);
}

void PrivateMessage_ack(void * hdpep)
{
    PrivateMessage_ack_t * ack = (PrivateMessage_ack_t * )hdpep;
    if(Hdpep_Sucess == ack->Result)
    {
        printf("Send Private Msg Success\r\n");
    }
    else
    {
       printf("Send Private Msg Failure\r\n");
    }
}

void PrivateMessage_sendack(void * p)
{
    PrivateMessage_trans_t  * rec = (PrivateMessage_trans_t * )p;
    PrivateMessage_ack_t PrivateMessage_ack, * ack = &PrivateMessage_ack;
    
    ack->Header.MshHdr = HDPEP_TMP;
    
    HdepeOpcode_t op;
    op.TMS.Ack = ACK_NotRequired;
    op.TMS.Option = Disable_OptionFiels;
    op.TMS.Opcode = PrivateMessageAck;      
    ack->Header.Opcode.Store = htons(op.Store);
    
    ack->Header.Length = htons(13);//msg lengtg + info

    ack->RequestID = htonl(rec->RequestID);
    ack->DestIP = htonl(rec->DestIP);
    ack->SrcIP = htonl(rec->SrcIP);
    ack->Result = Hdpep_Sucess;
       
    ack->End.Checksum = hdpep_checksum(ack, htons(ack->Header.Length));
    ack->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)ack,  sizeof(HdpepHeader_t) + sizeof(HdpepEnd_t) + 13);
}

void PrivateMessage_rec(void * hdpep)
{
    PrivateMessage_trans_t  *rec = (PrivateMessage_trans_t *)hdpep;
    
    rec->RequestID = htonl(rec->RequestID);
    rec->DestIP = htonl(rec->DestIP);
    rec->SrcIP = htonl(rec->SrcIP);
       
    if(ACK_Required == rec->Header.Opcode.TMS.Ack)
    {
        PrivateMessage_sendack(rec);
    } 
    
    printf("[RECEIVE PRIVATE MSG FORM RADIO %d]\r\n", IP2ID(rec->SrcIP)); 
    
    //直接发的是纯数据信息，没有相关的长度之类的结构。
   // msg_receive_event(rec->TMData); 
    
    unsigned char rxchecksum, rxmsgend;
    
    rxchecksum = rec->TMData[rec->Header.Length-12];
    rxmsgend = rec->TMData[rec->Header.Length-12+1];
    
    //因为数据长度的问题，end数据在TMdata最后
    rec->End.Checksum = rxchecksum;
    
    rec->End.MsgEnd = rxmsgend;
    
    
    /**************测试***************/
    
    //checksum Hdpep data
    unsigned char rxcheck = hdpep_checksum(rec, rec->Header.Length);
    if( rxcheck == rec->End.Checksum)
    {
          //注意测试一下结构
          OB_Message_t Msg;
          Msg.type = PrivateMsg;
          Msg.dest = IP2ID(rec->DestIP);
          Msg.src = IP2ID(rec->SrcIP);
          Msg.TMLen = rec->Header.Length - 12;
          memcpy(Msg.TMData, rec->TMData, Msg.TMLen);
          unsigned char i =0;
          unsigned char temp =0;
          
          for(i=0 ; i<Msg.TMLen; )
          {
            //将radio发过来的短信数据每两个字节进行大小端转换为正常的unicode码
            temp = Msg.TMData[i];
            Msg.TMData[i] = Msg.TMData[i+1];
            Msg.TMData[i+1] = temp;
            i+=2;
          
          }
          
          
          app_rec_msg(&Msg);
          
    }
    else{
      
      printf("\r\n Hdpep checksum error \r\n");
      
      //Hdpep checksum error
    
    }
    
    
   /*****************************/
    
    
}


void GroupMessage_trans(void * p)
{       
    static unsigned int req_id = 1;
    
    Message_t * msg = (Message_t *)p;
    GroupMessage_trans_t GroupMessage_trans, *req = &GroupMessage_trans;
    
    req->Header.MshHdr = HDPEP_TMP;
    
    HdepeOpcode_t op;
    op.TMS.Ack = ACK_NotRequired;
    op.TMS.Option = Disable_OptionFiels;
    op.TMS.Opcode = GroupMessageTransmission;      
    req->Header.Opcode.Store = htons(op.Store);
    
    req->Header.Length = htons(msg->Header.Length + sizeof(MessageHeader_t) + 2);//msg lengtg + info

    req->RequestID = htonl(req_id++);
    req->GroupID = htonl(msg->Header.Address);
    req->SrcIP = htonl(SrcIP);
    
    memcpy(req->TMData, msg, msg->Header.Length + sizeof(MessageHeader_t) + 2);
    
    HdpepEnd_t * end = (HdpepEnd_t *)((unsigned char *)req + sizeof(HdpepHeader_t) + msg->Header.Length + sizeof(MessageHeader_t) + 2);
    
    end->Checksum = hdpep_checksum(req, htons(req->Header.Length));
    end->MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdpepHeader_t) + sizeof(HdpepEnd_t) + msg->Header.Length + sizeof(MessageHeader_t) + 2);
}

void GroupMessage_ack(void * hdpep)
{
    GroupMessage_ack_t * ack = ( GroupMessage_ack_t * )hdpep;
    if(Hdpep_Sucess == ack->Result)
    {
        printf("Send Group Msg Success\r\n");
    }
    else
    {
       printf("Send Group Msg Failure\r\n");
    }
}

void GroupMessage_sendack(void * p)
{
    GroupMessage_trans_t  * rec = (GroupMessage_trans_t * )p;
    GroupMessage_ack_t GroupMessage_ack, * ack = &GroupMessage_ack;
    
    ack->Header.MshHdr = HDPEP_TMP;
    
    HdepeOpcode_t op;
    op.TMS.Ack = ACK_NotRequired;
    op.TMS.Option = Disable_OptionFiels;
    op.TMS.Opcode = GroupMessageAck;      
    ack->Header.Opcode.Store = htons(op.Store);
    
    ack->Header.Length = htons(19);//msg lengtg + info

    ack->RequestID = htonl(rec->RequestID);
    ack->GroupID = htonl(rec->GroupID);
    ack->Result = Hdpep_Sucess;
       
    ack->End.Checksum = hdpep_checksum(ack, htons(ack->Header.Length));
    ack->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)ack,  sizeof(HdpepHeader_t) + sizeof(HdpepEnd_t) + 9);
}

void GroupMessage_rec(void * hdpep)
{
    GroupMessage_trans_t  *rec = (GroupMessage_trans_t *)hdpep;
    
    rec->RequestID = htonl(rec->RequestID);
    rec->GroupID = htonl(rec->GroupID);
    rec->SrcIP = htonl(rec->SrcIP);
       
    if(ACK_Required == rec->Header.Opcode.TMS.Ack)
    {
        GroupMessage_sendack(rec);
    } 
    
    printf("[RECEIVE GROUP MSG FORM GROUP %d]\r\n", IP2ID(rec->SrcIP)); 
    
    msg_receive_event(rec->TMData);
    
//    Message_t Msg;
//    Msg.type = GroupMsg;
//    Msg.dest = rec->GroupID;
//    Msg.src = IP2ID(rec->SrcIP);
//    Msg.TMLen = rec->Header.Length - 12;
//    memcpy(Msg.TMData, rec->TMData, Msg.TMLen);
//    
//    app_rec_msg(&Msg);
}


