#include "linspire.hpp"
#include "logging.hpp"
#include <cassert>

/**
 * @brief Unit test for the linspire::solver class.
 *
 * This test verifies the following functionalities:
 * - Creating variables and adding equality and less-than constraints using new_eq and new_lt.
 * - Ensuring that the constraints are satisfiable and checking the solver's consistency.
 * - Testing the solver's ability to detect inconsistency when conflicting constraints are added.
 *
 * Assertions are used to ensure that each step behaves as expected.
 */
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

/**
 * @brief Unit test for the linspire::solver class.
 *
 * This test verifies the following functionalities:
 * - Creating variables and adding less-than and greater-than constraints using new_lt and new_gt.
 * - Ensuring that the constraints are satisfiable and checking the solver's consistency.
 * - Testing the solver's ability to detect inconsistency when conflicting constraints are added.
 *
 * Assertions are used to ensure that each step behaves as expected.
 */
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

/**
 * @brief Unit test for the linspire::solver class.
 *
 * This test verifies the following functionalities:
 * - Creating variables and adding constraints sharing the same reason.
 * - Ensuring that the constraints are satisfiable and checking the solver's consistency.
 * - Testing the solver's ability to retract constraints and ensure bounds are reset.
 *
 * Assertions are used to ensure that each step behaves as expected.
 */
void test2()
{
    linspire::solver s;
    auto x = s.new_var();

    auto c0 = std::make_shared<linspire::constraint>();

    // x >= 0
    bool res0 = s.new_gt({{x, 1}}, 0, false, c0);
    assert(res0);
    // x >= 1 (we add this constraint with the same reason as the previous one)
    bool res1 = s.new_gt({{x, 1}}, 1, false, c0);
    assert(res1);

    auto cons = s.check();
    assert(cons);

    s.retract(c0);
    assert(s.lb(x) == utils::rational::negative_infinite);
    assert(s.ub(x) == utils::rational::positive_infinite);
}

/**
 * @brief Unit test for the linspire::solver class.
 *
 * This test verifies the following functionalities:
 * - Creating variables and adding greater-than constraints using new_gt.
 * - Ensuring that the constraints are satisfiable and checking the solver's consistency.
 * - Testing the solver's ability to retract constraints and add new ones, ensuring continued consistency.
 *
 * Assertions are used to ensure that each step behaves as expected.
 */
void test3()
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

/**
 * @brief Unit test for the linspire::solver class.
 *
 * This test verifies the following functionalities:
 * - Creating variables and adding consistent constraints using new_eq and new_lt.
 * - Ensuring that the constraints are satisfiable and checking the solver's consistency.
 * - Creating conflicting constraints and verifying that the solver detects inconsistency.
 * - Testing the solver's ability to generate a conflict explanation when inconsistency is detected.
 * - Retracting a conflicting constraint and ensuring the solver returns to a consistent state.
 *
 * Assertions are used to ensure that each step behaves as expected.
 */
void test4()
{
    linspire::solver s;
    auto x = s.new_var();
    auto y = s.new_var();

    auto c0 = std::make_shared<linspire::constraint>();
    auto c1 = std::make_shared<linspire::constraint>();
    auto c2 = std::make_shared<linspire::constraint>();

    // x + y >= 1
    bool res0 = s.new_gt({{x, 1}, {y, 1}}, 1, false, c0);
    assert(res0);
    // x >= 2
    bool res1 = s.new_gt({{x, 1}}, 2, false, c1);
    assert(res1);
    bool cons = s.check();
    assert(cons);

    // x + y <= 0
    bool res2 = s.new_lt({{x, 1}, {y, 1}}, 0, false, c2);
    assert(res2);
    cons = s.check();
    assert(!cons);
    auto expl = s.get_conflict();
    assert(expl.size() == 2);
    assert((expl[0] == c0 && expl[1] == c2) || (expl[0] == c2 && expl[1] == c0));

    s.retract(c0);
    cons = s.check();
    assert(cons);
}

int main()
{
    test0();
    test1();
    test2();
    test3();
    test4();

    return 0;
}
