#include"packet.h"

#define PORT 8882   //Server's Port

int offset;


void die(char *s)
{
    perror(s);
    exit(1);
}

PKT* nextPacket(FILE* fptr, int channel){

    PKT* packet = (PKT*) malloc(sizeof(PKT));
    memset(packet->data,'\0',PACKET_SIZE+1);
    int check = fread(packet->data, sizeof(char)*PACKET_SIZE, 1, fptr);
    if(check==0) packet->isLast =1;
    else packet->isLast =0;
    packet->size =strlen(packet->data);
    packet->isACK =0;
    packet->channel =channel;
    packet->seq_no = offset;
    offset += packet->size;
    return packet;
}

void printPKT(PKT packet){

    if(packet.isACK==1) 
        printf("RCVD ACK: for PKT with Seq. No. %d from channel %d\n",packet.seq_no,packet.channel);
    else 
        printf("SENT PKT: Seq. No %d of size %d Bytes from channel %d\n",packet.seq_no,packet.size,packet.channel);
}

int main(){
    
    FILE* f = fopen("input.txt","r");
    offset=0;
    fd_set fds,fds2;
    struct sockaddr_in server_address;
    int s, s2, i, timeout, slen=sizeof(server_address); //s and s2 are to store file descriptors

    struct timeval tv;
    tv.tv_sec = TIMER;

    if ( (s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        die("socket");
    }
    
    if ( (s2=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        die("socket");
    }

    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    PKT send_pkt, rcv_ack, prev_pkt;
    PKT send_pkt2, rcv_ack2, prev_pkt2;

    int c = connect(s,(struct sockaddr*) &server_address,sizeof(server_address));
    if(c<0){
        printf("Error while establishing connection1\n");
        exit(0);
    }

    
    int c2 = connect(s2,(struct sockaddr*) &server_address,sizeof(server_address));
    
    if(c2<0){
        printf("Error while establishing connection2\n");
        exit(0);
    }

    struct timeval start1,start2,end1,end2;
    int active=1;
    int count_sent=0;
    int count_acked=0;

    prev_pkt = send_pkt = *nextPacket(f,0);
    if(send_pkt.size!=0){
        if (send(s, &(send_pkt), sizeof(send_pkt), 0) == -1)
        {    
            die("send"); 
        }
        printPKT(send_pkt);
        gettimeofday(&start1,NULL);
        count_sent++;
    }


    prev_pkt2 = send_pkt2 = *nextPacket(f,1);
    if(send_pkt2.size!=0){
        if (send(s2, &(send_pkt2), sizeof(send_pkt2), 0) == -1)
        {
            die("send");
        }
        printPKT(send_pkt2);
        gettimeofday(&start2,NULL);
        count_sent++;
    }
    else{
        active =0;
    }
    
    
    

    while(1){
        FD_ZERO(&fds);
        FD_SET(s,&fds);
        FD_ZERO(&fds2);
        FD_SET(s2,&fds2);
        if((timeout=select(s+1,&fds,NULL,NULL,&tv))<0){

            die("select");
        }

        else{
            if(FD_ISSET(s,&fds)){

                if ((recv(s, &(rcv_ack), sizeof(rcv_ack), 0)) == -1){
                    die("recv");
                }
                printPKT(rcv_ack);
                count_acked++;
                if(rcv_ack.isLast==1 && count_acked==count_sent) break;
                if(rcv_ack.isLast==1 && rcv_ack.size!=PACKET_SIZE) break;
                prev_pkt = send_pkt;
                send_pkt = *nextPacket(f,0);
                
                if(send_pkt.size!=0){
                    if (send(s, &(send_pkt), sizeof(send_pkt), 0) == -1)
                    {    
                        die("send");
                    }
                    printPKT(send_pkt);
                    gettimeofday(&start1,NULL);
                    count_sent++;
                }
                else send_pkt=prev_pkt;
                
                
            }
            else{
                gettimeofday(&end1,NULL);
                if((end1.tv_sec - start1.tv_sec)>TIMER){
                    gettimeofday(&start1,NULL);
                    if (send(s, &(send_pkt), sizeof(send_pkt), 0) == -1)
                    {    
                        die("send"); 
                    }
                    printPKT(send_pkt);
                }
            }
        }
        if(active==1){
            if((timeout=select(s2+1,&fds2,NULL,NULL,&tv))<0){
                die("select");
            }
        
            else{
                if(FD_ISSET(s2,&fds2)){

                    if ((recv(s2, &(rcv_ack2), sizeof(rcv_ack2), 0)) == -1){
                        die("recv");
                    }
                    count_acked++;
                    printPKT(rcv_ack2);
                    if(rcv_ack2.isLast==1 && count_acked==count_sent) break;
                    if(rcv_ack2.isLast==1 && rcv_ack2.size!=PACKET_SIZE) break;
                    prev_pkt2 = send_pkt2;
                    send_pkt2 = *nextPacket(f,1);
                    if(send_pkt2.size!=0){
                        if (send(s2, &(send_pkt2), sizeof(send_pkt2), 0) == -1)
                        {    
                            die("send"); 
                        }
                        printPKT(send_pkt2);
                        gettimeofday(&start2,NULL);
                        count_sent++;
                    }
                    else send_pkt2=prev_pkt2;
                }
                else{
                    gettimeofday(&end2,NULL);
                    if((end2.tv_sec - start2.tv_sec)>TIMER){
                        gettimeofday(&start2,NULL);
                        if (send(s2, &(send_pkt2), sizeof(send_pkt2), 0) == -1)
                        {    
                            die("send"); 
                        }
                        printPKT(send_pkt2);
                    }
                }
                
                
                
            }
        }
        // if(rcv_ack.isLast || rcv_ack2.isLast) break;

    }
    close(s);
    close(s2);
    fclose(f);
    return 0;
}
