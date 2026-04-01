#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <string>

class NetworkServer {
public:
    NetworkServer(const char* address, int port);
    ~NetworkServer();
    void Start();
    void Stop();

private:
    void OnAccept(int event, HWND hwnd, int eventID, DWORD dwData);
    void OnReceive(int event, HWND hwnd, int eventID, DWORD dwData);
    void OnClose(int event, HWND hwnd, int eventID, DWORD dwData);
    // Дополнительные поля и методы...
};