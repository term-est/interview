#include <fmt/core.h>
#include <fmt/color.h>
#include <spdlog/spdlog.h>
#include <argparse/argparse.hpp>
#include <asio.hpp>

#include "Client.hpp"
#include "UI.hpp"
#include "Debug.hpp"

// application entry point
auto main(int argc, char* argv[]) -> int try
{
	argparse::ArgumentParser program("Client");

	program.add_description("Client side TUI application developed as part of the ANAYURT interview process.");
	program.add_epilog("Can Cagri, 2022");

	program.add_argument("-port")
			.help("Port number used for communication (XML)")
			.default_value(static_cast<std::uint16_t>(1337));

	try
	{
		program.parse_args(argc, argv);
	}
	catch (std::runtime_error& err)
	{
		using namespace std::literals;
		std::stringstream s;
		s << program;
		error{error_helper{-2, ""s + err.what() + "\n" + s.str()}};
	}
	std::uint16_t port = program.get<std::uint16_t>("-port");

	auto ReceiveCallback = [](const asio::const_buffer& buffer, std::size_t transfer_size, auto* server)
	{
		std::string_view data{static_cast<const char*>(buffer.data()), transfer_size};
		fmt::print("{}\n", data);
	};

	Client client{port, ReceiveCallback};
	std::string data = "Ayse Kaya\n"
					   "6327569435\n"
					   "1\n"
					   "Ahmet Yilmaz\n"
					   "1234567890\n"
					   "0\n";
	client.write(reinterpret_cast<const std::byte*>(data.data()), data.size(), [](const asio::error_code& er, std::size_t transfer_size){
		if (er)
			fmt::print("{}\n", er.message());
	});

	std::this_thread::sleep_for(std::chrono::seconds::max());

	// if main returns, that means an error has occurred
	return -127;
}
catch(std::exception& e)
{
	using namespace std::literals;
	error{error_helper{-3, "Fatal Error, Stack Cleanup is successful\n"s + e.what()}};
}