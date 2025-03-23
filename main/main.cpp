#include "core.h"
#include <iostream>

int main(int argc, char** argv)
{
	using ndiag::HOPSDEFAULT;
	using ndiag::ICMP_OUTPUT_MAX_PAYLOAD_SIZE;
	using ndiag::trace_route;

	const char *const desc =
		"Usage: ndiag [-t <target>] [-m <max_ttl>] [-i <device>]\n"
		"Mandatory params:\n"
		"\t-t\tSpecify the target. Must be a valid IPv4/FQDN\n"
		"Optional params:\n"
		"\t-h\tPrint this message\n"
		"\t-i\tSpecify the network interface to operate with. Default is used if not specified\n"
		"\t-m\tSet the max number of hops. Range is 1-255 (clamped). Default is 64\n"
		"\t-p\tSet the ICMP additional payload size. Max value is 228 (clamped). Default is 0 (no additional data)\n";
	
	char target[NI_MAXHOST]{0};	// see man getnameinfo: NI_MAXHOST is max value for socklen_t __hostlen arg value
	char device[IFNAMSIZ]{0};
	int payloadsize = 0;
	int max_ttl {HOPSDEFAULT};
	int c = 0;

	while ((c = getopt(argc, argv, "hi:m:p:t:")) != EOF) {
		switch (c) {
		case 'i':
			if (optarg) std::strcpy(device, optarg);
			break;
		case 'm':
			if (optarg) max_ttl = std::clamp(std::atoi(optarg), 1, 255);
			break;
		case 'p':
			if (optarg) payloadsize = std::clamp(std::atoi(optarg), 1, ICMP_OUTPUT_MAX_PAYLOAD_SIZE);
			break;
		case 't':
			if (optarg) std::strcpy(target, optarg);
			break;
		case 'h':
		default:
			fprintf(stderr, desc);
			return EXIT_SUCCESS;
		}
	}

    if (argc <= 1 || !target[0]) {
		fprintf(stderr, desc);
		return EXIT_FAILURE;
	}

	if (!trace_route(target, device, payloadsize, max_ttl))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
