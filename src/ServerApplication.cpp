#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <chrono>
#include <algorithm>
#include <numeric>

#include <fmt/core.h>
#include <fmt/color.h>
#include <spdlog/spdlog.h>
#include <argparse/argparse.hpp>
#include <asio.hpp>

#include "Server.hpp"
#include "Database.hpp"
#include "UI.hpp"
#include "Debug.hpp"


//TODO: Logging via spdlog is not async, because I don't hate myself enough to implement that

// application entry point
auto main(int argc, char* argv[]) -> int try
{
	argparse::ArgumentParser program("Server");

	program.add_description("Server side TUI application developed as part of the ANAYURT interview process.");
	program.add_epilog("Can Cagri, 2022");

	program.add_argument("-database")
			.help("Filepath of the database file (XML)")
			.required();

	program.add_argument("-log")
			.help("Filepath for the log file (XML)")
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

	std::filesystem::path database_path = program.get("-database");
	std::filesystem::path log_path = program.get("-log");
	std::uint16_t port = program.get<std::uint16_t>("-port");

	Database db{database_path, log_path};

	auto ReceiveCallback = [&db](const asio::const_buffer& buffer, std::size_t transfer_size, auto* server)
	{
		auto error_handler = [server]()
		{
			std::string* msg = new std::string{};
			*msg = fmt::format(fmt::fg(fmt::color::dark_red), "Malformed input!");
			server->write(reinterpret_cast<std::byte*>(msg->data()), msg->size(), [msg](const asio::error_code& er, std::size_t transfered_size){
				auto formatted_msg = fmt::format("Operation result: {}\nTotal bytes written: {}\n", er.message(), transfered_size);
				spdlog::error(formatted_msg);
				delete msg;
			});
		};

		if (transfer_size > 1024)
		{
			error_handler();
			return;
		}

		std::string data{static_cast<const char*>(buffer.data()), transfer_size};

		auto&& newline_count = std::count_if(data.begin(), data.end(), [](char c){return c == '\n';});
		// make sure that data can be serialized into a Person struct
		if (newline_count < 5)
		{
			error_handler();
			return;
		}

		std::stringstream input{data};

		Person attendant{input};
		Person customer{input};

		bool&& result = db.query(attendant, customer);

		using namespace std::literals;
		std::string* msg = new std::string{};

		if (result)
		{
			*msg = fmt::format(fmt::fg(fmt::color::green),
			customer.fullname + " has been identified\n"s + attendant.fullname + " has been identified\n"s + "Query Success!\n"s +
			"Query recorded to transaction database\n"s + "("s + customer.fullname + ", "s + attendant.fullname + ", ACCESS_OK)\n");
		}

		else
		{
			*msg = fmt::format(fmt::fg(fmt::color::dark_red),
		   "Unknown Customer!\n"s + "Query recorded to transaction database\n"s + "This incident will be reported!\n"
		   + "("s + customer.fullname + ", "s + attendant.fullname + ", ACCESS_DENIED)\n");
		}

		server->write(reinterpret_cast<std::byte*>(msg->data()), msg->size(), [msg](const asio::error_code& er, std::size_t transfered_size){
			delete msg;
		});

	};


	Server server{port, ReceiveCallback};

	// Server will process all events itself asynchronously, main thread can sleep forever
	std::this_thread::sleep_for(std::chrono::seconds::max());

	// if main returns, that means an error has occurred
	return -127;
}
catch(std::exception& e)
{
	using namespace std::literals;
	error{error_helper{-3, "Fatal Error, Stack Cleanup is successful\n"s + e.what()}};
}