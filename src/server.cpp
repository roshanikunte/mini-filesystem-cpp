#include "filesystem.hpp"
#include "persistence.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <thread>
#include <mutex>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;

#define SOCKET_PATH "/tmp/mini_fs_socket"
#define DATA_FILE "data/filesystem.dat"

FileSystem fs;
mutex fs_mutex;

// Convert relative paths into absolute paths
string makeAbsolutePath(const string& cwd, const string& path) {
    if (path.empty())
        return cwd;

    if (path[0] == '/')
        return path;

    return (cwd == "/" ? "" : cwd) + "/" + path;
}

// Split a path into parent directory and name
void splitPath(const string& path, string& parent, string& name) {
    size_t pos = path.find_last_of('/');

    if (pos == string::npos) {
        parent = "/";
        name = path;
    }
    else if (pos == 0) {
        parent = "/";
        name = path.substr(1);
    }
    else {
        parent = path.substr(0, pos);
        name = path.substr(pos + 1);
    }
}

// Handle commands from each client
void handleClient(int clientSocket) {

    char buffer[1024];
    string cwd = "/";

    while (true) {

        memset(buffer, 0, sizeof(buffer));

        ssize_t bytes = recv(
            clientSocket,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (bytes <= 0)
            break;

        stringstream ss(string(buffer, bytes));

        string operation, argument, option;
        ss >> operation;

        string response;

        {
            lock_guard<mutex> lock(fs_mutex);

            // PWD command
            if (operation == "pwd") {
                response = cwd + "\n";
            }

            // CD command
            else if (operation == "cd") {

                ss >> argument;

                if (argument.empty())
                    argument = "/";

                string target = makeAbsolutePath(cwd, argument);

                if (fs.cd(target, cwd))
                    response = "Directory changed\n";
                else
                    response = "Directory not found\n";
            }

            // MKDIR and TOUCH commands
            else if (operation == "mkdir" ||
                     operation == "touch") {

                ss >> argument;

                if (argument.empty()) {
                    response = "Path required\n";
                }
                else {

                    string target =
                        makeAbsolutePath(cwd, argument);

                    string parent, name;

                    splitPath(target, parent, name);

                    bool success = false;

                    if (operation == "mkdir")
                        success = fs.mkdir(parent, name);
                    else
                        success = fs.touch(parent, name);

                    if (success) {

                        if (Persistence::save(fs, DATA_FILE))
                            response = "Created successfully\n";
                        else
                            response = "Created, but saving failed\n";

                    }
                    else {
                        response = "Operation failed\n";
                    }
                }
            }

            // LS command
            else if (operation == "ls") {

                ss >> argument;

                bool detailed = false;

                if (argument == "-l") {
                    detailed = true;
                    argument = "";
                }
                else {
                    ss >> option;
                    detailed = (option == "-l");
                }

                string target = argument.empty()
                    ? cwd
                    : makeAbsolutePath(cwd, argument);

                response = fs.ls(target, detailed);
            }

            // RM command
            else if (operation == "rm") {

                ss >> option;

                bool recursive =
                    (option == "-rf" || option == "-r");

                if (!recursive)
                    argument = option;
                else
                    ss >> argument;

                if (argument.empty()) {
                    response = "Path required\n";
                }
                else {

                    string target =
                        makeAbsolutePath(cwd, argument);

                    string parent, name;

                    splitPath(target, parent, name);

                    bool success =
                        fs.rm(parent, name, recursive);

                    if (success) {

                        if (Persistence::save(fs, DATA_FILE))
                            response = "Removed successfully\n";
                        else
                            response = "Removed, but saving failed\n";

                    }
                    else {
                        response = "Removal failed\n";
                    }
                }
            }

            // Unknown command
            else {
                response = "Unknown command\n";
            }
        }

        send(
            clientSocket,
            response.c_str(),
            response.size(),
            0
        );
    }

    close(clientSocket);
}

// Main server function
int main() {

    // Restore previously saved filesystem
    if (Persistence::load(fs, DATA_FILE))
        cout << "Previous filesystem data restored\n";
    else
        cout << "Starting with an empty filesystem\n";

    // Create Unix domain socket
    int serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (serverSocket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_un address{};

    address.sun_family = AF_UNIX;

    strncpy(
        address.sun_path,
        SOCKET_PATH,
        sizeof(address.sun_path) - 1
    );

    unlink(SOCKET_PATH);

    // Bind socket
    if (bind(
        serverSocket,
        (sockaddr*)&address,
        sizeof(address)
    ) < 0) {

        perror("Bind failed");
        close(serverSocket);
        return 1;
    }

    // Listen for clients
    if (listen(serverSocket, 5) < 0) {

        perror("Listen failed");
        close(serverSocket);
        unlink(SOCKET_PATH);

        return 1;
    }

    cout << "Mini File System Server Started\n";

    // Accept multiple clients
    while (true) {

        int clientSocket =
            accept(serverSocket, nullptr, nullptr);

        if (clientSocket >= 0) {

            thread(
                handleClient,
                clientSocket
            ).detach();

        }
    }

    close(serverSocket);
    unlink(SOCKET_PATH);

    return 0;
}
