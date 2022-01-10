#include "ValueRefPythonParser.h"

#include <stdexcept>

#include <boost/python/extract.hpp>
#include <boost/python/str.hpp>
#include <boost/python/raw_function.hpp>

#include "../universe/ValueRefs.h"
#include "../universe/NamedValueRefManager.h"
#include "../universe/Conditions.h"

#include "PythonParser.h"

value_ref_wrapper<double> operator*(int lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::Constant<double>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, int rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, double rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator-(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::MINUS,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

condition_wrapper operator<=(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::LESS_THAN_OR_EQUAL,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator<(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::LESS_THAN,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator==(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::EQUAL,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}


namespace {
    template<typename T>
    value_ref_wrapper<T> insert_named_lookup_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();

        return value_ref_wrapper<T>(std::make_shared<ValueRef::NamedRef<T>>(name, /*is_lookup_only*/true));
    }

    template<typename T>
    value_ref_wrapper<T> insert_named_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        std::unique_ptr<ValueRef::ValueRef<T>> value;
        
        auto value_arg = boost::python::extract<value_ref_wrapper<T>>(kw["value"]);
        if (value_arg.check()) {
            value = ValueRef::CloneUnique(value_arg().value_ref);
        } else {
            value = std::make_unique<ValueRef::Constant<T>>(boost::python::extract<T>(kw["value"])());
        }

        ::RegisterValueRef<T>(name, std::move(value));

        return value_ref_wrapper<T>(std::make_shared<ValueRef::NamedRef<T>>(name));
    }


    template <ValueRef::OpType O>
    boost::python::object insert_minmaxoneof_(const PythonParser& parser, const boost::python::tuple& args, const boost::python::dict& kw) {
        if (args[0] == parser.type_int) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<int>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++) {
                auto arg = boost::python::extract<value_ref_wrapper<int>>(args[i]);
                if (arg.check())
                    operands.push_back(ValueRef::CloneUnique(arg().value_ref));
                else
                    operands.push_back(std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(args[i])()));
            }
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Operation<int>>(O, std::move(operands))));
        } else if (args[0] == parser.type_float) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<double>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++) {
                auto arg = boost::python::extract<value_ref_wrapper<double>>(args[i]);
                if (arg.check())
                    operands.push_back(ValueRef::CloneUnique(arg().value_ref));
                else
                    operands.push_back(std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(args[i])()));
            }
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Operation<double>>(O, std::move(operands))));
        } else {
            ErrorLogger() << "Unsupported type for min/max/oneof : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    boost::python::object insert_game_rule_(const PythonParser& parser, const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto type_ = kw["type"];

        if (type_ == parser.type_int) {
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
                "GameRule",
                nullptr,
                nullptr,
                nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(name),
                nullptr)));
        } else if (type_ == parser.type_float) {
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
                "GameRule",
                nullptr,
                nullptr,
                nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(name),
                nullptr)));
        } else {
            ErrorLogger() << "Unsupported type for rule " << name << ": " << boost::python::extract<std::string>(boost::python::str(type_))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }
}

void RegisterGlobalsValueRefs(boost::python::dict& globals, const PythonParser& parser) {
    globals["NamedReal"] = boost::python::raw_function(insert_named_<double>);
    globals["NamedRealLookup"] = boost::python::raw_function(insert_named_lookup_<double>);
    globals["Value"] = value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE));
    globals["CurrentTurn"] = value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, "CurrentTurn"));

    std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_game_rule = [&parser](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_game_rule_(parser, args, kw); };
    globals["GameRule"] = boost::python::raw_function(f_insert_game_rule);
    std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_min = [&parser](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_minmaxoneof_<ValueRef::OpType::MINIMUM>(parser, args, kw); };
    globals["Min"] = boost::python::raw_function(f_insert_min, 3);
    std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_max = [&parser](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_minmaxoneof_<ValueRef::OpType::MAXIMUM>(parser, args, kw); };
    globals["Max"] = boost::python::raw_function(f_insert_max, 3);
}

