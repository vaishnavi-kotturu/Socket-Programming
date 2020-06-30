#include"packet.h" 
 
#define PORT1 8883   
#define PORT2 8887
#define PORTS 8885
#define PORTC 8889


void die(char *s)
{
    perror(s);
    exit(1);
}
 



char* getTimeStamp(){
    char* arr = (char*) malloc(sizeof(char)*20);
    char milliSec[8];
    time_t t = time(NULL);
    struct tm* lt = localtime(&t);

    struct timeval tv;
    gettimeofday(&tv,NULL);
    strftime(arr, 20, "%H:%M:%S", lt);
    sprintf(milliSec,".%06ld",tv.tv_usec);
    strcat(arr,milliSec);
    return arr;
}
int main(void)
{
    struct sockaddr_in si_me, si_me2, si_other, si_rec;
    int s, s2, i, slen = sizeof(si_other) , recv_len;
    int rlen = sizeof(si_rec);

    PKT  rcv_pkt;
    PKT  ack_pkt;
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    if ((s2=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT1);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((char *) &si_me2, 0, sizeof(si_me2));
     
    si_me2.sin_family = AF_INET;
    si_me2.sin_port = htons(PORT2);
    si_me2.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((char *) &si_rec, 0, sizeof(si_rec));
     
    si_rec.sin_family = AF_INET;
    si_rec.sin_port = htons(PORTS);
    si_rec.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((char *) &si_other, 0, sizeof(si_other));
     
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORTC);
    si_other.sin_addr.s_addr = htonl(INADDR_ANY);
    fd_set fds;
    
    if( bind(s, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
    if( bind(s2, (struct sockaddr*)&si_me2, sizeof(si_me2) ) == -1)
    {
        die("bind");
    }
    fd_set fds2;
    FD_ZERO(&fds);
    FD_ZERO(&fds2);
    FD_SET(s,&fds);
    FD_SET(s2,&fds2);
    struct timeval tv;
    tv.tv_sec = 0;
    while(1){
        FD_ZERO(&fds);
        FD_ZERO(&fds2);
        FD_SET(s,&fds);
        FD_SET(s2,&fds2);
        if(select(s+1,&fds,NULL,NULL,&tv)>0){
            if(FD_ISSET(s,&fds)){
                if ((recvfrom(s, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &si_other, (socklen_t*)&slen)) == -1)
                {
                    die("recvfrom()");
                }
                printf("RELAY2\tR \t%s\tDATA\t%d\tCLIENT\tRELAY2\n",getTimeStamp(),rcv_pkt.seq_no);
                int num = (rand() % (100)) + 1;
                if(num>PDR){
                    int del=rand();
                    del%=2001;
                    usleep(del);
                    if (sendto(s2, &(rcv_pkt), sizeof(rcv_pkt), 0, (struct sockaddr *) &si_rec, rlen) == -1)
                    {    
                        die("sendto"); 
                    }

                    printf("RELAY2\tS \t%s\tDATA\t%d\tRELAY2\tSERVER\n",getTimeStamp(),rcv_pkt.seq_no);
                }
                else{
                    printf("RELAY2\tD \t%s\tDATA\t%d\tRELAY2\tSERVER\n",getTimeStamp(),rcv_pkt.seq_no);
                }
                
            }
        }
        if(select(s2+1,&fds2,NULL,NULL,&tv)>0){
            if(FD_ISSET(s2,&fds2)){
                if ((recvfrom(s2, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *) &si_rec, (socklen_t*)&rlen)) == -1)
                {
                    die("recvfrom()");
                }
                printf("RELAY2\tR \t%s\tACK \t%d\tSERVER\tRELAY2\n",getTimeStamp(),ack_pkt.seq_no);
                    if ((sendto(s, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *) &si_other, slen)) == -1)
                    {
                        die("sendto");
                    }
                    printf("RELAY2\tS \t%s\tACK \t%d\tRELAY2\tCLIENT\n",getTimeStamp(),ack_pkt.seq_no);
                    if(ack_pkt.isLast==1) break;
                
            }
        }

        
    }
    close(s);
    close(s2);
    return 0;
}
