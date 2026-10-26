#include "TFTP.h"

struct sockaddr_in server_addr;
int client_sock = -1;
int validate_ip(char *ip)
{
    int i=0;
    int dots = 0;
    for(i=0;ip[i]!='\0';i++)
    {
        if( ip[i] == '.')
        {
            dots++;
        }
       else if(ip[i]<'0'|| ip[i] > '9')
       {
        return 0;
       }
    }
    return dots == 3;
}

int connect_to_server(char *ip,int port)
{
    if(!(validate_ip(ip)))
    {
        printf("Invalid ip address\n");
        return -1;
    }
     if (port < 1024 || port > 65535)
    {
        printf("Invalid port number\n");
        return -1;
    }

     client_sock = socket(AF_INET,SOCK_DGRAM,0);
    if(client_sock < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }

    /* wait only 3 seconds for the server's reply */
    struct timeval tv = {3, 0};
    setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    tftp_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.opcode = CONNECT_REQ;

    sendto(client_sock, &pkt, sizeof(pkt), 0,
           (struct sockaddr *)&server_addr, sizeof(server_addr));

    int n = recvfrom(client_sock, &pkt, sizeof(pkt), 0, NULL, NULL);
    if (n < 0 || pkt.opcode != ACK)
    {
        printf("Server not responding\n");
        close(client_sock);
        client_sock = -1;
        return -1;
    }

    printf("Connected to server\n");
    return 0;

}
int put_file(char *filename,char* mode)
{
    if(client_sock < 0)
    {
        printf("connection failed\n");
        return -1;
    }
    int fd1 = open(filename,O_RDONLY);
    if (fd1 < 0)
    {
        printf("File not found\n");
        return -1;
    }
    tftp_packet pkt;
    memset(&pkt,0,sizeof(pkt));
    pkt.opcode = WRQ;
    strcpy(pkt.body.request_packet.filename,filename);
    strcpy(pkt.body.request_packet.mode,mode);
    sendto(client_sock,&pkt,sizeof(pkt),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
    
    tftp_packet reply; //instead of storing in the variable pkt ,using reply to avoid overwritting
    int n = recvfrom(client_sock,&reply,sizeof(reply),0,NULL,NULL);

    if(n < 0)
    {
        printf("Server not responding\n");
        close(fd1);
        return -1;
    }
    if(pkt.opcode == ERROR)
    {
        printf("server error\n");
        return -1;
    }
    if(pkt.opcode == ACK)
    {
        printf("server ready,now start sending the data\n");
    }

    int bytes;
    int block = 1;
    do
    {
        bytes = read_data(fd1,pkt.body.data_packet.data,mode);
        if(bytes < 0)
        {
            printf("file error\n");
            close(fd1);
            return -1;
        }
        pkt.opcode = DATA;
        pkt.body.data_packet.packet_no = block;
        pkt.body.data_packet.size = bytes;
        

        int ack = 0,tries=0;
        while(!ack && tries < 5)  //loop to avoid resending same data forever
        {
            sendto(client_sock,&pkt,sizeof(pkt),0,(struct sockaddr *)&server_addr, sizeof(server_addr));

            int x = recvfrom(client_sock,&reply,sizeof(reply),0,NULL,NULL);
            if(x>=0 && reply.body.data_packet.packet_no == block && reply.opcode == ACK)
            {
                ack = 1; // success send next data packet
            }
            else
            {
                tries++;
            }
        }
        if(!ack)
        {
            printf("data transfer failed at block %d\n",block);
            close(fd1);
            return -1;
        }
        block++;
    }while(bytes > 0);

   printf("data sent\n");
   close(fd1);
   return 0;

}

int get_file(char *filename,char *mode)
{
    if(client_sock < 0)
    {
        printf("connection failed\n");
        return -1;
    }
    tftp_packet pkt;
    memset(&pkt,0,sizeof(pkt));
    pkt.opcode = RRQ;
    strcpy(pkt.body.request_packet.filename,filename);
    strcpy(pkt.body.request_packet.mode,mode);
    sendto(client_sock,&pkt,sizeof(pkt),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
    
    /*local file is created only after the first data packet is recieved*/
    int fd = -1;
    int expected = 1;

    while(1)
    {
        int n =recvfrom(client_sock,&pkt,sizeof(pkt),0,NULL,NULL);
        if(n<0)
        {
            printf("connection failed ,no data recived\n");
            if (fd >= 0)
            close(fd);
            return -1;
        }

        if(pkt.opcode == ERROR)
        {
            printf("file not found in the server\n");
            if (fd >= 0)
            close(fd);
            return -1;
        }
         if(pkt.opcode != DATA)
        {
            continue;
        }
        if(fd<0)
        {
            fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0644);
            if(fd < 0)
            {
                printf("failed to open file\n");
                return -1;
            }
        }
        int size = pkt.body.data_packet.size;
        int block = pkt.body.data_packet.packet_no;

        if(block == expected)
        {
            write_data(fd,pkt.body.data_packet.data,size,mode);
            expected++;
        }

        //ACK with a same packet number
        pkt.opcode = ACK;
        pkt.body.ack_packet.packet_no = block;

        sendto(client_sock,&pkt,sizeof(pkt),0,(struct sockaddr*)&server_addr,sizeof(server_addr));
        if(block == expected-1 && size == 0)
        {
            break;
        }
    }
    close(fd);
    printf("File received\n");
    return 0;

}
void set_mode(char *mode)
{
    int ch;

    printf("1.Normal\n2.Octet\n3.Net-ascii\n");
    scanf("%d", &ch);

    switch (ch)
    {
        case 1: strcpy(mode, "Normal");    
        break;
        case 2: strcpy(mode, "octet");     
        break;
        case 3: strcpy(mode, "netascii");  
        break;
        default: printf("Invalid choice, mode unchanged\n"); 
        return;
    }
    printf("Mode set to %s\n", mode);
}

