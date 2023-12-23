#include "packets.h"

void construct_packet(packet& packet, uint16_t len, uint32_t seqno, const char* data) {
    packet.len = len;
    packet.seqno = seqno;
    memcpy(packet.data, data, len);
    check_sum(reinterpret_cast<uint16_t*>(&packet), ACK_SIZE + len);
}

void construct_ack(packet& packet, ack_packet& ack) {
    ack.ackno = packet.seqno + packet.len;
    check_sum(reinterpret_cast<uint16_t*>(&ack), ACK_SIZE);
}