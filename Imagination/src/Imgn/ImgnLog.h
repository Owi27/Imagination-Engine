#pragma once

#include "ImgnCore.hpp"

namespace IMGN
{
    namespace LOG
    {
        constexpr const char* esc = "\033[";
        constexpr const char* blackBG = "0;";
        constexpr const char* redFG = "31m";
        constexpr const char* greenFG = "32m";
        constexpr const char* yellowFG = "33m";
        constexpr const char* blueFG = "34m"; //our critical cause i dont like blue
        constexpr const char* reset = "\033[0m";
    }
}

class IMGN_API Log
{
   // std::string _esc = , _blackBG = "0;", _redFG = "1m", _reset = "\033[m";

public:
    /* Class Functions */

    /* Class Defaults */
    Log()
    {

    }

    ~Log()
    {

    }
};

#define IMGN_CORE_TRACE(...) std::cout << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] TRACE: " << std::format(__VA_ARGS__) << '\n';
#define IMGN_CORE_INFO(...) std::cout << IMGN::LOG::esc << IMGN::LOG::blackBG << IMGN::LOG::greenFG << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] INFO: " <<std::format(__VA_ARGS__) << IMGN::LOG::reset << '\n';
#define IMGN_CORE_WARN(...) std::cout << IMGN::LOG::esc << IMGN::LOG::blackBG << IMGN::LOG::yellowFG << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] WARN: " <<std::format(__VA_ARGS__) << IMGN::LOG::reset << '\n';
#define IMGN_CORE_ERROR(...) std::cout << IMGN::LOG::esc << IMGN::LOG::blackBG << IMGN::LOG::redFG << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] ERROR: " <<std::format(__VA_ARGS__) << IMGN::LOG::reset << '\n';
#define IMGN_CORE_FATAL(...) std::cout << IMGN::LOG::esc << IMGN::LOG::blackBG << IMGN::LOG::blueFG << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] FATAL: " <<std::format(__VA_ARGS__) << IMGN::LOG::reset << '\n';

#define IMGN_TRACE(...) std::cout << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] TRACE: " << std::format(__VA_ARGS__) << '\n';
#define IMGN_INFO(...) std::cout << IMGN::LOG::esc << IMGN::LOG::blackBG << IMGN::LOG::greenFG << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] INFO: " <<std::format(__VA_ARGS__) << IMGN::LOG::reset << '\n';
#define IMGN_WARN(...) std::cout << IMGN::LOG::esc << IMGN::LOG::blackBG << IMGN::LOG::yellowFG << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] WARN: " <<std::format(__VA_ARGS__) << IMGN::LOG::reset << '\n';
#define IMGN_ERROR(...) std::cout << IMGN::LOG::esc << IMGN::LOG::blackBG << IMGN::LOG::redFG << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] ERROR: " <<std::format(__VA_ARGS__) << IMGN::LOG::reset << '\n';
#define IMGN_FATAL(...) std::cout << IMGN::LOG::esc << IMGN::LOG::blackBG << IMGN::LOG::blueFG << '[' << std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }) << "] FATAL: " <<std::format(__VA_ARGS__) << IMGN::LOG::reset << '\n';
