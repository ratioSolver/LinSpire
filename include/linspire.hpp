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

    [[nodiscard]] inline utils::inf_rational lb(const utils::var x) const noexcept { return vars[x].lb; }
    [[nodiscard]] inline utils::inf_rational ub(const utils::var x) const noexcept { return vars[x].ub; }
    [[nodiscard]] inline utils::inf_rational val(const utils::var x) const noexcept { return vars[x].val; }
    [[nodiscard]] inline utils::inf_rational lb(const utils::lin &l) const noexcept
    {
      utils::inf_rational b(l.known_term);
      for (const auto &[v, c] : l.vars)
        b += (is_positive(c) ? lb(v) * c : ub(v) * c);
      return b;
    }
    [[nodiscard]] inline utils::inf_rational ub(const utils::lin &l) const noexcept
    {
      utils::inf_rational b(l.known_term);
      for (const auto &[v, c] : l.vars)
        b += (is_positive(c) ? ub(v) * c : lb(v) * c);
      return b;
    }
    [[nodiscard]] inline utils::inf_rational val(const utils::lin &l) const noexcept
    {
      utils::inf_rational b(l.known_term);
      for (const auto &[v, c] : l.vars)
        b += val(v) * c;
      return b;
    }

    [[nodiscard]] bool new_lt(const utils::lin &lhs, const utils::lin &rhs, bool strict = false) noexcept;
    [[nodiscard]] bool new_eq(const utils::lin &lhs, const utils::lin &rhs) noexcept;
    [[nodiscard]] bool new_gt(const utils::lin &lhs, const utils::lin &rhs, bool strict = false) noexcept { return new_lt(rhs, lhs, strict); }

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
