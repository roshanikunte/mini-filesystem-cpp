# Persistent In-Memory File System with a Custom Shell

## 1. Project Overview

The Persistent In-Memory File System is a C++ application that implements a simplified filesystem with a custom command-line shell.

It follows a client-server architecture in which clients communicate with a central filesystem server using Unix Domain Sockets.

The filesystem is maintained in memory and automatically saved to disk whenever modifications are made. When the server restarts, previously saved filesystem data is restored.

## 2. Objectives

- Implement a custom in-memory filesystem.
- Support basic file and directory operations.
- Develop a custom command-line shell.
- Establish communication between client and server using Unix Domain Sockets.
- Implement persistent storage and automatic recovery.
- Support multiple concurrent clients.
- Synchronize access to shared filesystem data.

## 3. Features

### Filesystem Operations

| Command | Description |
|---|---|
| `pwd` | Display current working directory |
| `ls` | List files and directories |
| `ls -l` | Display detailed directory listing |
| `cd` | Change current directory |
| `mkdir` | Create a directory |
| `touch` | Create an empty file |
| `rm` | Remove a file or empty directory |
| `rm -rf` | Recursively remove a directory and its contents |

### Additional Features

- Client-server communication using Unix Domain Sockets.
- Multiple client connections using threads.
- Mutex-based synchronization for shared filesystem access.
- Independent current working directory for each client.
- Automatic filesystem persistence.
- Filesystem restoration after server restart.
- Makefile-based compilation.

## 4. Technologies Used

- Programming Language: C++
- Standard: C++17
- Operating System: Linux (Ubuntu)
- Compiler: G++
- Build System: GNU Make
- Communication: Unix Domain Sockets
- System Programming: POSIX system calls
- Concurrency: POSIX Threads and Mutexes

## 5. System Architecture

The project follows a client-server architecture.

### Client

The client provides the custom command-line interface. It accepts commands from users, sends requests to the server, and displays the responses.

### Server

The server manages the filesystem and processes incoming client requests. It handles directory navigation, creation, deletion, and listing operations.

### Filesystem Module

The filesystem module maintains the directory tree and file metadata using C++ data structures.

### Persistence Module

The persistence module saves the filesystem state to disk and restores it when the server starts.

### Communication Flow

User → Custom Shell → Client → Unix Domain Socket → Server → Filesystem → Persistence Storage

## 6. Project Structure

```text
mini-filesystem/
│
├── src/
│   ├── client.cpp
│   ├── server.cpp
│   ├── filesystem.cpp
│   └── persistence.cpp
│
├── include/
│   ├── filesystem.hpp
│   └── persistence.hpp
│
├── data/
│   └── filesystem.dat
│
├── docs/
│
├── Makefile
└── README.md
```

## 7. Installation and Execution

### Prerequisites

Install the required development tools on Ubuntu:

```bash
sudo apt update
sudo apt install g++ make
```

### Step 1: Build the Project

Navigate to the project directory:

```bash
cd ~/projects/mini-filesystem
```

Compile the project:

```bash
make
```

To perform a clean rebuild:

```bash
make clean
make
```

### Step 2: Start the Server

Open the first terminal and execute:

```bash
./fsserver
```

The server starts and restores previously saved filesystem data if available.

### Step 3: Start the Client

Open another terminal and execute:

```bash
cd ~/projects/mini-filesystem
./fsclient
```

The custom shell will display:

```text
Connected to Mini File System Server
Type 'exit' to quit.
mini-fs>
```

### Step 4: Execute Commands

Example:

```text
mini-fs> mkdir projects
mini-fs> cd projects
mini-fs> touch notes.txt
mini-fs> ls -l
mini-fs> pwd
```

## 8. Data Persistence

The filesystem state is stored in:

```text
data/filesystem.dat
```

Whenever a successful filesystem modification occurs, the updated state is saved to persistent storage.

When the server starts again, it attempts to restore the previous filesystem state.

This allows filesystem data to survive server restarts.

## 9. Testing

The following operations have been tested:

| Test | Result |
|---|---|
| Creating directories | Passed |
| Creating empty files | Passed |
| Listing directory contents | Passed |
| Detailed directory listing | Passed |
| Changing directories | Passed |
| Displaying current path | Passed |
| Removing files | Passed |
| Removing empty directories | Passed |
| Recursive directory deletion | Passed |
| Automatic persistence | Passed |
| Server restart recovery | Passed |
| Clean compilation using Makefile | Passed |
| Multiple client sessions | Basic test passed |

## 10. Future Improvements

- Support file content reading and writing.
- Improve error handling and protocol reliability.
- Add file permissions and ownership.
- Improve concurrent access and session management.
- Add more advanced filesystem operations.

## 11. Conclusion

This project demonstrates the implementation of a simplified persistent in-memory filesystem using C++ and Linux system programming concepts.

It combines data structures, client-server communication, concurrency, synchronization, and persistent storage into a single application.

The project provides practical exposure to operating system concepts and low-level software development.
