#ifndef TFTP_H
#define TFTP_H

#define RRQ         1
#define WRQ         2
#define DATA        3
#define ACK         4
#define ERROR       5
#define CONNECT_REQ 6

#include<stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include<stddef.h>
#include <arpa/inet.h>

#define FILENAME_SIZE 1024
#define MODE_SIZE 128
#define DATA_PACKET_SIZE 513

extern int server_sock;
extern struct sockaddr_in server_addr;
/* TFTP Packet Structure */
typedef struct 
{
    uint16_t opcode;    // Operation code (RRQ/WRQ/DATA/ACK/ERROR)

    union tftp
    {
        struct
        {
            char filename[FILENAME_SIZE];   
            char mode[MODE_SIZE];           // Typically "Normal"
        } request_packet;   // RRQ and WRQ

        struct
        {
            uint16_t packet_no;
            uint16_t size;
            char data[DATA_PACKET_SIZE];   
        } data_packet;      // Data

        struct
        {
            uint16_t packet_no;
            uint16_t size;
        } ack_packet;       // ACK

        struct
        {
            uint16_t error_code;
        } error_packet;     // ERROR
        
    }body;
    
} tftp_packet;

int validate_ip(char *ip);
int connect_to_server(char *ip,int port);

int put_file(char *filename,char* mode);
int get_file(char *filename,char* mode);
void set_mode(char *mode);
int read_data(int fd, char *buf, char *mode);
void write_data(int fd, char *buf, int size, char *mode);
void rec_file(int sock,tftp_packet *req, struct sockaddr_in *client, socklen_t len);
void send_file(int sock, tftp_packet *req, struct sockaddr_in *client, socklen_t len);
#endif