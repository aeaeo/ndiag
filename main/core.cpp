#include "core.h"

void trace_route(const char* target)
{
	struct sockaddr_in dest_sock_addr;

	/*domain: INET, type: RAW, proto: ICMP*/
	auto sockFD = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockFD < 0) {
		fprintf(stderr, "%s():%d: %s\n", __FOO__, __LINE__, std::strerror(errno));
		exit(EXIT_FAILURE);
	}

	close(sockFD);	// close fd
}

