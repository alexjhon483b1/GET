#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
   //argc is the argument count and represents the number of command-line arguments passed to the program, including the program name itself
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }
    
    //*hostname is a pointer and argv[1] is also a pointer.
    char *hostname = argv[1];
    int port = atoi(argv[2]);// converting char to integer
    
    //*server is the pointer of hostent structure
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Error: No such host\n");
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }

    struct sockaddr_in server_addr;
    //type casting of pointer.  
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    //server is a pointer of a structure. first casting the type of this pointer to char. h_addr is a field of the structure which server pointing to.
    //&server_addr.sin_addr.s_addr is the destination
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);
    //if a pointer is prepared to store the memory address of a char but you want to store the memory address of an int in it you have to 
    //convert the memory addres to int
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Error connecting");
    }
    
    //prepearing the request 
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", hostname);

    if (send(sockfd, request, strlen(request), 0) < 0) {
        error("Error sending request");
    }

    char response[BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = recv(sockfd, response, BUFFER_SIZE - 1, 0)) > 0) {
        response[bytesRead] = '\0';
        printf("%s", response);
    }

    if (bytesRead < 0) {
        error("Error receiving response");
    }

    close(sockfd);

    return 0;
}

