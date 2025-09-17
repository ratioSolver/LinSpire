#include "linspire.hpp"
#include "logging.hpp"
#include <cassert>

namespace linspire
{
    utils::var solver::new_var(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
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
            // we can set both the lower and upper bound of the variable to the right-hand side of the constraint..
            return set_lb(v, c_right) && set_ub(v, c_right);
        }
        default: // the expression is a general linear expression..
        {        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
            std::vector<utils::var> vs;
            vs.reserve(expr.vars.size());
            for ([[maybe_unused]] const auto &[v, c] : expr.vars)
                vs.push_back(v);
            for (const auto &v : vs)
                if (tableau.find(v) != tableau.cend())
                {
                    auto c = expr.vars.at(v);
                    expr.vars.erase(v);
                    expr += c * tableau.at(v);
                }
            // after substituting the basic variables, we check if the expression is now a single variable or a constant
            switch (expr.vars.size())
            {
            case 0: // the expression is a constant..
                return is_zero(expr.known_term);
            case 1: // the expression is a single variable..
            {
                const auto [v, c] = *expr.vars.cbegin();
                assert(c != 0);
                const utils::inf_rational c_right = utils::inf_rational(-expr.known_term) / c; // the right-hand side of the constraint is the division of the negation of the known term by the coefficient..
                if (lb(expr) > c_right || ub(expr) < c_right)
                    return false; // the variable's bounds are inconsistent with the constraint..
                // we can set both the lower and upper bound of the variable to the right-hand side of the constraint..
                return set_lb(v, c_right) && set_ub(v, c_right);
            }
            default: // the expression is still a general linear expression..
                const utils::inf_rational c_right = utils::inf_rational(-expr.known_term);
                expr.known_term = utils::rational::zero;
                // we add the expression to the tableau, associating it with a new (slack) variable
                utils::var slack = new_var(lb(expr), ub(expr));
                tableau[slack] = expr;
                return set_lb(slack, c_right) && set_ub(slack, c_right);
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
                return set_lb(v, c_right); // we are in the case `c * v < c_right`..
            else
                return set_ub(v, c_right); // we are in the case `c * v > c_right`..
        }
        default: // the expression is a general linear expression..
        {        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
            std::vector<utils::var> vs;
            vs.reserve(expr.vars.size());
            for ([[maybe_unused]] const auto &[v, c] : expr.vars)
                vs.push_back(v);
            for (const auto &v : vs)
                if (tableau.find(v) != tableau.cend())
                {
                    auto c = expr.vars.at(v);
                    expr.vars.erase(v);
                    expr += c * tableau.at(v);
                }
            // after substituting the basic variables, we check if the expression is now a single variable or a constant
            switch (expr.vars.size())
            {
            case 0: // the expression is a constant..
                return is_negative(expr.known_term) || (strict && is_zero(expr.known_term));
            case 1: // the expression is a single variable..
            {
                const auto [v, c] = *expr.vars.cbegin();
                assert(c != 0);
                const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0) / c; // the right-hand side of the constraint is the division of the negation of the known term minus an infinitesimal by the coefficient
                if (is_positive(c))
                    return set_lb(v, c_right); // we are in the case `c * v < c_right`..
                else
                    return set_ub(v, c_right); // we are in the case `c * v > c_right`..
            }
            default: // the expression is still a general linear expression..
                const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0);
                expr.known_term = utils::rational::zero;
                // we add the expression to the tableau, associating it with a new (slack) variable
                utils::var slack = new_var(lb(expr), ub(expr));
                tableau[slack] = expr;
                return set_ub(slack, c_right); // we are in the case `expr < c_right`..
            }
        }
        }
    }

    bool solver::set_lb(const utils::var v, const utils::inf_rational &val) noexcept
    {
        assert(v < vars.size());
        if (val <= lb(v))
            return true; // no update needed..
        else if (val > ub(v))
            return false; // inconsistent bounds..
        vars[v].lb = val;
    }
    bool solver::set_ub(const utils::var v, const utils::inf_rational &val) noexcept
    {
        assert(v < vars.size());
        if (val >= ub(v))
            return true; // no update needed..
        else if (val < lb(v))
            return false; // inconsistent bounds..
        vars[v].ub = val;
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
        for (const auto &[v, expr] : s.tableau)
            str += "x" + std::to_string(v) + " = " + utils::to_string(expr) + "\n";
        return str;
    }

    json::json to_json(const solver &s) noexcept
    {
        json::json j;
        json::json j_vars;
        for (utils::var i = 0; i < s.vars.size(); ++i)
            j_vars["x" + std::to_string(i)] = s.vars[i].to_json();
        j["vars"] = j_vars;
        json::json j_tableau;
        for (const auto &[v, expr] : s.tableau)
            j_tableau["x" + std::to_string(v)] = to_json(expr);
        j["tableau"] = j_tableau;
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

    json::json to_json(const utils::lin &l) noexcept
    {
        json::json j;
        json::json j_vars;
        for (const auto &[v, c] : l.vars)
            j_vars["x" + std::to_string(v)] = to_json(c);
        j["vars"] = j_vars;
        j["known_term"] = to_json(l.known_term);
        return j;
    }
} // namespace linspire
