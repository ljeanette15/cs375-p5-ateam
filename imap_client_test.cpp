// IMAP client test file for connecting to an IMAP server and reading messages
// Liam Jeanette and Loc Tran
// May 2, 2022

#include <poll.h>
#include "imap_client.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        cout << "need a command line arg" << endl;
    }

    char buf[MAXDATASIZE];
    int numbytes;

    Imap imap(argv[1]);

    imap.make_connection();

    // imap.capability();

    char username[MAXDATASIZE];
    char password[MAXDATASIZE];

    cout << "Enter your username: ";
    fgets(username, MAXDATASIZE, stdin);

    //  remove trailing newline

    if ((strlen(username) > 0) && (username[strlen(username) - 1] == '\n'))
        username[strlen(username) - 1] = '\0';

    cout << "Enter your password: ";
    fgets(password, MAXDATASIZE, stdin);

    if ((strlen(password) > 0) && (password[strlen(password) - 1] == '\n'))
        password[strlen(password) - 1] = '\0';

    cout << "username: " << username << " password: " << password << endl;

    imap.login(username, password);

    cout << "login successful..." << endl;

    imap.quit();

    return 0;
}