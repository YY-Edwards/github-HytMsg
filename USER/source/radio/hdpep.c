#include "radio/hdpep.h"
#include "App/app.h"


Queue_t HdpepExecQue = NULL;
unsigned int Local_RadioIP;
unsigned int Local_RadioID;
static unsigned int g_target_RadioID = 7;//需要手动调整
extern unsigned char ble_alive_flag;
bool msg_allowed_send_flag= true;

void hdpep_init(void)
{
    //hrnp_init(); 
    hdpep_cfg();
}

void hdpep_cfg(void)
{   
    if(HdpepExecQue!=NULL)
    {
      QueueDelete(HdpepExecQue);
      HdpepExecQue =NULL;
    }
    HdpepExecQue = QueueCreate(5, sizeof(HdpepExe_t));
    if(NULL == HdpepExecQue)return;
    
    HdpepExe_t exe;       
    exe.Opcode = REQUEST(PowerUpCheck);
    exe.HdpepFunc = PowerUpCheck_req;
    exe.Parameter = NULL;
    QueuePush(HdpepExecQue, &exe);
     
    exe.Opcode = NOTIFY(RadioidAndRadioipQuery);/*****注意：这里应该是REQUEST(RadioidAndRadioipQuery)****/
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
                     sta = Hdpep_Send;//超过3次重发则丢弃，并进入下一条发送
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
            if(HDPEP_RCP != ptr->MshHdr)//big-endian mode,根据hdpep协议文档，只有RCP协议是小端模式
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
    
    req->Traget = Radio_IP;//Radio_ID;//Radio_IP;
   
    req->End.Checksum = hdpep_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdpepHeader_t) + sizeof(HdpepEnd_t) + 1);
}

void RadioidAndRadioipQuery_reply(void *hdpep)
{
    RadioidAndRadioipQuery_reply_t * reply = (RadioidAndRadioipQuery_reply_t *)hdpep;
    if(Hdpep_Sucess == reply->Result)
    {
        if(Radio_IP == reply->Traget)//经测试发现，radio传过来的数据是大端模式，不知道为什么
        {
          Local_RadioIP = htonl(reply->Value);
          
          printf("Query Radio Ip Success : 0x%08x\r\n", Local_RadioIP);
        }
        else if(Radio_ID == reply->Traget)
        {
          Local_RadioID = reply->Value;
          printf("Query Radio Id Success : %d\r\n", Local_RadioID);
        }
        
        
    }
    else
    {
       printf("Query Radio Failure\r\n");
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
    
    memset(req, 0x00, sizeof(PrivateMessage_trans_t));//clear 
    
    
    req->Header.MshHdr = HDPEP_TMP;//0x09
    
    HdpepOpcode_t op;
    op.TMS.Ack = ACK_Required;//ACK_NotRequired;//不需要ACK
    op.TMS.Option = Disable_OptionFiels;
    op.TMS.Opcode = PrivateMessageTransmission;      
    req->Header.Opcode.Store = htons(op.Store);//0x00A1
    
    if(msg->Header.Length >= 200)req->Header.Length = 200;//如果ble发过来的数据超过200bytes,则留两个字节给END
    else
      req->Header.Length = htons(msg->Header.Length + 12);//RequestID[4]+DestID[4]+SrcIP[4]+TMData[msg->Header.Length]
    
    //req->Header.Length = htons(msg->Header.Length + sizeof(MessageHeader_t) + 2);//msg lengtg + info

    req->RequestID = htonl(req_id++);
    //req->DestIP = htonl(ID2IP(msg->Header.Address));
    //req->DestIP = htonl(ID2IP(7));//注意，此处添加目标Radio的IP,默认都是在10网段下
    req->DestIP = htonl(ID2IP(g_target_RadioID));
    
    //req->DestIP = htonl(ID2IP(1));
    
    printf("[RECEIVE BLE MSG AND SEND TO RADIO 0x%x]\r\n", htonl(req->DestIP));//打印正常IP 
    
    //req->SrcIP = htonl(ID2IP(3));//need to change;设置为自动获取
    req->SrcIP = htonl(Local_RadioIP);//上电后自动获取
    //req->SrcIP = htonl(ID2IP(2));
    
    
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
      
        //req->TMData[msg->Header.Length] =  0x17;
      //req->TMData[msg->Header.Length+1] =  0x03;
    
   req->TMData[msg->Header.Length] =  req->End.Checksum; 
   req->TMData[msg->Header.Length+1] = req->End.MsgEnd; 
    
      
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
    msg_allowed_send_flag = true;
}

void PrivateMessage_sendack(void * p)
{
    PrivateMessage_trans_t  * rec = (PrivateMessage_trans_t * )p;
    PrivateMessage_ack_t PrivateMessage_ack, * ack = &PrivateMessage_ack;
    
    ack->Header.MshHdr = HDPEP_TMP;
    
    HdpepOpcode_t op;
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
    
    //根据协议，TMS协议是大端模式，将收取到的数据再次进行大端转换则变换成小端数据
    //但是协议TMData里描述，内容为小端模式的unicode编码格式数据，实测也符合协议规定。
    rec->RequestID = htonl(rec->RequestID);
    rec->DestIP = htonl(rec->DestIP);
    rec->SrcIP = htonl(rec->SrcIP);
       
    if(ACK_Required == rec->Header.Opcode.TMS.Ack)
    {
        PrivateMessage_sendack(rec);
    } 
    
    printf("[1.RECEIVE PRIVATE MSG FORM RADIO %d]\r\n", IP2ID(rec->SrcIP)); 
    
    printf("[2.AND SEND MSG TO BLE]\r\n");
    
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
            //将radio发过来的短信数据每两个字节进行大小端转换为大端数据。
            //之前调试时默认与ble传输的协议格式为大端。
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


void GroupMessage_transfer(void * p)
{       
    static unsigned int req_id = 1;
    
    Message_t * msg = (Message_t *)p;
    GroupMessage_trans_t groupmsg_trans, *req = &groupmsg_trans;
    
    req->Header.MshHdr = HDPEP_TMP;
    
    HdpepOpcode_t op;
    op.TMS.Ack = ACK_NotRequired;
    op.TMS.Option = Disable_OptionFiels;
    op.TMS.Opcode = GroupMessageTransmission;      
    req->Header.Opcode.Store = htons(op.Store);
    
    req->Header.Length = htons(msg->Header.Length + sizeof(MessageHeader_t) + 2);//msg lengtg + info

    req->RequestID = htonl(req_id++);
    req->GroupID = htonl(msg->Header.Address);
    req->SrcIP = htonl(Local_RadioIP);
    
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
    
    HdpepOpcode_t op;
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


