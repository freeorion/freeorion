#include "ConditionPythonParser.h"

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>

#include "../universe/Conditions.h"

#include "EnumPythonParser.h"
#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"

namespace {
    template <typename T, typename... Args>
        requires std::is_base_of_v<Condition::Condition, std::decay_t<T>>
    auto make_wrapped(Args&&... args)
    { return condition_wrapper(std::make_shared<T>(std::forward<Args>(args)...)); }

    template <typename T>
    auto make_constant(auto&& arg)
    {
        if constexpr (std::is_enum_v<T>) {
            auto val = boost::python::extract<enum_wrapper<T>>(std::forward<decltype(arg)>(arg))().value;
            return std::make_unique<ValueRef::Constant<T>>(val);
        } else {
            return std::make_unique<ValueRef::Constant<T>>(std::forward<decltype(arg)>(arg));
        }
    }
}

condition_wrapper operator&(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    auto lhs_cond = std::dynamic_pointer_cast<const Condition::ValueTest>(lhs.condition);
    auto rhs_cond = std::dynamic_pointer_cast<const Condition::ValueTest>(rhs.condition);

    if (lhs_cond && rhs_cond) {
        const auto lhs_vals = lhs_cond->ValuesDouble();
        const auto rhs_vals = rhs_cond->ValuesDouble();

        if (!lhs_vals[2] && !rhs_vals[2]) {
            // Match (A < X) & (X < B) and convert it to (A < X < B)
            if (lhs_vals[1] && rhs_vals[0] && (*lhs_vals[1] == *rhs_vals[0])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals[0] ? lhs_vals[0]->Clone() : nullptr,
                    lhs_cond->CompareTypes()[0],
                    lhs_vals[1]->Clone(),
                    rhs_cond->CompareTypes()[0],
                    rhs_vals[1] ? rhs_vals[1]->Clone() : nullptr
                );
            // Match (X > A) & (X < B) and convert it to (A < X < B)
            } else if (lhs_vals[0] && rhs_vals[0] && (*lhs_vals[0] == *rhs_vals[0])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals[1] ? lhs_vals[1]->Clone() : nullptr,
                    Condition::ReverseComparisonType(lhs_cond->CompareTypes()[0]),
                    lhs_vals[0]->Clone(),
                    rhs_cond->CompareTypes()[0],
                    rhs_vals[1] ? rhs_vals[1]->Clone() : nullptr
                );
            // Match (A < X) & (B > X) and convert it to (A < X < B)
            } else if (lhs_vals[1] && rhs_vals[1] && (*lhs_vals[1] == *rhs_vals[1])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals[0] ? lhs_vals[0]->Clone() : nullptr,
                    lhs_cond->CompareTypes()[0],
                    lhs_vals[1]->Clone(),
                    Condition::ReverseComparisonType(rhs_cond->CompareTypes()[0]),
                    rhs_vals[0] ? rhs_vals[0]->Clone() : nullptr
                );
            // Match (X > A) & (B > X) and convert it to (A < X < B)
            } else if (lhs_vals[0] && rhs_vals[1] && (*lhs_vals[0] == *rhs_vals[1])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals[1] ? lhs_vals[1]->Clone() : nullptr,
                    Condition::ReverseComparisonType(lhs_cond->CompareTypes()[0]),
                    lhs_vals[0]->Clone(),
                    Condition::ReverseComparisonType(rhs_cond->CompareTypes()[0]),
                    rhs_vals[0] ? rhs_vals[0]->Clone() : nullptr
                );
            }
        }

        const auto lhs_vals_i = lhs_cond->ValuesInt();
        const auto rhs_vals_i = rhs_cond->ValuesInt();

        if (!lhs_vals_i[2] && !rhs_vals_i[2]) {
            if (lhs_vals_i[1] && rhs_vals_i[0] && (*lhs_vals_i[1] == *rhs_vals_i[0])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals_i[0] ? lhs_vals_i[0]->Clone() : nullptr,
                    lhs_cond->CompareTypes()[0],
                    lhs_vals_i[1]->Clone(),
                    rhs_cond->CompareTypes()[0],
                    rhs_vals_i[1] ? rhs_vals_i[1]->Clone() : nullptr
                );
            } else if (lhs_vals_i[0] && rhs_vals_i[0] && (*lhs_vals_i[0] == *rhs_vals_i[0])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals_i[1] ? lhs_vals_i[1]->Clone() : nullptr,
                    Condition::ReverseComparisonType(lhs_cond->CompareTypes()[0]),
                    lhs_vals_i[0]->Clone(),
                    rhs_cond->CompareTypes()[0],
                    rhs_vals_i[1] ? rhs_vals_i[1]->Clone() : nullptr
                );
            } else if (lhs_vals_i[1] && rhs_vals_i[1] && (*lhs_vals_i[1] == *rhs_vals_i[1])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals_i[0] ? lhs_vals_i[0]->Clone() : nullptr,
                    lhs_cond->CompareTypes()[0],
                    lhs_vals_i[1]->Clone(),
                    Condition::ReverseComparisonType(rhs_cond->CompareTypes()[0]),
                    rhs_vals_i[0] ? rhs_vals_i[0]->Clone() : nullptr
                );
            } else if (lhs_vals_i[0] && rhs_vals_i[1] && (*lhs_vals_i[0] == *rhs_vals_i[1])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals_i[1] ? lhs_vals_i[1]->Clone() : nullptr,
                    Condition::ReverseComparisonType(lhs_cond->CompareTypes()[0]),
                    lhs_vals_i[0]->Clone(),
                    Condition::ReverseComparisonType(rhs_cond->CompareTypes()[0]),
                    rhs_vals_i[0] ? rhs_vals_i[0]->Clone() : nullptr
                );
            }
        }

        const auto lhs_vals_s = lhs_cond->ValuesString();
        const auto rhs_vals_s = rhs_cond->ValuesString();

        if (!lhs_vals_s[2] && !rhs_vals_s[2]) {
            if (lhs_vals_s[1] && rhs_vals_s[0] && (*lhs_vals_s[1] == *rhs_vals_s[0])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals_s[0] ? lhs_vals_s[0]->Clone() : nullptr,
                    lhs_cond->CompareTypes()[0],
                    lhs_vals_s[1]->Clone(),
                    rhs_cond->CompareTypes()[0],
                    rhs_vals_s[1] ? rhs_vals_s[1]->Clone() : nullptr
                );
            } else if (lhs_vals_s[0] && rhs_vals_s[0] && (*lhs_vals_s[0] == *rhs_vals_s[0])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals_s[1] ? lhs_vals_s[1]->Clone() : nullptr,
                    Condition::ReverseComparisonType(lhs_cond->CompareTypes()[0]),
                    lhs_vals_s[0]->Clone(),
                    rhs_cond->CompareTypes()[0],
                    rhs_vals_s[1] ? rhs_vals_s[1]->Clone() : nullptr
                );
            } else if (lhs_vals_s[1] && rhs_vals_s[1] && (*lhs_vals_s[1] == *rhs_vals_s[1])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals_s[0] ? lhs_vals_s[0]->Clone() : nullptr,
                    lhs_cond->CompareTypes()[0],
                    lhs_vals_s[1]->Clone(),
                    Condition::ReverseComparisonType(rhs_cond->CompareTypes()[0]),
                    rhs_vals_s[0] ? rhs_vals_s[0]->Clone() : nullptr
                );
            } else if (lhs_vals_s[0] && rhs_vals_s[1] && (*lhs_vals_s[0] == *rhs_vals_s[1])) {
                return make_wrapped<Condition::ValueTest>(
                    lhs_vals_s[1] ? lhs_vals_s[1]->Clone() : nullptr,
                    Condition::ReverseComparisonType(lhs_cond->CompareTypes()[0]),
                    lhs_vals_s[0]->Clone(),
                    Condition::ReverseComparisonType(rhs_cond->CompareTypes()[0]),
                    rhs_vals_s[0] ? rhs_vals_s[0]->Clone() : nullptr
                );
            }
        }
    }

    return make_wrapped<Condition::And>(lhs.condition->Clone(), rhs.condition->Clone());
}

condition_wrapper operator&(const condition_wrapper& lhs, const value_ref_wrapper<double>& rhs)
{ return lhs & rhs.operator condition_wrapper(); }

condition_wrapper operator&(const value_ref_wrapper<double>& lhs, const condition_wrapper& rhs)
{ return lhs.operator condition_wrapper() & rhs; }

condition_wrapper operator&(const condition_wrapper& lhs, const value_ref_wrapper<int>& rhs)
{ return lhs & rhs.operator condition_wrapper(); }

condition_wrapper operator&(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs)
{ return lhs.operator condition_wrapper() & rhs.operator condition_wrapper(); }

condition_wrapper operator&(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs)
{ return lhs.operator condition_wrapper() & rhs.operator condition_wrapper(); }

condition_wrapper operator&(const value_ref_wrapper<int>& lhs, const condition_wrapper& rhs)
{ return lhs.operator condition_wrapper() & rhs; }


condition_wrapper operator|(const condition_wrapper& lhs, const condition_wrapper& rhs)
{ return make_wrapped<Condition::Or>(lhs.condition->Clone(), rhs.condition->Clone()); }

condition_wrapper operator|(const condition_wrapper& lhs, const value_ref_wrapper<int>& rhs)
{ return lhs | rhs.operator condition_wrapper(); }

condition_wrapper operator|(const value_ref_wrapper<int>& lhs, const condition_wrapper& rhs)
{ return lhs.operator condition_wrapper() | rhs; }

condition_wrapper operator|(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs)
{ return lhs.operator condition_wrapper() | rhs.operator condition_wrapper(); }

condition_wrapper operator~(const condition_wrapper& lhs)
{ return make_wrapped<Condition::Not>(lhs.condition->Clone()); }

condition_wrapper operator~(const value_ref_wrapper<double>& lhs)
{ return ~(lhs.operator condition_wrapper()); }

condition_wrapper operator~(const value_ref_wrapper<int>& lhs)
{ return ~(lhs.operator condition_wrapper()); }
