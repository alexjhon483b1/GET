#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define PACKET_SIZE     64
#define MAX_WAIT_TIME   5

struct icmp_packet {
    struct icmphdr header;
    char data[PACKET_SIZE - sizeof(struct icmphdr)];
};
//void *b is a generic pointer
unsigned short checksum(void *b, int len) {
    //buf is a pointer unsigned short, type casting b pointer to buf
    unsigned short *buf = b;
    unsigned int sum = 0;

    for (sum = 0; len > 1; len -= 2) {
        //sum = sum +a, sum = sum + b, sum = sum + c ....
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }
    //sum >> 16 means "shift the bits of sum 16 positions to the right."
    //sum & 0xFFFF means "perform a bitwise AND operation between the bits of sum and the bits of the binary value 1111111111111111
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    //not operation ~sum means "flip all the bits in sum.
    return (unsigned short)(~sum);
}

void send_ping_request(int sockfd, struct sockaddr_in *target_addr) {
    struct icmp_packet packet;
    memset(&packet, 0, sizeof(packet));

    packet.header.type = ICMP_ECHO;
    packet.header.code = 0;
    packet.header.checksum = 0;
    packet.header.un.echo.id = getpid();
    packet.header.un.echo.sequence = 1;
    memset(packet.data, 'A', sizeof(packet.data));
    packet.header.checksum = checksum(&packet, sizeof(packet));

    if (sendto(sockfd, &packet, sizeof(packet), 0,
               (struct sockaddr *)target_addr, sizeof(*target_addr)) == -1) {
        perror("sendto");
    }
}

int receive_ping_reply(int sockfd, struct sockaddr_in *target_addr) {
    struct icmp_packet packet;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    ssize_t bytes_received = recvfrom(sockfd, &packet, sizeof(packet), 0,
                                      (struct sockaddr *)target_addr, &addr_len);

    if (bytes_received == -1) {
        perror("recvfrom");
        return -1;
    }

    return packet.header.un.echo.sequence;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <target_ip>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *target_ip = argv[1];

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, target_ip, &target_addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", target_ip);
        close(sockfd);
        return EXIT_FAILURE;
    }

    send_ping_request(sockfd, &target_addr);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    struct timeval timeout;
    timeout.tv_sec = MAX_WAIT_TIME;
    timeout.tv_usec = 0;

    int result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

    if (result == -1) {
        perror("select");
        close(sockfd);
        return EXIT_FAILURE;
    } else if (result == 0) {
        printf("No response from %s\n", target_ip);
    } else {
        int sequence = receive_ping_reply(sockfd, &target_addr);
        if (sequence != -1) {
            printf("%s is active\n", target_ip);
        }
    }

    close(sockfd);
    return EXIT_SUCCESS;
}

