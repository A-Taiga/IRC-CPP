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

    std::string test_buffer(500, '0');
    auto send_result{send(client_socket, test_buffer.data(), test_buffer.size(), 0)};
    if (send_result < 0) {
        std::cerr << "send() failed because the server closed the connection" << std::endl;
        return -1;
    } else if (send_result < -1) {
        std::cerr << "send() failed with error: "
            << std::strerror(errno) << std::endl;
        return -1;
    }
    
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);

    return 0;
}
