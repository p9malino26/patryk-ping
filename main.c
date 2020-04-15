#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>


int main(int argc, char** argv) {
    int outSoc = socket(PF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (outSoc == -1) {
        printf("Socket Error.\n");
    }

    struct sockaddr_in server_desc;
    server_desc.sin_addr.s_addr = inet_addr("92.222.139.190");
    server_desc.sin_family = AF_INET;


    if(connect(outSoc, (struct sockaddr *)&server_desc, sizeof(server_desc)) < 0) {
        printf("Connection error.");
    }
    if (send(outSoc, "hello", 5, 0) < 0) {
        printf("Send failed. Error code: %d\n", errno);
    }
}
