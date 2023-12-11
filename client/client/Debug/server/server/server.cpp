#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <thread>
#include <mutex>

#include <string>

#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clients;
std::mutex clients_mutex;
std::mutex cout_mutex;

void HandleClient(SOCKET clientSocket) {
    char buf[4096];
    while (true) {
        // Receive data
        ZeroMemory(buf, sizeof(buf));
        int bytesReceived = recv(clientSocket, buf, sizeof(buf), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error receiving data! Error code: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            std::cout << "Client disconnected: " << clientSocket << std::endl;
            break;
        }

        // Send data to other clients (here, just output to the console)
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Received message from client(" << clientSocket << "): " << buf << std::endl;
        }
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (SOCKET& otherClient : clients) {
            if (otherClient != clientSocket) {
                send(otherClient, ("A Message from " + std::to_string(clientSocket) + ": " + buf).c_str(), sizeof(buf), 0);
            }
        }
    }

    // Remove the closed client from the list
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = std::find(clients.begin(), clients.end(), clientSocket);
        if (it != clients.end()) {
            closesocket(clientSocket);
            clients.erase(it);
        }
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOK = WSAStartup(ver, &wsData);
    if (wsOK != 0) {
        std::cerr << "Unable to initialize Winsock! Error code: " << wsOK << std::endl;
        return -1;
    }

    // Create a socket
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        std::cerr << "Unable to create socket! Error code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // Bind the socket to an address and port
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;
    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Listen for incoming connections
    listen(listening, SOMAXCONN);

    std::cout << "Waiting for connections..." << std::endl;

    // Accept clients in a loop
    while (true) {
        // Accept a client connection
        SOCKET clientSocket = accept(listening, NULL, NULL);
        std::cout << "Client connected!" << " : " << clientSocket << std::endl;

        // Add the client to the list
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(clientSocket);
        }

        // Create a thread to handle the client
        std::thread clientThread(HandleClient, clientSocket);
        clientThread.detach(); // Detach the thread from the main server thread
    }

    // Close the listening socket
    closesocket(listening);

    // Clean up Winsock
    WSACleanup();

    return 0;
}