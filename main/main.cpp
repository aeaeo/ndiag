#include "core.h"

int main(int argc, char** argv)
{
	using ndiag::trace_route;

	const char* desc =
		"Usage: ndiag -t <target> -m <max_ttl> -i <device>\n"
		"Mandatory arguments:\n"
		"\t-t\tSpecify the target. Must be a valid IPv4/FQDN.\n"
		"Optional arguments:\n"
		"\t-m\tSet the max number of hops. Default is 64.\n"
		"\t-i\tSpecify the network interface to operate with. Default is used if not specified.\n";
	
	char target[NI_MAXHOST]{0};	// see man getnameinfo: NI_MAXHOST is max value for socklen_t __hostlen arg value
	char device[IFNAMSIZ]{0};
	uint16_t max_ttl {HOPSDEFAULT};
	int c{};

	while ((c = getopt(argc, argv, "t:m:i:")) != -1) {
		switch (c) {
		case 't':
			if (optarg) std::strcpy(target, optarg);
			break;
		case 'm':
			if (optarg) max_ttl = std::atoi(optarg);
			break;
		case 'i':
			if (optarg) std::strcpy(device, optarg);
			break;
		default:
			fprintf(stderr, desc);
			return EXIT_FAILURE;
		}
	}

	if (argc <= 1 || !target[0]) {
		fprintf(stderr, desc);
		return EXIT_FAILURE;
	}
	
	if (!trace_route(target, device, max_ttl))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
