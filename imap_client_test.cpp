// IMAP client test file for connecting to an IMAP server and reading messages
// Liam Jeanette and Loc Tran
// May 2, 2022

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

    cout << "Connection established..." << endl;

    // imap.login();

    imap.quit();

    return 0;
}