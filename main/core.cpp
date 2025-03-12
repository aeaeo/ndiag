#include "core.h"

bool resolve_fqdn(const char* target, sockaddr_storage& dest_sockaddrin_any, int& ipver, char* resolved_ip)
{
    addrinfo addrreq {0};
    addrreq.ai_family = AF_INET;
    addrreq.ai_socktype = SOCK_STREAM;
    //addrreq.ai_flags = AI_ADDRCONFIG; // not sure

    addrinfo *res {nullptr};
    auto status = getaddrinfo(target, nullptr, &addrreq, &res);
    if (status < 0) {
        fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, gai_strerror(status));
        return false;
    }

    if (res->ai_family != AF_INET)
        return false;

    memcpy(
        &dest_sockaddrin_any,
        res->ai_addr,
        sizeof(sockaddr_in)   // 16
    );
    ipver = res->ai_family;

    inet_ntop(
        res->ai_family,
        reinterpret_cast<void*>(&reinterpret_cast<sockaddr_in*>(res->ai_addr)->sin_addr),
        resolved_ip,
        INET_ADDRSTRLEN    // because capable of both
    );

    freeaddrinfo(res);

    return true;
}

void trace_route(const char* target, const char* netint, int hops)
{
	sockaddr_storage dest_sockaddrin {0};
    int ipver{};
    char resolvedIP[INET6_ADDRSTRLEN];
    
    if (resolve_fqdn(target, dest_sockaddrin, ipver, resolvedIP))
        printf("name: %s\n", resolvedIP);

	/*domain: INET, type: RAW, proto: ICMP*/
	auto sockFD = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockFD < 0) {
		fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, std::strerror(errno));
		exit(EXIT_FAILURE);
	}
    

    /* ... */

	close(sockFD);	// close fd
}

