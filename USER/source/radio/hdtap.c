#include "hdtap.h"

Queue_t HdtapExecQue = NULL;

static unsigned int RadioID  = 0;

unsigned char hdtap_checksum(void * hdtap,  unsigned int PayloadLen)
{
    /* hrcc Checksum = ~ ( Opcode + # of bytes + payload ) + 0x33 */
    unsigned short  Sumofdata   = 0;
    unsigned char *index = 0;
    
    index =((unsigned char *)hdtap + 1);
 
    if(NULL == hdtap)return 0;
    
    for(int i = 0; i < (4 + PayloadLen); i++)
    {
      Sumofdata += *((unsigned char *)index + i);
    }
    
    Sumofdata = ~ Sumofdata +0x33;
    return (unsigned char)Sumofdata;
}



void MessageSendingRequest(void * p)
{
  
  /***需要调整协议**/
  
    Message_t * msg = (Message_t *)p;
    
    TrunkingMessage_req_t TrunkingMessage_req, *req = &TrunkingMessage_req;
    
    req->Header.MshHdr = HDTAP;//0x02
   
    HdtapOpcode_t op;
    
    op.DTBS.L_Opcode = 0x08;
    op.DTBS.H_Opcode = 0x0c;
    
    req->Header.Opcode.Store = op.Store;//0x0c08
       
    if(msg->Header.Length >= 510)req->Header.Length = 510;//如果ble发过来的数据超过510bytes,则留两个字节给END
    else
      req->Header.Length = (msg->Header.Length + 8);//TargetID[4]+CallType[1]+Option[1]+Datalen[2]+Msg[Datalen]
    
    /**Target radio ID (air interface value) which a message to be sent to**/
    //req->TargetID = ID2IP(2);
    req->TargetID = RadioID ;
    //req->DestIP = htonl(ID2IP(msg->Header.Address));
    
    printf("[RECEIVE BLE MSG AND SEND TO TargetID: ox%08X]\r\n", req->TargetID); 
    
    req->CallType = Private_Call;
    
    //Most significant 3-bit indicates the message type 
    //while least significant 5-bit indicates the data format of the text message:
    req->Option = 0x00;
    req->Datalen = msg->Header.Length;
      
    memcpy(req->TMData, msg->Payload, msg->Header.Length);
    
    unsigned char i =0;
    unsigned char temp =0;
 
    for(i=0 ; i<msg->Header.Length; )
          {
            //将Ble发过来的短信数据每两个字节进行大小端转换为Radio可以识别大端的unicode码
            temp = req->TMData[i];
            req->TMData[i] = req->TMData[i+1];
            req->TMData[i+1] = temp;
            i+=2;
          
          }
    
    memset(&(req->TMData[msg->Header.Length]), 0x00, 512-msg->Header.Length);//TMData未用的部分清零。

  
    //注意地址
    req->End.Checksum =  hdtap_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END;
      
    //协议结构处理,将checksum 和msgend 贴在TMData的末尾以便传输。
      req->TMData[msg->Header.Length] =  req->End.Checksum; 
      req->TMData[msg->Header.Length+1] =  req->End.MsgEnd; 
      
    //HdtapEnd_t * end = (HdtapEnd_t *)((unsigned char *)req + sizeof(HdtapHeader_t) + msg->Header.Length);
    
    //end->Checksum = hdtap_checksum(req, htons(req->Header.Length));
    //end->MsgEnd = MSH_END; 

    
    hrnp_data((void *)req,  sizeof(HdtapHeader_t) + sizeof(HdtapEnd_t) + msg->Header.Length + 8);



}




void TrunkingPowerUpCheck_req(void * p)
{
  
    TrunkingPowerUpCheck_req_t  TrunkingPowerUpCheck_req,  * req = &TrunkingPowerUpCheck_req;
    
    req->Header.MshHdr = HDTAP;
    req->Header.Opcode.Struct.Mask = REQUEST_MASK;
    req->Header.Opcode.Struct.Opcode = TrunkingPowerUpCheck;
    req->Header.Length = 0;
    
    req->End.Checksum = hdtap_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdtapHeader_t) + sizeof(HdtapEnd_t));

}

void TrunkingPowerUpCheck_reply(void *hdtap)
{
  
     TrunkingPowerUpCheck_reply_t * reply = (TrunkingPowerUpCheck_reply_t *)hdtap;
    if(Hdtap_Sucess == reply->Result)
    {
        printf("TrunkingPowerUpCheck Success\r\n");
    }
    else
    {
       printf("TrunkingPowerUpCheck Failure\r\n");
    }
  


}

void RadioIDQuery_req(void *p)
{
  
     RadioIDQuery_req_t  RadioIDQuery_req,  * req = &RadioIDQuery_req;
    
    req->Header.MshHdr = HDTAP;
    req->Header.Opcode.Struct.Mask = REQUEST_MASK;
    req->Header.Opcode.Struct.Opcode = RadioIDQuery;
    req->Header.Length = 0;
   
    req->End.Checksum = hdpep_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdtapHeader_t) + sizeof(HdtapEnd_t));
 

}



void RadioIDQuery_reply(void *hdtap)
{

    RadioIDQuery_reply_t * reply = (RadioIDQuery_reply_t *)hdtap;
        
    if(Hdtap_Sucess == reply->Result)
    {
         
        RadioID = reply->ID;
          
        printf("Query Radio ID Success : 0x%08X\r\n", reply->ID);
        printf("This ID is an air interface value\r\n");
          
     
    }
    else
    {
       printf("Query Radio ID Failure\r\n");
    }



}

void DigitalTrunkingBusinessService_req(void * p)
{
    DigitalTrunkingBusinessService_req_t  DigitalTrunkingBusinessService_req,  * req = &DigitalTrunkingBusinessService_req;
    
    req->Header.MshHdr = HDTAP;
    req->Header.Opcode.Struct.Mask = NOTIFY_MASK;
    req->Header.Opcode.Struct.Opcode = DigitalTrunkingBusinessService;
    
    req->Number =1;
    req->ServiceData[0].Target = Message_Receipt;
    req->ServiceData[0].Operation = Inform_Peripheral;
    
    
    req->Header.Length = ((req->Number)*2+1);

    
    req->End.Checksum = hdtap_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    
    /***数据结构处理***/
    //将checksum 和msgend 贴在ServiceData的末尾以便传输。
    req->ServiceData[1].Target = req->End.Checksum;
    req->ServiceData[1].Operation = req->End.MsgEnd;
    
    
    
    hrnp_data((void *)req,  sizeof(HdtapHeader_t) + sizeof(HdtapEnd_t) + (req->Header.Length));


}

extern unsigned char ble_alive_flag;

void DigitalTrunkingBusinessService_reply(void * hdtap)
{

    DigitalTrunkingBusinessService_reply_t * reply = (DigitalTrunkingBusinessService_reply_t *)hdtap;
    if(Hdtap_Sucess == reply->Result)
    {
        printf(" Digital Trunking Business Service Notify CFG Success\r\n");
        ble_alive_flag =1;
    }
    else
    {
       printf("Digital Trunking Business Service Notify CFG Failure\r\n");
    }



}





unsigned char hdtap_receive(unsigned char *hdtap)
{
    Hrnp_t hrnp;
    if(SUCCESS == hrnp_receive(&hrnp))
    {
        //save reg hdtap
        if(hrnp.Header.Length >= sizeof(HrnpHeader_t) + sizeof(HdtapHeader_t) +sizeof(HdtapEnd_t))
        {
            HdtapHeader_t * ptr = (HdtapHeader_t *)hrnp.Payload.Data;//hdtap-header结构指向
         
            //把剩下的数据部分贴在hedep-header结构的后面
            memcpy(hdtap, hrnp.Payload.Data, hrnp.Header.Length - sizeof(HrnpHeader_t));
            
            memset((hdtap + hrnp.Header.Length- sizeof(HrnpHeader_t)) , 0x00, 540-hrnp.Header.Length);//数组空余部分清零
            
            return SUCCESS;
        }
    }
    
    return FAILURE;
}


void hdtap_exe( void * hdtap)
{
    HdtapHeader_t * ptr = (HdtapHeader_t *)hdtap;
    
    printf("receive hdtap:[ 0x%04X ]\r\n", ptr->Opcode.Store);
    
    for(int i = 0; i < MAX_HDTAP_EXE_LIST; i++)
    {
        if((ptr->Opcode.Store & 0xFFF) == (HdtapExeList[i].Opcode & 0xFFF) )
        {
            HdtapExeList[i].HdtapFunc(ptr);
            break;
        }
    } 
    
}



void hdtap_init(void)
{

    hdtap_cfg();
    
}


void hdtap_cfg(void)
{    
  /****需要调整***/  
  
    HdtapExecQue = QueueCreate(5, sizeof(HdtapExe_t));
    
    if(NULL == HdtapExecQue)return;
    
    HdtapExe_t exe;    
    
    exe.Opcode = REQUEST(TrunkingPowerUpCheck);//0x00c6
    exe.HdtapFunc = TrunkingPowerUpCheck_req;
    exe.Parameter = NULL;
    QueuePush(HdtapExecQue, &exe);
     
    exe.Opcode = REQUEST(RadioIDQuery);//0x0C02
    exe.HdtapFunc = RadioIDQuery_req;
    exe.Parameter = NULL;
    QueuePush(HdtapExecQue, &exe);
    
    
    //此指令暂时无法测试
    exe.Opcode = NOTIFY(DigitalTrunkingBusinessService);//0x1C06
    exe.HdtapFunc = DigitalTrunkingBusinessService_req;
    exe.Parameter = NULL;
    QueuePush(HdtapExecQue, &exe);
     
    HdtapSta_t sta = Hdtap_Send;
    unsigned char hdtap[540];
    
    unsigned short timeout = 0;
    unsigned short resendtimes = 0;
    
    memset(hdtap, 0x00, sizeof(hdtap));
    
    for(;;)
    {
        switch(sta)
        {
          case Hdtap_Send:
            if(NULL != exe.Parameter)
            {
                free(exe.Parameter);
                exe.Parameter = NULL;
            }
            if(queue_ok == QueuePull(HdtapExecQue, &exe))
            {
                exe.HdtapFunc(exe.Parameter);
                timeout = 0;
                sta = Hdtap_Reply;
            }
            else
            {
                QueueDelete(HdtapExecQue);
                return;
            }
            break;
            
            
          case Hdtap_Resend:
            
                exe.HdtapFunc(exe.Parameter);
                timeout = 0;
                sta = Hdtap_Reply;    
                
            break;
            
            
            
          case Hdtap_Reply:       
            delaynms(100);  
            if((SUCCESS == hdtap_receive(hdtap)) && (((HdtapHeader_t *)hdtap)->Opcode.Struct.Opcode == (exe.Opcode & 0x0FFF)))//receive
            {
                hdtap_exe(hdtap);   
                sta = Hdtap_Send;
                break;
            }
            if((timeout += 1) >= 5)//timeout resend
            {
                timeout = 0;
                sta = Hdtap_Resend;  
                if((resendtimes += 1) >= 3)//resend times
                {
                     resendtimes = 0;
                     printf("Resend times is over\r\n");
                      //sta = ExecFunc;
                }
            }  
            
            break;
          default:
              sta = Hdtap_Send;
              break;
        }
    } 
}


void MessageSending_reply(void * hdtap)
{
    MessageSending_reply_t * reply = (MessageSending_reply_t * )hdtap;
    
    printf("reply->Result:0x%x\r\n", reply->Result);
    
    if(Hdtap_Sucess == reply->Result)
    {
        printf("Message Send Service Success\r\n");
    }
    else
    {
       printf("Message Send Service Failure\r\n");
    }
}



void MessageReceivingReport_rec(void * hdtap)
{
  
  MessageReceivingReport_brd_t *rec = (MessageReceivingReport_brd_t *)hdtap;
  
//    rec->RequestID = htonl(rec->RequestID);
//    rec->DestIP = htonl(rec->DestIP);
//    rec->SrcIP = htonl(rec->SrcIP);
       
//    if(ACK_Required == rec->Header.Opcode.TMS.Ack)
//    {
//        PrivateMessage_sendack(rec);
//    } 
    
  printf("[1.RECEIVE MSG NOTICE CallType: %d]\r\n", rec->CallType);
    
  printf("[2.RECEIVE MSG NOTICE Option: %d]\r\n", rec->Option);
    
  printf("[3.RECEIVE MSG NOTICE Length: %d]\r\n", rec->Datalen);
  
  printf("[4.RECEIVE MSG NOTICE GroupID: 0x%x]\r\n", rec->GroupID);
  
  printf("[5.RECEIVE MSG NOTICE SourceID: 0x%x]\r\n", rec->SourceID);
    
  printf("[6.AND SEND MSG TO BLE]\r\n");
    
    //直接发的是纯数据信息，没有相关的长度之类的结构。
   // msg_receive_event(rec->TMData); 
    
    unsigned char rxchecksum, rxmsgend;
    
    rxchecksum = rec->MsgData[rec->Datalen];
    rxmsgend = rec->MsgData[rec->Datalen+1];
    
    //因为数据长度的问题，end数据在TMdata最后
    rec->End.Checksum = rxchecksum;
    rec->End.MsgEnd = rxmsgend;
    
    
    /**************测试***************/
    
    //checksum Hdtap data
    unsigned char rxcheck = hdtap_checksum(rec, rec->Header.Length);
    if( rxcheck == rec->End.Checksum)
    {
          //注意测试一下结构
          OB_Message_t Msg;
          
          Msg.type = TrunkingMsg;
          Msg.dest = RadioID;//连接的时候从本地Radio获取
          Msg.src = rec->SourceID;
          Msg.TMLen = rec->Datalen;
          
          memcpy(Msg.TMData, rec->MsgData, Msg.TMLen);
          
          memset(&(Msg.TMData[Msg.TMLen]), 0x00, 512 - Msg.TMLen );//剩余部分清零
          
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
      
      printf("\r\n Hdtap checksum error \r\n");
      
      //Hdtap checksum error
    
    }
    
  
  
  


}


void MessageSendingReq_rec(void * hdtap)
{
  /*收到中继台发过来的短消息准备发送给BLE设备，并回复Reply*/
  //待调试
  printf("OB-Rx：Message Req \r\n ");

}



