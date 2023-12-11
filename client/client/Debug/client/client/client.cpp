#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")

void ReceiveMessages(SOCKET sock) {
    char buf[4096];
    while (true) {
        ZeroMemory(buf, sizeof(buf));
        int bytesReceived = recv(sock, buf, sizeof(buf), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cerr << "Error receiving data! Connection closed." << std::endl;
            break;
        }

        std::cout << "Received message from server: " << buf << std::endl;
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
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Unable to create socket! Error code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // Set up the server address and port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Connect to the server
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to the server! Error code: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Create a thread to receive messages
    std::thread receiveThread(ReceiveMessages, sock);

    // Send messages from the client
    std::string userInput;
    do {
        std::cout << "> ";
        std::getline(std::cin, userInput);

        if (userInput.size() > 0) {
            // Send data to the server
            send(sock, userInput.c_str(), userInput.size() + 1, 0);
        }

    } while (userInput.size() > 0);

    // Close the client socket
    closesocket(sock);

    // Wait for the receive thread to finish
    receiveThread.join();

    // Clean up Winsock
    WSACleanup();

    return 0;
}