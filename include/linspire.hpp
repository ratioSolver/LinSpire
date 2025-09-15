#pragma once
#include "lin.hpp"
#include "inf_rational.hpp"
#include <vector>

namespace linspire
{
  class var
  {
  public:
    var(const utils::rational &lb = utils::rational::negative_infinite, const utils::rational &ub = utils::rational::positive_infinite) : lb(lb), ub(ub) {}

  private:
    utils::inf_rational lb, ub, val; // lower bound, upper bound, value
  };

  class solver
  {
  private:
    std::vector<var> variables;
  };
} // namespace linspire
