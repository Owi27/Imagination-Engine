#include <iostream>
#include <expected>

#define DEBUG(msg) std::cout << msg << '\n';

#define RETURN(t1) std::expected<t1, const char*>
#define RETURNSPECIFIC(t1, t2) std::expected<t1, t2>