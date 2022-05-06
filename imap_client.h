// IMAP client object for connecting to an IMAP server and reading messages
// Liam Jeanette and Loc Tran
// May 2, 2022

#include <regex.h>
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
#include <poll.h>

#define MAXDATASIZE 1000

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
        int numbytes;
        struct addrinfo hints, *servinfo, *ptr;
        int rv;
        char s[INET6_ADDRSTRLEN];
        char buf[MAXDATASIZE];
        memset(buf, 0, MAXDATASIZE);

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

            if (connect(sockfd, ptr->ai_addr, ptr->ai_addrlen) != 0)
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

        struct pollfd pfds[1];

        pfds[0].fd = sockfd;
        pfds[0].events = POLLIN;

        int count = 0;

        while (count < 10)
        {
            int poll_count = poll(pfds, 1, 200);

            if (poll_count == -1)
            {
                perror("poll");
                exit(1);
            }

            for (int i = 0; i < 1; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    // Ack from receiver
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';

                        // std::cout << "received: " << buf << std::endl;
                    }
                }
            }
            count++;
        }

        return 0;
    }

    // Request Capabilities from Server
    int capability()
    {
        char buf[MAXDATASIZE];
        char received_message[MAXDATASIZE];
        memset(received_message, 0, MAXDATASIZE);

        int len, bytes_sent, numbytes;

        bytes_sent = send(sockfd, "A0002 CAPABILITY\n", strlen("A0002 CAPABILITY\n"), 0);

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        struct pollfd pfds[1];

        pfds[0].fd = sockfd;
        pfds[0].events = POLLIN;

        int count = 0;

        while (count < 10)
        {
            int poll_count = poll(pfds, 1, 200);

            if (poll_count == -1)
            {
                perror("poll");
                exit(1);
            }

            for (int i = 0; i < 1; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    // Ack from receiver
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';

                        strcat(received_message, buf);
                    }
                }
            }
            count++;
        }

        // std::cout << received_message << std::endl;

        return 0;
    }

    // Authenticate user using username and password
    int login(char *username, char *password)
    {
        char tag_received[6];
        char status_received[2];
        char received_message[MAXDATASIZE];
        char buf[MAXDATASIZE];
        memset(received_message, 0, MAXDATASIZE);

        int user_len = strlen(username);
        int pass_len = strlen(password);
        char tag[6] = {'A', '0', '0', '0', '1', ' '};

        // Add initial unique ID and command to message
        char command[6] = {'L', 'O', 'G', 'I', 'N', ' '};

        char login_message[MAXDATASIZE];

        // Add ID, username, and password to message
        for (int i = 0; i < 6; i++)
        {
            login_message[i] = tag[i];
        }

        for (int i = 0; i < 6; i++)
        {
            login_message[i + 6] = command[i];
        }

        for (int i = 0; i < user_len; i++)
        {
            login_message[i + 12] = username[i];
        }

        login_message[12 + user_len] = ' ';

        for (int i = 0; i < pass_len; i++)
        {
            login_message[i + 12 + user_len + 1] = password[i];
        }

        login_message[pass_len + user_len + 12 + 1] = '\n';
        login_message[pass_len + user_len + 12 + 2] = '\0';

        int len, bytes_sent, numbytes;

        // Send login command to server
        bytes_sent = send(sockfd, login_message, strlen(login_message), 0);

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        struct pollfd pfds[1];

        pfds[0].fd = sockfd;
        pfds[0].events = POLLIN;

        int count = 0;

        while (count < 10)
        {
            int poll_count = poll(pfds, 1, 200);

            if (poll_count == -1)
            {
                perror("poll");
                exit(1);
            }

            for (int i = 0; i < 1; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    // Ack from receiver
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';

                        strcat(received_message, buf);
                    }
                }
            }
            count++;
        }

        // std::cout << received_message << std::endl;

        memset(tag_received, 0, 6);
        memset(status_received, 0, 2);

        for (int i = 0; i < 6; i++)
        {
            tag_received[i] = received_message[i];
        }

        if (strncmp(tag_received, tag, 5) != 0)
        {
            perror("tag received does not match");
            exit(-1);
        }

        for (int i = 0; i < 2; i++)
        {
            status_received[i] = received_message[i + 6];
        }
        if (strncmp(status_received, "OK", 2) != 0)
        {
            perror("Login Failed.");
            exit(-1);
        }

        return 0;
    }

    // Display subjects of each message and its message number
    int display_messages()
    {
        char buf[MAXDATASIZE];
        char received_message[MAXDATASIZE];
        memset(received_message, 0, MAXDATASIZE);

        int len, bytes_sent, numbytes;

        bytes_sent = send(sockfd, "A0003 SELECT inbox\n", strlen("A0003 SELECT INBOX\n"), 0);

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        struct pollfd pfds[1];

        pfds[0].fd = sockfd;
        pfds[0].events = POLLIN;

        int count = 0;

        while (count < 100)
        {
            int poll_count = poll(pfds, 1, 10);

            if (poll_count == -1)
            {
                perror("poll");
                exit(1);
            }

            for (int i = 0; i < 1; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    // Ack from receiver
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';

                        strcat(received_message, buf);
                    }
                }
            }
            count++;
        }

        // Get number of messages in mailbox
        regex_t reegex;
        regmatch_t pmatch[1];
        char num_messages[100];
        int num;
        memset(num_messages, 0, 100);

        int value;

        value = regcomp(&reegex, "[1,2,3,4,5,6,7,8,9] EXISTS", 0);

        if (regexec(&reegex, received_message, 1, pmatch, 0) == 0)
        {
            int i = 0;
            while (received_message[i + pmatch[0].rm_so] != 'E')
            {

                num_messages[i] = received_message[i + pmatch[0].rm_so];

                i++;
            }
            num_messages[i - 1] = '\0';

            num = atoi(num_messages);
        }

        else
        {
            std::cout << "pattern not found" << std::endl;
        }

        std::cout << "You have " << num << " messages " << std::endl;

        // Fetch messages

        bytes_sent = send(sockfd, "A654 FETCH 1:* (BODY[HEADER.FIELDS (SUBJECT)])\n", strlen("A654 FETCH 1:* (BODY[HEADER.FIELDS (SUBJECT)])\n"), 0);

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        count = 0;

        memset(received_message, 0, MAXDATASIZE);
        while (count < 100)
        {
            int poll_count = poll(pfds, 1, 10);

            if (poll_count == -1)
            {
                perror("poll");
                exit(1);
            }

            for (int i = 0; i < 1; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    // Ack from receiver
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';

                        strcat(received_message, buf);
                    }
                }
            }
            count++;
        }

        char *newptr;
        char *newnewptr;
        newptr = strstr(received_message, "Subject:");

        // Print out message subjects with their message number
        for (int j = 0; j < num; j++)
        {
            int i = 0;
            std::cout << j + 1 << " ";
            while (newptr[i] != '\n')
            {
                std::cout << newptr[i];
                i++;
            }

            newnewptr = newptr + 8;
            newptr = strstr(newnewptr, "Subject:");
            std::cout << "\n";
        }

        return 0;
    }

    // Access contents of message and mark as read
    int read_message(char message_num[])
    {

        char buf[MAXDATASIZE];
        char received_message[MAXDATASIZE];
        memset(received_message, 0, MAXDATASIZE);

        char tag[] = "A0005 ";
        char command[] = "FETCH ";
        char argument[] = "BODY[TEXT]\n";

        char total_message[MAXDATASIZE];

        for (int i = 0; i < strlen(tag); i++)
        {
            total_message[i] = tag[i];
        }

        for (int i = 0; i < strlen(command); i++)
        {
            total_message[i + strlen(tag)] = command[i];
        }

        for (int i = 0; i < strlen(message_num); i++)
        {
            total_message[i + strlen(tag) + strlen(command)] = message_num[i];
        }

        total_message[strlen(tag) + strlen(command) + strlen(message_num)] = ' ';

        for (int i = 0; i < strlen(argument); i++)
        {
            total_message[i + strlen(tag) + strlen(command) + strlen(message_num) + 1] = argument[i];
        }

        int len, bytes_sent, numbytes;

        bytes_sent = send(sockfd, total_message, strlen(total_message), 0);

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        struct pollfd pfds[1];

        pfds[0].fd = sockfd;
        pfds[0].events = POLLIN;

        int count = 0;

        while (count < 10)
        {
            int poll_count = poll(pfds, 1, 200);

            if (poll_count == -1)
            {
                perror("poll");
                exit(1);
            }

            for (int i = 0; i < 1; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    // Ack from receiver
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';

                        strcat(received_message, buf);
                    }
                }
            }
            count++;
        }

        char *newstr = strchr(received_message, '\n');
        int i = 0;
        while (newstr[i] != ')')
        {
            std::cout << newstr[i];
            i++;
        }
        std::cout << '\n';
        return 0;
    }

    // Delete messages from server
    int delete_messages(char *message_num)
    {
        char buf[MAXDATASIZE];
        char received_message[MAXDATASIZE];
        memset(received_message, 0, MAXDATASIZE);

        int len, bytes_sent, numbytes;

        bytes_sent = send(sockfd, "A0003 CAPABILITY\n", strlen("A0003 CAPABILITY\n"), 0);

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        struct pollfd pfds[1];

        pfds[0].fd = sockfd;
        pfds[0].events = POLLIN;

        int count = 0;

        while (count < 10)
        {
            int poll_count = poll(pfds, 1, 200);

            if (poll_count == -1)
            {
                perror("poll");
                exit(1);
            }

            for (int i = 0; i < 1; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    // Ack from receiver
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';

                        strcat(received_message, buf);
                    }
                }
            }
            count++;
        }

        // std::cout << received_message << std::endl;

        return 0;
    }

    // Logout from mail server
    int logout()
    {

        char buf[MAXDATASIZE];
        char received_message[MAXDATASIZE];
        memset(received_message, 0, MAXDATASIZE);

        int len, bytes_sent, numbytes;

        bytes_sent = send(sockfd, "A0004 LOGOUT\n", strlen("A0004 LOGOUT\n"), 0);

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        struct pollfd pfds[1];

        pfds[0].fd = sockfd;
        pfds[0].events = POLLIN;

        int count = 0;

        while (count < 100)
        {
            int poll_count = poll(pfds, 1, 10);

            if (poll_count == -1)
            {
                perror("poll");
                exit(1);
            }

            for (int i = 0; i < 1; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    // Ack from receiver
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';

                        strcat(received_message, buf);
                    }
                }
            }
            count++;
        }

        // std::cout << received_message << std::endl;

        return 0;
    }

    // leave program and disconnect from server
    int quit()
    {
        close(sockfd);
        return 0;
    }
};