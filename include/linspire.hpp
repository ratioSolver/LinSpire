#pragma once

#include "var.hpp"
#include <set>
#include <unordered_map>

namespace linspire
{
  class solver
  {
    friend class constraint;

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
     * @brief Creates a new variable defined by a linear expression.
     *
     * This function constructs a new variable that is defined by a given linear expression.
     * The linear expression specifies the relationship between the new variable and other variables.
     *
     * @param l The linear expression defining the new variable.
     * @return A new instance of utils::var representing the variable.
     */
    [[nodiscard]] utils::var new_var(utils::lin &&l) noexcept;

    /**
     * @brief Returns the lower bound of the specified variable.
     *
     * This function retrieves the lower bound (`lb`) associated with the variable `x`.
     *
     * @param x The variable whose lower bound is to be retrieved.
     * @return utils::inf_rational The lower bound of the variable.
     */
    [[nodiscard]] utils::inf_rational lb(const utils::var x) const noexcept;
    /**
     * @brief Returns the upper bound of the specified variable.
     *
     * This function retrieves the upper bound (`ub`) associated with the variable `x`.
     *
     * @param x The variable whose upper bound is to be retrieved.
     * @return utils::inf_rational The upper bound of the variable.
     */
    [[nodiscard]] utils::inf_rational ub(const utils::var x) const noexcept;
    /**
     * @brief Returns the current value of the specified variable.
     *
     * This function retrieves the current value (`val`) associated with the variable `x`.
     *
     * @param x The variable whose current value is to be retrieved.
     * @return utils::inf_rational The current value of the variable.
     */
    [[nodiscard]] utils::inf_rational val(const utils::var x) const noexcept;
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
     * @param reason An optional shared pointer to a constraint object that serves as the reason for this constraint.
     * @return true if the constraint was successfully added; false if it leads to inconsistency.
     */
    [[nodiscard]] bool new_lt(const utils::lin &lhs, const utils::lin &rhs, bool strict = false, std::shared_ptr<constraint> reason = nullptr) noexcept;
    /**
     * @brief Adds a new equality constraint to the solver.
     *
     * This function adds a new equality constraint between two linear expressions to the solver.
     * It ensures that the two expressions are equal by adding appropriate constraints.
     *
     * @param lhs The left-hand side linear expression of the equality constraint.
     * @param rhs The right-hand side linear expression of the equality constraint.
     * @param reason An optional shared pointer to a constraint object that serves as the reason for this equality constraint.
     * @return true if the equality constraint was successfully added; false if it leads to inconsistency.
     */
    [[nodiscard]] bool new_eq(const utils::lin &lhs, const utils::lin &rhs, std::shared_ptr<constraint> reason = nullptr) noexcept;
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
     * @param reason An optional shared pointer to a constraint object that serves as the reason for this constraint.
     * @return true if the constraint was successfully added; false if it leads to inconsistency.
     */
    [[nodiscard]] bool new_gt(const utils::lin &lhs, const utils::lin &rhs, bool strict = false, std::shared_ptr<constraint> reason = nullptr) noexcept { return new_lt(rhs, lhs, strict, reason); }

    /**
     * @brief Retracts a previously added constraint from the solver.
     *
     * This function removes a previously added constraint from the solver. It updates the
     * internal state of the solver to reflect the removal of the constraint.
     *
     * @param c A shared pointer to the constraint object to be retracted.
     */
    void retract(const std::shared_ptr<constraint> c) noexcept;

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

    /**
     * @brief Retrieves the last conflict explanation.
     *
     * This function returns a reference to the vector containing the last conflict explanation.
     * The conflict explanation consists of a set of constraints that led to an inconsistency
     * in the solver.
     *
     * @return A constant reference to a vector of shared pointers to constraint objects representing the last conflict explanation.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<constraint>> &get_conflict() const noexcept { return cnfl; }

    friend std::string to_string(const solver &s) noexcept;
    friend json::json to_json(const solver &s) noexcept;

  private:
    [[nodiscard]] bool is_basic(const utils::var v) const noexcept { return tableau.count(v); }

    [[nodiscard]] bool set_lb(const utils::var x_i, const utils::inf_rational &v, std::shared_ptr<constraint> reason = nullptr) noexcept;
    [[nodiscard]] bool set_ub(const utils::var x_i, const utils::inf_rational &v, std::shared_ptr<constraint> reason = nullptr) noexcept;

    void update(const utils::var x_i, const utils::inf_rational &v) noexcept;

    void pivot_and_update(const utils::var x_i, const utils::var x_j, const utils::inf_rational &v) noexcept;

    void pivot(const utils::var x_i, const utils::var x_j) noexcept;

    void new_row(const utils::var x, utils::lin &&l) noexcept;

    std::vector<var> vars;                             // index is the variable id
    std::unordered_map<std::string, utils::var> exprs; // the expressions (string to numeric variable) for which already exist slack variables..
    std::map<utils::var, utils::lin> tableau;          // variable -> expression
    std::vector<std::set<utils::var>> t_watches;       // for each variable `v`, a set of tableau rows watching `v`..
    std::vector<std::shared_ptr<constraint>> cnfl;     // the last conflict explanation..
  };

  class constraint
  {
    friend class solver;

  private:
    std::map<utils::var, utils::inf_rational> lbs, ubs;
  };

  [[nodiscard]] std::string to_string(const solver &s) noexcept;
  [[nodiscard]] json::json to_json(const solver &s) noexcept;
  [[nodiscard]] json::json to_json(const utils::rational &r) noexcept;
  [[nodiscard]] json::json to_json(const utils::inf_rational &r) noexcept;
  [[nodiscard]] json::json to_json(const utils::lin &l) noexcept;
} // namespace linspire
