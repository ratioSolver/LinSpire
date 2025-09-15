#include "linspire.hpp"
#include <cassert>

void test0()
{
    linspire::solver s;
    bool res = s.new_lt(utils::lin(utils::rational::one), utils::lin(utils::rational::zero), true);
    assert(!res);
}

int main()
{
    test0();

    return 0;
}
