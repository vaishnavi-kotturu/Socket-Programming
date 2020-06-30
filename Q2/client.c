#include"packet.h" 

#define PORT1 8882  
#define PORT2 8883
#define PORTA 8888  
#define PORTB 8889   

int seq;

void die(char *s)
{
    perror(s);
    exit(1);
}


PKT* nextPacket(FILE* fptr){

    PKT* packet = (PKT*) malloc(sizeof(PKT));
    memset(packet->data,'\0',PACKET_SIZE+1);
    int check = fread(packet->data, 1,sizeof(char)*PACKET_SIZE, fptr);
    if(feof(fptr)||feof(fptr+1) || check<0)
        packet->isLast =1;
    else packet->isLast =0;
    packet->size =strlen(packet->data);
    packet->isACK =0;
    packet->seq_no = seq++;
    return packet;
}

void shiftWindow(PKT* arr, PKT new){
    int i=0;
    for(;i<(WINDOW_SIZE-1);i++){
        arr[i]=arr[i+1];
    }
    arr[i]=new;
}

void shiftAckRec(int* arr){
    int i=0;
    for(;i<(WINDOW_SIZE-1);i++){
        arr[i]=arr[i+1];
    }
    arr[i]=0;
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


int main(){
    FILE* f = fopen("input.txt","r");
    seq=1;
    fd_set fds;
    struct sockaddr_in si_other1,si_other2,si_me,si_me2;
    int s,s2;
    int slen1 = sizeof(si_other1);
    int slen2 = sizeof(si_other2);
    struct timeval tv;
    tv.tv_sec = 0;
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    if ( (s2=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    memset((char *) &si_other1, 0, sizeof(si_other1));
    si_other1.sin_family = AF_INET;
    si_other1.sin_port = htons(PORT1);
	si_other1.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((char *) &si_other2, 0, sizeof(si_other2));
    si_other2.sin_family = AF_INET;
    si_other2.sin_port = htons(PORT2);
	si_other2.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORTA);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((char *) &si_me2, 0, sizeof(si_me2));
    si_me2.sin_family = AF_INET;
    si_me2.sin_port = htons(PORTB);
	si_me2.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(s, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
    if( bind(s2, (struct sockaddr*)&si_me2, sizeof(si_me2) ) == -1)
    {
        die("bind");
    }

    PKT send_pkt, rcv_ack, ack_pkt, temp;
    PKT window[WINDOW_SIZE];
    for(int i=0;i<WINDOW_SIZE;i++){
        window[i]= *nextPacket(f);
        if(window[i].isLast==1) break;
    }
    int flag=0;
    int j=1;
    fd_set fds2;
    FD_ZERO(&fds);
    FD_ZERO(&fds2);
    FD_SET(s,&fds);
    FD_SET(s2,&fds2);

    int sent_count=0,ack_count=0;
    int i=0,base=1;
    int isAckRec[WINDOW_SIZE];
    memset((int*)isAckRec,0,sizeof(isAckRec));
    struct timeval start, end;

    while(i<WINDOW_SIZE){

        if (sendto(s, &(window[i]), sizeof(window[i]), 0, (struct sockaddr *) &si_other1, slen1) == -1)
        {    
            die("sendto"); 
        }
        gettimeofday(&start,NULL);
        printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY1\n",getTimeStamp(),window[i].seq_no);
        sent_count++;
        i++;

        if(i==WINDOW_SIZE||window[i-1].isLast==1) break;
        if (sendto(s2, &(window[i]), sizeof(window[i]), 0, (struct sockaddr *) &si_other2, slen2) == -1)
        {    
            die("sendto"); 
        }
        gettimeofday(&start,NULL);
        printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY2\n",getTimeStamp(),window[i].seq_no);
        sent_count++;
        i++;

        if(window[i-1].isLast==1) break;
        
    }
    while(1){
        FD_ZERO(&fds);
        FD_ZERO(&fds2);
        FD_SET(s,&fds);
        FD_SET(s2,&fds2);
        if(select(s+1,&fds,NULL,NULL,&tv)>0){
            if(FD_ISSET(s,&fds)){
                if (recvfrom(s, &(ack_pkt), sizeof(ack_pkt), 0, (struct sockaddr *) &si_other1, (socklen_t*)&slen1) == -1)
                {    
                    die("recvfrom"); 
                }
                printf("CLIENT\tR \t%s\tACK \t%d\tRELAY1\tCLIENT\n",getTimeStamp(),ack_pkt.seq_no);
                ack_count++;
                if(ack_pkt.isLast==1 && ack_count==sent_count) 
                {
                    break;
                }
                if(ack_pkt.isLast==1 && ack_pkt.size!=PACKET_SIZE) 
                {
                    break;
                }
                if(ack_pkt.seq_no==base){
                    PKT new=*nextPacket(f);

                    shiftWindow(window,new);
                    shiftAckRec(isAckRec);
                    base++;
                    if(new.size!=0){

                        if(new.seq_no%2==0){
                            if (sendto(s2, &(new), sizeof(new), 0, (struct sockaddr *) &si_other2, slen2) == -1)
                            {    
                                die("sendto"); 
                            }
                            gettimeofday(&start,NULL);
                            printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY2\n",getTimeStamp(),new.seq_no);
                            sent_count++;
                        }
                        else{
                            if (sendto(s, &(new), sizeof(new), 0, (struct sockaddr *) &si_other1, slen1) == -1)
                            {    
                                die("sendto"); 
                            }
                            gettimeofday(&start,NULL);
                            printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY1\n",getTimeStamp(),new.seq_no);
                            sent_count++;
                        }
                        
                    }
                    for(int i=0;i<WINDOW_SIZE;i++){
                        if(isAckRec[0]==1){
                            PKT new=*nextPacket(f);
                            shiftWindow(window,new);
                            shiftAckRec(isAckRec);
                            base++;
                            if(new.size!=0){
                                if(new.seq_no%2==0){
                                    if (sendto(s2, &(new), sizeof(new), 0, (struct sockaddr *) &si_other2, slen2) == -1)
                                    {    
                                        die("sendto"); 
                                    }
                                    gettimeofday(&start,NULL);
                                    printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY2\n",getTimeStamp(),new.seq_no);
                                    sent_count++;
                                }
                                else{
                                    if (sendto(s, &(new), sizeof(new), 0, (struct sockaddr *) &si_other1, slen1) == -1)
                                    {    
                                        die("sendto"); 
                                    }
                                    gettimeofday(&start,NULL);
                                    printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY1\n",getTimeStamp(),new.seq_no);
                                    sent_count++;
                                }
                                
                            }
                        }else break;
                    }
                }
                else if(ack_pkt.seq_no>base && ack_pkt.seq_no<(base+WINDOW_SIZE)){
                    isAckRec[ack_pkt.seq_no-base]=1;
                }
            }
            
        }
        else{
            gettimeofday(&end,NULL);
            if((end.tv_sec - start.tv_sec)>TIMER){
                int flaga=0;
                
                for(int i=0;i<WINDOW_SIZE;i++){
                    if(window[i].isLast==1) flaga=1;
                    if(isAckRec[i]==0){
                        if(window[i].seq_no%2==0){
                            if (sendto(s2, &(window[i]), sizeof(window[i]), 0, (struct sockaddr *) &si_other2, slen2) == -1)
                            {    
                                die("sendto"); 
                            }
                            if(window[i].isLast==1 && window[i].size==0) 
                            {
                                break;
                            }
                            gettimeofday(&start,NULL);
                            printf("CLIENT\tTO \t%s\tACK \t%d\tCLIENT\tRELAY2\n",getTimeStamp(),window[i].seq_no);
                            printf("CLIENT\tRE \t%s\tDATA\t%d\tCLIENT\tRELAY2\n",getTimeStamp(),window[i].seq_no);
                        }else{
                            if (sendto(s, &(window[i]), sizeof(window[i]), 0, (struct sockaddr *) &si_other1, slen1) == -1)
                            {    
                                die("sendto"); 
                            }
                            if(window[i].isLast==1 && window[i].size==0) 
                            {
                                break;
                            }
                            gettimeofday(&start,NULL);
                            printf("CLIENT\tTO \t%s\tACK \t%d\tCLIENT\tRELAY1\n",getTimeStamp(),window[i].seq_no);
                            printf("CLIENT\tRE \t%s\tDATA\t%d\tCLIENT\tRELAY1\n",getTimeStamp(),window[i].seq_no);
                        }
                        
                                
                    }
                    if(flaga==1) break;
                }
            }
        }
        if(select(s2+1,&fds2,NULL,NULL,&tv)>0){
            if(FD_ISSET(s2,&fds2)){
                if (recvfrom(s2, &(ack_pkt), sizeof(ack_pkt), 0, (struct sockaddr *) &si_other2, (socklen_t*)&slen2) == -1)
                {    
                    die("recvfrom"); 
                }
                printf("CLIENT\tR \t%s\tACK \t%d\tRELAY2\tCLIENT\n",getTimeStamp(),ack_pkt.seq_no);
                ack_count++;
                if(ack_pkt.isLast==1 && ack_count==sent_count) 
                {
                    break;
                }
                if(ack_pkt.isLast==1 && ack_pkt.size!=PACKET_SIZE) 
                {
                    break;
                }
                if(ack_pkt.seq_no==base){
                    PKT new=*nextPacket(f);
                    shiftWindow(window,new);
                    shiftAckRec(isAckRec);
                    base++;
                    if(new.size!=0){
                        if(new.seq_no%2==0){
                            if (sendto(s2, &(new), sizeof(new), 0, (struct sockaddr *) &si_other2, slen2) == -1)
                            {    
                                die("sendto"); 
                            }
                            gettimeofday(&start,NULL);
                            printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY2\n",getTimeStamp(),new.seq_no);
                            sent_count++;
                        }
                        else{
                            if (sendto(s, &(new), sizeof(new), 0, (struct sockaddr *) &si_other1, slen1) == -1)
                            {    
                                die("sendto"); 
                            }
                            gettimeofday(&start,NULL);
                            printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY1\n",getTimeStamp(),new.seq_no);
                            sent_count++;
                        }
                        
                    }
                    for(int i=0;i<WINDOW_SIZE;i++){
                        if(isAckRec[0]==1){
                            PKT new=*nextPacket(f);
                            shiftWindow(window,new);
                            shiftAckRec(isAckRec);
                            base++;
                            if(new.size!=0){
                                if(new.seq_no%2==0){
                                    if (sendto(s2, &(new), sizeof(new), 0, (struct sockaddr *) &si_other2, slen2) == -1)
                                    {    
                                        die("sendto"); 
                                    }
                                    gettimeofday(&start,NULL);
                                    printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY2\n",getTimeStamp(),new.seq_no);
                                    sent_count++;
                                }
                                else{
                                    if (sendto(s, &(new), sizeof(new), 0, (struct sockaddr *) &si_other1, slen1) == -1)
                                    {    
                                        die("sendto"); 
                                    }
                                    gettimeofday(&start,NULL);
                                    printf("CLIENT\tS \t%s\tDATA\t%d\tCLIENT\tRELAY1\n",getTimeStamp(),new.seq_no);
                                    sent_count++;
                                }
                                
                            }
                        }else break;
                    }
                }
                else if(ack_pkt.seq_no>base && ack_pkt.seq_no<(base+WINDOW_SIZE)){
                    isAckRec[ack_pkt.seq_no-base]=1;
                }
            }
            
        }
        else{
            gettimeofday(&end,NULL);
            if((end.tv_sec - start.tv_sec)>TIMER){
                int flaga=0;
                for(int i=0;i<WINDOW_SIZE;i++){
                    if(window[i].isLast==1) flaga=1;
                    if(isAckRec[i]==0){
                        if(window[i].seq_no%2==0){
                            if (sendto(s2, &(window[i]), sizeof(window[i]), 0, (struct sockaddr *) &si_other2, slen2) == -1)
                            {    
                                die("sendto"); 
                            }
                            gettimeofday(&start,NULL);
                            if(window[i].isLast==1 && window[i].size==0) 
                            {
                                break;
                            }
                            printf("CLIENT\tTO \t%s\tACK \t%d\tCLIENT\tRELAY2\n",getTimeStamp(),window[i].seq_no);
                            printf("CLIENT\tRE \t%s\tDATA\t%d\tCLIENT\tRELAY2\n",getTimeStamp(),window[i].seq_no);
                        }else{
                            if (sendto(s, &(window[i]), sizeof(window[i]), 0, (struct sockaddr *) &si_other1, slen1) == -1)
                            {    
                                die("sendto"); 
                            }
                            gettimeofday(&start,NULL);
                            if(window[i].isLast==1 && window[i].size==0) 
                            {
                                break;
                            }
                            printf("CLIENT\tTO \t%s\tACK \t%d\tCLIENT\tRELAY1\n",getTimeStamp(),window[i].seq_no);
                            printf("CLIENT\tRE \t%s\tDATA\t%d\tCLIENT\tRELAY1\n",getTimeStamp(),window[i].seq_no);
                        }
                        
                                
                    }
                    if(flaga==1) break;
                }
            }
        }
    }
    

    
    
    close(s);
    close(s2);
    fclose(f);
    return 0;
}

