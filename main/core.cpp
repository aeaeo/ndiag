#include "core.h"

bool resolve_fqdn(const char* target, sockaddr_in& dest_sockaddrin,/*int& ipver,*/ char* resolved_ip)
{
    addrinfo addrreq {0};
    addrreq.ai_family = AF_INET;
    addrreq.ai_socktype = SOCK_STREAM;

    addrinfo *res {nullptr};
    int status = getaddrinfo(target, nullptr, &addrreq, &res);
    if (status < 0) {
        fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, gai_strerror(status));
        return false;
    }
    //puts("testword\n"); 
    if (res->ai_family != AF_INET)
        return false;

    memcpy(
        &dest_sockaddrin,
        res->ai_addr,
        sizeof(sockaddr_in)   // 16
    );
    //ipver = res->ai_family;

    inet_ntop(
        res->ai_family,
        reinterpret_cast<void*>(&dest_sockaddrin.sin_addr),
        resolved_ip,
        INET_ADDRSTRLEN
    );

    freeaddrinfo(res);

    return true;
}

bool trace_route(const char* target, const char* netint, int hops)
{
	sockaddr_in dest_sockaddrin {0};
    char resolvedIP[INET_ADDRSTRLEN];

    if (!resolve_fqdn(target, dest_sockaddrin, resolvedIP))
        return false;    

    printf("name: %s\n", resolvedIP);

	/*domain: INET, type: RAW, proto: ICMP*/
    // CAP_NET_RAW required
	int sockFD = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockFD < 0) {
		fprintf(stderr, "%s():%d: %s\n", __func__, __LINE__, std::strerror(errno));
        return false;
	}

//  for (int i = 0; i < hops; ++i) {
        /* ... */
//  }

	close(sockFD);	// close fd
    return true;
}

