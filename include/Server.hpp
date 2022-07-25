#pragma once

#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>

#include "Debug.hpp"

/*
	T is a callable with signature Write<OnCompletionCallback>(std::asio::const_buffer, std::size_t) that
	is called after receiving some data
*/
template <typename T, std::size_t BufferSize = 4096>
class Server
{

	asio::io_context context;
	asio::ip::tcp::socket socket{context};
	asio::ip::tcp::acceptor acceptor;

	std::byte buffer[BufferSize]{};
	static_assert(BufferSize > 256); // Buffer Size Must Be At Least 256Bytes

	std::jthread context_runner;

	T  ReceiveCallback;

public:

	explicit Server(std::uint16_t port, T ReceiveCallback) : acceptor{context, asio::ip::tcp::endpoint{asio::ip::make_address("127.0.0.1"), port}}, ReceiveCallback{ReceiveCallback}
	{
		awaitConnection();
		context_runner = std::jthread{[&](){context.run();}};
	}

	void write(const std::byte* data, std::size_t transfer_size, auto Callback)	requires requires { Callback(asio::error_code{}, std::size_t{}); }
	{
		auto write_ready = [server = this, data, transfer_size, Callback](const asio::error_code& er)
		{
			if (er)
			{
				Callback(er, 0);
				return;
			}

			server->awaitWrite(data, transfer_size, Callback);
		};

		socket.template async_wait(asio::socket_base::wait_write, write_ready);
	}

	// TODO: graceful exit
	~Server()
	{
		context.stop();
		acceptor.close();
		socket.close();
	}

private:

	void awaitConnection()
	{
		using namespace std::literals;

		socket.close();
		acceptor.listen();

		// async wait for connection, on connection, set up an async_receiver that calls Server::awaitReceive
		// awaitReceive sets up async receive handler for receiving data
		acceptor.async_accept(socket,[server = this](const asio::error_code& er){
			if (er)
				error{error_helper{-4, "Connection Error\n"s + er.message()}};

			server->socket.template set_option(asio::socket_base::keep_alive{true});
			server->awaitReceive();
		});
	}

	void awaitReceive() requires
	requires { ReceiveCallback(std::declval<asio::const_buffer>(), std::size_t{}, this); }
	{
		using namespace std::literals;

		socket.async_receive(asio::buffer(buffer, 4096), [server = this](const asio::error_code& er, std::size_t transfer_size){
			if (er)
			{
				server->awaitConnection();
				return;
			}

			server->buffer[transfer_size] = static_cast<std::byte>('\0');
			server->ReceiveCallback(asio::buffer(server->buffer, transfer_size), transfer_size, server);
			server->awaitReceive();
		});
	}

	void awaitWrite(const std::byte* data, std::size_t transfer_size, auto Callback)
	{
		socket.template async_write_some(asio::buffer(data, transfer_size), [Callback](const asio::error_code& er, std::size_t transfered_size){
			Callback(er, transfered_size);
		});
	}
};