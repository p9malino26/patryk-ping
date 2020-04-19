/**
 * Dear Reader,
 * 
 * You are about to see my go at writing a simple Ping CLI application in C++.
 * It was a big challenge for me and an activity where I have learned a lot of things about
 * network programming and a big step up in my programming journey.
 * 
 * I hope you will enjoy exploring all my work done to construct this application.
 * 
 * I would like to credit the IBM Knowledge Center and the Linux Man pages for providing very useful 
 * information which helped me learn about socket programming and helped me develop new skills.
 * Big thanks also to people who gave me good facts on things on StackOverflow.
 * 
 * -Patryk. M
 */

#include <asm-generic/errno-base.h>
#include <ostream>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <chrono>
#include <sstream>

/**
 * Usage: IP address and ttl passed into constructor. Then call sendPackets to send n packets to server.
 */
class Ping {

    std::string ipAddress;
    static const std::string message;

    bool replyReceived;
    int DATA_LENGTH = 2048;
    const int family = AF_INET;
    int serverSocket;
    sockaddr_in server;
    icmphdr sendHeader, recvHeader;
    socklen_t socketLen;
    fd_set responseFiles;
    timeval timeout = {3, 0};
    //arbitrary ID of choice
    static const int HEADER_ID = 1234;
   
    typedef char packetDataType; 
    std::string sendData;
    std::string receiveData;

    //we use std::chrono to measure RTT time
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
    time_point send_begin;

public:
    /**
     * Constructor initializes socket and the server and all the constant data
     */
    Ping(const std::string& ipAddress, int ttl) 
        :ipAddress(ipAddress)
    {
        socketLen = 0;
        // we are using c++ std strings to store the send and receive data because they help us append data without keeping track of the length/ byte positions
        sendData.reserve(DATA_LENGTH);
        receiveData.resize(DATA_LENGTH);
        //init socket
        serverSocket = socket(family, SOCK_DGRAM, IPPROTO_ICMP);
        if (serverSocket == -1)
            throw std::string("Socket initialization failed. Quitting.");

        setsockopt(serverSocket, ttl, IPPROTO_ICMP, &ttl, sizeof(ttl));
        initServer();
        
    }

    /**
     * sends and then receives packets, reporting RTT and packet loss
     */
    void sendPackets(int sendCount) {
        //we measure RTT by remembering the start time and subtracting it from the finish time
        send_begin = std::chrono::high_resolution_clock::now();


        for (int i = 0; i < sendCount; i++) {
            sendPacket(i, HEADER_ID);
        }

        if (!replyReceived) {
            std::cout << "All packets failed to send. This means there is an error connecting to the server.\n";
            return;
        }

        int receiveCount = 0;

        for (int i = 0; i < sendCount; i++) {
            if (receivePacket()) {
                receiveCount++;
            }
        }

        //end timer
        std::chrono::high_resolution_clock::duration rttTime = std::chrono::high_resolution_clock::now() - send_begin;

        //report statistics
        std::cout << "Packets sent: " <<  sendCount << std::endl
                  << "Packet lost: " <<  sendCount - receiveCount << std::endl
                  << "Round Trip Time: " << rttTime.count() << " nanoseconds." << std::endl; 

        if (sendCount != receiveCount)
            std::cout << "Note: Some packets were lost. This means that the connection to the server is unstable or the server is too overwhelmed by the requests or the TTL has been exceeded.\n";

        std::cout << std::endl;
    }
private:


    /**
     * Selects the socket and copies received data into buffer
     */
    inline bool receivePacket() {
        FD_ZERO(&responseFiles);
        FD_SET(serverSocket, &responseFiles);
        //listens out for activity in the server socket and writes the output, error
        if ( select(serverSocket + 1, &responseFiles, NULL, NULL, &timeout) <= 0 ) {
            return false;
        }

        int replyLength = recvfrom(serverSocket, &receiveData.at(0), receiveData.size(), 0, (struct sockaddr*)&server, &socketLen);

        replyReceived = true;
        memcpy(&recvHeader, receiveData.c_str(), sizeof recvHeader);

        return true;

    }

    /**
     * Initializes ICMP send header based on sequence and id variables and the packet data and calls send method.
     */
    inline void sendPacket(int sequence, int id) {
        memset(&sendHeader, 0, sizeof (sendHeader));

        sendHeader.type = ICMP_ECHO;
        sendHeader.un.echo.id = id;
        sendHeader.un.echo.sequence = sequence;
        
        initSendData();

        performSend();
    }

    /** Calls raw sendo function to send packet
     */
    inline void performSend() {
        if (sendto(serverSocket, sendData.c_str(), sendData.length(), 0, (sockaddr*)&server, sizeof(server)) >= 0)
            replyReceived = true;

    }

    /**
     * copies send header and message into buffer
     */
    inline void initSendData() {
        sendData.clear();
        sendData.append((packetDataType*)&sendHeader, sizeof sendHeader);
        sendData.append(message);

    }

    /**
     * Initializes server struct, which holds the IP address
     */
    inline void initServer()
    {
        in_addr address;
        //converts the string ip address into a special struct which stores the IP address naturally in a 32 bit integer
        if (inet_aton(ipAddress.c_str(), &address) == 0)
            throw std::string("Error. IP address format invalid. Quitting.");

        memset(&server, 0, sizeof server);
        server.sin_addr = address;
        server.sin_family = family;
    }

};  

// we store the options message to send
const std::string Ping::message("hello world");

int main(int argc, char** argv) {
    const int packetSendCount = 20;
    int ttl = 58;
    std::string ipAddress;

    try {
        if (argc >= 2) {
            ipAddress = argv[1];
        } else {
            throw std::string("You must specify an IPv4 address as an argument. Quitting.\n");
        }
        
        if (argc >=3) {
            if (!(std::stringstream(argv[2]) >> ttl)) {
                throw std::string("Error. Second argument which is supposed to be ttl is not a valid integer. Quitting.\n");
            }
        }

        Ping ping(ipAddress, ttl);

        while (true) {
            ping.sendPackets(packetSendCount);
            sleep(1);
        }
    } catch (std::string s) {
        //to simplify things, exceptions are thrown as strings, which are printed when they are caught.
        std::cerr << s << std::endl;
        return -1;
    }
}
