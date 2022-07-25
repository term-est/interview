#include "UI.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>

#include <fmt/core.h>

void initialize()
{
	using namespace ftxui;

	auto summary = [&]
	{
		auto content = vbox({
									hbox({text(L"- done:   "), text(L"3") | bold}) | color(Color::Green),
									hbox({text(L"- active: "), text(L"2") | bold}) | color(Color::RedLight),
									hbox({text(L"- queue:  "), text(L"9") | bold}) | color(Color::Red),
							});
		return window(text(L" Summary "), content);
	};

	auto document =  //
			vbox({
						 hbox({
									  summary(),
									  summary(),
									  summary() | flex,
							  }),
						 summary(),
						 summary(),
				 });

// Limit the size of the document to 80 char.
	document = document | size(WIDTH, LESS_THAN, 80);

	auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
	Render(screen, document);

	fmt::print("{:s}\0\n", screen.ToString());

	return;
}

