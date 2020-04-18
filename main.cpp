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
    std::basic_string<packetDataType> sendData;
    std::basic_string<packetDataType> receiveData;

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
        //init send header
        initHeader();
        initSendData();

        while (true) {
            performSend();
            performReceive();
            sleep(1);
            std::cout << std::endl;
        }


        
    }

private:

    inline void performReceive() {
        FD_ZERO(&responseFiles);
        FD_SET(serverSocket, &responseFiles);
        if ( select(serverSocket + 1, &responseFiles, NULL, NULL, &timeout) <= 0 ) {
            throw std::string("Error selecting");                       
        }

        int replyLength = recvfrom(serverSocket, &receiveData.at(0), receiveData.size(), 0, (struct sockaddr*)&server, &socketLen);

        if (replyLength != -1) {
                    printf("Received reply. Packet length: %d bytes.\n", replyLength);
                    std::chrono::high_resolution_clock::duration rttTime = std::chrono::high_resolution_clock::now() - send_begin;
                    std::cout << " Round Trip Time: " << rttTime.count() << " nanoseconds." << std::endl; 
        } else {
                    printf("got no reply\n");
                        
        }
    
    }

    inline void sendPacket(int sequence, int id) {
        sendHeader.un.echo.id = id;
        sendHeader.un.echo.sequence = sequence;
    }

    inline void receivePacket() {
        
    }

    inline void performSend() {
        send_begin = std::chrono::high_resolution_clock::now();
        if (sendto(serverSocket, sendData.c_str(), sendData.length(), 0, (sockaddr*)&server, sizeof(server)) < 0)
             std::cout << "Error sending. Code " << errno << std::endl;
        else
            std::cout << "Send succeeded.\n";
    }

    inline void initHeader() {
        memset(&sendHeader, 0, sizeof (sendHeader));
        sendHeader.type = ICMP_ECHO;
        sendHeader.un.echo.id = headerId;
    }


    inline void initSendData() {
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
    } catch (std::string s) {
        std::cerr << s << std::endl;
    }
}
