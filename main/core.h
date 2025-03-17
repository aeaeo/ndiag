#pragma once

#include <cstdio>	// c/cpp headers
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <chrono>

#ifndef __linux__
#error wrong platform
#endif

#include <netdb.h>  // unix headers
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define errmsg(type) fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, type)

bool resolve_hostname(const char* target, sockaddr_in& dest_sockaddrin,/* int& ipver,*/ char* resolved_ip);
uint16_t calculate_checksum(void *buf, int len);
bool trace_route(const char* target, const char* netint, uint16_t hops);

constexpr auto PACKETSIZE = 64;