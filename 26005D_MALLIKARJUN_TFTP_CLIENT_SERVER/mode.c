#include "TFTP.h"

int read_data(int fd,char *buf,char *mode)
{
    if(strcmp(mode,"octet")==0)
    {
        return read(fd,buf,1);
    }
    if(strcmp(mode,"netascii"))
    {
        int size = 0;
        char c;
        while(size < DATA_PACKET_SIZE - 1 && read(fd,&c,1)==1)
        {
            if(c == '\n')
            {
                buf[size++] = '\r';
            }
            buf[size++] = c; 
        }
        return size;
    }
    return read(fd,buf,DATA_PACKET_SIZE);
}
void write_data(int fd,char *buf,int size,char *mode)
{
    if(strcmp(mode,"netascii")==0)
    {
        for(int i=0;buf[i]!='\0';i++)
        {
            if(buf[i]!='\r')
            write(fd,&buf[i],1);
        }
    }
    else
    write(fd,buf,size);
}