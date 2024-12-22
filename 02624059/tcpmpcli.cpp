#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8080

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " ip_server_address server_port_number" << std::endl;
        WSACleanup();
        return 1;
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);

    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Create TCP socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket creation error" << std::endl;
        WSACleanup();
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    std::cout << "Connected to server" << std::endl;

    // Send and receive data
    while (true) {
        std::cout << "Enter arithmetic operation (e.g., '+ 3 4'): ";
        std::cin.getline(buffer, sizeof(buffer));

        // Send operation to server
        send(sock, buffer, strlen(buffer), 0);

        // Receive response from server
        int valread = recv(sock, buffer, 1024, 0);
        if (valread > 0)
            std::cout << "Server response: " << buffer << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
