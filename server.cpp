#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include "cmake-build-debug/packets.h"

const int MAX_DATA_SIZE = 500;
const int ACK_SIZE = 8;
const int TIMEOUT_SECONDS = 1;

using namespace std;

bool evaluate_check_sum(packet& packet) {
    uint32_t sum = 0;

    sum += packet.len;
    sum += packet.cksum;
    sum += static_cast<uint16_t>((packet.seqno >> 16) & 0xFFFF);
    sum += static_cast<uint16_t>(packet.seqno & 0xFFFF);
    for (int i = 0; i < packet.len; i += 2)
        sum += (packet.data[i] << 8) + packet.data[i + 1];
    sum = (sum + (sum >> 16)) & 0xFFFF;
    return ~static_cast<uint16_t>(sum & 0xFFFF) == 0;
}

void process_data(const char* data, int size) {
    cout.write(data, size);
}

void handle_client(int client_socket, sockaddr_in& client_address, double plp) {
    while (true) {
        char buffer[MAX_DATA_SIZE + ACK_SIZE];
        socklen_t client_address_len = sizeof(client_address);

        ssize_t recv_size = recvfrom(client_socket, buffer, sizeof(buffer), 0,
                                     (struct sockaddr*)&client_address, &client_address_len);
        cout << "from child process" << endl;

        if (recv_size > 0 && static_cast<double>(rand()) / RAND_MAX > plp) {
            process_data(buffer + ACK_SIZE, recv_size - ACK_SIZE);
            // Send ACK
            sendto(client_socket, buffer, ACK_SIZE, 0,
                   (struct sockaddr*)&client_address, client_address_len);
        } else {
            cout << "Packet loss, ignoring the received datagram." << endl;
        }
    }
}

void server(int port, int seed, double plp) {
    srand(seed);

    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    while (true) {
        char buffer[MAX_DATA_SIZE + ACK_SIZE];
        sockaddr_in client_address{};
        socklen_t client_address_len = sizeof(client_address);

        ssize_t recv_size = recvfrom(server_socket, buffer, sizeof(buffer), 0,
                                     (struct sockaddr*)&client_address, &client_address_len);
        cout << "from main process" << endl;

        if (recv_size > 0 && static_cast<double>(rand()) / RAND_MAX > plp) {
            process_data(buffer + ACK_SIZE, recv_size - ACK_SIZE);
            // Send ACK
            sendto(server_socket, buffer, ACK_SIZE, 0,
                   (struct sockaddr*)&client_address, client_address_len);
        } else {
            std::cout << "Packet loss, ignoring the received datagram." << std::endl;
        }

        pid_t child_pid = fork();

        if (child_pid == 0) { // Child process
            close(server_socket); // Close the server socket in the child process
            handle_client(server_socket, client_address, plp);
            exit(EXIT_SUCCESS);
        } else if (child_pid > 0) { // Parent process
            // Continue listening for new clients
        } else {
            perror("Error forking process");
        }
    }
}

int main() {
    ifstream infile("server.in");
    int port, seed;
    double plp;

    infile >> port >> seed >> plp;
    infile.close();

    server(port, seed, plp);

    return 0;
}