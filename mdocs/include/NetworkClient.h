#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <string>

class NetworkClient {
public:
    NetworkClient(const std::vector<std::string>& serverAddresses, int port);
    ~NetworkClient();
    void Start();
    void Stop();

    void AddServer(const std::string& address, int port);
    void OnConnect(int event, HWND hwnd, int eventID, DWORD dwData);
    void OnReceive(int event, HWND hwnd, int eventID, DWORD dwData);
    void OnClose(int event, HWND hwnd, int eventID, DWORD dwData);

private:
    std::vector<SOCKET> clientSockets;
    std::vector<char> buffer;
};