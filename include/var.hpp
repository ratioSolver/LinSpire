#pragma once

#include "lin.hpp"
#include "inf_rational.hpp"
#include "json.hpp"
#include <set>
#include <memory>

namespace linspire
{
  class constraint;

  class var
  {
    friend class solver;

  public:
    var(const utils::inf_rational &lb = utils::inf_rational(utils::rational::negative_infinite), const utils::inf_rational &ub = utils::inf_rational(utils::rational::positive_infinite)) noexcept;

    [[nodiscard]] utils::inf_rational get_lb() const noexcept;
    [[nodiscard]] utils::inf_rational get_ub() const noexcept;
    [[nodiscard]] utils::inf_rational get_val() const noexcept { return val; }

    friend std::string to_string(const var &x) noexcept;
    friend json::json to_json(const var &x) noexcept;

  private:
    void set_lb(const utils::inf_rational &v, std::shared_ptr<const constraint> reason = nullptr) noexcept;
    void unset_lb(const utils::inf_rational &v, std::shared_ptr<const constraint> reason) noexcept;
    void set_ub(const utils::inf_rational &v, std::shared_ptr<const constraint> reason = nullptr) noexcept;
    void unset_ub(const utils::inf_rational &v, std::shared_ptr<const constraint> reason) noexcept;

  private:
    std::map<utils::inf_rational, std::set<std::shared_ptr<const constraint>>> lbs, ubs; // the lower and upper bounds with their reasons..
    utils::inf_rational val;                                                             // the current value of this variable..
  };

  [[nodiscard]] std::string to_string(const var &x) noexcept;
  [[nodiscard]] json::json to_json(const var &x) noexcept;
} // namespace linspire
