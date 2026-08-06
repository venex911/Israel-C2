#pragma once

#ifndef TCPRST_ATTACK_H
#define TCPRST_ATTACK_H

#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include "attack_params.h"

void* tcprst_attack(void* arg);

#endif // TCPRST_ATTACK_H
