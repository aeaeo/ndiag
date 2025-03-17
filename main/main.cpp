#include "core.h"

int main(int argc, char** argv)
{
	// const char* hostname = nullptr;
	// int hops_max = 0;
	//
	// here parse arguments
	
	if (!trace_route(argv[1], argv[2], std::atoi(argv[3])))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
