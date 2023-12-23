#ifndef UNTITLED_PACKETS_H
#define UNTITLED_PACKETS_H

#include <cstdint>
#include <cstring>

const int MAX_DATA_SIZE = 500;
const int ACK_SIZE = 8;
const int TIMEOUT_SECONDS = 2;

struct packet {
    uint16_t len;
    uint16_t cksum;
    uint32_t seqno;
    char data[500];

    packet() : len(0), cksum(0), seqno(0), data{} {}
};

struct ack_packet {
    uint16_t len;
    uint16_t cksum;
    uint32_t ackno;

    ack_packet() : len(0), cksum(0), ackno(0) {}
};

bool check_sum(uint16_t* packet, uint16_t size);

void construct_packet(packet& packet, uint16_t len, uint32_t seqno, const char* data);

void construct_ack(packet& packet, ack_packet& ack);

#endif //UNTITLED_PACKETS_H
