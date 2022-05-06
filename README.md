# cs375-p5-ateam

# Usage:

## To Compile:

g++ -o imap_client imap_client.h and imap_client_test.cpp

## To Run:

./imap_client rogue1.cs.denison.edu

When prompted, enter your Denison username and Denison password (unfortunately the password will be shown on your terminal, so don't let others see)

*You must have at least one message in your mailbox in order for the fetch to work. Additionally, it is assumed that only one inbox titled "inbox" exists.

# API

## Creating an IMAP Client object

In order to interface with the IMAP server, an IMAP client object is created. To create this object:

Imap imap_object(hostname);

This will create an IMAP client that can communicate with a server running at the hostname address.

## Connecting to the Server

The imap.make_connection() function must be called before using any of the other functionality. This function sets up a TCP connection with the server so that commands can be sent and responses can be received.

## Authentication

In order to view mail, the user must first authenticate their identity. To do this, use the imap.login() function. Your username and password should be passed as strings to this function. If the server sends a bad response, the user will see that the response did not match what was anticipated, and the program will exit.

## Displaying Messages

To see what mail is in your inbox, the imap.display_messages() function is used. This function selects the inbox "inbox", and determines how many messages exist in the mailbox. The function then fetches all subjects of messages in the mailbox and displays them to the terminal. 

## Read Messages

To read a message, use the imap.read_message() function. The correct message number should be passed as an argument. This function fetches the body of the message and displays only the text without the server response tag or flags.

## Logout

When the user is finished checking their mail, the imap.logout() function is called. This function logs the user out from the server.

## Quit

Finally, the imap.quit() funciton is used to close the TCP connection with the server.
