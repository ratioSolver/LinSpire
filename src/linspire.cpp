#include "linspire.hpp"
#include "logging.hpp"
#include <algorithm>
#include <cassert>

namespace linspire
{
    utils::var solver::new_var(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
    {
        assert(lb <= ub);
        const auto x = vars.size();
        vars.emplace_back(lb, ub);
        t_watches.emplace_back();
        return x;
    }

    utils::var solver::new_var(utils::lin &&l) noexcept
    {
        assert(l.vars.size() > 1);
        const auto s_expr = to_string(l);
        if (const auto it = exprs.find(s_expr); it != exprs.cend()) // we already have a slack variable for this expression..
            return it->second;
        // we create a new slack variable for this expression..
        utils::var slack = new_var(lb(l), ub(l));
        vars[slack].val = val(l);
        exprs.emplace(s_expr, slack);
        new_row(slack, std::move(l));
        return slack;
    }

    bool solver::new_eq(const utils::lin &lhs, const utils::lin &rhs) noexcept
    {
        LOG_TRACE(utils::to_string(lhs) + " == " + utils::to_string(rhs));
        utils::lin expr = lhs - rhs;
        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
        std::vector<utils::var> vs;
        vs.reserve(expr.vars.size());
        for ([[maybe_unused]] const auto &[x, c] : expr.vars)
            vs.push_back(x);
        for (const auto &v : vs)
            if (tableau.find(v) != tableau.cend())
            {
                auto c = expr.vars.at(v);
                expr.vars.erase(v);
                expr += c * tableau.at(v);
            }

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant..
            return is_zero(expr.known_term);
        case 1: // the expression is a single variable..
        {
            const auto [x, c] = *expr.vars.cbegin();
            assert(c != 0);
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term) / c; // the right-hand side of the constraint is the division of the negation of the known term by the coefficient..
            // we can set both the lower and upper bound of the variable to the right-hand side of the constraint..
            return set_lb(x, c_right) && set_ub(x, c_right);
        }
        default: // the expression is still a general linear expression..
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term);
            expr.known_term = utils::rational::zero;
            // we add the expression to the tableau, associating it with a new (slack) variable
            utils::var slack = new_var(std::move(expr));
            return set_lb(slack, c_right) && set_ub(slack, c_right);
        }
    }

    bool solver::new_lt(const utils::lin &lhs, const utils::lin &rhs, bool strict) noexcept
    {
        LOG_TRACE(utils::to_string(lhs) + (strict ? " < " : " <= ") + utils::to_string(rhs));
        utils::lin expr = lhs - rhs;
        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
        std::vector<utils::var> vs;
        vs.reserve(expr.vars.size());
        for ([[maybe_unused]] const auto &[x, c] : expr.vars)
            vs.push_back(x);
        for (const auto &x : vs)
            if (tableau.find(x) != tableau.cend())
            {
                auto c = expr.vars.at(x);
                expr.vars.erase(x);
                expr += c * tableau.at(x);
            }

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant..
            return is_negative(expr.known_term) || (strict && is_zero(expr.known_term));
        case 1: // the expression is a single variable..
        {
            const auto [x, c] = *expr.vars.cbegin();
            assert(c != 0);
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0) / c; // the right-hand side of the constraint is the division of the negation of the known term minus an infinitesimal by the coefficient..
            if (is_positive(c))
                return set_ub(x, c_right); // we are in the case `c * v < c_right`..
            else
                return set_lb(x, c_right); // we are in the case `c * v > c_right`..
        }
        default: // the expression is still a general linear expression..
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0);
            expr.known_term = utils::rational::zero;
            // we add the expression to the tableau, associating it with a new (slack) variable
            utils::var slack = new_var(std::move(expr));
            return set_ub(slack, c_right); // we are in the case `expr < c_right`..
        }
    }

    bool solver::check() noexcept
    {
        while (true)
        {
            // we search for a variable whose value is not within its bounds..
            const auto &x_i_it = std::find_if(tableau.cbegin(), tableau.cend(), [this](const auto &v)
                                              { return val(v.first) < lb(v.first) || val(v.first) > ub(v.first); });
            if (x_i_it == tableau.cend())
                return true;                // all the variables are within their bounds..
            const auto x_i = x_i_it->first; // we select the variable `x_i`..
            const auto &l = x_i_it->second; // we select the linear expression `x_i = ...`..
            if (val(x_i) < lb(x_i))
            { // the value of `x_i` is below its lower bound..
                const auto &x_j_it = std::find_if(l.vars.cbegin(), l.vars.cend(), [l, this](const std::pair<utils::var, utils::rational> &v)
                                                  { return (is_positive(l.vars.at(v.first)) && val(v.first) < ub(v.first)) || (is_negative(l.vars.at(v.first)) && val(v.first) > lb(v.first)); });
                if (x_j_it != l.vars.cend()) // var x_j can be used to increase the value of x_i..
                    pivot_and_update(x_i, x_j_it->first, lb(x_i));
                else // no var x_j can be used to increase the value of x_i, so the constraints are inconsistent..
                    return false;
            }
            else if (val(x_i) > ub(x_i))
            { // the value of `x_i` is above its upper bound..
                const auto &x_j_it = std::find_if(l.vars.cbegin(), l.vars.cend(), [l, this](const std::pair<utils::var, utils::rational> &v)
                                                  { return (is_positive(l.vars.at(v.first)) && val(v.first) > lb(v.first)) || (is_negative(l.vars.at(v.first)) && val(v.first) < ub(v.first)); });
                if (x_j_it != l.vars.cend()) // var x_j can be used to decrease the value of x_i..
                    pivot_and_update(x_i, x_j_it->first, ub(x_i));
                else // no var x_j can be used to decrease the value of x_i, so the constraints are inconsistent..
                    return false;
            }
        }
    }

    bool solver::set_lb(const utils::var x, const utils::inf_rational &v) noexcept
    {
        assert(x < vars.size());
        LOG_TRACE("x" << std::to_string(x) << " = " << utils::to_string(val(x)) << " [" << utils::to_string(lb(x)) << " -> " << utils::to_string(v) << ", " << utils::to_string(ub(x)) << "]");
        if (v <= lb(x))
            return true; // no update needed..
        else if (v > ub(x))
            return false; // inconsistent bounds..
        vars[x].lb = v;
        if (val(x) < v && !is_basic(x))
            update(x, v);
        return true;
    }
    bool solver::set_ub(const utils::var x, const utils::inf_rational &v) noexcept
    {
        assert(x < vars.size());
        LOG_TRACE("x" << std::to_string(x) << " = " << utils::to_string(val(x)) << " [" << utils::to_string(lb(x)) << ", " << utils::to_string(v) << " <- " << utils::to_string(ub(x)) << "]");
        if (v >= ub(x))
            return true; // no update needed..
        else if (v < lb(x))
            return false; // inconsistent bounds..
        vars[x].ub = v;
        if (val(x) > v && !is_basic(x))
            update(x, v);
        return true;
    }

    void solver::update(const utils::var x_i, const utils::inf_rational &v) noexcept
    {
        assert(x_i < vars.size());
        assert(!is_basic(x_i));
        assert(v >= lb(x_i) && v <= ub(x_i));

        // the tableau rows containing `x_i` as a non-basic variable..
        for (const auto &x_j : t_watches[x_i])
        { // x_j = x_j + a_ji(v - x_i)..
            LOG_TRACE("x" << std::to_string(x_j) << " = " << utils::to_string(val(x_j)) << " -> " << utils::to_string(val(x_j) + tableau.at(x_j).vars.at(x_i) * (v - vars.at(x_i).val)) << " [" << utils::to_string(lb(x_j)) << ", " << utils::to_string(ub(x_j)) << "]");
            vars[x_j].val += tableau.at(x_j).vars.at(x_i) * (v - vars.at(x_i).val);
        }

        LOG_TRACE("x" << std::to_string(x_i) << " = " << utils::to_string(val(x_i)) << " -> " << utils::to_string(v) << " [" << utils::to_string(lb(x_i)) << ", " << utils::to_string(ub(x_i)) << "]");
        vars[x_i].val = v;
    }

    void solver::pivot_and_update(const utils::var x_i, const utils::var x_j, const utils::inf_rational &v) noexcept
    {
        assert(x_i < vars.size());
        assert(x_j < vars.size());
        assert(is_basic(x_i));
        assert(!is_basic(x_j));
        assert(tableau.at(x_i).vars.count(x_j));
        assert(v >= lb(x_j) && v <= ub(x_j));

        const auto theta = (v - val(x_i)) / tableau.at(x_i).vars.at(x_j);
        LOG_TRACE("x" << std::to_string(x_i) << " = " << utils::to_string(val(x_i)) << " -> " << utils::to_string(v) << " [" << utils::to_string(lb(x_i)) << ", " << utils::to_string(ub(x_i)) << "]");
        vars[x_i].val = v;
        LOG_TRACE("x" << std::to_string(x_j) << " = " << utils::to_string(val(x_j)) << " -> " << utils::to_string(val(x_j) + theta) << " [" << utils::to_string(lb(x_j)) << ", " << utils::to_string(ub(x_j)) << "]");
        vars[x_j].val += theta;

        // the tableau rows containing `x_j` as a non-basic variable..
        for (const auto &x_k : t_watches[x_j])
            if (x_k != x_i)
            { // x_k += a_kj * theta..
                LOG_TRACE("x" << std::to_string(x_k) << " = " << utils::to_string(val(x_k)) << " -> " << utils::to_string(val(x_k) + tableau.at(x_k).vars.at(x_j) * theta) << " [" << utils::to_string(lb(x_k)) << ", " << utils::to_string(ub(x_k)) << "]");
                vars[x_k].val += tableau.at(x_k).vars.at(x_j) * theta;
            }

        pivot(x_i, x_j);
    }

    void solver::pivot(const utils::var x_i, const utils::var x_j) noexcept
    {
        assert(x_i < vars.size());
        assert(x_j < vars.size());
        assert(is_basic(x_i));
        assert(!is_basic(x_j));
        assert(tableau.at(x_i).vars.count(x_j));
        assert(t_watches.at(x_i).empty());

        // we remove the row from the watches
        for ([[maybe_unused]] const auto &[v, c] : tableau[x_i].vars)
        {
            assert(t_watches.at(v).count(x_i));
            t_watches.at(v).erase(x_i);
        }

        // we rewrite `x_i = ...` as `x_j = ...`
        utils::lin l = std::move(tableau.at(x_i));
        utils::rational cc = l.vars.at(x_j);
        l.vars.erase(x_j);
        l /= -cc;
        l.vars.emplace(x_i, utils::rational::one / cc);
        tableau.erase(x_i);

        // we update the rows that contain `x_j`
        for (auto &r : t_watches.at(x_j))
        {
            auto &c_l = tableau.at(r);
            assert(c_l.known_term == utils::rational::zero);
            cc = c_l.vars.at(x_j);
            c_l.vars.erase(x_j);
            for (const auto &[v, c] : l.vars)
                if (const auto trm_it = c_l.vars.find(v); trm_it == c_l.vars.cend()) // `v` is not in the linear expression of `r`, so we add it
                {
                    c_l.vars.emplace(v, c * cc); // we add `c * cc` to the linear expression of `r`
                    t_watches[v].insert(r);      // we add `r` to the watches of `v`
                }
                else
                {
                    trm_it->second += c * cc;
                    if (trm_it->second == 0) // if the coefficient of `v` is zero, we remove the term from the linear expression
                    {
                        c_l.vars.erase(trm_it); // we remove `v` from the linear expression of `r`
                        t_watches[v].erase(r);  // we remove `r` from the watches of `v`
                    }
                }
            LOG_TRACE("x" << std::to_string(r) << " = " << to_string(c_l));
        }
        t_watches.at(x_j).clear();

        // we add the new row `y = ...`
        new_row(x_j, std::move(l));
    }

    void solver::new_row(const utils::var x, utils::lin &&l) noexcept
    {
        assert(x < vars.size());
        assert(!is_basic(x));
        LOG_TRACE("x" << std::to_string(x) << " = " << utils::to_string(l));
        for (const auto &[v, _] : l.vars)
            t_watches.at(v).insert(x);
        tableau.emplace(x, std::move(l));
    }

    std::string solver::var::to_string() const noexcept { return utils::to_string(val) + " [" + utils::to_string(lb) + ", " + utils::to_string(ub) + "]"; }

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
