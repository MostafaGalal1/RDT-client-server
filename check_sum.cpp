#include "packets.h"

using namespace std;

bool check_sum(uint16_t* packet, uint16_t size) {
    uint32_t sum = 0;
    for (int i = 0; i < (size + 1) / 2; i++)
        sum += packet[i];
    sum = (sum + (sum >> 16)) & 0xFFFF;
    packet[1] = static_cast<uint16_t>(~sum);
    return packet[1] == 0;
}