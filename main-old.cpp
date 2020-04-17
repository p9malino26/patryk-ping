#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>



int main(int argc, char** argv) {
    //get socket
    int outSoc = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (outSoc == -1) {
        printf("Socket Error.\n");
    }

    //get ip address
    struct in_addr dst;
    if (inet_aton("216.58.210.227", &dst) == 0) {
        printf("Invalid IP address.\n");
    }

    //get packet header

    struct icmphdr header;
    header.type = ICMP_ECHO;
    header.un.echo.id = 1234;//arbitrary id

    unsigned char buffer[2048];
    const char* text = "hello world";

    memcpy(buffer, &header, sizeof header);
    memcpy(buffer + sizeof header, text, strlen(text));
    struct sockaddr_in server_desc;
    //memset(&server_desc, 0, sizeof server_desc);
    server_desc.sin_addr = dst;
    server_desc.sin_family = AF_INET;



    if (sendto(outSoc, buffer, sizeof header + strlen(text), 0, (struct sockaddr*)&server_desc, sizeof(server_desc)) < 0) {
        printf("Send failed. Error code: %d\n", errno);
    } else {
        printf("Send succeeded\n");
    }

    struct timeval timeout = {3, 0};
    fd_set response;
    memset(&response, 0, sizeof response);
    FD_SET(outSoc, &response);
    //receive
    if ( select(outSoc + 1, &response, NULL, NULL, &timeout) <= 0) {
        printf("Error selecting");
    }

    //receive
    socklen_t slen = sizeof server_desc;
    int replyLength = recvfrom(outSoc, buffer, sizeof buffer, 0, (struct sockaddr*)&server_desc, &slen);
    if (replyLength != -1) {
        printf("Received reply. Length: %d", replyLength);
    } else {
        printf("got no reply");
    }


}
