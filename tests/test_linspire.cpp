#include "linspire.hpp"
#include "logging.hpp"
#include <cassert>

void test0()
{
    linspire::solver s;
    bool res0 = s.new_eq(utils::lin(utils::rational::zero), utils::lin(utils::rational::zero));
    assert(res0);
    bool res1 = s.new_lt(utils::lin(utils::rational::one), utils::lin(utils::rational::zero), true);
    assert(!res1);

    auto x0 = s.new_var();
    LOG_DEBUG(to_string(s));
    bool res2 = s.new_eq(utils::lin(x0, utils::rational(2)) + utils::lin(utils::rational(3)), utils::lin(utils::rational(7)));
    assert(res2);
    LOG_DEBUG(to_string(s));
    assert(s.lb(x0) == utils::inf_rational(2));
    assert(s.ub(x0) == utils::inf_rational(2));
    assert(s.val(x0) == utils::inf_rational(2));

    auto cons = s.check();
    assert(cons);
}

void test1()
{
    linspire::solver s;
    auto x = s.new_var();
    auto y = s.new_var();
    auto s1 = s.new_var(utils::lin(x, -utils::rational::one) + utils::lin(y, utils::rational::one));
    auto s2 = s.new_var(utils::lin(x, utils::rational::one) + utils::lin(y, utils::rational::one));

    // x <= -4
    bool res0 = s.new_lt(utils::lin(x, utils::rational::one), utils::lin(utils::rational(-4)));
    // x >= -8
    bool res1 = s.new_gt(utils::lin(x, utils::rational::one), utils::lin(utils::rational(-8)));
    // s1 <= 1
    bool res2 = s.new_lt(utils::lin(s1, utils::rational::one), utils::lin(utils::rational(1)));
    // s2 >= -3
    bool res3 = s.new_gt(utils::lin(s2, utils::rational::one), utils::lin(utils::rational(-3)));

    auto cons = s.check();
}

int main()
{
    test0();
    test1();

    return 0;
}
