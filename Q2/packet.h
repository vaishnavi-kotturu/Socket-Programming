#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<time.h>
#include <sys/time.h>
 
#define PACKET_SIZE 100
#define TIMER 2
#define WINDOW_SIZE 7
#define BUFFERSIZE 10
#define PDR 10

 
typedef struct packet{
    int seq_no;
    int size;
    int isLast;
    int isACK;
    char data[PACKET_SIZE+1]; // + 1 is for '\0'
}PKT;