#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include <sys/time.h> 

#define PACKET_SIZE 100 //Size of payload in a packet
#define TIMER 2 //Timeout duration
#define PDR 10 //Packet Drop Percentage
#define BUFFERSIZE 2 //Buffer to handle out-of-order packets

typedef struct packet{
    int seq_no;
    int size;
    int isLast;
    int isACK;
    int channel;
    char data[PACKET_SIZE+1];
}PKT;