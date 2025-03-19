#include "core.h"

constexpr auto IFNAMSIZ {16};    // see https://elixir.bootlin.com/linux/v5.6/source/include/uapi/linux/if.h#L33
constexpr auto HOPSDEFAULT {64};

int main(int argc, char** argv)
{
	using ndiag::trace_route;

	const char* desc =
		"Usage: ndiag -t <target> -m <max_ttl> -i <device>\n"
		"Arguments:\n"
		"\t-t\t(mandatory) Specify the target. Must be a valid IPv4/FQDN.\n"
		"\t-m\t(optional) Set the max number of hops. Default is 64.\n"
		"\t-i\t(optional) Specify the network interface to operate with. Default is used if not specified.\n";
	
	char target[NI_MAXHOST]{0};	// see man getnameinfo: NI_MAXHOST is max value for socklen_t __hostlen arg value
	char device[IFNAMSIZ]{0};
	uint16_t max_ttl {HOPSDEFAULT};
	int c{};

	if (argc <= 1) {
		fprintf(stderr, desc);
		return EXIT_FAILURE;
	}

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
	
	if (!trace_route(target, device, max_ttl))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
