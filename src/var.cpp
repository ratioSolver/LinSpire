#include "var.hpp"
#include "linspire.hpp"
#include <cassert>

namespace linspire
{
    var::var(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept { assert(lb < ub); }

    utils::inf_rational var::get_lb() const noexcept { return lbs.empty() ? utils::rational::negative_infinite : lbs.rbegin()->first; }
    utils::inf_rational var::get_ub() const noexcept { return ubs.empty() ? utils::rational::positive_infinite : ubs.begin()->first; }

    void var::set_lb(const utils::inf_rational &v, std::optional<std::reference_wrapper<constraint>> reason) noexcept
    {
        assert(v <= get_ub()); // we cannot set a lower bound greater than the current upper bound..
        if (reason)
        { // we add a new lower bound `v` with the given reason..
            if (auto it = lbs.find(v); it != lbs.end())
                it->second.insert(&reason->get()); // we add the reason to the existing lower bound `v`..
            else
                lbs.emplace(v, std::set<constraint *>{&reason->get()}); // we create a new lower bound `v`..
        }
        else
        { // we remove all the lower bounds that are less than `v`..
            auto it = lbs.upper_bound(v);
            lbs.erase(lbs.begin(), it);
            lbs.emplace(v, std::set<constraint *>());
        }
    }
    void var::unset_lb(const utils::inf_rational &v, constraint &reason) noexcept
    {
        assert(lbs.find(v) != lbs.end());
        auto it = lbs.find(v);
        it->second.erase(&reason);
        if (it->second.empty())
            lbs.erase(it);
    }

    void var::set_ub(const utils::inf_rational &v, std::optional<std::reference_wrapper<constraint>> reason) noexcept
    {
        assert(v >= get_lb()); // we cannot set an upper bound less than the current lower bound..
        if (reason)
        { // we add a new upper bound `v` with the given reason..
            if (auto it = ubs.find(v); it != ubs.end())
                it->second.insert(&reason->get()); // we add the reason to the existing upper bound `v`..
            else
                ubs.emplace(v, std::set<constraint *>{&reason->get()}); // we create a new upper bound `v`..
        }
        else
        { // we remove all the upper bounds that are greater than `v`..
            auto it = ubs.lower_bound(v);
            ubs.erase(it, ubs.end());
            ubs.emplace(v, std::set<constraint *>());
        }
    }
    void var::unset_ub(const utils::inf_rational &v, constraint &reason) noexcept
    {
        assert(ubs.find(v) != ubs.end());
        auto it = ubs.find(v);
        it->second.erase(&reason);
        if (it->second.empty())
            ubs.erase(it);
    }

    std::string to_string(const var &x) noexcept { return utils::to_string(x.val) + " [" + utils::to_string(x.get_lb()) + ", " + utils::to_string(x.get_ub()) + "]"; }

    json::json to_json(const var &x) noexcept
    {
        json::json j = to_json(x.val);
        if (!x.lbs.empty())
            j["lb"] = to_json(x.lbs.rbegin()->first);
        if (!x.ubs.empty())
            j["ub"] = to_json(x.ubs.begin()->first);
        return j;
    }
} // namespace linspire
