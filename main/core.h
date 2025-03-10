#include <cstdio>	// cpp headers (well c ones actually)
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cerrno>

#include <netinet/ip.h>	// unix headers
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef __FOO__
    #ifdef WIN32
        #define __FOO__  __FUNCTION__
    #else
        #define __FOO__  __func__
    #endif
#endif

void trace_route(const char*);