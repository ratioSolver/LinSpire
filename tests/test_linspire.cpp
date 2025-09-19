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

int main()
{
    test0();

    return 0;
}
