/*
** server.c -- a stream socket server demo
*/
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <vector>
#include <dirent.h>
#include <sstream>
#include <iterator>


#include "Stack.hpp"

#define _MMAP_SIZE 1000*1024               // Approx. 1 mb
#define PORT "3490"                        // the port users will be connecting to
#define BACKLOG 10                         // how many pending connections queue will hold

Stack* clients_stack;                      // Global children stack
int* nodes_amount;
struct flock lock;
int fd;

using namespace std;

void sigchld_handler(int s) {
    (void) s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}
void *THREAD(void* input) {

    int *current_fd = (int *) input;
    int new_fd = *current_fd;

    //sleep(10);//to show a connection of several threads and not close quickly

    while(true){
        cout << getpid() << endl;
        cout << clients_stack << endl;
        char buf[MAX_TEXT_SIZE];
        int numbytes;
        int err_flag = 0;
        if ((numbytes = recv(new_fd, buf, MAX_TEXT_SIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
        string str_usr{buf};
        stringstream ss{str_usr};
        istream_iterator<string> begin(ss);
        istream_iterator<string> end;

        vector<string> texts(begin, end);
        cout << "Command: " << texts[0] << endl;

        // Thread locks code section, to avoid race condition
        lock.l_type = F_WRLCK;
        fcntl(fd, F_SETLKW, &lock);
        if (texts[0] == "PUSH"/*PUSH <SOMETHING>*/){
            (*nodes_amount)++;
            string word;
            for (int i = 1; i < texts.size(); ++i, word += ' ') {word += texts[i];}
            node_ptr node = (node_ptr)(clients_stack + ++(*nodes_amount));
            PUSH(clients_stack, word.c_str(), node);
            //print_stack(clients_stack); // BAD BEHAVIOUR FOUND!
        }
        else if (texts[0] == "POP"/*POP*/){
            POP(clients_stack, &err_flag);
            if (!err_flag){
                // Good response
                if(send(new_fd, "1", 1 ,0) == -1){
                    perror("send");
                }
            }
            else {
                // bad response back to user to present error
                if(send(new_fd, "0", 1 ,0) == -1){
                    perror("send");
                }
            }
        }
        else if (texts[0] == "TOP"/*TOP*/){
            char output[9] = "OUTPUT: ";
            char* top = TOP(clients_stack, &err_flag);
            char* out = strcat(output, top);
            if (!err_flag){
                if(send(new_fd, out, strlen(out),0) == -1){
                    perror("send");
                }
            }
            else {
                // bad response back to user to present error
                if(send(new_fd, "0", 1 ,0) == -1){
                    perror("send");
                }
            }

        }

        else if (texts[0] == "EXIT"){
            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &lock);
            break;
        }
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &lock);
    }
    //close(new_fd);
}

int main(void) {
    memset(&lock, 0, sizeof(lock));
    fd = open("tmp_lock.txt", O_WRONLY); // open temporary file for locks.
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    // CREATING SHARED MEMORY
    // Sharing before forking
    clients_stack = (Stack*) mmap(NULL, _MMAP_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    nodes_amount = (int*) mmap(NULL, _MMAP_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *nodes_amount = 0; // reset variable
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while (true) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *) &their_addr),
                          s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            THREAD(&new_fd);
        }
        else{
           //wait(NULL);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}
