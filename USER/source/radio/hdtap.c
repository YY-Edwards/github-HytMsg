#include "hdtap.h"


void MessageSendingRequest(void * p)
{
  
  /***��Ҫ����Э��**/
  
    Message_t * msg = (Message_t *)p;
    
    TrunkingMessage_req_t TrunkingMessage_req, *req = &TrunkingMessage_req;
    
    req->Header.MshHdr = HDTAP;//0x02
   
    HdtapOpcode_t op;
    
    op.DTBS.H_Opcode = 0x0c;
    op.DTBS.L_Opcode = 0x08;
    
    req->Header.Opcode.Store = op.Store;//0x0c08
       
    if(msg->Header.Length >= 510)req->Header.Length = 510;//���ble�����������ݳ���510bytes,���������ֽڸ�END
    else
      req->Header.Length = (msg->Header.Length + 8);//TargetID[4]+CallType[1]+Option[1]+Datalen[2]+Msg[Datalen]
    
    req->TargetID = ID2IP(2);
    //req->DestIP = htonl(ID2IP(msg->Header.Address));
    
    printf("[RECEIVE BLE MSG AND SEND TO TargetID: %x]\r\n", req->TargetID); 
    
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
            //��radio�������Ķ�������ÿ�����ֽڽ��д�С��ת��Ϊ������unicode��
            temp = req->TMData[i];
            req->TMData[i] = req->TMData[i+1];
            req->TMData[i+1] = temp;
            i+=2;
          
          }
    
    //memset(&(req->TMData[msg->Header.Length]), 0x00, 512-msg->Header.Length);//TMDataδ�õĲ������㡣

  
    //ע���ַ
    req->End.Checksum =  hdpep_checksum(req, req->Header.Length);
    req->End.MsgEnd = MSH_END;
      
    //Э��ṹ����,��checksum ��msgend ����TMData��ĩβ�Ա㴫�䡣
      
      //req->TMData[msg->Header.Length] =  req->End.Checksum; 
      //req->TMData[msg->Header.Length+1] =  req->End.MsgEnd; 
      
    //HdtapEnd_t * end = (HdtapEnd_t *)((unsigned char *)req + sizeof(HdtapHeader_t) + msg->Header.Length);
    
    //end->Checksum = hdpep_checksum(req, htons(req->Header.Length));
    //end->MsgEnd = MSH_END; 

    
    hrnp_data((void *)req,  sizeof(HdtapHeader_t) + sizeof(HdtapEnd_t) + msg->Header.Length + 8);



}