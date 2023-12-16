#ifndef UNTITLED_PACKETS_H
#define UNTITLED_PACKETS_H

#include <cstdint>

struct packet {
    /* header */
    uint16_t len;
    uint16_t cksum;
    uint32_t seqno;
    /* data */
    char data[500];
};

struct ack_packet {
    uint16_t cksum;
    uint16_t len;
    uint32_t ackno;
};

#endif //UNTITLED_PACKETS_H
