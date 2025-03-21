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

#define errmsg(type) fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, type)

namespace ndiag {

bool trace_route(const char* target, const char* device, uint8_t hops);

bool resolve_host(const char* target, sockaddr_in& dest_sockaddrin,/* int& ipver,*/ char* resolved_ip);
uint16_t calculate_checksum(void *buf, size_t len);
bool setupsighandlers(void);
bool setupsocket(int& fdoutput, const char* device, timeval& timeout);

constexpr auto MAX_PACKET_SIZE { 256u };  // should be twice more than enough; may be defined somewhere else so placed it here
};

constexpr auto IFNAMSIZ { 16 };    // see https://elixir.bootlin.com/linux/v5.6/source/include/uapi/linux/if.h#L33
constexpr auto HOPSDEFAULT { 64 };