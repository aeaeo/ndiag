#include "core.h"

int main(int argc, char** argv)
{
	// const char* hostname = nullptr;
	// int hops_max = 0;
	//
	// here parse arguments
	if (!trace_route(argv[1], nullptr, 0))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
