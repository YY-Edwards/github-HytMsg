#include "radio/hrnp.h"

static unsigned char GobCurrID = OB_ID_BASS;
static unsigned short HrnpPN = 0;

static unsigned char IsHrnpConnect = 0;

unsigned char  hrnp_connect(void);

unsigned char hrnp_close_ack(Hrnp_t *);
unsigned char hrnp_data_ack(Hrnp_t *);

void hrnp_init(void)
{
    physical_init();
    IsHrnpConnect = hrnp_connect();
    printf("Hrnp Connect Finish\r\n");
}

unsigned short GetHrnpChecksum(unsigned char * pHeader, unsigned char *pData, unsigned int len)
{
    unsigned int  HalfLen, sum      = 0; 
    unsigned short  temp, Checksum    = 0;  
    unsigned char   i,OddFlag;
        
    /* HRNP packet check */
    if((NULL == pHeader)||(len < 12))
    {
        return 0;
    }
    
    /* calculate header */
    for( i = 0; i < 10; i += 2 )
    {
        temp = (pHeader[i] << 8) + pHeader[i+1];
        sum += temp;
    }

    /* calculate HdpepData */
    if((len > 12)&&(NULL != pData))
    {   
        OddFlag = (len - 12)%2;
		HalfLen = (len - 12)/2;
        for( i = 0; i < HalfLen; i ++ )
        {
            temp = (pData[2*i] << 8) + pData[2*i+1];
            sum += temp;
        }
        if(1 == OddFlag)
        {
            sum += pData[2*i] << 8;
        }
    }
    while(sum >> 16)
	{
        sum = (sum & 0xFFFF) + (sum >> 16);
	}
    Checksum = (uint16_t)sum;
    return ~Checksum;
}

void hrnp_send(Hrnp_t * hrnp)
{
    hrnp->Header.Header = HRNP_HEADER;
    hrnp->Header.Version = HRNP_VER;
    hrnp->Header.Block = 0;
    //opcodeÔÚÍâ²ã
    hrnp->Header.SourceID = GobCurrID;
    hrnp->Header.DestinationID = MASTER_ID;   
    
    hrnp->Header.PN = (DEFAULT_PN == hrnp->Header.PN ) ? htons(HrnpPN) : htons(hrnp->Header.PN);
    
    hrnp->Header.Length = htons(hrnp->Header.Length);
    hrnp->Header.CheckSum = htons(GetHrnpChecksum((unsigned char * )hrnp, hrnp->Payload.Data, htons(hrnp->Header.Length))); 
    usart_send((void *)hrnp,htons(hrnp->Header.Length));
}

unsigned char hrnp_receive(Hrnp_t * hrnp)
{
    unsigned char timeout = 0;
    unsigned char res = SUCCESS;
    for(;;)
    {
      unsigned char ch = 0;  
      if( SUCCESS != usart_receive(&ch))return FAILURE;//no data
      
      if(HRNP_HEADER == ch)//header
      {                   
        hrnp->Header.Header = 0x7E;
        //if( SUCCESS != usart_receive(&ch))return FAILURE;
        
        for(int i = 0; i < sizeof(HrnpHeader_t) - 1; i++)
        {              
            while(1)
            {
              if(SUCCESS != usart_receive((unsigned char *)&hrnp->Header + 1 + i))
              {
                  delaynms(100);
                  if((timeout+=1) >=10)return FAILURE;//timeout 1s
              }
              else
              {
                  timeout = 0;
                  break;
              }   
            }
        }
                       
        //get  payload
        unsigned short payload_length = htons(hrnp->Header.Length) - sizeof(HrnpHeader_t);
              
        for(int i = 0; i < payload_length; i++)
        {
            while(1)
            {
              if(SUCCESS != usart_receive(hrnp->Payload.Data + i))
              {
                  delaynms(100);
                  if( (timeout += 1) >=10)return FAILURE;//timeout
              }
              else
              {
                  timeout = 0;
                  break;
              }
            }
        }     
          
        //checksum
        unsigned short checksum = GetHrnpChecksum((unsigned char * )hrnp, hrnp->Payload.Data,htons(hrnp->Header.Length));
        hrnp->Header.CheckSum = htons(hrnp->Header.CheckSum);
        
        if(checksum != hrnp->Header.CheckSum)return FAILURE;//check sum error;
        
        hrnp->Header.Length = htons(hrnp->Header.Length);
        hrnp->Header.PN = htons(hrnp->Header.PN);
                
        
        printf("OB-R-Hrnp_Opcode:0x%x\r\n", hrnp->Header.Opcode);
        //send ack
        if(HRNP_CLOSE == hrnp->Header.Opcode)
        {
            hrnp_close_ack(hrnp);
        }
        else if(HRNP_DATA == hrnp->Header.Opcode)
        {
            
            hrnp_data_ack(hrnp);
            
            
        }
        else
        {
          //other Opcode
         // printf("OB-R-Hrnp_Opcode:%x\r\n", hrnp->Header.Opcode);
          //Mater answer to a HRNP_DATA_ACK packet
          if(hrnp->Header.Opcode == HRNP_DATA_ACK)
          {
            
            ;//ACK_NotRequired, so no ACK 
          
          }
          
         
        }

        return SUCCESS;
      }
      else
      {
        continue;
      }
    }
}

unsigned char  hrnp_connect(void)
{
    HrnpSta_t sta = WaitToSend;
    unsigned char timeoutnumber = 0;
    
    Hrnp_t hrnp;
    for(;;)
    {
        switch(sta)
        {
            case WaitToSend:
                  
                  hrnp.Header.Opcode = HRNP_CONNECT;
                  hrnp.Header.PN = DEFAULT_PN;
                  hrnp.Header.Length = sizeof(HrnpHeader_t);
                      
                  hrnp_send(&hrnp); 
               
                  sta = WaitAck;
                  break;
            case WaitAck:
                
                if(SUCCESS != hrnp_receive(&hrnp))
                {
                    delaynms(100);
                    timeoutnumber++;
                    if(timeoutnumber >= 10)//1s
                    {
                      sta = WaitToSend; 
                      //return FAILURE;
                    }
                }
                else
                {
                    if(HRNP_ACCPET == hrnp.Header.Opcode)
                    {
                        HrnpPN += 1;
                        GobCurrID =  hrnp.Header.DestinationID;                     
                        return SUCCESS;
                    }
                    else
                    {
                         sta = WaitToSend;
                    }
                }
              break;
        }
    }
}

unsigned char hrnp_close(void)
{
    HrnpSta_t sta = WaitToSend;
    unsigned char timeoutnumber = 0;
    
    if(SUCCESS != IsHrnpConnect )return FAILURE;
    
    Hrnp_t hrnp;
    for(;;)
    {
        switch(sta)
        {
            case WaitToSend:
                  
                  hrnp.Header.Opcode = HRNP_CLOSE;
                  hrnp.Header.Length = sizeof(HrnpHeader_t);
                  hrnp.Header.PN = DEFAULT_PN;    
                  hrnp_send(&hrnp); 
                  
                  sta = WaitAck;
                  break;
            case WaitAck:
                delaynms(100);
                if(SUCCESS != hrnp_receive(&hrnp))
                {
                    timeoutnumber++;
                    if(timeoutnumber >= 10)//1s
                    {
                      sta = WaitToSend; 
                      //return FAILURE;
                    }
                }
                else
                {
                    if(HRNP_CLOSE_ACK == hrnp.Header.Opcode)
                    {
                        HrnpPN += 1;
                        GobCurrID =  hrnp.Header.DestinationID;   
                        return SUCCESS;
                    }
                    else
                    {
                         sta = WaitToSend;
                    }
                }
              break;
        }
    }
}

unsigned char hrnp_close_ack(Hrnp_t * rx)
{
  if(SUCCESS != IsHrnpConnect )return FAILURE;  
    Hrnp_t hrnp;  
    hrnp.Header.Opcode = HRNP_CLOSE_ACK;
    hrnp.Header.PN = rx->Header.PN;
    hrnp.Header.Length = sizeof(HrnpHeader_t);
                      
    hrnp_send(&hrnp);
    
     return SUCCESS;
}

unsigned char hrnp_data(unsigned char * dat, unsigned short length)
{
    HrnpSta_t sta = WaitToSend;
    unsigned char timeoutnumber = 0;
    if(SUCCESS != IsHrnpConnect )return FAILURE;
    Hrnp_t hrnp;
    for(;;)
    {
        switch(sta)
        {
            case WaitToSend:
                  
                  hrnp.Header.Opcode = HRNP_DATA;
                  hrnp.Header.Length = sizeof(HrnpHeader_t) + length;
                  hrnp.Header.PN = DEFAULT_PN;
                  memcpy(hrnp.Payload.Data, dat, length);
                  hrnp_send(&hrnp); 
                  
                  sta = WaitAck;
                  break;
            case WaitAck:
                delaynms(100);
                if(SUCCESS != hrnp_receive(&hrnp))
                {
                    timeoutnumber++;
                    if(timeoutnumber >= 10)//1s
                    {
                      sta = WaitToSend; 
                      printf("Time-Out\r\n");
                      
                      return FAILURE;
                    }
                }
                else
                {
                    if(HRNP_DATA_ACK == hrnp.Header.Opcode)
                    {
                        HrnpPN += 1;
                        GobCurrID =  hrnp.Header.DestinationID;   
                        return SUCCESS;
                    }
                    else
                    {                       
                        sta = WaitToSend;
                    }
                }
              break;
        }
    }
}

unsigned char hrnp_data_ack(Hrnp_t * rx)
{    
  if(SUCCESS != IsHrnpConnect )return FAILURE;
    Hrnp_t hrnp;  
    hrnp.Header.Opcode = HRNP_DATA_ACK;
    hrnp.Header.PN = rx->Header.PN;
    hrnp.Header.Length = sizeof(HrnpHeader_t);
                         
    hrnp_send(&hrnp);
    return SUCCESS;
}




