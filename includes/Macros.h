#include <iostream>
#include <expected>


#ifdef _WIN32
#include <windows.h>
inline void SetColorRed() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_INTENSITY);
}
inline void ResetColor() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}
#else
inline void SetColorRed() { std::cout << "\033[31m"; }
inline void ResetColor() { std::cout << "\033[0m"; }
#endif

#define DEBUG(msg) \
    do { SetColorRed(); std::cout << msg << '\n'; ResetColor(); } while(0)

//#define DEBUG(msg) std::cout << msg << '\n';

#define RETURN(t1) std::expected<t1, const char*>
#define RETURNSPECIFIC(t1, t2) std::expected<t1, t2>

#define ATTEMPT(expression) \
	do \
	{\
		auto r = expression; \
		if (r) *r; \
		else DEBUG(r.error()); \
	}\
	while (0)\
