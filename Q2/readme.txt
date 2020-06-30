-- packet.h contains 5 MACROS :
    - PACKET_SIZE : Number of bytes in payload of a packet
    - TIMER : Timeout interval
    - WINDOW_SIZE : Size of the window used in Selective Repeat Algorithm
    - PDR : Packet drop rate for RELAY1 and RELAY2
    - BUFFERSIZE : Number of packets that can be buffered

-- In client.c, a window (array of PKT) and isAckRec (array of int) of size WINDOW_SIZE are created. Initially send all the packets of the window (odd to RELAY1 and even to RELAY2), if ACK is received:
    - If ACK is for the first packet of the window, then window is slided and the new packet is sent.
    - If ACK is for any other packet of the window, then isAckRec is assigned for that packet.
    - else ignore the ACK.

-- The sequence numbers used are incremental.
-- gettimeofday function has been used for implementing timer.
-- A random number is generated between [0,100] and if the number is less than or equal to PDR, packet is dropped.
-- usleep function has been used to implement delay for each packet at each Relay Node.
-- A min heap has been used for implementing buffer for storing out-of-order packets. The BUFFER_SIZE should always be greater than the WINDOW_SIZE.

I.  COMPILATION

    gcc -o server server.c
    gcc -o relay1 relay1.c
    gcc -o relay2 relay2.c
    gcc -o client client.c

II. ORDER OF EXECUTION

    ./server
    ./relay1
    ./relay2
    ./client


