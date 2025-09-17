#pragma once
#include "lin.hpp"
#include "inf_rational.hpp"
#include "json.hpp"

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
    [[nodiscard]] utils::var new_var(const utils::rational &lb = utils::rational::negative_infinite, const utils::rational &ub = utils::rational::positive_infinite) noexcept;

    [[nodiscard]] bool new_eq(const utils::lin &lhs, const utils::lin &rhs) noexcept;
    [[nodiscard]] bool new_lt(const utils::lin &lhs, const utils::lin &rhs, bool strict = false) noexcept;

    friend std::string to_string(const solver &s) noexcept;
    friend json::json to_json(const solver &s) noexcept;

  private:
    class var
    {
      friend class solver;

    public:
      var(const utils::rational &lb = utils::rational::negative_infinite, const utils::rational &ub = utils::rational::positive_infinite) : lb(lb), ub(ub) {}

      [[nodiscard]] std::string to_string() const noexcept;
      [[nodiscard]] json::json to_json() const noexcept;

    private:
      utils::inf_rational lb, ub, val; // lower bound, upper bound, value
    };

    std::vector<var> vars;                    // index is the variable id
    std::map<utils::var, utils::lin> tableau; // variable -> expression
  };

  [[nodiscard]] std::string to_string(const solver &s) noexcept;
  [[nodiscard]] json::json to_json(const solver &s) noexcept;
  [[nodiscard]] json::json to_json(const utils::rational &r) noexcept;
  [[nodiscard]] json::json to_json(const utils::inf_rational &r) noexcept;
} // namespace linspire
