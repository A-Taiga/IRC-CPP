#include <cstdlib>
#include <filesystem>
#include <server.hpp>
#include <xlog.hpp>

/*
  set the current path to where the executable is located
  in case someone starts the server from a different path
  it should be still able to load its files w/o loss
*/

static void initialize_working_directory(std::string passed_path)
{
	std::filesystem::path working_path(passed_path);
	std::filesystem::current_path(working_path.parent_path());
}

static bool check_port_number(const char* port)
{
	for (size_t i = 0; i < strlen(port); i++)
		if (!std::isdigit(port[i]))
			return false;
	return true;
}

int main(int argc, char* args[])
{
	initialize_working_directory(args[0]);
	
	if (false == xlog::initialize())
	{
		std::abort();

		return 1;
	}

	if (argc == 1)
	{
		std::cout << "PORT NUMBER NOT ENTERED" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	if (!check_port_number(args[1]))
		std::cout << "PORT NOT A VALID NUMBER" << std::endl;

	Server server(args[1]);
	server.listen(10);
	server.run();

	return 0;
}
