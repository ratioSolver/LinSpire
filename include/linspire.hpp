#pragma once
#include "lin.hpp"
#include "inf_rational.hpp"
#include "json.hpp"
#include <set>

namespace linspire
{
  class solver
  {
  public:
    /**
     * @brief Creates a new variable with specified lower and upper bounds.
     *
     * This function constructs a new variable, optionally specifying its lower and upper bounds.
     * By default, the bounds are set to negative and positive infinity, respectively.
     *
     * @param lb The lower bound of the variable (default: negative infinity).
     * @param ub The upper bound of the variable (default: positive infinity).
     * @return A new instance of utils::var representing the variable.
     */
    [[nodiscard]] utils::var new_var(const utils::inf_rational &lb = utils::inf_rational(utils::rational::negative_infinite), const utils::inf_rational &ub = utils::inf_rational(utils::rational::positive_infinite)) noexcept;

    /**
     * @brief Returns the lower bound of the specified variable.
     *
     * This function retrieves the lower bound (`lb`) associated with the variable `x`.
     *
     * @param x The variable whose lower bound is to be retrieved.
     * @return utils::inf_rational The lower bound of the variable.
     */
    [[nodiscard]] inline utils::inf_rational lb(const utils::var x) const noexcept { return vars[x].lb; }
    /**
     * @brief Returns the upper bound of the specified variable.
     *
     * This function retrieves the upper bound (`ub`) associated with the variable `x`.
     *
     * @param x The variable whose upper bound is to be retrieved.
     * @return utils::inf_rational The upper bound of the variable.
     */
    [[nodiscard]] inline utils::inf_rational ub(const utils::var x) const noexcept { return vars[x].ub; }
    /**
     * @brief Returns the current value of the specified variable.
     *
     * This function retrieves the current value (`val`) associated with the variable `x`.
     *
     * @param x The variable whose current value is to be retrieved.
     * @return utils::inf_rational The current value of the variable.
     */
    [[nodiscard]] inline utils::inf_rational val(const utils::var x) const noexcept { return vars[x].val; }
    /**
     * @brief Returns the lower bound of a linear expression.
     *
     * This function computes the lower bound of a linear expression `l` by evaluating its terms
     * and considering the bounds of the variables involved.
     *
     * @param l The linear expression whose lower bound is to be computed.
     * @return utils::inf_rational The computed lower bound of the linear expression.
     */
    [[nodiscard]] inline utils::inf_rational lb(const utils::lin &l) const noexcept
    {
      utils::inf_rational b(l.known_term);
      for (const auto &[v, c] : l.vars)
        b += (is_positive(c) ? lb(v) * c : ub(v) * c);
      return b;
    }
    /**
     * @brief Returns the upper bound of a linear expression.
     *
     * This function computes the upper bound of a linear expression `l` by evaluating its terms
     * and considering the bounds of the variables involved.
     *
     * @param l The linear expression whose upper bound is to be computed.
     * @return utils::inf_rational The computed upper bound of the linear expression.
     */
    [[nodiscard]] inline utils::inf_rational ub(const utils::lin &l) const noexcept
    {
      utils::inf_rational b(l.known_term);
      for (const auto &[v, c] : l.vars)
        b += (is_positive(c) ? ub(v) * c : lb(v) * c);
      return b;
    }
    /**
     * @brief Returns the current value of a linear expression.
     *
     * This function computes the current value of a linear expression `l` by evaluating its terms
     * using the current values of the variables involved.
     *
     * @param l The linear expression whose current value is to be computed.
     * @return utils::inf_rational The computed current value of the linear expression.
     */
    [[nodiscard]] inline utils::inf_rational val(const utils::lin &l) const noexcept
    {
      utils::inf_rational b(l.known_term);
      for (const auto &[v, c] : l.vars)
        b += val(v) * c;
      return b;
    }

    /**
     * @brief Adds a new constraint to the solver.
     *
     * This function adds a new constraint to the solver, which can be either a less-than or
     * less-than-or-equal-to constraint between two linear expressions. The `strict` parameter
     * determines whether the constraint is strict (i.e., `<`) or non-strict (i.e., `<=`).
     *
     * @param lhs The left-hand side linear expression of the constraint.
     * @param rhs The right-hand side linear expression of the constraint.
     * @param strict A boolean indicating whether the constraint is strict (default: false).
     * @return true if the constraint was successfully added; false if it leads to inconsistency.
     */
    [[nodiscard]] bool new_lt(const utils::lin &lhs, const utils::lin &rhs, bool strict = false) noexcept;
    /**
     * @brief Adds a new equality constraint to the solver.
     *
     * This function adds a new equality constraint between two linear expressions to the solver.
     * It ensures that the two expressions are equal by adding appropriate constraints.
     *
     * @param lhs The left-hand side linear expression of the equality constraint.
     * @param rhs The right-hand side linear expression of the equality constraint.
     * @return true if the equality constraint was successfully added; false if it leads to inconsistency.
     */
    [[nodiscard]] bool new_eq(const utils::lin &lhs, const utils::lin &rhs) noexcept;
    /**
     * @brief Adds a new greater-than or greater-than-or-equal-to constraint to the solver.
     *
     * This function adds a new constraint to the solver, which can be either a greater-than or
     * greater-than-or-equal-to constraint between two linear expressions. The `strict` parameter
     * determines whether the constraint is strict (i.e., `>`) or non-strict (i.e., `>=`).
     *
     * @param lhs The left-hand side linear expression of the constraint.
     * @param rhs The right-hand side linear expression of the constraint.
     * @param strict A boolean indicating whether the constraint is strict (default: false).
     * @return true if the constraint was successfully added; false if it leads to inconsistency.
     */
    [[nodiscard]] bool new_gt(const utils::lin &lhs, const utils::lin &rhs, bool strict = false) noexcept { return new_lt(rhs, lhs, strict); }

    /**
     * @brief Checks the consistency of the current set of constraints.
     *
     * This function checks whether the current set of constraints in the solver is consistent.
     * It attempts to find a feasible solution that satisfies all the constraints. If a solution
     * is found, the function returns true; otherwise, it returns false, indicating that the
     * constraints are inconsistent.
     *
     * @return true if the constraints are consistent and a solution is found; false otherwise.
     */
    [[nodiscard]] bool check() noexcept;

    friend std::string to_string(const solver &s) noexcept;
    friend json::json to_json(const solver &s) noexcept;

  private:
    [[nodiscard]] bool is_basic(const utils::var v) const noexcept { return tableau.count(v); }

    [[nodiscard]] bool set_lb(const utils::var x, const utils::inf_rational &v) noexcept;
    [[nodiscard]] bool set_ub(const utils::var x, const utils::inf_rational &v) noexcept;

    void update(const utils::var x, const utils::inf_rational &v) noexcept;

    void pivot_and_update(const utils::var x, const utils::var y, const utils::inf_rational &v) noexcept;

    void pivot(const utils::var x, const utils::var y) noexcept;

    void new_row(const utils::var x, utils::lin &&l) noexcept;

    class var
    {
      friend class solver;

    public:
      var(const utils::inf_rational &lb = utils::inf_rational(utils::rational::negative_infinite), const utils::inf_rational &ub = utils::inf_rational(utils::rational::positive_infinite)) : lb(lb), ub(ub) {}

      [[nodiscard]] std::string to_string() const noexcept;
      [[nodiscard]] json::json to_json() const noexcept;

    private:
      utils::inf_rational lb, ub, val; // lower bound, upper bound, value
    };

    std::vector<var> vars;                       // index is the variable id
    std::map<utils::var, utils::lin> tableau;    // variable -> expression
    std::vector<std::set<utils::var>> t_watches; // for each variable `v`, a set of tableau rows watching `v`..
  };

  [[nodiscard]] std::string to_string(const solver &s) noexcept;
  [[nodiscard]] json::json to_json(const solver &s) noexcept;
  [[nodiscard]] json::json to_json(const utils::rational &r) noexcept;
  [[nodiscard]] json::json to_json(const utils::inf_rational &r) noexcept;
  [[nodiscard]] json::json to_json(const utils::lin &l) noexcept;
} // namespace linspire
