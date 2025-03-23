#pragma once

#include <algorithm>    // c/cpp headers
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <chrono>

#ifndef __linux__
#error wrong platform
#endif

#include <arpa/inet.h>
#include <netdb.h>  // unix headers
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// finds filename at compile time
constexpr const char* file_name(const char* path) {
    const char* file = path;
    while (*path) {
        if (*path++ == '/') {
            file = path;
        }
    }
    return file;
}

#define errmsg(type) fprintf(stderr, "%s:%s():%d: %s\n", file_name(__FILE__), __func__, __LINE__, type)

namespace ndiag {

bool trace_route(const char* target, const char* device, uint16_t opt_payload, uint8_t hops);

bool resolve_host(const char* target, sockaddr_in& dest_sockaddrin,/* int& ipver,*/ char* resolved_ip);
uint16_t calculate_internet_checksum(void *buf, size_t len);
void generate_cyclic_alphabet_msg(char* area, uint16_t payloadsize);
bool setupsighandlers(void);
bool setupsocket(int& fd, const char* device, timeval& timeout);

constexpr int PACKET_OUTPUT_SIZE { 256 };
constexpr int PACKET_INPUT_SIZE { 1024 };
constexpr int ICMP_OUTPUT_MAX_PAYLOAD_SIZE { PACKET_OUTPUT_SIZE - (sizeof(iphdr) + sizeof(icmphdr)) }; // 256 - 28 = 228
constexpr int HOPSDEFAULT { 64 };
};

constexpr int IFNAMSIZ { 16 };    // see https://elixir.bootlin.com/linux/v5.6/source/include/uapi/linux/if.h#L33