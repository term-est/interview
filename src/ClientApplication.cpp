#include <string>
#include <string_view>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>

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

	program.add_argument("-attendant_name")
			.required();

	program.add_argument("-attendant_id")
			.required();

	program.add_argument("-customer_name")
			.required();

	program.add_argument("-customer_id")
			.required();

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
	std::string attendant_name = program.get("-attendant_name");
	std::string attendant_id = program.get("-attendant_id");
	std::string customer_name = program.get("-customer_name");
	std::string customer_id = program.get("-customer_id");



	auto ReceiveCallback = [](const asio::const_buffer& buffer, std::size_t transfer_size, auto* server)
	{
		std::string_view data{static_cast<const char*>(buffer.data()), transfer_size};
		fmt::print("{}\n", data);
	};

	Client client{port, ReceiveCallback};
	const std::string data = attendant_name + "\n" + attendant_id + "\n1\n" + customer_name + "\n" + customer_id + "\n0\n";

	client.write(reinterpret_cast<const std::byte*>(data.data()), data.size(), [](const asio::error_code& er, std::size_t transfer_size){
		if (er)
		{
			error{error_helper{-6, er.message()}};
			return;
		}
	});

	using namespace std::chrono;
	std::this_thread::sleep_for(5s);

	return 0;
}
catch(std::exception& e)
{
	using namespace std::literals;
	error{error_helper{-3, "Fatal Error, Stack Cleanup is successful\n"s + e.what()}};
}