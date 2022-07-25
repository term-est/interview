#pragma once

#include <filesystem>
#include <string>
#include <sstream>
#include <string_view>
#include <algorithm>
#include <numeric>


#include <pugixml.hpp>
#include "Debug.hpp"


struct Person
{
	Person(std::string fullname, std::size_t id_no, bool is_attendant) noexcept : fullname{std::move(fullname)}, id_no{id_no}, is_attendant{is_attendant}
	{}

	// NOTICE: modifies the input stream
	explicit Person(std::stringstream& input)
	{
		std::string temp;

		std::getline(input, fullname, '\n');
		std::getline(input, temp, '\n');
		id_no = std::stoull(temp);
		std::getline(input, temp, '\n');
		is_attendant = temp[0] == '1';
	}

	std::string fullname;
	std::size_t id_no;

	bool is_attendant;

	[[nodiscard]]
	bool operator==(const pugi::xml_node& cmp) const;
};

class Database
{
	const std::filesystem::path m_db_path;
	const std::filesystem::path m_log_path;

	pugi::xml_document database_file;
	pugi::xml_document log_file;

	// current transaction id, where -1 denotes a failed transaction attempt (auth error, no such element etc.)
	int transaction_id = -1;

public:
	explicit Database(std::filesystem::path db_path, std::filesystem::path log_path) noexcept
	: m_db_path{std::move(db_path)}, m_log_path{std::move(log_path)}
	{
		using namespace std::literals;

		if (not database_file.load_file(m_db_path.c_str()))
			error{error_helper{-1, "Could not load the database file: "s + m_db_path.string()}};

		if (not log_file.load_file(m_log_path.c_str()))
			error{error_helper{-1, "Could not load the log file: "s + m_log_path.string()}};

		//get latest transaction id
		transaction_id = -1;
		auto log_xml = log_file.child("TransactionDatabase");
		for (const pugi::xml_node& e : log_xml.children("Transaction"))
		{
			const auto& id = e.child("TransactionID");

			if (id.hash_value())
				transaction_id = std::max(transaction_id, std::stoi(id.text().get()));

			++transaction_id;
		}
	}

	[[nodiscard]]
	bool query(const Person& attendant, const Person& customer);


private:

	struct Transaction
	{
		Person attendant;
		Person customer;
		int transaction_id = -1;
	};

	void saveTransaction(const Transaction& t);
};
