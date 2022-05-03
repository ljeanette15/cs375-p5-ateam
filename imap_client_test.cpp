// IMAP client test file for connecting to an IMAP server and reading messages
// Liam Jeanette and Loc Tran
// May 2, 2022

#include "imap_client.h"
#include <iostream>

#define MAXDATALEN 100

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "need a command line arg" << endl;
    }

    Imap imap(argv[1]);

    imap.make_connection();

    cout << "Connection established..." << endl;

    imap.capability();

    char username[MAXDATALEN];
    char password[MAXDATALEN];

    cout << "Enter your username: ";
    fgets(username, MAXDATALEN, stdin);

    //  remove trailing newline

    if ((strlen(username) > 0) && (username[strlen(username) - 1] == '\n'))
        username[strlen(username) - 1] = '\0';

    cout << "Enter your password: ";
    fgets(password, MAXDATALEN, stdin);

    if ((strlen(password) > 0) && (password[strlen(password) - 1] == '\n'))
        password[strlen(password) - 1] = '\0';

    imap.login(username, password);

    imap.quit();

    return 0;
}