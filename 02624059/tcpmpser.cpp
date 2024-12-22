#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MAX_EVENTS FD_SETSIZE
#define PORT 8080

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return -1;
    }

    int server_fd, new_socket, max_sd, activity, event_count;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    fd_set readfds;
    SOCKET client_sockets[MAX_EVENTS];

    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        WSACleanup();
        return -1;
    }

    // Set socket options to allow reuse of address and port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to localhost:PORT
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    // Initialize client sockets array
    for (int i = 0; i < MAX_EVENTS; ++i) {
        client_sockets[i] = 0;
    }

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add client sockets to set
        for (int i = 0; i < MAX_EVENTS; ++i) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);
                if (client_sockets[i] > max_sd) {
                    max_sd = client_sockets[i];
                }
            }
        }

        // Wait for activity on sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity == SOCKET_ERROR) {
            perror("select");
            WSACleanup();
            return -1;
        }

        // Check for activity on server socket (new connection)
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                closesocket(server_fd);
                WSACleanup();
                return -1;
            }

            std::cout << "New client connected: " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;

            // Add new client socket to array
            for (int i = 0; i < MAX_EVENTS; ++i) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // Check for activity on client sockets (received data)
        for (int i = 0; i < MAX_EVENTS; ++i) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                int valread = recv(sd, buffer, 1024, 0);
                if (valread <= 0) {
                    // Client disconnected or error
                    if (valread == 0) {
                        std::cout << "Client disconnected" << std::endl;
                    } else {
                        perror("recv");
                    }
                    closesocket(sd);
                    client_sockets[i] = 0;
                } else {
                    // Process data and send response
                    // Example: echo back received data
                    send(sd, buffer, valread, 0);
                }
            }
        }
    }

    // Cleanup
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
