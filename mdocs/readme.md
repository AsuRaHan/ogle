

Я создам структурированный и модульный класс для работы с сетью на C++ с использованием WinAPI. Этот класс можно будет轻松 подключать к разным проектам.

```cpp
// include/network.h
#pragma once
#include windows.h>
#include winsock2.h>
#include ws2tcpip.h>
#include vector>
#include string>

class NetworkServer {
private:
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr;
    std::vector<SOCKET> clientSockets;
    std::vector<char> buffer;

public:
    NetworkServer(const char* address, int port);
    ~NetworkServer();
    void Start();
    void Stop();

    // Event handlers that can be overridden by derived classes
    virtual void OnAccept(int event, HWND hwnd, int eventID, DWORD dwData) = 0;
    virtual void OnReceive(int event, HWND hwnd, int eventID, DWORD dwData) = 0;
    virtual void OnClose(int event, HWND hwnd, int eventID, DWORD dwData) = 0;
};

class NetworkClient {
private:
    WSADATA wsaData;
    std::vector<SOCKET> serverSockets;
    std::vector<char> buffer;

public:
    NetworkClient(const std::vector<std::string>& serverAddresses, int port);
    ~NetworkClient();
    void AddServer(const std::string& address, int port);
    void Start();
    void Stop();

    // Event handlers that can be overridden by derived classes
    virtual void OnConnect(int event, HWND hwnd, int eventID, DWORD dwData) = 0;
    virtual void OnReceive(int event, HWND hwnd, int eventID, DWORD dwData) = 0;
    virtual void OnClose(int event, HWND hwnd, int eventID, DWORD dwData) = 0;
};
```

```cpp
// src/network.cpp
#include "network.h"
#include iostream>

NetworkServer::NetworkServer(const char* address, int port) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout < "Error: WSAStartup failed." < std::endl;
        return;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cout < "Error: socket creation failed." < std::endl;
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = inet_addr(address);
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout < "Error: bind failed." < std::endl;
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout < "Error: listen failed." < std::endl;
        return;
    }

    if (WSAAsyncSelect(serverSocket, NULL, FD_ACCEPT, NULL) == SOCKET_ERROR) {
        std::cout < "Error: WSAAsyncSelect failed." < std::endl;
        return;
    }
}

NetworkServer::~NetworkServer() {
    Stop();
}

void NetworkServer::Start() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void NetworkServer::Stop() {
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
    }
    WSACleanup();
}

void NetworkServer::OnAccept(int event, HWND hwnd, int eventID, DWORD dwData) {
    if (event == FD_ACCEPT) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) {
            clientSockets.push_back(clientSocket);
            if (WSAAsyncSelect(clientSocket, NULL, FD_READ | FD_CLOSE, NULL) == SOCKET_ERROR) {
                closesocket(clientSocket);
            }
        }
    }
}

void NetworkServer::OnReceive(int event, HWND hwnd, int eventID, DWORD dwData) {
    // Override this in derived class
}

void NetworkServer::OnClose(int event, HWND hwnd, int eventID, DWORD dwData) {
    // Override this in derived class
}

NetworkClient::NetworkClient(const std::vector<std::string>& serverAddresses, int port) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout < "Error: WSAStartup failed." < std::endl;
        return;
    }

    for (const std::string& address : serverAddresses) {
        AddServer(address, port);
    }
}

NetworkClient::~NetworkClient() {
    for (SOCKET sock : serverSockets) {
        closesocket(sock);
    }
    WSACleanup();
}

void NetworkClient::AddServer(const std::string& address, int port) {
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cout < "Error: socket creation failed." < std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = inet_addr(address.c_str());
    serverAddr.sin_port = htons(port);

    if (WSAAsyncSelect(clientSocket, NULL, FD_CONNECT | FD_READ | FD_CLOSE, NULL) == SOCKET_ERROR) {
        std::cout < "Error: WSAAsyncSelect failed." < std::endl;
        return;
    }

    serverSockets.push_back(clientSocket);
}

void NetworkClient::Start() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void NetworkClient::Stop() {
    for (SOCKET sock : serverSockets) {
        closesocket(sock);
    }
}

void NetworkClient::OnConnect(int event, HWND hwnd, int eventID, DWORD dwData) {
    // Override this in derived class
}

void NetworkClient::OnReceive(int event, HWND hwnd, int eventID, DWORD dwData) {
    // Override this in derived class
}

void NetworkClient::OnClose(int event, HWND hwnd, int eventID, DWORD dwData) {
    // Override this in derived class
}
```

Этот код предоставляет базовую библиотеку, которая можно использовать в разных проектах. Для использования:

1. Создайте проект в VS Code с C++ поддержкой
2. Создайте папки include и src
3. Добавьте файлы в эти папки
4. Используйте cmake для сборки:

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(NetworkLibrary)

add_library(network_lib src/network.cpp)
```

Пример использования:

```cpp
#include "network.h"

class MyNetworkServer : public NetworkServer {
private:
    void OnReceive(int event, HWND hwnd, int eventID, DWORD dwData) override {
        // Handle received data
    }
};

int main() {
    MyNetworkServer server("127.0.0.1", 1234);
    server.Start();
    return 0;
}
```

Эта библиотека предоставляет асинхронную модель обработки и может быть расширена для добавления дополнительных функций в соответствии с вашими потребностями.



Я создам необходимые файлы для вашего проекта. Вот файлы, которые будут созданы:

1. CMakeLists.txt - Для компиляции проекта
2. include/network.h - Header-файл с классами NetworkServer и NetworkClient
3. src/network.cpp - Реализация классов
4. examples/server.cpp - Пример использования сервера
5. examples/client.cpp - Пример использования клиента

Файлы можно найти в вашем проекте в папках:
- CMakeLists.txt в корне проекта
- include/network.h
- src/network.cpp
- examples/server.cpp
- examples/client.cpp

Для компиляции проекта используйте cmake и bat файлы.