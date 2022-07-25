cmake_minimum_required(VERSION 3.21)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


project(embedded_qualification)

set(CMAKE_CXX_STANDARD 23)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(pugixml REQUIRED)
find_package(ftxui CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)

add_executable(Server src/ServerApplication.cpp src/Database.cpp)
target_include_directories(Server PUBLIC include utilities)
target_link_libraries(Server argparse::argparse ftxui::dom ftxui::screen ftxui::component pugixml asio asio::asio spdlog::spdlog fmt::fmt)

add_executable(Client src/ClientApplication.cpp src/UI.cpp include/Client.hpp)
target_include_directories(Client PUBLIC include utilities)
target_link_libraries(Client argparse::argparse ftxui::dom ftxui::screen ftxui::component pugixml asio asio::asio spdlog::spdlog fmt::fmt)

if(MINGW)
    target_link_libraries(Server ws2_32 wsock32)
    target_compile_definitions(Server PUBLIC _WIN32_WINDOWS=0x0601)

    target_link_libraries(Client ws2_32 wsock32)
    target_compile_definitions(Client PUBLIC _WIN32_WINDOWS=0x0601)
endif()