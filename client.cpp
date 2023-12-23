#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include "packets.h"

using namespace std;

void send_data(packet& packet, int client_socket, const sockaddr_in& server_address) {
    int maxfd = client_socket + 1;
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(client_socket, &readSet);

    struct timeval timeout{};
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    sendto(client_socket, &packet, sizeof(packet), 0,
           (struct sockaddr*)&server_address, sizeof(server_address));

    if (strlen(packet.data) == 0)
        return;

    int result = select(maxfd, &readSet, nullptr, nullptr, &timeout);

    if (result == -1) {
        perror("Error in select");
        exit(EXIT_FAILURE);
    } else if (result == 0) {
        cout << "Timeout: No ACK received. Resending data." << endl;
        send_data(packet, client_socket, server_address);
    } else {
        ack_packet ack_packet{};
        socklen_t server_address_len = sizeof(server_address);

        ssize_t recv_size = recvfrom(client_socket, &ack_packet, sizeof(ack_packet), 0,
                                     (struct sockaddr*)&server_address, &server_address_len);

        if (recv_size == -1) {
            perror("Error receiving ACK");
            exit(EXIT_FAILURE);
        } else {
            if (ack_packet.ackno > 750000)
                sleep(1);
            cout << "ACK number: " << ack_packet.ackno << endl;
        }
    }
}

void client(const char* server_ip, int server_port, const char* filename) {
    // done till end //
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

    uint32_t seqno = 0;
    char buffer[MAX_DATA_SIZE];

    while (!file.eof()) {
        packet packet{};
        file.read(buffer, MAX_DATA_SIZE);
        auto read_size = static_cast<uint16_t>(file.gcount());
        construct_packet(packet, read_size, seqno, buffer);
        send_data(packet, client_socket, server_address);
        seqno += read_size;
    }

    packet packet{};
    packet.seqno = seqno;
    send_data(packet, client_socket, server_address);

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
