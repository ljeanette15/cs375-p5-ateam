// IMAP client object for connecting to an IMAP server and reading messages
// Liam Jeanette and Loc Tran
// May 5, 2022

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
    int tag;

    Imap(char *host)
    {
        port = 143;

        portstr[0] = '1';
        portstr[1] = '4';
        portstr[2] = '3';
        portstr[3] = '\0';

        hostname = host;

        tag = 1;
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
            // Construct socket
            if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
            {
                perror("client: socket");
                continue;
            }

            // Connect to Server
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

        // Poll to listen for response from server (could come in multiple recvs)
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
                    // Receive response from server
                    if (pfds[i].fd == sockfd)
                    {
                        memset(buf, 0, MAXDATASIZE);
                        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
                        {
                            perror("recv");
                            exit(1);
                        }

                        buf[numbytes] = '\0';
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

        // Generate command message to send to server
        char command[] = " CAPABILITY\n";

        char tag_str[MAXDATASIZE];
        memset(tag_str, 0, MAXDATASIZE);

        sprintf(tag_str, "%d", tag);

        char message[MAXDATASIZE];
        memset(message, 0, MAXDATASIZE);

        for (int i = 0; i < strlen(tag_str); i++)
        {
            message[i] = tag_str[i];
        }

        for (int i = 0; i < strlen(command); i++)
        {
            message[i + strlen(tag_str)] = command[i];
        }

        int len, bytes_sent, numbytes;

        // Send command to server
        bytes_sent = send(sockfd, message, strlen(message), 0);

        tag++;

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        // Listen for response from server
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
                    // response from server
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

        std::cout << received_message << std::endl;

        return 0;
    }

    // Authenticate user using username and password
    int login(char *username, char *password)
    {
        char status_received[2];
        char received_message[MAXDATASIZE];
        char buf[MAXDATASIZE];
        memset(received_message, 0, MAXDATASIZE);

        int user_len = strlen(username);
        int pass_len = strlen(password);

        char tag_str[MAXDATASIZE];
        memset(tag_str, 0, MAXDATASIZE);

        sprintf(tag_str, "%d", tag);

        // Add initial unique ID and command to message
        char command[] = " LOGIN ";

        char login_message[MAXDATASIZE];

        // Add ID, username, and password to message
        for (int i = 0; i < strlen(tag_str); i++)
        {
            login_message[i] = tag_str[i];
        }

        for (int i = 0; i < strlen(command); i++)
        {
            login_message[i + strlen(tag_str)] = command[i];
        }

        for (int i = 0; i < user_len; i++)
        {
            login_message[i + strlen(tag_str) + strlen(command)] = username[i];
        }

        login_message[strlen(tag_str) + strlen(command) + user_len] = ' ';

        for (int i = 0; i < pass_len; i++)
        {
            login_message[i + strlen(tag_str) + strlen(command) + user_len + 1] = password[i];
        }

        login_message[pass_len + user_len + strlen(tag_str) + strlen(command) + 1] = '\n';
        login_message[pass_len + user_len + strlen(tag_str) + strlen(command) + 2] = '\0';

        int len, bytes_sent, numbytes;

        // Send login command to server
        bytes_sent = send(sockfd, login_message, strlen(login_message), 0);

        tag++;

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
                    // Response from Server
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

        char tag_received[strlen(tag_str)];
        memset(tag_received, 0, strlen(tag_str));
        memset(status_received, 0, 2);

        for (int i = 0; i < strlen(tag_str); i++)
        {
            tag_received[i] = received_message[i];
        }

        if (strncmp(tag_received, tag_str, strlen(tag_str)) != 0)
        {
            perror("tag received does not match");
            exit(-1);
        }

        for (int i = 0; i < 2; i++)
        {
            status_received[i] = received_message[i + strlen(tag_str) + 1];
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

        // Send select command to get inbox

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
                    // response from server
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

        // Fetch all message subjects

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

        // Construct command

        char tag_str[100];
        memset(tag_str, 0, 100);
        sprintf(tag_str, "%d", tag);

        char command[] = " FETCH ";
        char argument[] = "BODY[TEXT]\n";

        char total_message[MAXDATASIZE];
        memset(total_message, 0, MAXDATASIZE);

        for (int i = 0; i < strlen(tag_str); i++)
        {
            total_message[i] = tag_str[i];
        }

        for (int i = 0; i < strlen(command); i++)
        {
            total_message[i + strlen(tag_str)] = command[i];
        }

        for (int i = 0; i < strlen(message_num); i++)
        {
            total_message[i + strlen(tag_str) + strlen(command)] = message_num[i];
        }

        total_message[strlen(tag_str) + strlen(command) + strlen(message_num)] = ' ';

        for (int i = 0; i < strlen(argument); i++)
        {
            total_message[i + strlen(tag_str) + strlen(command) + strlen(message_num) + 1] = argument[i];
        }

        int len, bytes_sent, numbytes;

        // Send command
        bytes_sent = send(sockfd, total_message, strlen(total_message), 0);

        tag++;

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
                    // response from server
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

        // Print out message without the extra stuff from server

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
    int delete_message(char *message_num)
    {
        char buf[MAXDATASIZE];
        char received_message[MAXDATASIZE];
        memset(received_message, 0, MAXDATASIZE);

        // construct command

        char tag_str[100];
        memset(tag_str, 0, 100);
        sprintf(tag_str, "%d", tag);

        char command[] = " STORE ";
        char arg[] = "+FLAGS (\\Deleted)\n";

        char message[MAXDATASIZE];
        memset(message, 0, MAXDATASIZE);

        for (int i = 0; i < strlen(tag_str); i++)
        {
            message[i] = tag_str[i];
        }

        for (int i = 0; i < strlen(command); i++)
        {
            message[i + strlen(tag_str)] = command[i];
        }

        for (int i = 0; i < strlen(message_num); i++)
        {
            message[i + strlen(tag_str) + strlen(command)] = message_num[i];
        }

        message[strlen(tag_str) + strlen(command) + strlen(message_num)] = ' ';

        for (int i = 0; i < strlen(arg); i++)
        {
            message[i + strlen(tag_str) + strlen(command) + strlen(message_num) + 1] = arg[i];
        }

        int len, bytes_sent, numbytes;

        // Send command
        bytes_sent = send(sockfd, message, strlen(message), 0);

        tag++;

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

        // Expunge messages with deleted flag set

        memset(tag_str, 0, 100);
        sprintf(tag_str, "%d", tag);

        char command2[] = " EXPUNGE\n";

        memset(message, 0, MAXDATASIZE);

        for (int i = 0; i < strlen(tag_str); i++)
        {
            message[i] = tag_str[i];
        }

        for (int i = 0; i < strlen(command2); i++)
        {
            message[i + strlen(tag_str)] = command2[i];
        }

        memset(received_message, 0, MAXDATASIZE);

        bytes_sent = send(sockfd, message, strlen(message), 0);

        tag++;

        if (bytes_sent == -1)
        {
            std::cout << "error sending" << std::endl;
            exit(1);
        }

        count = 0;

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
        std::cout << received_message << std::endl;

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