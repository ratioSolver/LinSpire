#include "linspire.hpp"
#include "logging.hpp"
#include <cassert>

void test0()
{
    linspire::solver s;
    [[maybe_unused]] bool res0 = s.new_eq(0, 0);
    assert(res0);
    [[maybe_unused]] bool res1 = s.new_lt(1, 0, true);
    assert(!res1);

    auto x0 = s.new_var();
    LOG_DEBUG(to_string(s));
    [[maybe_unused]] bool res2 = s.new_eq({{{x0, 2}}, 3}, 7);
    assert(res2);
    LOG_DEBUG(to_string(s));
    assert(s.lb(x0) == 2);
    assert(s.ub(x0) == 2);
    assert(s.val(x0) == 2);

    [[maybe_unused]] auto cons = s.check();
    assert(cons);
}

void test1()
{
    linspire::solver s;
    auto x = s.new_var();
    auto y = s.new_var();
    auto s1 = s.new_var(utils::lin{{x, -1}, {y, 1}});
    auto s2 = s.new_var(utils::lin{{x, 1}, {y, 1}});

    // x <= -4
    bool res0 = s.new_lt({{x, 1}}, -4);
    assert(res0);
    // x >= -8
    bool res1 = s.new_gt({{x, 1}}, -8);
    assert(res1);
    // s1 <= 1
    bool res2 = s.new_lt({{s1, 1}}, 1);
    assert(res2);
    // s2 >= -3
    bool res3 = s.new_gt({{s2, 1}}, -3);
    assert(res3);

    auto cons = s.check();
    assert(!cons);
}

void test2()
{
    linspire::solver s;
    auto x = s.new_var();
    auto y = s.new_var();
    auto z = s.new_var();

    auto c0 = std::make_shared<linspire::constraint>();
    auto c1 = std::make_shared<linspire::constraint>();

    // y >= x + 1
    bool res0 = s.new_gt({{y, 1}, {x, -1}}, 1, false, c0);
    assert(res0);
    // z >= y + 1
    bool res1 = s.new_gt({{z, 1}, {y, -1}}, 1, false, c1);
    assert(res1);
    bool cons = s.check();
    assert(cons);

    s.retract(c0);

    // x >= z + 1
    bool res2 = s.new_gt({{x, 1}, {z, -1}}, 1);
    assert(res2);
    cons = s.check();
    assert(cons);
}

int main()
{
    test0();
    test1();
    test2();

    return 0;
}
