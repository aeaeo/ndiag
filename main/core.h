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

bool resolve_fqdn(const char* target, sockaddr_storage& dest_sockaddrin_any, int& ipver, char* resolved_name);
void trace_route(const char* target, const char* netint, int hops);