#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>



int main(int argc, char** argv) {
    int outSoc = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (outSoc == -1) {
        printf("Socket Error.\n");
    }

    struct in_addr dst;
    if (inet_aton("216.58.210.227", &dst) == 0) {
        printf("Invalid IP address.\n");
    }

    struct icmphdr header;
    header.type = ICMP_ECHO;
    header.un.echo.id = 1234;//arbitrary id

    char buffer[2048];
    const char* text = "hello world";

    memcpy(buffer, &header, sizeof header);
    memcpy(buffer + sizeof header, text, strlen(text));
    struct sockaddr_in server_desc;
    //memset(&server_desc, 0, sizeof server_desc);
    server_desc.sin_addr = dst;
    server_desc.sin_family = AF_INET;



    if (sendto(outSoc, buffer, sizeof buffer, 0, (struct sockaddr*)&server_desc, sizeof(server_desc)) < 0) {
        printf("Send failed. Error code: %d\n", errno);
    } else {
        printf("Send succeeded\n");
    }


}
