#include "NetworkServer.h"
#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

// Конструктор
NetworkServer::NetworkServer(const char* address, int port) {
    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout < "Error: WSAStartup failed." < std::endl;
        return;
    }

    // Создание сокета
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cout < "Error: socket creation failed." < std::endl;
        return;
    }

    // Настройка адреса и порта
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = inet_addr(address);
    serverAddr.sin_port = htons(port);

    // Привязка сокета к адресу и порту
    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout < "Error: bind failed." < std::endl;
        return;
    }

    // Начало прослушивания
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout < "Error: listen failed." < std::endl;
        return;
    }

    // Регистрация обработчиков событий
    if (WSAAsyncSelect(serverSocket, NULL, FD_ACCEPT, NULL) == SOCKET_ERROR) {
        std::cout < "Error: WSAAsyncSelect failed." < std::endl;
        return;
    }
}

// Деструктор
NetworkServer::~NetworkServer() {
    // Остановка сервера
    Stop();
}

// Метод запуска сервера
void NetworkServer::Start() {
    // Здесь можно добавить код для запуска сервера
}

// Метод остановки сервера
void NetworkServer::Stop() {
    // Здесь можно добавить код для остановки сервера
}

// Обработчик события принятия подключения
void NetworkServer::OnAccept(int event, HWND hwnd, int eventID, DWORD dwData) {
    // Здесь можно добавить код для обработки принятия подключения
}

// Обработчик события получения данных
void NetworkServer::OnReceive(int event, HWND hwnd, int eventID, DWORD dwData) {
    // Здесь можно добавить код для обработки получения данных
}

// Обработчик события закрытия соединения
void NetworkServer::OnClose(int event, HWND hwnd, int eventID, DWORD dwData) {
    // Здесь можно добавить код для обработки закрытия соединения
}