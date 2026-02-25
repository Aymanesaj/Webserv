#include "Config.hpp"

int main() {

	try
	{
		ConfigParser config("conf/default.conf");
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return 0;
}