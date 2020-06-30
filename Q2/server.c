#include"packet.h" 
 
#define PORT1 8884   
#define PORT2 8885
#define PORTA 8886
#define PORTB 8887


void die(char *s)
{
    perror(s);
    exit(1);
}

PKT fileBuffer[BUFFERSIZE]; 
int currentHS;
 
void insert(PKT packet) {
    currentHS++;
    fileBuffer[currentHS] = packet; 
    int i = currentHS;
    while (packet.seq_no<=fileBuffer[i / 2].seq_no) {
        fileBuffer[i] = fileBuffer[i / 2];
        i /= 2;
    }
    fileBuffer[i] = packet;
}
 
PKT topOfHeap() {
    PKT resultPkt, endPkt;
    int j, i;
    resultPkt = fileBuffer[1];
    endPkt = fileBuffer[currentHS--];
    for (i = 1; i * 2 <= currentHS; i = j) {
        j = i * 2;
        if (j != currentHS && fileBuffer[j + 1].seq_no < fileBuffer[j].seq_no) j++;
        if (endPkt.seq_no > fileBuffer[j].seq_no) fileBuffer[i] = fileBuffer[j];
        else break;
    }
    fileBuffer[i] = endPkt;
    return resultPkt;
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
    struct sockaddr_in si_me, si_me2,si_other1, si_other2;
    int s, s2, i, slen1 = sizeof(si_other1);
    int slen2 = sizeof(si_other2);
    fd_set fds;
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

    memset((char *) &si_other1, 0, sizeof(si_other1));
     
    si_other1.sin_family = AF_INET;
    si_other1.sin_port = htons(PORTA);
    si_other1.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((char *) &si_other2, 0, sizeof(si_other2));
     
    si_other2.sin_family = AF_INET;
    si_other2.sin_port = htons(PORTB);
    si_other2.sin_addr.s_addr = htonl(INADDR_ANY);
     
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
    
    currentHS = 0;
    fileBuffer[0].seq_no = -1;

    FILE* outputf=fopen("output.txt","w");
    int expected=1;
    int flagg=0;
    while(1){
        FD_ZERO(&fds);
        FD_ZERO(&fds2);
        FD_SET(s,&fds);
        FD_SET(s2,&fds2);
        if(select(s+1,&fds,NULL,NULL,&tv)>0){
            if(FD_ISSET(s,&fds)){
                if ((recvfrom(s, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &si_other1, (socklen_t*)&slen1)) == -1)
                {
                    die("recvfrom()");
                }
                    
                printf("SERVER\tR \t%s\tDATA\t%d\tRELAY1\tSERVER\n",getTimeStamp(),rcv_pkt.seq_no);
                if(rcv_pkt.seq_no==expected){
                    fprintf(outputf,"%s",rcv_pkt.data);
                    expected++;
                    if(rcv_pkt.isLast==1){
                        flagg=1;
                    }
                    while(fileBuffer[1].seq_no==expected){
                        PKT tmp=topOfHeap();
                        fprintf(outputf,"%s",tmp.data);
                        if(tmp.isLast==1){ flagg=1;break;}
                        expected++;
                    }
                    
                }
                else{
                        insert(rcv_pkt);
                }
                ack_pkt.isLast=rcv_pkt.isLast;
                ack_pkt.isACK=1;
                ack_pkt.seq_no=rcv_pkt.seq_no;
                ack_pkt.size=rcv_pkt.size;
                if (sendto(s, &(ack_pkt), sizeof(ack_pkt), 0, (struct sockaddr *) &si_other1, slen1) == -1)
                {    
                    die("sendto"); 
                }
                printf("SERVER\tS \t%s\tACK \t%d\tSERVER\tRELAY1\n",getTimeStamp(),ack_pkt.seq_no);
                if(flagg==1) break;
            }
        }
        if(select(s2+1,&fds2,NULL,NULL,&tv)>0){
            if(FD_ISSET(s2,&fds2)){
                if ((recvfrom(s2, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &si_other2, (socklen_t*)&slen2)) == -1)
                {
                    die("recvfrom()");
                }
                    
                printf("SERVER\tR \t%s\tDATA\t%d\tRELAY2\tSERVER\n",getTimeStamp(),rcv_pkt.seq_no);
                if(rcv_pkt.seq_no==expected){
                    fprintf(outputf,"%s",rcv_pkt.data);
                    expected++;
                    if(rcv_pkt.isLast==1){
                        flagg=1;
                    }
                    while(fileBuffer[1].seq_no==expected){
                        PKT tmp=topOfHeap();
                        fprintf(outputf,"%s",tmp.data);
                        if(tmp.isLast==1){ flagg=1;break;}
                        expected++;
                    }
                    

                }
                else{
                        insert(rcv_pkt);
                }
                ack_pkt.isLast=rcv_pkt.isLast;
                ack_pkt.isACK=1;
                ack_pkt.seq_no=rcv_pkt.seq_no;
                ack_pkt.size=rcv_pkt.size;
                if (sendto(s2, &(ack_pkt), sizeof(ack_pkt), 0, (struct sockaddr *) &si_other2, slen2) == -1)
                {    
                    die("sendto"); 
                }
                printf("SERVER\tS \t%s\tACK \t%d\tSERVER\tRELAY2\n",getTimeStamp(),ack_pkt.seq_no);
                if(flagg==1) break;
            }
        }
        
    }
    close(s);
    close(s2);
    fclose(outputf);
    return 0;
}
