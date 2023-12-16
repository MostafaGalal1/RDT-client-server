#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include "cmake-build-debug/packets.h"

const int MAX_DATA_SIZE = 500;
const int ACK_SIZE = 8;
const int TIMEOUT_SECONDS = 1;

using namespace std;

void calculate_check_sum(packet& packet) {
    uint32_t sum = 0;

    sum += packet.len;
    sum += static_cast<uint16_t>((packet.seqno >> 16) & 0xFFFF);
    sum += static_cast<uint16_t>(packet.seqno & 0xFFFF);
    for (int i = 0; i < packet.len; i += 2)
        sum += (packet.data[i] << 8) + packet.data[i + 1];

    sum = (sum + (sum >> 16)) & 0xFFFF;
    packet.cksum = ~sum;
}

void construct_packet(packet& packet, const char* data, int size) {
    packet.len = size;
    calculate_check_sum(packet);
    //TODO: set seqno number
    memcpy(packet.data, data, size);
}

void send_data(const char* data, int size, int client_socket, const sockaddr_in& server_address) {
    packet packet{};
    construct_packet(packet, data, size);

    char buffer[MAX_DATA_SIZE + ACK_SIZE];
    memcpy(buffer + ACK_SIZE, data, size);

    // Send data
    sendto(client_socket, packet, sizeof(packet), 0,
           (struct sockaddr*)&server_address, sizeof(server_address));

    // Wait for ACK
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(client_socket, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    int activity = select(client_socket + 1, &read_fds, nullptr, nullptr, &timeout);

    if (activity == -1) {
        perror("Error in select");
        exit(EXIT_FAILURE);
    } else if (activity == 0) {
        cout << "Timeout, resending the datagram." << endl;
        send_data(data, size, client_socket, server_address);
    } else {
        // ACK received
        recvfrom(client_socket, buffer, sizeof(buffer), 0, nullptr, nullptr);
    }
}

void client(const char* server_ip, int server_port, const char* filename) {
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &(server_address.sin_addr));

    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_DATA_SIZE];
    while (!file.eof()) {
        file.read(buffer, MAX_DATA_SIZE);
        int read_size = static_cast<int>(file.gcount());
        send_data(buffer, read_size, client_socket, server_address);
    }

    // Signal end of file
    send_data("", 0, client_socket, server_address);

    file.close();
    close(client_socket);
}

int main() {
    ifstream infile("client.in");
    string server_ip, filename;
    int server_port;

    infile >> server_ip >> server_port >> filename;
    infile.close();

    client(server_ip.c_str(), server_port, filename.c_str());

    return 0;
}
