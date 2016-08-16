#include "convert.h"

unsigned short htons(unsigned short in)
{
  unsigned char tmp[2];
  tmp[0] = (in >> 8) & 0xFF;
  tmp[1] = in & 0xFF;
  return  (tmp[1] << 8) + tmp[0];
}

unsigned int htonl(unsigned int in)
{
  unsigned char tmp[4];
  tmp[0] = (in >> 24) & 0xFF;
  tmp[1] = (in >> 16) & 0xFF;
  tmp[2] = (in >> 8) & 0xFF;
  tmp[3] = in & 0xFF;
  return  (tmp[3] << 24) + (tmp[2] << 16) + (tmp[1] << 8) + tmp[0];
}

unsigned int utf82unicode(unsigned char * unicode, unsigned char * utf8, unsigned int utf8_bytescount)
{
    unsigned char *p = utf8;
    unsigned int unicode_bytescount = 0;
    
    while(1)
    {
        if (0 == (*p &0x80))
        {
            unicode[unicode_bytescount++] = *p;
            unicode[unicode_bytescount++] = 0;
            p++;
        }
        else if( 0xE0 == (*p & 0xf0)) 
        {
            unicode[unicode_bytescount++] = (( *(p + 1) & 0x03) <<6) + (*(p + 2) & 0x3F);
            unicode[unicode_bytescount++] = ((*p & 0x0F) <<4) + ((*(p + 1) & 0x3C ) >>2);
            p += 3;
        }
        else
        {
            break;
        }
        
        if(p >= utf8 + utf8_bytescount)break;       
    }
    
    return unicode_bytescount;
}
