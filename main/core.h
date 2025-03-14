#pragma once

#include <cstdio>	// c/cpp headers
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cerrno>

#ifndef __linux__
#error wrong platform
#endif

#include <netinet/ip.h>	// unix headers
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define errmsg(type) fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, type)

bool resolve_fqdn(const char* target, sockaddr_in& dest_sockaddrin_any,/* int& ipver,*/ char* resolved_ip);
bool trace_route(const char* target, const char* netint, int hops);