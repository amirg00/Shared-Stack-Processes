/*
** client.c -- a stream socket client demo
*/
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>
#include <arpa/inet.h>

#include "Stack.hpp"

#define PORT "3490" // the port client will be connecting to


using namespace std;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    char buf[MAX_TEXT_SIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    while (true) {
        cout << "Enter command:";
        string input;
        getline(cin, input);
        stringstream ss{input};
        istream_iterator<string> begin(ss);
        istream_iterator<string> end;
        vector<string> texts(begin, end);


        //Send command to server
        if (send(sockfd, input.c_str(), input.length(), 0) == -1) {
            perror("send");
        }

        if (texts[0] == "EXIT") {
            break;
        }

        if (texts[0] == "POP"/*TOP*/) {
            if ((numbytes = recv(sockfd, buf, MAX_TEXT_SIZE - 1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';

            if (!strcmp(buf, "0")) {
                perror("ERROR: Stack is empty, thus cannot pop!");
            }
        }
        // If command is TOP client has to wait for response.
        if (texts[0] == "TOP"/*TOP*/) {
            if ((numbytes = recv(sockfd, buf, MAX_TEXT_SIZE - 1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';

            if (!strcmp(buf, "0")) {
                perror("ERROR: Stack is empty, thus cannot top!");
                continue;
            }
            printf("%s\n", buf);

        }
    }
    close(sockfd);
    return 0;
}