#include "core.h"

bool resolve_fqdn(const char* target, sockaddr_storage& dest_sockaddrin_any, int& ipver, char* resolved_name)
{
    addrinfo addrreq {0};
    addrreq.ai_family = AF_INET;
    addrreq.ai_socktype = SOCK_STREAM;
    addrreq.ai_flags = AI_ADDRCONFIG;

    addrinfo *res {nullptr};
    auto status = getaddrinfo(target, nullptr, &addrreq, &res);
    if (status < 0) {
        fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, gai_strerror(status));
        return false;
    }

    if (res->ai_family != AF_INET/* && res->ai_family != AF_INET6*/)
        return false;

    memcpy(&dest_sockaddrin_any,
            res->ai_addr,
//            (res->ai_family == AF_INET)
                sizeof(sockaddr_in)   // 16
//                : sizeof(sockaddr_in6)  // 28
    );
    ipver = res->ai_family;

    inet_ntop(
        res->ai_family,
        //(res->ai_family == AF_INET) 
            reinterpret_cast<void*>(&reinterpret_cast<sockaddr_in*>(res->ai_addr)->sin_addr),
        //    : reinterpret_cast<void*>(&reinterpret_cast<sockaddr_in6*>(res->ai_addr)->sin6_addr),
        resolved_name,
        INET6_ADDRSTRLEN
    );
    
    freeaddrinfo(res);

    return true;
}

void trace_route(const char* target, const char* netint, int hops)
{
	sockaddr_storage dest_sockaddrin {0};
    /*int ipver{};
    char resolvedIP[INET6_ADDRSTRLEN];
    
    resolve_fqdn(target, dest_sockaddrin, ipver, resolvedIP);
    printf("name: %s\n", resolvedIP);*/

	/*domain: INET, type: RAW, proto: ICMP*/
	auto sockFD = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockFD < 0) {
		fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, std::strerror(errno));
		exit(EXIT_FAILURE);
	}
    

    /* ... */

	close(sockFD);	// close fd
}

