#include "ValueRefsPythonModuleParser.h"

#include <boost/mpl/vector.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>

#include "../Empire/ResourcePool.h"
#include "../universe/NamedValueRefManager.h"

#include "PythonParser.h"
#include "EnumPythonParser.h"
#include "ValueRefPythonParser.h"

namespace py = boost::python;

namespace {
    value_ref_wrapper<std::vector<std::string>> insert_empire_adopted_policies_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }
        return value_ref_wrapper<std::vector<std::string>>(std::make_shared<ValueRef::ComplexVariable<std::vector<std::string>>>(
            "EmpireAdoptedPolicies",
            std::move(empire),
            nullptr,
            nullptr,
            nullptr,
            nullptr
        ));
    }

  value_ref_wrapper<int> insert_complex_i1(std::string&& vname, const boost::python::tuple& args, const boost::python::dict& kw, const std::string& keyint1) {
        std::unique_ptr<ValueRef::ValueRef<int>> int1;
        auto int1_args = boost::python::extract<value_ref_wrapper<int>>(kw[keyint1]);
        if (int1_args.check()) {
            int1 = ValueRef::CloneUnique(int1_args().value_ref);
        } else {
            int1 = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["keyint1"])());
        }

        return value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            std::move(vname),
            std::move(int1),
            nullptr,
            nullptr,
            nullptr,
            nullptr
        ));
    }

    value_ref_wrapper<int> insert_complex_i1_s1(std::string&& vname, const boost::python::tuple& args, const boost::python::dict& kw, const std::string& keyint1, const std::string& keystr1) {
        std::unique_ptr<ValueRef::ValueRef<int>> int1;
        if (kw.has_key(keyint1)) {
            int1 = pyobject_to_vref<int>(kw[keyint1]);
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> str1;
        if (kw.has_key(keystr1)) {
            str1 = pyobject_to_vref<std::string>(kw[keystr1]);
        }

        return value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            std::move(vname),
            std::move(int1),
            nullptr,
            nullptr,
            std::move(str1),
            nullptr
        ));
    }

    py::object insert_random_number_operation(const PythonTypes& types, const py::object& type, const py::object& min, const py::object& max) {
        if (type == types.type_int) {
            auto min_arg = pyobject_to_vref<int>(min);
            auto max_arg = pyobject_to_vref<int>(max);
            return py::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Operation<int>>(ValueRef::OpType::RANDOM_UNIFORM,
                std::move(min_arg),
                std::move(max_arg))));
        } else if (type == types.type_float) {
            auto min_arg = pyobject_to_vref_or_cast<double, int>(min);
            auto max_arg = pyobject_to_vref_or_cast<double, int>(max);
            return py::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::RANDOM_UNIFORM,
                std::move(min_arg),
                std::move(max_arg))));
        }

        auto error_str = std::string{"Unsupported type for RandomNumber: "} + py::extract<std::string>(py::str(type))() + " (" + __func__ + ")";
        throw std::runtime_error(error_str);
    }

    template <typename T>
    boost::python::object insert_reduce_vector_(const PythonTypes& types, const ValueRef::StatisticType type, const boost::python::tuple& args, const boost::python::dict& kw) {
        auto vector = boost::python::extract<value_ref_wrapper<std::vector<T>>>(args[1]);
        if (!vector.check()) {
            ErrorLogger() << "No vector vref in ReduceVector";
        }
        if (args[0] == types.type_int) {
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::ReduceVector<int,T>>(ValueRef::CloneUnique(vector().value_ref), type)));
        } else if (args[0] == types.type_float) {
            ErrorLogger() << "insert_reduce_vector_(parser.type_float) using untested functionality - remove this log entry after test";
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::ReduceVector<double,T>>(ValueRef::CloneUnique(vector().value_ref), type)));
        //} else if (args[0] == type_str) { // only supporting arithmetic T currently
        //    return boost::python::object(value_ref_wrapper<std::string>(std::make_shared<ValueRef::ReduceVector<std::string,T>>(nullptr, type)));
        } else {
            ErrorLogger() << "Unsupported type for ReduceVector : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    py::object insert_game_rule_(const PythonTypes& types, const py::tuple& args, const py::dict& kw) {
        auto name{py::extract<std::string>(kw["name"])()};
        auto type_ = kw["type"];

        if (type_ == types.type_int) {
            return py::object(value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
                "GameRule",
                nullptr,
                nullptr,
                nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(std::move(name)),
                nullptr)));
        } else if (type_ == types.type_float) {
            return py::object(value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
                "GameRule",
                nullptr,
                nullptr,
                nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(std::move(name)),
                nullptr)));
        } else {
            ErrorLogger() << "Unsupported type for rule " << name << ": "
                          << py::extract<std::string>(py::str(type_))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }
    }

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
        if (value_arg.check())
            value = ValueRef::CloneUnique(value_arg().value_ref);
        else
            value = std::make_unique<ValueRef::Constant<T>>(boost::python::extract<T>(kw["value"])());

        ::RegisterValueRef<T>(name, std::move(value));

        return value_ref_wrapper<T>(std::make_shared<ValueRef::NamedRef<T>>(name));
    }

    boost::python::object insert_minmaxoneof_(const PythonTypes& types, ValueRef::OpType op, const boost::python::tuple& args, const boost::python::dict& kw) {
        if (args[0] == types.type_int) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<int>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++)
                operands.push_back(pyobject_to_vref<int>(args[i]));
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Operation<int>>(op, std::move(operands))));
        } else if (args[0] == types.type_float) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<double>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++)
                operands.push_back(pyobject_to_vref_or_cast<double, int>(args[i]));
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Operation<double>>(op, std::move(operands))));
        } else if (args[0] == types.type_str) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++)
                operands.push_back(pyobject_to_vref<std::string>(args[i]));
            return boost::python::object(value_ref_wrapper<std::string>(std::make_shared<ValueRef::Operation<std::string>>(op, std::move(operands))));
        } else if (args[0] == "Visibility") {
            std::vector<std::unique_ptr<ValueRef::ValueRef<Visibility>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++)
                operands.push_back(pyobject_to_vref_enum<Visibility>(args[i]));
            return boost::python::object(value_ref_wrapper<Visibility>(std::make_shared<ValueRef::Operation<Visibility>>(op, std::move(operands))));
        } else if (args[0] == "PlanetType") {
            std::vector<std::unique_ptr<ValueRef::ValueRef<PlanetType>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++)
                operands.push_back(pyobject_to_vref_enum<PlanetType>(args[i]));
            return boost::python::object(value_ref_wrapper<PlanetType>(std::make_shared<ValueRef::Operation<PlanetType>>(op, std::move(operands))));
        } else if (args[0] == "PlanetSize") {
            std::vector<std::unique_ptr<ValueRef::ValueRef<PlanetSize>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++)
                operands.push_back(pyobject_to_vref_enum<PlanetSize>(args[i]));
            return boost::python::object(value_ref_wrapper<PlanetSize>(std::make_shared<ValueRef::Operation<PlanetSize>>(op, std::move(operands))));
        } else if (args[0] == "StarType") {
            std::vector<std::unique_ptr<ValueRef::ValueRef<StarType>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++)
                operands.push_back(pyobject_to_vref_enum<StarType>(args[i]));
            return boost::python::object(value_ref_wrapper<StarType>(std::make_shared<ValueRef::Operation<StarType>>(op, std::move(operands))));
        } else {
            ErrorLogger() << "Unsupported type for min/max/oneof : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    boost::python::object insert_1arg_(const PythonTypes& types, const ValueRef::OpType op, const boost::python::tuple& args, const boost::python::dict& kw) {
        if (args[0] == types.type_int) {
            std::unique_ptr<ValueRef::ValueRef<int>> operand;
            auto arg = boost::python::extract<value_ref_wrapper<int>>(args[1]);
            if (arg.check())
                operand = ValueRef::CloneUnique(arg().value_ref);
            else
                operand = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(args[1])());
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Operation<int>>(op, std::move(operand))));
        } else if (args[0] == types.type_float) {
            std::unique_ptr<ValueRef::ValueRef<double>> operand;
            auto arg = boost::python::extract<value_ref_wrapper<double>>(args[1]);
            if (arg.check())
                operand = ValueRef::CloneUnique(arg().value_ref);
            else
                operand = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(args[1])());
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Operation<double>>(op, std::move(operand))));
        } else if (args[0] == types.type_str) {
            std::unique_ptr<ValueRef::ValueRef<std::string>> operand;
            auto arg = boost::python::extract<value_ref_wrapper<std::string>>(args[1]);
            if (arg.check())
                operand = ValueRef::CloneUnique(arg().value_ref);
            else
                operand = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(args[1])());
            return boost::python::object(value_ref_wrapper<std::string>(std::make_shared<ValueRef::Operation<std::string>>(op, std::move(operand))));
        } else {
            ErrorLogger() << "Unsupported type for 1arg : " << boost::python::extract<std::string>(boost::python::str(args[0]))();
            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    boost::python::object insert_statistic_(const PythonTypes& types, const ValueRef::StatisticType type, const boost::python::tuple& args, const boost::python::dict& kw) {
        auto condition = boost::python::extract<condition_wrapper>(kw["condition"])();
        if (args[0] == types.type_int) {
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Statistic<int>>(nullptr, type, ValueRef::CloneUnique(condition.condition))));
        } else if (args[0] == types.type_float) {
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Statistic<double>>(nullptr, type, ValueRef::CloneUnique(condition.condition))));
        } else {
            ErrorLogger() << "Unsupported type for statistic : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    template<typename T>
    boost::python::object insert_statictic_value_return_typed(const PythonTypes& types,
                                                              const boost::python::object& return_type,
                                                              std::unique_ptr<ValueRef::ValueRef<T>>&& value,
                                                              ValueRef::StatisticType type,
                                                              std::unique_ptr<Condition::Condition>&& condition) 
    {
        if (return_type == types.type_int) {
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Statistic<int, T>>(std::move(value), type, std::move(condition))));
        } else if (return_type == types.type_float) {
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Statistic<double, T>>(std::move(value), type, std::move(condition))));
        } else if constexpr (std::is_same<T, std::string>::value) {
            if (return_type == types.type_str) {
                return boost::python::object(value_ref_wrapper<std::string>(std::make_shared<ValueRef::Statistic<std::string, T>>(std::move(value), type, std::move(condition))));
            }
        }

        ErrorLogger() << "Unsupported type for statistic : " << boost::python::extract<std::string>(boost::python::str(return_type))();
        throw std::runtime_error(std::string("Not implemented ") + __func__);

    }

    boost::python::object insert_statistic_value_(const PythonTypes& types, const boost::python::tuple& args, const boost::python::dict& kw) {
        const auto condition = boost::python::extract<condition_wrapper>(kw["condition"])().condition;

        const auto return_type = args[0];
        const auto args1 = boost::python::extract<enum_wrapper<ValueRef::StatisticType>>(args[1]);
        const auto value_type = args1.check() ? return_type : args[1];
        const auto type = args1.check() ? args1().value : boost::python::extract<enum_wrapper<ValueRef::StatisticType>>(args[2])().value;

        if (value_type == types.type_int) {
            const auto value_arg = boost::python::extract<value_ref_wrapper<int>>(kw["value"]);
            std::unique_ptr<ValueRef::ValueRef<int>> value;
            if (value_arg.check()) {
                value = ValueRef::CloneUnique(value_arg().value_ref);
            } else {
                value = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["value"])());
            }
            return insert_statictic_value_return_typed(types, return_type, std::move(value), type, ValueRef::CloneUnique(condition));
        } else if (value_type == types.type_float) {
            const auto value_arg = boost::python::extract<value_ref_wrapper<double>>(kw["value"]);
            std::unique_ptr<ValueRef::ValueRef<double>> value;
            if (value_arg.check()) {
                value = ValueRef::CloneUnique(value_arg().value_ref);
            } else {
                const auto value_int_arg = boost::python::extract<value_ref_wrapper<int>>(kw["value"]);
                if (value_int_arg.check()) {
                    value = std::make_unique<ValueRef::StaticCast<int, double>>(ValueRef::CloneUnique(value_int_arg().value_ref));
                } else {
                    value = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["value"])());
               }
            }
            return insert_statictic_value_return_typed(types, return_type, std::move(value), type, ValueRef::CloneUnique(condition));
        } else if (value_type == types.type_str) {
            const auto value_arg = boost::python::extract<value_ref_wrapper<std::string>>(kw["value"]);
            std::unique_ptr<ValueRef::ValueRef<std::string>> value;
            if (value_arg.check()) {
                value = ValueRef::CloneUnique(value_arg().value_ref);
            } else {
                value = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["value"])());
            }
            return insert_statictic_value_return_typed(types, return_type, std::move(value), type, ValueRef::CloneUnique(condition));
        }
        
        ErrorLogger() << "Unsupported type for statistic : " << boost::python::extract<std::string>(boost::python::str(return_type))() << ", " << boost::python::extract<std::string>(boost::python::str(value_type))();
        throw std::runtime_error(std::string("Not implemented ") + __func__);
    }

    boost::python::object insert_int_complex_variable_(const char* variable,
                                                       const boost::python::tuple& args,
                                                       const boost::python::dict& kw)
    {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name")) {
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }

        return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            variable,
            std::move(empire),
            nullptr,
            nullptr,
            std::move(name),
            nullptr)));
    }

    value_ref_wrapper<double> insert_double_complex_variable_(const char* variable, const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name")) {
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            variable,
            nullptr,
            nullptr,
            nullptr,
            std::move(name),
            nullptr));
    }

    value_ref_wrapper<double> insert_double_complex_variable_species_(const char* variable, const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> species;
        auto species_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["species"]);
        if (species_args.check()) {
            species = ValueRef::CloneUnique(species_args().value_ref);
        } else {
            species = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["species"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            variable,
            std::move(empire),
            nullptr,
            nullptr,
            std::move(species),
            nullptr));

    }

      value_ref_wrapper<double> insert_double_complex_variable_object_i1_name_s1_(const char* variable, const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> object;
        auto object_args = boost::python::extract<value_ref_wrapper<int>>(kw["object"]);
        if (object_args.check()) {
            object = ValueRef::CloneUnique(object_args().value_ref);
        } else {
            object = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["object"])());
        }

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            variable,
            std::move(object),
            nullptr,
            nullptr,
            std::move(name),
            nullptr));

    }

    value_ref_wrapper<double> insert_direct_distance_between_(boost::python::object arg1, boost::python::object arg2) {
        std::unique_ptr<ValueRef::ValueRef<int>> id1;
        auto id1_args = boost::python::extract<value_ref_wrapper<int>>(arg1);
        if (id1_args.check()) {
            id1 = ValueRef::CloneUnique(id1_args().value_ref);
        } else {
            id1 = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(arg1)());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> id2;
        auto id2_args = boost::python::extract<value_ref_wrapper<int>>(arg2);
        if (id2_args.check()) {
            id2 = ValueRef::CloneUnique(id2_args().value_ref);
        } else {
            id2 = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(arg2)());
        }

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            "DirectDistanceBetween",
            std::move(id1),
            std::move(id2),
            nullptr,
            nullptr,
            nullptr
        ));
    }

    value_ref_wrapper<double> insert_shortest_path_(boost::python::object arg1, boost::python::object arg2) {
        auto id1 = pyobject_to_vref<int>(arg1);
        auto id2 = pyobject_to_vref<int>(arg2);

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            "ShortestPathDistance",
            std::move(id1),
            std::move(id2),
            nullptr,
            nullptr,
            nullptr
        ));
    }

    value_ref_wrapper<int> insert_jumps_between_(boost::python::object arg1, boost::python::object arg2) {
        std::unique_ptr<ValueRef::ValueRef<int>> id1;
        auto id1_args = boost::python::extract<value_ref_wrapper<int>>(arg1);
        if (id1_args.check()) {
            id1 = ValueRef::CloneUnique(id1_args().value_ref);
        } else {
            id1 = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(arg1)());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> id2;
        auto id2_args = boost::python::extract<value_ref_wrapper<int>>(arg2);
        if (id2_args.check()) {
            id2 = ValueRef::CloneUnique(id2_args().value_ref);
        } else {
            id2 = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(arg2)());
        }

        return value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            "JumpsBetween",
            std::move(id1),
            std::move(id2),
            nullptr,
            nullptr,
            nullptr
        ));
    }

    value_ref_wrapper<std::string> insert_user_string(const boost::python::object& expr) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> expr_;
        auto expr_args = boost::python::extract<value_ref_wrapper<std::string>>(expr);
        if (expr_args.check()) {
            expr_ = ValueRef::CloneUnique(expr_args().value_ref);
        } else {
            expr_ = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(expr)());
        }


        return value_ref_wrapper<std::string>(std::make_shared<ValueRef::UserStringLookup<std::string>>(std::move(expr_)));
    }

    value_ref_wrapper<int> insert_parts_in_ship_design_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name")) {
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> design;
        auto design_args = boost::python::extract<value_ref_wrapper<int>>(kw["design"]);
        if (design_args.check()) {
            design = ValueRef::CloneUnique(design_args().value_ref);
        } else {
            design = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["design"])());
        }

        return value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            "PartsInShipDesign",
            std::move(design),
            nullptr,
            nullptr,
            std::move(name),
            nullptr
        ));
    }

    value_ref_wrapper<double> insert_ship_part_meter_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> part;
        if (kw.has_key("part")) {
            auto part_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["part"]);
            if (part_args.check()) {
                part = ValueRef::CloneUnique(part_args().value_ref);
            } else {
                part = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["part"])());
            }
        }

        auto meter_name = MeterToName(boost::python::extract<enum_wrapper<MeterType>>(kw["meter"])().value);
        auto meter_type = std::make_unique<ValueRef::Constant<std::string>>(std::string{meter_name});

        std::unique_ptr<ValueRef::ValueRef<int>> ship_id;
        auto ship_args = boost::python::extract<value_ref_wrapper<int>>(kw["ship"]);
        if (ship_args.check()) {
            ship_id = ValueRef::CloneUnique(ship_args().value_ref);
        } else {
            ship_id = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["ship"])());
        }

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            "ShipPartMeter",
            std::move(ship_id),
            nullptr,
            nullptr,
            std::move(part),
            std::move(meter_type)
        ));
    }

    value_ref_wrapper<double> insert_empire_meter_value_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        std::string meter = boost::python::extract<std::string>(kw["meter"])();

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            "EmpireMeterValue",
            std::move(empire),
            nullptr,
            nullptr,
            std::make_unique<ValueRef::Constant<std::string>>(meter),
            nullptr
        ));
    }

    value_ref_wrapper<double> insert_empire_stockpile_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }
        auto resource = boost::python::extract<enum_wrapper<ResourceType>>(kw["resource"])();
        std::string resource_str;
        switch (resource.value) {
            case ResourceType::RE_INFLUENCE:
                resource_str = "Influence";
                break;
            case ResourceType::RE_INDUSTRY:
                resource_str = "Industry";
                break;
            default:
                throw std::runtime_error(std::string("Not supported") + __func__);
        }

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            "EmpireStockpile",
            std::move(empire),
            nullptr,
            nullptr,
            std::make_unique<ValueRef::Constant<std::string>>(resource_str),
            nullptr
        ));
    }

    value_ref_wrapper<int> insert_planet_type_difference_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<PlanetType>> from;
        auto from_args = boost::python::extract<value_ref_wrapper<PlanetType>>(kw["from_"]);
        if (from_args.check()) {
            from = ValueRef::CloneUnique(from_args().value_ref);
        } else {
            from = std::make_unique<ValueRef::Constant<PlanetType>>(boost::python::extract<enum_wrapper<PlanetType>>(kw["from_"])().value);
        }

        std::unique_ptr<ValueRef::ValueRef<PlanetType>> to;
        auto to_args = boost::python::extract<value_ref_wrapper<PlanetType>>(kw["to"]);
        if (to_args.check()) {
            to = ValueRef::CloneUnique(to_args().value_ref);
        } else {
            to = std::make_unique<ValueRef::Constant<PlanetType>>(boost::python::extract<enum_wrapper<PlanetType>>(kw["to"])().value);
        }

        return value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            "PlanetTypeDifference",
            std::make_unique<ValueRef::StaticCast<::PlanetType, int>>(std::move(from)),
            std::make_unique<ValueRef::StaticCast<::PlanetType, int>>(std::move(to)),
            nullptr,
            nullptr,
            nullptr
        ));       
    }

    boost::python::object insert_const_(const PythonTypes& types, const boost::python::object& type, const boost::python::object& value) {
        if (type == types.type_int) {
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Constant<int>>(
                boost::python::extract<int>(value)
            )));
        } else if (type == types.type_float) {
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Constant<double>>(
                boost::python::extract<double>(value)
            )));
        } else {
            ErrorLogger() << "Unsupported type for const: "
                          << boost::python::extract<std::string>(boost::python::str(type))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }
    }
}

BOOST_PYTHON_MODULE(_value_refs) {
    boost::python::docstring_options doc_options(true, true, false);

    const auto f_insert_empire_ships_destroyed_ = [](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_complex_i1("EmpireShipsDestroyed", args, kw, "empire"); };
    boost::python::def("EmpireShipsDestroyed", boost::python::raw_function(f_insert_empire_ships_destroyed_));

    for (std::string_view name : {"ShipDesignsDestroyed",
                                  "ShipDesignsLost",
                                  "ShipDesignsInProduction",
                                  "ShipDesignsOwned",
                                  "ShipDesignsProduced",
                                  "ShipDesignsScrapped"})
    {
            const auto f_insert_ship_designs = [name](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_complex_i1_s1(name.data(),args,kw,"empire","design"); };
            boost::python::def(name.data(), boost::python::raw_function(f_insert_ship_designs));
    }

    const auto f_insert_num_part_classes_in_ship_design_ = [](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_complex_i1("NumPartClassesInShipDesign", args, kw, "design"); };
    boost::python::def("NumPartClassesInShipDesign", boost::python::raw_function(f_insert_num_part_classes_in_ship_design_));

    const auto f_insert_part_of_class_in_ship_design_ = [](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_complex_i1_s1("PartOfClassInShipDesign", args, kw, "design", "name"); };
    boost::python::def("PartOfClassInShipDesign", boost::python::raw_function(f_insert_part_of_class_in_ship_design_));

    // free_variable_name : Double
    for (const char* variable : {"UniverseCentreX",
                                 "UniverseCentreY",
                                 "UniverseWidth"})
    {
        py::scope().attr(variable) = value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, variable));
    }

    const PythonTypes types;

    py::def("EmpireAdoptedPolicies", boost::python::raw_function(insert_empire_adopted_policies_));

    py::def("RandomNumber", py::make_function(
        [types](const py::object& type, const py::object& min, const py::object& max) { return insert_random_number_operation(types, type, min, max); },
        py::default_call_policies(),
        boost::mpl::vector<py::object, const py::object&, const py::object&, const py::object&>()));

    const auto f_insert_vector_count = [types](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_reduce_vector_<std::string>(types, ValueRef::StatisticType::COUNT, args, kw); };
    py::def("VectorCount", boost::python::raw_function(f_insert_vector_count, 1));

    auto f_insert_game_rule = [types](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_game_rule_(types, args, kw); };
    py::def("GameRule", boost::python::raw_function(std::move(f_insert_game_rule)));

    py::def("NamedInteger", boost::python::raw_function(insert_named_<int>));
    py::def("NamedIntegerLookup", boost::python::raw_function(insert_named_lookup_<int>));
    py::def("NamedReal", boost::python::raw_function(insert_named_<double>));
    py::def("NamedRealLookup", boost::python::raw_function(insert_named_lookup_<double>));
    py::scope().attr("Value") = value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE));
    py::scope().attr("ValueVisibility") = value_ref_wrapper<Visibility>(std::make_shared<ValueRef::Variable<Visibility>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE));

    // free variable name
    for (const char* variable : {"CombatBout",
                                 "CurrentTurn",
                                 "GalaxyAge",
                                 "GalaxyMaxAIAggression",
                                 "GalaxyMonsterFrequency",
                                 "GalaxyNativeFrequency",
                                 "GalaxyPlanetDensity",
                                 "GalaxyShape",
                                 "GalaxySize",
                                 "GalaxySpecialFrequency",
                                 "GalaxyStarlaneFrequency",
                                 "UsedInDesignID"})
    {
        py::scope().attr(variable) = value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, variable));       
    }


    // Integer complex variables
    for (const char* variable : {"BuildingTypesOwned",
                                 "BuildingTypesProduced",
                                 "BuildingTypesScrapped",
                                 "SpeciesColoniesOwned",
                                 "SpeciesPlanetsBombed",
                                 "SpeciesPlanetsDepoped",
                                 "SpeciesPlanetsInvaded",
                                 "SpeciesShipsDestroyed",
                                 "SpeciesShipsLost",
                                 "SpeciesShipsOwned",
                                 "SpeciesShipsProduced",
                                 "SpeciesShipsScrapped",
                                 "TurnTechResearched",
                                 "TurnPolicyAdopted",
                                 "TurnsSincePolicyAdopted",
                                 "CumulativeTurnsPolicyAdopted",
                                 "LatestTurnPolicyAdopted",
                                 "NumPoliciesAdopted"})
    {
        const auto f_insert_int_complex_variable = [variable](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_int_complex_variable_(variable, args, kw); };
        py::def(variable, boost::python::raw_function(f_insert_int_complex_variable));
    }

    // name_property_rule
    for (const char* variable : {"HullFuel",
                                 "HullStructure",
                                 "HullStealth",
                                 "HullSpeed",
                                 "PartCapacity",
                                 "PartSecondaryStat"})
    {
        const auto f_insert_name_property = [variable](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_double_complex_variable_(variable, args, kw); };
        py::def(variable, boost::python::raw_function(f_insert_name_property));
    }

    // name_object_rule (?)
    for (const char* variable : {"SpecialCapacity"})
    {
        const auto f_insert_name_object = [variable](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_double_complex_variable_object_i1_name_s1_(variable, args, kw); };
        py::def(variable, boost::python::raw_function(f_insert_name_object));
    }


    // species_empire_opinion
    for (const char* variable : {"SpeciesEmpireOpinion",
                                 "SpeciesEmpireTargetOpinion"})
    {
        py::def(variable, boost::python::raw_function([variable](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_double_complex_variable_species_(variable, args, kw); }));
    }

    // single-parameter math functions
    for (const auto& op : std::initializer_list<std::pair<const char*, ValueRef::OpType>>{
            {"Sin",   ValueRef::OpType::SINE},
            {"Cos",   ValueRef::OpType::COSINE},
            {"Log",   ValueRef::OpType::LOGARITHM},
            {"NoOp",  ValueRef::OpType::NOOP},
            {"Abs",   ValueRef::OpType::ABS},
            {"Round", ValueRef::OpType::ROUND_NEAREST},
            {"Ceil",  ValueRef::OpType::ROUND_UP},
            {"Floor", ValueRef::OpType::ROUND_DOWN},
            {"Sign",  ValueRef::OpType::SIGN}})
    {
        py::def(op.first, boost::python::raw_function([types, op](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_1arg_(types, op.second, args, kw); }, 2));
    }

    // CurrentContent
    const auto current_content = value_ref_wrapper<std::string>(
        std::make_shared<ValueRef::Constant<std::string>>(std::string{ValueRef::Constant<std::string>::current_content}));
    for (const char* variable : {ValueRef::Constant<std::string>::current_content.data(),
                                 "ThisBuilding",
                                 "ThisField",
                                 "ThisHull",
                                 "ThisPart",  // various aliases for this reference in scripts, allowing scripter to use their preference
                                 "ThisPolicy",
                                 "ThisTech",
                                 "ThisSpecies",
                                 "ThisSpecial"})
    {
        py::scope().attr(variable) = current_content;
    }

    // selection_operator
    for (const auto& op : std::initializer_list<std::pair<const char*, ValueRef::OpType>>{
            {"OneOf",   ValueRef::OpType::RANDOM_PICK},
            {"MinOf",   ValueRef::OpType::MINIMUM},
            {"MaxOf",   ValueRef::OpType::MAXIMUM}})
    {
        const auto e = op.second;
        const auto f = [types, e](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_minmaxoneof_(types, e, args, kw); };
        py::def(op.first, boost::python::raw_function(f, 3));
    }
    const auto noop = ValueRef::OpType::NOOP;
    const auto xf = [types](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_1arg_(types, noop, args, kw); };
    py::def("NoOpValue", boost::python::raw_function(xf, 2)); // needs type and value like NoOpValue(int, 1)

    const auto f_insert_statistic_if = [types](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_statistic_(types, ValueRef::StatisticType::IF, args, kw); };
    py::def("StatisticIf", boost::python::raw_function(f_insert_statistic_if, 1));

    const auto f_insert_statistic_count = [types](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_statistic_(types, ValueRef::StatisticType::COUNT, args, kw); };
    py::def("StatisticCount", boost::python::raw_function(f_insert_statistic_count, 1));

    const auto f_insert_statistic = [types](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_statistic_value_(types, args, kw); };
    py::def("Statistic", boost::python::raw_function(f_insert_statistic, 2));

    py::def("DirectDistanceBetween", insert_direct_distance_between_);
    py::def("ShortestPath", insert_shortest_path_);
    py::def("JumpsBetween", insert_jumps_between_);
    py::def("UserString", insert_user_string);
    py::def("PartsInShipDesign", boost::python::raw_function(insert_parts_in_ship_design_));
    py::def("ShipPartMeter", boost::python::raw_function(insert_ship_part_meter_));
    py::def("EmpireMeterValue", boost::python::raw_function(insert_empire_meter_value_));
    py::def("EmpireStockpile", boost::python::raw_function(insert_empire_stockpile_));
    py::def("PlanetTypeDifference", boost::python::raw_function(insert_planet_type_difference_));
    py::def("Const", boost::python::make_function([types](const boost::python::object& type, const boost::python::object& value) { return insert_const_(types, type, value); },
        boost::python::default_call_policies(),
        boost::mpl::vector<boost::python::object, boost::python::object, boost::python::object>()));
}
