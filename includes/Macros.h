#pragma once
#include <iostream>
#include <expected>


//#ifdef _WIN32
//#include <windows.h>
//inline void SetColorRed() {
//	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
//		FOREGROUND_RED | FOREGROUND_INTENSITY);
//}
//inline void ResetColor() {
//	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
//}
//#else
//inline void SetColorRed() { std::cout << "\033[31m"; }
//inline void ResetColor() { std::cout << "\033[0m"; }
//#endif
//
//#define LOG(msg) \
//    do { SetColorRed(); std::cout << '\n' << msg << '\n'; ResetColor(); } while(0)

#define LOG(msg) std::cout << msg << '\n';

#define RETURN(t1) std::expected<t1, const char*>
#define RETURNSPECIFIC(t1, t2) std::expected<t1, t2>

template <typename T, typename F>
inline T Attempt(std::expected<T, F>&& ex)
{
	if (!ex)
	{
		LOG(ex.error());
		throw ex.error();
	}

	return std::move(*ex);
}

// Specialization for void return types
inline void Attempt(std::expected<void, const char*>&& ex)
{
	if (!ex)
	{
		LOG(ex.error());
		return;
	}

	ex.value();
}