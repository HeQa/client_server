#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock." << std::endl;
        return 1;
    }

    SOCKET serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket." << std::endl;
        closesocket(serverSocket);
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on socket." << std::endl;
        closesocket(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port 8000..." << std::endl;

    SOCKET clientSocket;
    SOCKADDR_IN clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize)) == INVALID_SOCKET) {
        std::cerr << "Failed to accept client connection." << std::endl;
        closesocket(serverSocket);
        return 1;
    }

    char buffer[4096];
    std::string message;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            break;
        }

        message = buffer;
        std::cout << "Received message from client: " << message << std::endl;

        std::cout << "Enter your response: ";
        std::getline(std::cin, message);
        strcpy(buffer, message.c_str());
        send(clientSocket, buffer, strlen(buffer), 0);
    }

    std::cout << "Client disconnected." << std::endl;
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}