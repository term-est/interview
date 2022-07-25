#pragma once

#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>

#include "Debug.hpp"


template <typename T, std::size_t BufferSize = 4096>
class Client
{

	asio::io_context context;
	asio::ip::tcp::socket socket{context};

	std::byte buffer[BufferSize]{};
	static_assert(BufferSize > 256); // Buffer Size Must Be At Least 256Bytes

	std::jthread context_runner;

	T  ReceiveCallback;

public:

	explicit Client(std::uint16_t port, T ReceiveCallback) : ReceiveCallback{ReceiveCallback}
	{
		asio::error_code er;
		//the reason connection is not async is because integrating an async connect with the UI is too much of a hassle
		socket.connect(asio::ip::tcp::endpoint{asio::ip::address_v4::from_string("127.0.0.1"), port}, er);
		socket.wait(asio::socket_base::wait_write);

		if (er)
			error{error_helper{-6, er.message()}};


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
	~Client()
	{
		context.stop();
		socket.close();
	}

private:

	void awaitReceive() requires
	requires { ReceiveCallback(std::declval<asio::const_buffer>(), std::size_t{}, this); }
	{
		using namespace std::literals;

		socket.async_receive(asio::buffer(buffer, 4096), [server = this](const asio::error_code& er, std::size_t transfer_size){
			if (er)
				error{error_helper{-6, er.message()}};


			server->buffer[transfer_size] = static_cast<std::byte>('\0');
			server->ReceiveCallback(asio::buffer(server->buffer, transfer_size), transfer_size, server);
		});
	}

	void awaitWrite(const std::byte* data, std::size_t transfer_size, auto Callback)
	{
		awaitReceive();
		socket.template async_write_some(asio::buffer(data, transfer_size), [server = this, Callback](const asio::error_code& er, std::size_t transfered_size){
			Callback(er, transfered_size);
		});
	}
};