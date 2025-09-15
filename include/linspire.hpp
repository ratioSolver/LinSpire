#pragma once
#include "lin.hpp"
#include "inf_rational.hpp"
#include "json.hpp"
#include <vector>

namespace linspire
{
  class solver
  {
  public:
    [[nodiscard]] utils::var new_var(const utils::rational &lb = utils::rational::negative_infinite, const utils::rational &ub = utils::rational::positive_infinite) noexcept;

    void new_lt(const utils::lin &lhs, const utils::lin &rhs, bool strict = false);

  private:
    class var
    {
    public:
      var(const utils::rational &lb = utils::rational::negative_infinite, const utils::rational &ub = utils::rational::positive_infinite) : lb(lb), ub(ub) {}

      [[nodiscard]] json::json to_json() const noexcept;

    private:
      utils::inf_rational lb, ub, val; // lower bound, upper bound, value
    };

    std::vector<var> variables;               // index is the variable id
    std::map<utils::var, utils::lin> tableau; // variable -> expression
  };

  json::json to_json(const utils::inf_rational &r) noexcept;
} // namespace linspire
