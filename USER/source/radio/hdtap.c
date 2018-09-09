#include "hdtap.h"
#include "bluetooth.h"

Queue_t HdtapExecQue = NULL;

static unsigned int RadioID  = 0;
static unsigned int Master_Turnking_ID  = 0x00FC02B2;//南方电网基站集群下的ID号
bool trunking_msg_send_okay_flag= true;
void ( *HdtapOnMessageFunc)(void *); 
void ( *HdtapMsgResultFunc)(unsigned char, unsigned char); 

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
  
    Ble_Message_Pro_t * msg = (Ble_Message_Pro_t *)p;
    
    TrunkingMessage_req_t TrunkingMessage_req, *req = &TrunkingMessage_req;
    
    req->Header.MshHdr = HDTAP;//0x02
   
    HdtapOpcode_t op;
    
    op.DTBS.L_Opcode = 0x08;
    op.DTBS.H_Opcode = 0x0c;
    
    req->Header.Opcode.Store = op.Store;//0x0c08
    
    //集群模式最大可发送248bytes的负载数据
    if(msg->Header.Length >= 248)req->Header.Length = (248+8);//如果ble发过来的数据超过248bytes,则留两个字节给END
    else
      req->Header.Length = (msg->Header.Length + 8);//TargetID[4]+CallType[1]+Option[1]+Datalen[2]+Msg[Datalen]
    
    /**Target radio ID (air interface value) which a message to be sent to**/
    //req->TargetID = ID2IP(2);
    req->TargetID = Master_Turnking_ID;//RadioID ;
    //req->DestIP = htonl(ID2IP(msg->Header.Address));
    
    printf("[RECEIVE BLE MSG AND SEND TO TargetID: 0x%08X]\r\n", req->TargetID); 
    
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
    
    memset(&(req->TMData[msg->Header.Length]), 0x00, Hdtap_Msg_Payload_Len-(msg->Header.Length));//TMData未用的部分清零。

  
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





void RegisterStatusQuery_req(void *p)
{

  RegisterStatusQuery_req_t  RegisterStatusQuery_req,  * req = &RegisterStatusQuery_req;
    
  req->Header.MshHdr = HDTAP;
  req->Header.Opcode.Struct.Mask = REQUEST_MASK;
  req->Header.Opcode.Struct.Opcode = RegisterServiceQuery;
  req->Header.Length = 0;
 
  req->End.Checksum = hdtap_checksum(req, req->Header.Length);
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


void RadioIDQuery_req(void *p)
{
  
     RadioIDQuery_req_t  RadioIDQuery_req,  * req = &RadioIDQuery_req;
    
    req->Header.MshHdr = HDTAP;
    req->Header.Opcode.Struct.Mask = REQUEST_MASK;
    req->Header.Opcode.Struct.Opcode = RadioIDQuery;
    req->Header.Length = 0;
   
    req->End.Checksum = hdtap_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdtapHeader_t) + sizeof(HdtapEnd_t));
 

}



void RegisterStatusQuery_reply(void *hdtap)
{

    RegisterStatusQuery_reply_t * reply = (RegisterStatusQuery_reply_t *)hdtap;
        
    if(Hdtap_Sucess == reply->Status)
    {               
        printf("Radio register successful. \r\n");       
     
    }
    else
    {
       printf("Radio unregister!\r\n");
    }



}


void SystemModeOperation_req(void *p)
{
     SystemModeOperation_req_t  SystemModeOperation_req,  * req = &SystemModeOperation_req;
    
    req->Header.MshHdr = HDTAP;
    req->Header.Opcode.Struct.Mask = REQUEST_MASK;
    req->Header.Opcode.Struct.Opcode = SystemModeOperation;
    req->Header.Length = 2;
    req->Option = SystemModeOperation_Read;
    req->Mode = SystemMode_DigitalTrunking;//when the "option" is "set", this value is valid;
   
    req->End.Checksum = hdtap_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END; 
    
    hrnp_data((void *)req,  sizeof(HdtapHeader_t) + sizeof(HdtapEnd_t));

}
void SystemModeOperation_reply(void *hdtap)
{
    SystemModeOperation_reply_t * reply = (SystemModeOperation_reply_t *)hdtap;
        
    if(Hdtap_Sucess == reply->Result)
    {               
        //printf("Radio register successful. \r\n");   
      printf("request system run-mode: \n");  
      printf("operation: 0x%x\n", reply->Option); 
      printf("mode: 0x%x\n", reply->Mode); 
    }
    else
    {
       printf("request system mode failure\r\n");
    }


}




void DigitalTrunkingBusinessService_req(void * p)
{
    DigitalTrunkingBusinessService_req_t  DigitalTrunkingBusinessService_req,  * req = &DigitalTrunkingBusinessService_req;
    
    req->Header.MshHdr = HDTAP;
    req->Header.Opcode.Struct.Mask = NOTIFY_MASK;
    req->Header.Opcode.Struct.Opcode = (DigitalTrunkingBusinessService & 0x0fff);
    
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
            //HdtapHeader_t * ptr = (HdtapHeader_t *)hrnp.Payload.Data;//hdtap-header结构指向
         
            //把剩下的数据部分贴在hedep-header结构的后面
            memcpy(hdtap, hrnp.Payload.Data, hrnp.Header.Length - sizeof(HrnpHeader_t));
            
            memset((hdtap + hrnp.Header.Length- sizeof(HrnpHeader_t)) , 0x00, 540-hrnp.Header.Length);//数组空余部分清零
            
            return SUCCESS;
        }
        else if(hrnp.Header.Opcode == HRNP_REJECT)//需要重连
        {         
          printf("[need to reconnect radio.] \r\n");
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

void set_hdtap_on_message_callback(void( *cb)(void *))
{

  HdtapOnMessageFunc = cb;//注册集群模式下radio收到短信内容的回调函数
    
}

void set_hdtap_msg_trans_result_callback(void( *cb)(unsigned char, unsigned char))
{
  HdtapMsgResultFunc = cb;//注册集群模式下radio短信发送结果的回调函数
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
    
//    exe.Opcode = REQUEST(SystemModeOperation);//0x0C10
//    exe.HdtapFunc = SystemModeOperation_req;
//    exe.Parameter = NULL;
//    QueuePush(HdtapExecQue, &exe);
    
    exe.Opcode = REQUEST(RegisterServiceQuery);//0x0C18
    exe.HdtapFunc = RegisterStatusQuery_req;
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
    
    
    for(;;)
    {
        memset(hdtap, 0x00, sizeof(hdtap));//clear 
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
            
            if(SUCCESS == hdtap_receive(hdtap))
            {
              if(((HdtapHeader_t *)hdtap)->Opcode.Struct.Opcode == (exe.Opcode & 0x0FFF))
              {
                hdtap_exe(hdtap);   
                sta = Hdtap_Send;
                break;
              }
              else
              {
                printf("receive unkown hdtap:[ 0x%04X ]\r\n", ((HdtapHeader_t *)hdtap)->Opcode.Struct.Opcode);
              }
            
            }
      
            if((timeout += 1) >= 5)//timeout resend
            {
                timeout = 0;
                sta = Hdtap_Resend;  
                if((resendtimes += 1) >= 3)//resend times
                {
                     resendtimes = 0;
                     printf("Resend times is overflow\r\n");
                     sta = Hdtap_Send;//超时则丢弃，进行下一次发送
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
    
    //printf("reply->Result:0x%x\r\n", reply->Result);
    
     switch(reply->Result)
    {
        case Success:
              
              trunking_msg_send_okay_flag = true;
              printf("Message Send Service: Success.\r\n");          
        break;
        
        case Failure:
          
              printf("Message Send Service: Failure.\r\n");
          
        break;
        
        case Unregistered:
              printf("Message Send Service: Mater Radio is Unregistered.\r\n");
              
        break;
        
        case Low_Battery:
          
              printf("Message Send Service: Low_Battery.\r\n");
        break;
        
        case Disabled_ID:
               printf("Message Send Service: target radio ID is disabled.\r\n");
          
        break;
        
        case Disabled_Status_Code:
               printf("Message Send Service: Disabled_Status_Code.\r\n");
        break;
        
    default:
      break;
      
    
    }
    
    HdtapMsgResultFunc(CMD_NOTIFY_MSG_SEND_RESULT, reply->Result);
    
//    if(Hdtap_Sucess == reply->Result)
//    {
//        printf("Message Send Service Success\r\n");
//    }
//    else
//    {
//       printf("Message Send Service Failure\r\n");
//    }
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
    if(rxcheck == rec->End.Checksum)
    {
      //注意测试一下结构
      OB_Message_t Msg;
      
      Msg.type = TrunkingMsg;
      Msg.dest = RadioID;//连接的时候从本地Radio获取
      Msg.src = rec->SourceID;
      Msg.TMLen = rec->Datalen;
      
      memcpy(Msg.TMData, rec->MsgData, Msg.TMLen);    
      memset(&(Msg.TMData[Msg.TMLen]), 0x00, 1002 - Msg.TMLen );//剩余部分清零
      
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
      
     HdtapOnMessageFunc(&Msg);//ble_assemble_data_packet(&Msg);
     //app_rec_msg(&Msg);         
    }
    else{
      
      printf("\r\n Hdtap checksum error \r\n");
      
      //Hdtap checksum error
    
    }
    
}


//void MessageSendingReq_rec(void * hdtap)
//{
//  /*收到中继台发过来的短消息准备发送给BLE设备，并回复Reply*/
//  //待调试
//  printf("OB-Rx：Message Req \r\n ");
//
//}



