#include "TFTP.h"

int main()
{
int server_sock = socket(AF_INET,SOCK_DGRAM,0);
if(server_sock < 0)
{
    printf("socket creation failed\n");
    return 0;
}

struct sockaddr_in server_details;
memset(&server_details, 0, sizeof(server_details));
server_details.sin_family=AF_INET;
server_details.sin_port=htons(5000);
server_details.sin_addr.s_addr=inet_addr("127.0.0.1");

/*binding*/
if(bind(server_sock,(struct sockaddr*)&server_details,sizeof(server_details))<0)
{
    printf("bind failed\n");
    return 0;
}

printf("server waiting...\n");

tftp_packet pkt;
struct sockaddr_in client_details;
socklen_t client_len=sizeof(client_details);

while(1)
{
ssize_t bytes_recieved = recvfrom(server_sock,&pkt,sizeof(pkt),0,(struct sockaddr *)&client_details,&client_len);

if(bytes_recieved < 0)
{
    continue;
}
switch(pkt.opcode)
{
    case RRQ:send_file(server_sock, &pkt, &client_details, client_len);
    printf("RRQ Recived:%s  file in %s mode\n",pkt.body.request_packet.filename,pkt.body.request_packet.mode);
    break;
    case WRQ:rec_file(server_sock, &pkt, &client_details, client_len);
    break;
    case DATA:
    printf("Unexpected DATA packet ignored\n");
    break;
    case ACK:
    printf("Unexpected ACK packet ignored\n");
    break;
    case ERROR:
    printf("ERROR packet received from client\n");
    break;
    case CONNECT_REQ:
    printf("Client connected\n");
    pkt.opcode = ACK;
    sendto(server_sock, &pkt, sizeof(pkt), 0,(struct sockaddr *)&client_details, client_len);
    break;
    default:
    break;
}

}

}
void rec_file(int sock,tftp_packet *req, struct sockaddr_in *client, socklen_t len)
{
    tftp_packet pkt;
    int expected = 1;
    int fd2 = open(req->body.request_packet.filename,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if(fd2 < 0)
    {
        printf("file creation failed\n");
        return;
    }
 //ack for WRQ
    memset(&pkt,0,sizeof(pkt));
    pkt.opcode = ACK;
    sendto(sock,&pkt,sizeof(pkt),0,(struct sockaddr *)client, len);
    
    //adjusting server not to wait more than 5 seconds
     struct timeval tv = {5, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while(1)
    {
        int n = recvfrom(sock,&pkt,sizeof(pkt),0,NULL,NULL);
        if(n < 0)
        {
            printf("failed to recieve data\n");
            break;
        }
        int size = pkt.body.data_packet.size;
        int block = pkt.body.data_packet.packet_no;
        if((size_t)n < offsetof(tftp_packet,body.data_packet.data)+size)
        continue;
        if(block == expected)
        {
            write_data(fd2,pkt.body.data_packet.data,size,req->body.request_packet.mode);
            expected++;
        }
        pkt.opcode = ACK;
        pkt.body.ack_packet.packet_no = block;
        sendto(sock,&pkt,sizeof(pkt),0,(struct sockaddr *)client,len);
        if(block == expected-1 && size == 0)
        {
            printf("data recieved\n");
            break;
        }
    }
    close(fd2);
    //remove the timeout ,let server wait normally for next packet
    struct timeval none = {0, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &none, sizeof(none));
}

void send_file(int sock, tftp_packet *req, struct sockaddr_in *client, socklen_t len)
{
    tftp_packet pkt, reply;

    printf("RRQ received: %s (%s)\n",req->body.request_packet.filename, req->body.request_packet.mode);

    /* 1. the file must exist, send ACK */
    int fd = open(req->body.request_packet.filename, O_RDONLY);
    if (fd < 0)
    {
        memset(&pkt, 0, sizeof(pkt));
        pkt.opcode = ERROR;
        pkt.body.error_packet.error_code = 1;
        sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)client, len);
        printf("File not found\n");
        return;
    }

    /* wait only 3 seconds for each ACK */
    struct timeval tv = {3, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /*sending the file packet by packet */
    int block = 1;
    int bytes;
    int done = 1;

    do
    {
        memset(&pkt, 0, sizeof(pkt));
        bytes = read_data(fd, pkt.body.data_packet.data,req->body.request_packet.mode); 

        pkt.opcode = DATA;
        pkt.body.data_packet.packet_no = block;
        pkt.body.data_packet.size = bytes;

        int ack = 0, tries = 0;
        while (!ack && tries < 5)
        {
            sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)client, len);

            int n = recvfrom(sock, &reply, sizeof(reply), 0, NULL, NULL);
            if (n >= 0 && reply.opcode == ACK && reply.body.ack_packet.packet_no == block)
                ack = 1;         //success,send the next packet
            else
                tries++;          //failure send the same packet again
        }

        if (!ack)
        {
            printf("Client not responding, transfer stopped\n");
            done = 0;
            break;
        }

        block++;
    } while (bytes > 0);          

    if (done)
        printf("File sent\n");

    /*clean up and go back to waiting for requests with no timeout */
    close(fd);
    struct timeval none = {0, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &none, sizeof(none));
}