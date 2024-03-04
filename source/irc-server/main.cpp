#include <filesystem>
#include <server.hpp>
#include <xlog.hpp>

namespace
{
	/*
		set the current path to where the executable is located
		in case someone starts the server from a different path
		it should be still able to load its files w/o loss
	*/
	void initialize_working_directory(std::string passed_path)
	{
		std::filesystem::path working_path(passed_path);
		std::filesystem::current_path(working_path.parent_path());
	}
}

int main(int argc, char* args[])
{
	initialize_working_directory(args[0]);
	xlog::initialize();

	Server server("4000");

	return 0;
}
