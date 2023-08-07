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
#include <cstdio>
#include <algorithm>

using namespace std;


#define PORT 10104

int main(int argc, char *argv[]) {
    bool exists = false;
    bool isLoggedIn = false;
    char buffer[256];
    string user;

    struct sockaddr_in serverAddress;
    bzero((char*)&serverAddress,sizeof(serverAddress));
    int opt = 1;
    int clientSd;
    
//creating socket
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSd < 0) {
        perror("Error: Socket Failed\n");
        return 1;
    }

    if (setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("socketopt\n");
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT);

    //bind socket to address

    int bindStatus = ::bind(serverSd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if ( bindStatus < 0 ) {
        cerr << "bind failed" << endl;
        return 1;
    }

    cout << "My chat room server. Version One.\n\n" << endl;

//passive listen for new connections
    if (listen(serverSd, 1) < 0) {
        cerr << "listen" << endl;
        return 1;
    }

    int addressLen = sizeof(serverAddress);

    int bytesRead = 0;

    //accept connection with client
    
    clientSd = accept(serverSd, (struct sockaddr*)&serverAddress, (socklen_t*)&addressLen);

    if(clientSd < 0) {
        cerr << "error accepting request from client" << endl;
        close(serverSd);
    }

    while (1) {

        // gather user infortmation
        ifstream myFile("users.txt");
        string line;
        vector<string> pairs;
                
        if (myFile.is_open() && !myFile.eof()) {
            while (getline(myFile, line,'(') && getline(myFile, line, ')')) {
                line.erase(remove(line.begin(), line.end(), ','), line.end());
                pairs.push_back(line);
            }
            myFile.close();
        }

        //read in commands from client
        memset(&buffer, 0, sizeof(buffer));
        bytesRead += recv(clientSd, (char*)&buffer, sizeof(buffer), 0);

        //gets specified command

        vector <string> commandToken;
        stringstream check1(buffer);
        string intermediate;
        while(getline(check1, intermediate, ' ')) {
            commandToken.push_back(intermediate);
        }
        string command = commandToken[0];

        commandToken.erase(commandToken.begin());

        // command arguements

        stringstream msgToSend;

            for (size_t i = 0; i < commandToken.size(); i++) {
                if (i != 0) {
                    msgToSend << " ";
                }
                msgToSend << commandToken[i];
            }

        // logic for commands && sending responses to client
        memset(&buffer, 0, sizeof(buffer));
        string toDisplay;
        if(command == "logout") {
            if(isLoggedIn) {
                string logoutMsg =  user + " left.";
                strcpy(buffer, logoutMsg.c_str());
                send(clientSd, (char*)&buffer, strlen(buffer), 0);
                cout << user << " " << "logout" << endl;
            }
            
            close(clientSd);
            break;
        } else if (command == "login") {
            //checks if user is already logged in
            //if not checks if credentials map to credentials in database
            if (isLoggedIn) {
                char error[] = "Must be logged out to login";
                send(clientSd, (char*)&error, strlen(error), 0);
            }else {
                if(find(pairs.begin(), pairs.end(), msgToSend.str()) != pairs.end()) {
                    user = commandToken[0];
                    isLoggedIn = true;
                    cout << user << " " << "login" << endl;
                    char error[] = "login confirmed";
                    send(clientSd, (char*)&error, strlen(error), 0);
                    continue;
                }

                char error[] = "Denied. User name or password incorrect.";
                send(clientSd, (char*)&error, strlen(error), 0);
                continue;
            }
            continue;
        } else if(command == "newuser") {
            //create new user
            //checks if already logged in
            //if not checks if user already exists in database
            if(isLoggedIn) {
                char error[] = "Must be logged out to create a new user";
                send(clientSd, (char*)&error, strlen(error), 0);
            } else {
                if(find(pairs.begin(), pairs.end(), msgToSend.str()) != pairs.end()) {
                    exists = true;
                    char error[] = "Denied. User account already exists.";
                    send(clientSd, (char*)&error, strlen(error), 0);
                    continue;
                }

                if (!exists) {
                    string dbEntry = "(" + commandToken[0] + ", " + commandToken[1] + ")";

                    ofstream file("users.txt", std::ios_base::app);

                    if(file.is_open()) {
                        file << dbEntry << endl;
                        file.close();
                    }
                
                    cout << "New user account created." << endl;
                    char success[] = "New user account created. Please login";
                    send(clientSd, (char*)&success, strlen(success), 0);
                    continue;
                }
            }
            continue;
        } else if (command == "send") {
            //checks if user is logged in
            //if not cannot use command 
            if (isLoggedIn) {
                toDisplay = user + ": " + msgToSend.str();
                cout << toDisplay << endl;
                strcpy(buffer, toDisplay.c_str());
                send(clientSd, (char*)&buffer, strlen(buffer), 0);
            } else {
                char error[] = "Denied. Please login first.";
                send(clientSd, (char*)&error, strlen(error), 0);
            }
            continue;
        }

        cout << buffer << endl;
    }

    close(clientSd);
    close(serverSd);

    return 0;
}