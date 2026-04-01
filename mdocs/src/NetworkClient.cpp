#include "NetworkClient.h"
#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

// Конструктор
NetworkClient::NetworkClient(const std::vector<std::string>& serverAddresses, int port) {
    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout < "Error: WSAStartup failed." < std::endl;
        return;
    }

    // Подключение к серверам
    for (const std::string& address : serverAddresses) {
        AddServer(address, port);
    }
}

// Деструктор
NetworkClient::~NetworkClient() {
    // Закрытие всех соединений
    for (SOCKET sock : clientSockets) {
        closesocket(sock);
    }
    // Очистка ресурсов Winsock
    WSACleanup();
}

// Метод запуска клиента
void NetworkClient::Start() {
    // Запуск обработчика сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Метод остановки клиента
void NetworkClient::Stop() {
    // Остановка клиента
    closesocket(serverSocket);
}

// Метод добавления сервера
void NetworkClient::AddServer(const std::string& address, int port) {
    // Создание сокета
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cout < "Error: socket creation failed." < std::endl;
        return;
    }

    // Настройка адреса сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = inet_addr(address.c_str());
    serverAddr.sin_port = htons(port);

    // Подключение к серверу
    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout < "Error: connect failed." < std::endl;
        closesocket(clientSocket);
        return;
    }

    // Регистрация обработчика событий для нового сокета
    if (WSAAsyncSelect(clientSocket, NULL, FD_READ | FD_CLOSE, NULL) == SOCKET_ERROR) {
        std::cout < "Error: WSAAsyncSelect failed." < std::endl;
        closesocket(clientSocket);
        return;
    }

    // Сохранение сокета
    clientSockets.push_back(clientSocket);
}