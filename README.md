# CS375: Final Project: IMAP Client
## Liam Jeanette and Loc (Lucas) Tran
### May 5th, 2022

# Usage:

## To Compile:

g++ -o imap_client imap_client.h and imap_client_test.cpp

## To Run:

./imap_client rogue1.cs.denison.edu

When prompted, enter your Denison username and Denison password (unfortunately the password will be shown on your terminal, so don't let others see)

* You must have at least one message in your mailbox in order for the fetch to work. Additionally, it is assumed that only one inbox titled "inbox" exists.
** Sometimes a segmentation fault is thrown randomly. We are not sure where this comes from. We have found that if you recompile and run it again, there's a good chance it will work. We plan to fix this in future updates, but we did not have time.

# General Design

This IMAP client was designed using object oriented programming so that different test interfaces could be built using these IMAP client functions. The current test program asks the user to login, then displays the user's inbox. The user is then prompted to either read, delete, or display messages, or quit the program. The functions are implemented by sending IMAP commands over a TCP connection to an IMAP server running on rogue1.cs.denison.edu. Each time a command is sent, a new tag is generated for that command.

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

## Delete Messages

To delete messages, use the imap.delete_messagge() function. The correct message number should be passed as an argument. This function sets the deleted flag for the given message, then deletes that message.

## Logout

When the user is finished checking their mail, the imap.logout() function is called. This function logs the user out from the server.

## Quit

Finally, the imap.quit() funciton is used to close the TCP connection with the server.
