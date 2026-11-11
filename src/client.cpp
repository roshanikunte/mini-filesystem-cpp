#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;

#define SOCKET_PATH "/tmp/mini_fs_socket"

int main() {
    int clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (clientSocket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;

    strncpy(address.sun_path, SOCKET_PATH,
            sizeof(address.sun_path) - 1);

    if (connect(clientSocket, (sockaddr*)&address,
                sizeof(address)) < 0) {
        perror("Connection failed. Is the server running?");
        close(clientSocket);
        return 1;
    }

    cout << "Connected to Mini File System Server\n";
    cout << "Type 'exit' to quit.\n";

    string command;

    while (true) {
        cout << "mini-fs> ";
        getline(cin, command);

        if (command == "exit") {
            break;
        }

        if (command.empty()) {
            continue;
        }

        ssize_t sent = send(clientSocket, command.c_str(),
                            command.size(), 0);

        if (sent <= 0) {
            cout << "Failed to send command.\n";
            break;
        }

        char buffer[4096] = {0};

        ssize_t received = recv(clientSocket, buffer,
                                sizeof(buffer) - 1, 0);

        if (received <= 0) {
            cout << "Server disconnected.\n";
            break;
        }

        buffer[received] = '\0';
        cout << buffer;
    }

    close(clientSocket);

    cout << "Disconnected from server.\n";

    return 0;
}
