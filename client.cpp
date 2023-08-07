// Ethan Loftis Elk3x 03/16/2023
// client server chat room using sockets

#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <numeric>

using namespace std;
#define PORT 10104
  
int main(int argc, char const* argv[])
{
    int status, valRead, clientFd;
    struct sockaddr_in serverAddress;
    char buffer[256] = {0};

    if ((clientFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }
  
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
  
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr)
        <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    status = connect(clientFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  
    if (status  < 0) {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }

   cout << "My chat room client. Version one." << endl;

    while(1) {
        cout << ">";
        string data;
        getline(cin, data);

        memset(&buffer, 0, sizeof(buffer));
        strcpy(buffer, data.c_str());

        vector <string> commandToken;
        stringstream check1(data);
        string intermediate;
        while(getline(check1, intermediate, ' ')){
            commandToken.push_back(intermediate);
        }
        string command = commandToken[0];

        //send messages and error checking user input 
        if (command == "logout") {
            if (commandToken.size() > 1) {
                cout << ">Invalid use of command: too many arguements" << endl;
            } else {
                send(clientFd, (char*)&buffer, strlen(buffer), 0); 
                break;
            }
        } else if (command == "send") {
            send(clientFd, (char*)&buffer, strlen(buffer), 0);
        } else if (command == "login") {
            if (commandToken.size() > 3) {
                cout << ">Invalid use of command: too many arguements" << endl;
                continue;
            }else if (commandToken.size() < 3) {
                cout << ">Invalid use of command: too few arguements" << endl;
                continue;
            }else {
                send(clientFd, (char*)&buffer, strlen(buffer), 0);
            }
        } else if (command == "newuser") {
            if (commandToken.size() > 3) {
                cout << ">Invalid use of command: too many arguements" << endl;
                continue;
            } else if (commandToken.size() < 3) {
                cout << ">Invalid use of command: too few arguements" << endl;
                continue;
            } else if (commandToken[1].size() < 3 || commandToken[1].size() > 32) {
                cout << "> UserId must be within 3 and 32 characters" << endl;
                continue;
            } else if (commandToken[2].size() < 4 || commandToken[2].size() > 8) {
                cout << "> Password must be within 4 and 8 characters" << endl;
                continue;
            } else {
                send(clientFd, (char*)&buffer, strlen(buffer), 0);
            }
        } else {
            cout << ">Unkown command used." << endl;
            continue;
        }

        //responses from the server
        memset(&buffer, 0, sizeof(buffer));
        recv(clientFd, (char*)&buffer, sizeof(buffer), 0);

         if (command == "send") {
            cout << ">" <<  buffer << endl;
        }else if (command == "newuser") {
            cout << ">" <<  buffer << endl;
        }else if (command == "login") {
            cout << ">" <<  buffer << endl;
        } else if (command == "logout") {
            cout << ">" <<  buffer << endl;
            break;
        }
    }
  
    close(clientFd);
    return 0;
}