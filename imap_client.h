// IMAP client object for connecting to an IMAP server and reading messages
// Liam Jeanette and Loc Tran
// May 2, 2022

#include <poll.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <signal.h>

#define MAXDATASIZE 100

class Imap
{
public:
    int port;
    char portstr[4];
    char *hostname;
    int sockfd;

    Imap(char *host)
    {
        port = 143;

        portstr[0] = '1';
        portstr[1] = '4';
        portstr[2] = '3';
        portstr[3] = '\0';

        hostname = host;
    }

    // Send initial message to connect to server using TCP
    int make_connection()
    {
        char buf[MAXDATASIZE];
        int numbytes;
        struct addrinfo hints, *servinfo, *ptr;
        int rv;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo(hostname, portstr, &hints, &servinfo)) != 0)
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        }

        for (ptr = servinfo; ptr != NULL; ptr = ptr->ai_next)
        {
            if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
            {
                perror("client: socket");
                continue;
            }

            if (connect(sockfd, ptr->ai_addr, ptr->ai_addrlen) == -1)
            {
                close(sockfd);
                perror("client: connect");
                continue;
            }
            break;
        }

        if (ptr == NULL)
        {
            fprintf(stderr, "client: failed to connect\n");
            return 2;
        }

        freeaddrinfo(servinfo);

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';

        std::cout << "received: " << buf << std::endl;

        return 0;
    }

    // Authenticate user using username and password
    int login(char *username, char *password)
    {
        int user_len = strlen(username);
        int pass_len = strlen(password);

        char identifier[5] = {'a', '0', '0', '0', '1'};

        char login_message[MAXDATASIZE];

        for (int i = 0; i < 5; i++)
        {
            login_message[i] = identifier[i];
        }

        login_message[5] = ' ';

        for (int i = 0; i < user_len; i++)
        {
            login_message[i + 5 + 1] = username[i];
        }

        login_message[5 + user_len + 1] = ' ';

        for (int i = 0; i < pass_len; i++)
        {
            login_message[i + 5 + 1 + user_len + 1] = password[i];
        }

        login_message[pass_len + 5 + 1 + user_len + 1] = '\0';

        int len, bytes_sent, numbytes;
        char buf[MAXDATASIZE];

        len = strlen(login_message);
        bytes_sent = send(sockfd, login_message, len, 0);

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';

        std::cout << "received: " << buf << std::endl;
        return 0;
    }

    // Display subjects of each message
    int display_messages()
    {
        return 0;
    }

    // Access contents of message and mark as read
    int read_message()
    {
        return 0;
    }

    // Delete messages from server
    int delete_messages()
    {
        return 0;
    }

    // leave program and disconnect from server
    int quit()
    {
        close(sockfd);
        return 0;
    }
};