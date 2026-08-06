#pragma once

#ifndef TCPACK_ATTACK_H
#define TCPACK_ATTACK_H

#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include "attack_params.h"

void* tcpack_attack(void* arg);

#endif // TCPACK_ATTACK_H
