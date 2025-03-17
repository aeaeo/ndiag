#include "core.h"

bool resolve_host(const char* target, sockaddr_in& dest_sockaddrin,/*int& ipver,*/ char* resolved_ip)
{
    addrinfo addrreq {0};
    addrreq.ai_family = AF_INET;
    addrreq.ai_socktype = SOCK_STREAM;

    addrinfo *res {nullptr};
    int status = getaddrinfo(target, nullptr, &addrreq, &res);
    if (status != 0) {
        errmsg(gai_strerror(status));
        return false;
    }

    memcpy(&dest_sockaddrin, res->ai_addr, sizeof(sockaddr_in));

    inet_ntop(res->ai_family, reinterpret_cast<void*>(&dest_sockaddrin.sin_addr), resolved_ip, INET_ADDRSTRLEN);

    freeaddrinfo(res);

    return true;
}

uint16_t calculate_checksum(void *buf, size_t len)
{
    uint16_t* u16buf = static_cast<uint16_t*>(buf);
    uint32_t sum = 0;

    for (; len > 1; len -= 2)
        sum += *u16buf++;
    
    if (len == 1)
        sum += *reinterpret_cast<uint8_t*>(u16buf);

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return static_cast<uint16_t>(~sum);
}


bool trace_route(const char* target, const char* netint, uint16_t hops)
{
	sockaddr_in dest_sockaddrin {0};
    char resolvedIP[INET_ADDRSTRLEN];   // or inet6_addrstrlen for both support

    if (!resolve_host(target, dest_sockaddrin, resolvedIP))
        return false;

	/*domain: INET, type: RAW, proto: ICMP*/
    // CAP_NET_RAW required
	int sockFD = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockFD < 0) {
        errmsg(std::strerror(errno));
        return false;
	}

    if (setsockopt(sockFD, SOL_SOCKET, SO_BINDTODEVICE, netint, sizeof(netint)+1)) {
        errmsg(std::strerror(errno));
        close(sockFD);
        return false;
    }

    timeval timeout{1, 0};
    if (setsockopt(sockFD, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
        errmsg(std::strerror(errno));
        close(sockFD);
        return false;
    }

    printf("Tracing route to %s (%s) with %d hop(s) max\n", target, resolvedIP, hops);

    // todo: check whether IP_HDRINCL is needed or not for this sockt
    char bufo[4096], bufi[4096];
    sockaddr_in reply_sockaddrin;
    socklen_t reply_sockaddrin_len = sizeof(reply_sockaddrin);

    for (uint16_t ttl = 1; ttl <= hops; ++ttl) {
        memset(&bufo, '\0', sizeof(bufo));
        memset(&bufi, '\0', sizeof(bufi));
        memset(&reply_sockaddrin, 0, sizeof(reply_sockaddrin));

        setsockopt(sockFD, IPPROTO_IP, IP_TTL, &ttl, sizeof(uint16_t)); // time to live attr

        // ntk: sizeof(iphdr) = 24

        auto icmp_pack_o = reinterpret_cast<icmphdr*>(bufo);    //
        icmp_pack_o->type = ICMP_ECHO; // [0-8) ; request
        icmp_pack_o->code = 0u;   // [8-16)
        icmp_pack_o->checksum = 0u; // [16-32)
        icmp_pack_o->un.echo.id = getpid(); // [[32-48) // getpid() may be bad idea
        icmp_pack_o->un.echo.sequence = ttl; // [48-64)]
        icmp_pack_o->checksum = calculate_checksum(reinterpret_cast<uint16_t*>(icmp_pack_o), sizeof(icmphdr));

        auto time_start = std::chrono::high_resolution_clock::now();
        if (sendto(
             sockFD,    // socket file descriptor
             reinterpret_cast<void*>(&bufo),    // icmp packet
             sizeof(icmphdr),
             0, // no flags
             reinterpret_cast<sockaddr*>(&dest_sockaddrin),
             sizeof(sockaddr)
            ) <= 0
        ) {
            printf("sendto() error: %s\n", std::strerror(errno));   // 
            continue;
        }

        auto icmp_pack_i = reinterpret_cast<icmphdr*>(bufi+20);
        if (recvfrom(
             sockFD,
             &bufi,
             sizeof(bufi),
             0, // no flags
             reinterpret_cast<sockaddr*>(&reply_sockaddrin), 
             &reply_sockaddrin_len
            ) < 0
        ) {
            printf("ttl=%hu\t| recvfrom() timeout error: %s\n", ttl, std::strerror(errno));
            continue;
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto rtt = std::chrono::duration<float, std::milli>(end_time - time_start).count();

        char ipstrbuf[INET_ADDRSTRLEN]{0};
        inet_ntop(
            AF_INET,
            reinterpret_cast<void*>(&reply_sockaddrin.sin_addr),
            ipstrbuf,
            INET_ADDRSTRLEN
        );

        char resolved_hostnamebuf[NI_MAXHOST]{0};    // from man getnameinfo: NI_MAXHOST is max value for socklen_t __hostlen arg value
        getnameinfo(
             reinterpret_cast<sockaddr*>(&reply_sockaddrin),
             sizeof(reply_sockaddrin),
             resolved_hostnamebuf,
             sizeof(resolved_hostnamebuf),
             nullptr,
             0,
             0// no flags
        );
        printf("ttl=%hu\t| %s (%s) |\trtt=%.4f ms\n", ttl, ipstrbuf, resolved_hostnamebuf, rtt);

        if (icmp_pack_i->type == ICMP_ECHOREPLY) { // if structs are eqauel trace is complete 
            printf("Reached %s (%s) with %hu hop(s)\n", target, resolvedIP, ttl);
            break;
        }
    }

	close(sockFD);	// close fd
    return true;
}
