#include "linspire.hpp"
#include "logging.hpp"
#include <cassert>

namespace linspire
{
    utils::var solver::new_var(const utils::rational &lb, const utils::rational &ub) noexcept
    {
        vars.emplace_back(lb, ub);
        return vars.size() - 1;
    }

    bool solver::new_eq(const utils::lin &lhs, const utils::lin &rhs) noexcept
    {
        LOG_TRACE(utils::to_string(lhs) + " == " + utils::to_string(rhs));
        utils::lin expr = lhs - rhs;

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant..
            return is_zero(expr.known_term);
        case 1: // the expression is a single variable..
        {
            const auto [v, c] = *expr.vars.cbegin();
            assert(c != 0);
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term) / c; // the right-hand side of the constraint is the division of the negation of the known term by the coefficient..
            if (vars[v].lb > c_right || vars[v].ub < c_right)
                return false; // the variable's bounds are inconsistent with the constraint..
            // we can set both the lower and upper bound of the variable to the right-hand side of the constraint..
            vars[v].lb = c_right;
            vars[v].ub = c_right;
            return true;
        }
        default: // the expression is a general linear expression..
        {        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
            std::vector<utils::var> vars;
            vars.reserve(expr.vars.size());
            for ([[maybe_unused]] const auto &[v, c] : expr.vars)
                vars.push_back(v);
            for (const auto &v : vars)
                if (tableau.find(v) != tableau.cend())
                {
                    auto c = expr.vars.at(v);
                    expr.vars.erase(v);
                    expr += c * tableau.at(v);
                }
        }
        }
    }

    bool solver::new_lt(const utils::lin &lhs, const utils::lin &rhs, bool strict) noexcept
    {
        LOG_TRACE(utils::to_string(lhs) + (strict ? " < " : " <= ") + utils::to_string(rhs));
        utils::lin expr = lhs - rhs;

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant..
            return is_negative(expr.known_term) || (strict && is_zero(expr.known_term));
        case 1: // the expression is a single variable..
        {
            const auto [v, c] = *expr.vars.cbegin();
            assert(c != 0);
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0) / c; // the right-hand side of the constraint is the division of the negation of the known term minus an infinitesimal by the coefficient..
            if (is_positive(c))
            { // we are in the case `c * v < c_right`..
                if (vars[v].ub < c_right)
                    return false; // the variable's bounds are inconsistent with the constraint..
                if (vars[v].lb > c_right)
                    vars[v].lb = c_right; // we can only update the lower bound of the variable..
            }
            else
            { // we are in the case `c * v > c_right`..
                if (vars[v].lb > c_right)
                    return false; // the variable's bounds are inconsistent with the constraint..
                if (vars[v].ub < c_right)
                    vars[v].ub = c_right; // we can only update the upper bound of the variable..
            }
            return true;
        }
        default: // the expression is a general linear expression..
        {        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
            std::vector<utils::var> vars;
            vars.reserve(expr.vars.size());
            for ([[maybe_unused]] const auto &[v, c] : expr.vars)
                vars.push_back(v);
            for (const auto &v : vars)
                if (tableau.find(v) != tableau.cend())
                {
                    auto c = expr.vars.at(v);
                    expr.vars.erase(v);
                    expr += c * tableau.at(v);
                }
        }
        }
    }

    std::string solver::var::to_string() const noexcept
    {
        std::string str = utils::to_string(val) + " [";
        if (is_infinite(lb))
            str += "-inf";
        else
            str += utils::to_string(lb);
        str += ", ";
        if (is_infinite(ub))
            str += "+inf";
        else
            str += utils::to_string(ub);
        str += "]";
        return str;
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

    std::string to_string(const solver &s) noexcept
    {
        std::string str;
        for (utils::var i = 0; i < s.vars.size(); ++i)
            str += "x" + std::to_string(i) + " = " + s.vars[i].to_string() + "\n";
        return str;
    }

    json::json to_json(const solver &s) noexcept
    {
        json::json j(json::json_type::array);
        for (utils::var i = 0; i < s.vars.size(); ++i)
            j.push_back(s.vars[i].to_json());
        return j;
    }

    json::json to_json(const utils::rational &r) noexcept { return json::json{{"num", r.numerator()}, {"den", r.denominator()}}; }

    json::json to_json(const utils::inf_rational &r) noexcept
    {
        json::json j = to_json(r.get_rational());
        if (!is_zero(r.get_infinitesimal()))
            j["inf"] = to_json(r.get_infinitesimal());
        return j;
    }
} // namespace linspire
