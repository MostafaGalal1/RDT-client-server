#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include "packets.h"

using namespace std;

void handle_client(int client_socket, sockaddr_in& client_address, double plp) {
    while (true) {
        // done till end //
        packet packet{};
        socklen_t client_address_len = sizeof(client_address);

        ssize_t recv_size = recvfrom(client_socket, &packet, sizeof(packet), 0, (struct sockaddr*)&client_address, &client_address_len);

        cout << "from handle client" << endl;
        if (recv_size == -1) {
            perror("Erjjjjror receiving data");
            exit(EXIT_FAILURE);
        } else if (strlen(packet.data) == 0) {
            cout << "Client disconnected" << endl;
            break;
        }
        // end //

        cout << "Packet number: " << packet.seqno << endl;
        cout << check_sum(reinterpret_cast<uint16_t*>(&packet), ACK_SIZE + packet.len) << endl;
        if (static_cast<double>(rand()) / RAND_MAX > plp) {
            // Send ACK
            ack_packet ack_packet{};
            construct_ack(packet, ack_packet);
            sendto(client_socket, &ack_packet, ACK_SIZE, 0,
                   (struct sockaddr*)&client_address, client_address_len);
        } else {
            cout << "Packet loss due to PLP, ignoring sending ACK." << endl;
        }
    }
    close(client_socket);
}

void server(int port, int seed, double plp) {
    // done till end //
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
    // end //

    while (true) {
        sockaddr_in client_address{};
        packet packet{};
        socklen_t client_address_len = sizeof(client_address);

        ssize_t recv_size = recvfrom(server_socket, &packet, sizeof(packet), 0,
                                     (struct sockaddr*)&client_address, &client_address_len);

        cout << "Packet number: " << packet.seqno << endl;
        cout << "from main process" << endl;
        if (recv_size == -1) {
            perror("Error receiving data");
            exit(EXIT_FAILURE);
        } else if (strlen(packet.data) == 0) {
            cout << "Client disconnected" << endl;
        }

        if (static_cast<double>(rand()) / RAND_MAX > plp) {
            // Send ACK
            ack_packet ack_packet{};
            construct_ack(packet, ack_packet);
            cout << "Sending ACK number: " << ack_packet.ackno << endl;
            sendto(server_socket, &ack_packet, ACK_SIZE, 0,
                   (struct sockaddr*)&client_address, client_address_len);
            if (ack_packet.ackno > 750000)
                sleep(1);
        } else {
            cout << "Packet loss due to PLP, ignoring sending ACK." << endl;
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