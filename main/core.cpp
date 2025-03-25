#include "core.h"

namespace ndiag {

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

uint16_t calculate_internet_checksum(void *buf, size_t len)
{
    uint16_t* u16buf = static_cast<uint16_t*>(buf);
    uint32_t sum = 0;

    for (; len > 1; len -= 2)
        sum += *u16buf++;
    
    if (len == 1)
        sum += *reinterpret_cast<uint8_t*>(u16buf);

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return ~static_cast<uint16_t>(sum);
}

void generate_cyclic_alphabet_msg(char* area, uint16_t payloadsize) {
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";   // test

    if (!area || payloadsize <= 0 || payloadsize > ICMP_OUTPUT_MAX_PAYLOAD_SIZE)
        return;

    for (uint16_t i = 0; i <= payloadsize; ++i) {
        area[i] = alphabet[i % (sizeof(alphabet) - 1)];
    }
}

volatile bool gContinue = true;

bool setupsighandlers(void) {
	struct sigaction act{};	// using c-style struct instance declaration since c++ one is ambiguous: struct 'sigaction' and function 'sigaction' have exact names
	
    act.sa_handler = [](int sig) -> void {
		if (sig == SIGINT || sig == SIGQUIT) {
			gContinue = false;
		}
	};

	if (sigaction(SIGINT, &act, nullptr)) {
        errmsg(std::strerror(errno));
        return false;
    }
    
    if (sigaction(SIGQUIT, &act, nullptr)) {
        errmsg(std::strerror(errno));
        return false;
    }  // same as above

    return true;
}

bool setupsocket(int& fd, const char* device, timeval& timeout) {
	fd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);     // CAP_NET_RAW required
	if (fd < 0) {
        errmsg(std::strerror(errno));
        return false;
	}

    if (device != nullptr && device[0]) {
        if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, device, IFNAMSIZ)) {
            errmsg(std::strerror(errno));
            return false;
        }
    }

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
        errmsg(std::strerror(errno));
        return false;
    }

    return true;
}

bool trace_route(const char* target, const char* device, uint16_t opt_payload, uint8_t hops)
{
    struct icmppack : icmphdr {
        char data[ICMP_OUTPUT_MAX_PAYLOAD_SIZE];
    };

	sockaddr_in dest_sockaddrin {0};
    char resolvedIP[INET_ADDRSTRLEN];   // or inet6_addrstrlen for both support
    int fd = -1; // file descriptor
    timeval timeout{1L, 0L};    // == 1 s 0 μs

    if (!resolve_host(target, dest_sockaddrin, resolvedIP))
        return false;

    if (!setupsocket(fd, device, timeout)){
        if (fd > 0) close(fd);
        return false;
    }

    if (!setupsighandlers())
        return false;

    printf("Tracing route to %s (%s) with %hhu hop(s) max, %lu bytes of data (ICMP), on interface %s\n", 
        target, resolvedIP, hops, sizeof(icmphdr) + opt_payload, !device[0] ? "[default]" : device);

    char bufo[MAX_PACKET_OUTPUT_SIZE], bufi[MAX_PACKET_INPUT_SIZE];
    char ipstrbuf[INET_ADDRSTRLEN];
    char resolved_hostnamebuf[NI_MAXHOST];    // from man getnameinfo: NI_MAXHOST is max value for socklen_t __hostlen arg value
    sockaddr_in reply_sockaddrin;
    socklen_t reply_sockaddrin_len = sizeof(reply_sockaddrin);

    // sizeof iphdr = 20
    // sizeof icmphdr = 8

    for (uint8_t ttl = 1; ttl <= hops && gContinue; ++ttl) {
        using namespace std::chrono;

        memset(bufo, '\0', sizeof(bufo));
        memset(bufi, '\0', sizeof(bufi));
        memset(&reply_sockaddrin, 0, sizeof(reply_sockaddrin));

        setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)); // time to live

        /* not used now
        auto iphdr_o = reinterpret_cast<iphdr*>(bufo);
        iphdr_o->ihl = 5u;  // 20
        iphdr_o->version = 4u;
        iphdr_o->tos = 0u;
        iphdr_o->tot_len = htons(sizeof(iphdr) + sizeof(icmphdr));  // little-endian (intel) to big-endian (network)
        iphdr_o->id = htons(49999u + ttl);
        iphdr_o->frag_off = htons(IP_DF);   // no packet fragmentation
        iphdr_o->ttl = ttl;
        iphdr_o->protocol = IPPROTO_ICMP;
        iphdr_o->check = 0u;
        inet_pton(AF_INET, "ipv4_address_of_given_network_interface", &(iphdr_o->saddr));
        iphdr_o->daddr = static_cast<uint32_t>(dest_sockaddrin.sin_addr.s_addr);
        iphdr_o->check = calculate_checksum(reinterpret_cast<uint16_t*>(iphdr_o), sizeof(iphdr));
        */

        auto icmp_pack_o = reinterpret_cast<icmppack*>(bufo);
        icmp_pack_o->type = ICMP_ECHO; // [0-8) ; icmphdr start
        icmp_pack_o->code = 0u;   // [8-16)
        icmp_pack_o->checksum = 0u; // [16-32)
        icmp_pack_o->un.echo.id = static_cast<uint16_t>(getpid()); // [[32-48)
        icmp_pack_o->un.echo.sequence = htons(static_cast<uint16_t>(ttl)); // [48-64)] ; icmphdr end
        if (opt_payload) generate_cyclic_alphabet_msg(icmp_pack_o->data, opt_payload);   // icmp data
        icmp_pack_o->checksum = calculate_internet_checksum(reinterpret_cast<uint16_t*>(icmp_pack_o), sizeof(icmphdr) + opt_payload);

        auto time_start = high_resolution_clock::now();

        ssize_t send_bytes = sendto(
            fd,    // socket file descriptor
            reinterpret_cast<void*>(bufo),    // icmp packet
            sizeof(icmphdr) + opt_payload,
            0, // no flags
            reinterpret_cast<sockaddr*>(&dest_sockaddrin),
            sizeof(sockaddr)
        );

        if (send_bytes < 0)
        {
            printf("\t%hhu\tsendto(): %s\n", ttl, std::strerror(errno));   // 
            continue;
        }

        ssize_t recv_bytes = recvfrom(
            fd,
            reinterpret_cast<void*>(bufi),
            sizeof(bufi),
            0, // no flags
            reinterpret_cast<sockaddr*>(&reply_sockaddrin), 
            &reply_sockaddrin_len
        );

        if (recv_bytes < 0)
        {
            printf("\t%hhu\trecvfrom(): %s\n", ttl, std::strerror(errno));
        }
        else
        {
            auto end_time = high_resolution_clock::now();
            auto rtt = duration<float, std::milli>(end_time - time_start).count();
            const auto icmp_pack_i = reinterpret_cast<icmppack*>(bufi + sizeof(iphdr));

            memset(ipstrbuf, '\0', sizeof(ipstrbuf));
            inet_ntop(AF_INET, reinterpret_cast<void*>(&reply_sockaddrin.sin_addr), ipstrbuf, INET_ADDRSTRLEN);

            memset(resolved_hostnamebuf, '\0', sizeof(resolved_hostnamebuf));
            getnameinfo(
                reinterpret_cast<sockaddr*>(&reply_sockaddrin),
                sizeof(reply_sockaddrin),
                resolved_hostnamebuf,
                sizeof(resolved_hostnamebuf),
                nullptr,
                0,
                0 // no flags
            );

            printf("\t%hhu\t%s\t(%s)\trtt=%.3f ms recieved=%ld B\n", ttl, ipstrbuf, resolved_hostnamebuf, rtt, recv_bytes);
            
            if (icmp_pack_i->type == ICMP_ECHOREPLY) {
                printf("Reached %s (%s) with %hhu hop(s)\n", target, resolvedIP, ttl);
                break;
            }
        }

        if (ttl+1 > hops) {
            printf("Failed to reach %s (%s): Hops limit exceeded\n", target, resolvedIP);
        }
    }

    close(fd);	// close FD

    return true;
}
};