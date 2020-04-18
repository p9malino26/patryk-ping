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

class Ping {

    std::string ipAddress;
    static const std::string message;

    bool sendSucceeded;
    int DATA_LENGTH = 2048;
    const int family = AF_INET;
    int serverSocket;
    sockaddr_in server;
    icmphdr sendHeader, recvHeader;
    socklen_t socketLen;
    fd_set responseFiles;
    timeval timeout = {3, 0};
    //arbitrary ID of choice
    static const int headerId = 1234;
   
    typedef char packetDataType; 
    std::string sendData;
    std::string receiveData;

    typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
    time_point send_begin, send_end;
public:
    Ping(const std::string& ipAddress) 
        :ipAddress(ipAddress)
    {
        socketLen = 0;
        sendData.reserve(DATA_LENGTH);
        receiveData.resize(DATA_LENGTH);
        //init socket
        serverSocket = socket(family, SOCK_DGRAM, IPPROTO_ICMP);
        if (serverSocket == -1)
            throw std::string("Socket initialization failed");

        initServer();
        
    }

    void sendPackets(int sendCount) {
        
        send_begin = std::chrono::high_resolution_clock::now();


        for (int i = 0; i < sendCount; i++) {
            sendPacket(i, -1);
        }

        if (!sendSucceeded) {
            std::cout << "All packets failed to send. This means there is an error connecting to the server.\n";
            return;
        }

        int receiveCount = 0;

        for (int i = 0; i < sendCount; i++) {
            if (receivePacket()) {
                receiveCount++;
            }
        }


        //TODO move this to when end of packets is reached
        std::chrono::high_resolution_clock::duration rttTime = std::chrono::high_resolution_clock::now() - send_begin;

        std::cout << "Packets sent: " <<  sendCount << std::endl
                  << "Packet lost: " <<  sendCount - receiveCount << std::endl
                  << "Round Trip Time: " << rttTime.count() << " nanoseconds." << std::endl; 

        if (sendCount != receiveCount)
            std::cout << "Note: Some packets were lost. This means that the connection to the server is unstable or the server is too overwhelmed by the requests.\n";

        std::cout << std::endl;
    }

private:

    inline bool performReceive() {
        FD_ZERO(&responseFiles);
        FD_SET(serverSocket, &responseFiles);
        if ( select(serverSocket + 1, &responseFiles, NULL, NULL, &timeout) <= 0 ) {
            return false;
        }

        int replyLength = recvfrom(serverSocket, &receiveData.at(0), receiveData.size(), 0, (struct sockaddr*)&server, &socketLen);


        if (replyLength != -1) {
                    sendSucceeded = true;
                    memcpy(&recvHeader, receiveData.c_str(), sizeof recvHeader);
                    /*printf("Received reply.\nPacket length: %d bytes.\n", replyLength);
                    std::cout << "Packet id: " << recvHeader.un.echo.id << std::endl
                        << "Packet sequence: \n" << recvHeader.un.echo.sequence << std::endl;*/


        } else {
                    printf("got no reply\n");
                        
        }

        return true;

    }

    inline void sendPacket(int sequence, int id) {
        memset(&sendHeader, 0, sizeof (sendHeader));

        sendHeader.type = ICMP_ECHO;
        sendHeader.un.echo.id = id;
        sendHeader.un.echo.sequence = sequence;
        initSendData();

        performSend();
    }

    inline bool receivePacket() {
        return performReceive();
    }

    inline void decodeData() {

    }

    inline void performSend() {
        if (sendto(serverSocket, sendData.c_str(), sendData.length(), 0, (sockaddr*)&server, sizeof(server)) >= 0)
            sendSucceeded = true;

    }


    inline void initSendData() {
        sendData.clear();
        sendData.append((packetDataType*)&sendHeader, sizeof sendHeader);
        sendData.append(message);

    }

    /**
     * Generates server struct
     */
    inline void initServer()
    {
        in_addr address;
        if (inet_aton(ipAddress.c_str(), &address) == 0)
            throw std::string("Invalid IP address.");

        memset(&server, 0, sizeof server);
        server.sin_addr = address;
        server.sin_family = family;
    }


        
    

    
};  

const std::string Ping::message("hello world");

int main() {
    try {

        Ping p("216.58.210.227");

        while (true) {
            p.sendPackets(20);
            sleep(1);
        }
    } catch (std::string s) {
        std::cerr << s << std::endl;
    }
}
