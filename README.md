# IPC-communicator

This application functions as a message broadcasting system to all users subscribed to a particular message type. It is developed in C language and utilizes interprocess communication mechanisms. The application is designed for Unix systems.

## Compilation Instructions

#### For the Server:
gcc server.c -o server

#### For the Client:
gcc client.c -o client

## How to Run?

#### For the Server:
./server

#### For the Client:
./client

## Brief Description of *.c Files

- `server.c`: This file contains the server-side part of the project. It stores all data entered by clients and includes appropriate functions required for executing commands issued by clients, such as logging in, creating communication topics with others, and handling messages.

- `client.c`: This file represents the client. It enables sending commands to the server.





