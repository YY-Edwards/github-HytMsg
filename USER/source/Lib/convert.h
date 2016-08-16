#ifndef _CONVERT_H_
#define _CONVERT_H_

unsigned short htons(unsigned short in);
unsigned int htonl(unsigned int in);
unsigned int utf82unicode(unsigned char * unicode, unsigned char * utf8, unsigned int utf8_bytescount);

#endif