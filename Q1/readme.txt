Name  :  Vaishnavi Kotturu
ID    :  2017A7PS0088P

-- packet.h contains 4 MACROS :
    - PACKET_SIZE : Number of bytes in payload of a packet
    - TIMER : Timeout interval
    - PDR : Packet drop rate 
    - BUFFERSIZE : Number of packets that can be buffered

-- gettimeofday function has been used for implementing timer.
-- A random number is generated between [0,100] and if the number is less than or equal to PDR, packet is dropped.
-- When an expected packet is received, it is written to the output file along with the buffer, otherwise it is stored to the buffer.

I.  COMPILATION

    gcc -o server server.c
    gcc -o client client.c

II. ORDER OF EXECUTION

    ./server
    ./client


