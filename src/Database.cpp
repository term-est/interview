#include "Database.hpp"

// this looks wet, but there isn't really a good way of going DRY with this without making everything a string
[[nodiscard]]
bool Person::operator==(const pugi::xml_node& cmp) const
{
	using namespace std::literals;

	const pugi::xml_node& cmp_fullname = cmp.child("FullName");
	const pugi::xml_node& cmp_id_no = cmp.child("IDNo");
	const pugi::xml_node& cmp_is_attendant = cmp.child("isAttendant");


	if (cmp_fullname.empty() || cmp_id_no.empty() || cmp_is_attendant.empty())
		return false;

	if (fullname != cmp_fullname.text().get())
		return false;

	if (std::to_string(id_no) != cmp_id_no.text().get())
		return false;

	if ((is_attendant ? "1"sv : "0"sv) != cmp_is_attendant.text().get())
		return false;

	return true;
}

//TODO: db node names shouldn't be hardcoded, it's quite error prone

[[nodiscard]]
bool Database::query(const Person& attendant, const Person& customer)
{
	pugi::xml_object_range people = database_file.child("IDDatabase").children("Person");

	// make sure that customer is actually a customer and attendant is actually an attendant
	if (attendant.is_attendant == false || customer.is_attendant == true)
	{
		saveTransaction(Transaction{attendant, customer, -1});
		return false;
	}

	// check if both attendant and the customer are in the database
	bool result[2]{};
	for (const pugi::xml_node& e : people)
	{
		if (attendant == e)
			result[0] = true;

		else if (customer == e)
			result[1] = true;
	}

	bool&& is_success = result[0] && result[1];

	saveTransaction(Transaction{attendant, customer, is_success ? transaction_id : -1});
	transaction_id += is_success;

	return is_success;
}

void Database::saveTransaction(const Transaction& t)
{
	pugi::xml_node transactions = log_file.child("TransactionDatabase");
	pugi::xml_node new_transaction = transactions.append_child("Transaction");

	auto set_text = new_transaction.append_child("Customer");
	set_text.text().set(t.customer.fullname.data());

	set_text = new_transaction.append_child("Attendant");
	set_text.text().set(t.attendant.fullname.data());

	set_text = new_transaction.append_child("TransactionID");
	set_text.text().set(std::to_string(t.transaction_id).c_str());


	log_file.save_file(m_log_path.c_str());
}