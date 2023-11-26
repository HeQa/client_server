#include <iostream>
#include <WinSock2.h>
#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock" << std::endl;
        return -1;
    }

    // Создание серверного сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return -1;
    }

    // Установка параметров адреса сервера
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Привязка сокета к адресу сервера
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    // Ожидание подключения клиента
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed to listen for incoming connections" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Server started. Waiting for clients..." << std::endl;

    // Принятие подключения от клиента
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to accept client connection" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Client connected" << std::endl;

    // Отправка данных клиенту
    std::string message = "Hello from server";
    if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Failed to send data to client" << std::endl;
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Data sent to client" << std::endl;

    // Закрытие сокетов и освобождение ресурсов
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}