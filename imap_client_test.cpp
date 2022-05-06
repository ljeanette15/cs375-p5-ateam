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

    Imap imap(argv[1]);

    imap.make_connection();

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

    cout << "logging in..." << endl;
    cout << "\n";

    imap.login(username, password);

    cout << "sending capability message..." << endl;
    cout << "\n";

    imap.capability();

    cout << "selecting inbox..." << endl;
    cout << "\n";

    imap.display_messages();

    cout << "enter a message number to read... " << endl;

    char message_num[10];

    scanf("%s", message_num);

    imap.read_message(message_num);

    cout << "logging out..." << endl;
    cout << "\n";

    imap.logout();

    imap.quit();

    return 0;
}