#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* args[]) {
    if (argc != 3) {
        std::cerr << "not enough arguments, expected {address} {port}" << std::endl;
        return 1;
    }

    int server_port{std::stoi(args[2])};
    auto server_address{args[1]};

    int client_socket = -1;
    sockaddr_in address { };
    address.sin_family = AF_INET;
    address.sin_port = htons(server_port);

    if(inet_pton(AF_INET, server_address, &address.sin_addr) <= 0) {
        std::cerr << "inet_pton() failed with error: "
            << std::strerror(errno) << std::endl;
        return -1;
    }

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "socket(...) failed with error: "
            << std::strerror(errno) << std::endl;
        return -1;
    }

    if (connect(client_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "connect(...) failed with error: "
            << std::strerror(errno) << std::endl;
        return -1;
    }
    
    close(client_socket);

    return 0;
}
