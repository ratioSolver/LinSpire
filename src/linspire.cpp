#include "linspire.hpp"

namespace linspire
{
    utils::var solver::new_var(const utils::rational &lb, const utils::rational &ub) noexcept
    {
        variables.emplace_back(lb, ub);
        return variables.size() - 1;
    }

    json::json solver::var::to_json() const noexcept
    {
        json::json j = linspire::to_json(val);
        if (!is_infinite(lb))
            j["lb"] = linspire::to_json(lb);
        if (!is_infinite(ub))
            j["ub"] = linspire::to_json(ub);
        return j;
    }

    json::json to_json(const utils::inf_rational &r) noexcept
    {
        json::json j;
        j["num"] = r.get_rational().numerator();
        j["den"] = r.get_rational().denominator();
        if (!is_zero(r.get_infinitesimal()))
        {
            json::json jinf;
            jinf["num"] = r.get_infinitesimal().numerator();
            jinf["den"] = r.get_infinitesimal().denominator();
            j["inf"] = jinf;
        }
        return j;
    }
} // namespace linspire
