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
void test_basic_eq_and_lt()
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
void test_detect_inconsistent_bounds()
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
void test_shared_reason_retraction()
{
    linspire::solver s;
    auto x = s.new_var();

    linspire::constraint c0;

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
void test_chained_retraction()
{
    linspire::solver s;
    auto x = s.new_var();
    auto y = s.new_var();
    auto z = s.new_var();

    linspire::constraint c0;
    linspire::constraint c1;

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
void test_conflict_explanation_generation()
{
    linspire::solver s;
    auto x = s.new_var();
    auto y = s.new_var();

    linspire::constraint c0;
    linspire::constraint c1;
    linspire::constraint c2;

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
    assert((&expl[0].get() == &c0 && &expl[1].get() == &c2) || (&expl[0].get() == &c2 && &expl[1].get() == &c0));

    s.retract(c0);
    cons = s.check();
    assert(cons);
}

/**
 * @brief Unit test for adding, retracting, and re-adding a constraint.
 *
 * This test verifies:
 * - Adding a constraint and checking consistency.
 * - Retracting the constraint and ensuring bounds are reset.
 * - Adding the same constraint again and checking consistency.
 */
void test_add_retract_readd_constraint()
{
    linspire::solver s;
    auto x = s.new_var();

    linspire::constraint c0;

    // Add constraint: x >= 5
    bool res0 = s.new_gt({{x, 1}}, 5, false, c0);
    assert(res0);
    bool cons = s.check();
    assert(cons);
    assert(s.lb(x) == 5);
    assert(s.ub(x) == utils::rational::positive_infinite);
    assert(s.val(x) >= 5);

    // Retract constraint
    s.retract(c0);
    cons = s.check();
    assert(cons);
    assert(s.lb(x) == utils::rational::negative_infinite);
    assert(s.ub(x) == utils::rational::positive_infinite);

    // Add the same constraint again
    bool added_again = s.add_constraint(c0);
    assert(added_again);
    cons = s.check();
    assert(cons);
    assert(s.lb(x) == 5);
    assert(s.ub(x) == utils::rational::positive_infinite);
    assert(s.val(x) >= 5);
}

void test_constant_inequality_strictness()
{
    linspire::solver s;

    bool non_strict = s.new_lt(0, 0, false);
    assert(non_strict);

    bool strict = s.new_lt(0, 0, true);
    assert(!strict);
}

void test_slack_variable_reuse_for_duplicate_expression()
{
    linspire::solver s;
    auto x = s.new_var();
    auto y = s.new_var();

    auto slack1 = s.new_var(utils::lin{{x, 1}, {y, -1}});
    auto slack2 = s.new_var(utils::lin{{x, 1}, {y, -1}});

    assert(slack1 == slack2);
    assert(s.lb(slack1) == utils::rational::negative_infinite);
    assert(s.ub(slack1) == utils::rational::positive_infinite);
}

void test_expression_bounds_and_match()
{
    linspire::solver s;
    auto x = s.new_var();

    bool ge0 = s.new_gt({{x, 1}}, 0);
    assert(ge0);
    bool le10 = s.new_lt({{x, 1}}, 10);
    assert(le10);
    assert(s.check());

    utils::lin expr_x{{{x, 1}}};
    utils::lin expr_shifted{{{x, 1}}, -5};
    utils::lin expr_far{{{x, 1}}, 20};

    assert(s.lb(expr_shifted) == -5);
    assert(s.ub(expr_shifted) == 5);

    assert(s.match(expr_x, expr_shifted));
    assert(!s.match(expr_x, expr_far));
}

void test_add_constraint_inconsistency_detection()
{
    linspire::solver s;
    auto x = s.new_var();

    linspire::constraint c_lb;
    bool lb_ok = s.new_gt({{x, 1}}, 5, false, c_lb);
    assert(lb_ok);
    s.retract(c_lb);

    linspire::constraint c_ub;
    bool ub_ok = s.new_lt({{x, 1}}, 1, false, c_ub);
    assert(ub_ok);
    assert(s.check());

    bool add_ok = s.add_constraint(c_lb);
    assert(!add_ok);
    assert(s.lb(x) == utils::rational::negative_infinite);
    assert(s.ub(x) == 1);
}

int main()
{
    test_basic_eq_and_lt();
    test_detect_inconsistent_bounds();
    test_shared_reason_retraction();
    test_chained_retraction();
    test_conflict_explanation_generation();
    test_add_retract_readd_constraint();
    test_constant_inequality_strictness();
    test_slack_variable_reuse_for_duplicate_expression();
    test_expression_bounds_and_match();
    test_add_constraint_inconsistency_detection();

    return 0;
}
