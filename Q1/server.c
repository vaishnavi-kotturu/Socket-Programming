#include"packet.h"

#define PORT1 8882   
#define PORT2 8883

void die(char *s)
{
    perror(s);
    exit(1);
}
 

char buffer[BUFFERSIZE*(PACKET_SIZE)+1];

void printPKT(PKT packet){
    if(packet.isACK==1) printf("SENT ACK: for PKT with Seq. No. %d from channel %d\n",packet.seq_no,packet.channel);
    else printf("RCVD PKT: Seq. No %d of size %d Bytes from channel %d\n",packet.seq_no,packet.size,packet.channel);
}

int main(void)
{
    struct sockaddr_in si_me, si_other;
    int i, slen = sizeof(si_other) , recv_len;
    int master, c1=0,c2=0;
    fd_set fds;
    PKT  rcv_pkt;
    PKT  ack_pkt;
    int opt = 1;
    if ((master=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        die("socket");
    }
    if( setsockopt(master, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    } 
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT1);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

     
    //bind socket to port
    if( bind(master, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    int temp1 = listen(master,2);
    if(temp1<0){
        die("listen");
    }


    struct timeval tv;
    tv.tv_sec = 2;
    int timeout;
    int maxfd,sd,new_socket;
    int addlen=sizeof(si_me);
    if((c1=accept(master,(struct sockaddr*)&si_me,(socklen_t*)&addlen))<0){
        die("accept");
    }
    if((c2=accept(master,(struct sockaddr*)&si_me,(socklen_t*)&addlen))<0){
        die("accept");
    }

    int expected=0;
    FILE* outputf=fopen("output.txt","w");
    while(1)
    {
        FD_ZERO(&fds);  
        FD_SET(master,&fds);
        maxfd=master;
        sd=c1;
        if(sd>0) FD_SET(sd,&fds);
        if(sd>maxfd) maxfd = sd;

        sd=c2;
        if(sd>0) FD_SET(sd,&fds);
        if(sd>maxfd) maxfd = sd;
        
        if((timeout=select(maxfd+1,&fds,NULL,NULL,&tv))<0){
            die("select");
        }
        else{
            sd=c1;
            if(FD_ISSET(sd,&fds)){
                if ((recv_len = recv(sd, &(rcv_pkt), sizeof(rcv_pkt), 0)) == -1){
                    die("recv");
                }
                
                strcpy(ack_pkt.data,rcv_pkt.data);
                ack_pkt.isLast = rcv_pkt.isLast;
                ack_pkt.channel=rcv_pkt.channel;
                ack_pkt.isACK=1;
                ack_pkt.seq_no=rcv_pkt.seq_no;
                ack_pkt.size=rcv_pkt.size;
                int num = (rand() % (100)) + 1;
                if(num>PDR){
                    if(rcv_pkt.seq_no==expected){
                        fprintf(outputf,"%s",rcv_pkt.data);
                        fprintf(outputf,"%s",buffer);
                        expected+=PACKET_SIZE+strlen(buffer);
                        strcpy(buffer,"\0");
                        
                    }
                    else{
                            strcat(buffer,rcv_pkt.data);
                    }
                    if (send(sd, &(ack_pkt), sizeof(ack_pkt), 0) == -1)
                    {    
                        die("send"); 
                    }
                    printPKT(rcv_pkt);
                    printPKT(ack_pkt);
                    if(rcv_pkt.isLast==1) break;
                }
                    
            }
            sd=c2;
            if(FD_ISSET(sd,&fds)){
                if ((recv_len = recv(sd, &(rcv_pkt), sizeof(rcv_pkt), 0)) == -1){
                    die("recv");
                }
                
                strcpy(ack_pkt.data,rcv_pkt.data);
                ack_pkt.isLast = rcv_pkt.isLast;
                ack_pkt.channel=rcv_pkt.channel;
                ack_pkt.isACK=1;
                ack_pkt.seq_no=rcv_pkt.seq_no;
                ack_pkt.size=rcv_pkt.size;
                int num = (rand() % (100)) + 1;
                if(num>PDR){
                    if(rcv_pkt.seq_no==expected){
                        fprintf(outputf,"%s",rcv_pkt.data);
                        fprintf(outputf,"%s",buffer);
                        expected+=PACKET_SIZE+strlen(buffer);
                        strcpy(buffer,"\0");
                    }
                    else{
                            strcat(buffer,rcv_pkt.data);
                    }
                    if (send(sd, &(ack_pkt), sizeof(ack_pkt), 0) == -1)
                    {    
                        die("send"); 
                    } 
                    printPKT(rcv_pkt);
                    printPKT(ack_pkt);
                    if(rcv_pkt.isLast==1) break;
                }
                    
            }
                
                
            }

    }
    close(master);
    fclose(outputf);
    return 0;
}
