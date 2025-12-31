#include "ValueRefsPythonModuleParser.h"

#include <boost/mpl/vector.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>

#include "ValueRefPythonParser.h"

namespace py = boost::python;

namespace {
    value_ref_wrapper<int> insert_empire_ships_destroyed(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        return value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            "EmpireShipsDestroyed",
            std::move(empire),
            nullptr,
            nullptr,
            nullptr,
            nullptr
        )); 
    }

    value_ref_wrapper<int> insert_ship_designs(std::string_view name, const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            empire = pyobject_to_vref<int>(kw["empire"]);
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> design;
        if (kw.has_key("design")) {
            design = pyobject_to_vref<std::string>(kw["design"]);
        }

        return value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            name.data(),
            std::move(empire),
            nullptr,
            nullptr,
            std::move(design),
            nullptr
        )); 
    }

    py::object insert_random_number_operation(const py::object& type_int, const py::object& type_float, const py::object& type, const py::object& min, const py::object& max) {
        if (type == type_int) {
            auto min_arg = pyobject_to_vref<int>(min);
            auto max_arg = pyobject_to_vref<int>(max);
            return py::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Operation<int>>(ValueRef::OpType::RANDOM_UNIFORM,
                std::move(min_arg),
                std::move(max_arg))));
        } else if (type == type_float) {
            auto min_arg = pyobject_to_vref_or_cast<double, int>(min);
            auto max_arg = pyobject_to_vref_or_cast<double, int>(max);
            return py::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::RANDOM_UNIFORM,
                std::move(min_arg),
                std::move(max_arg))));
        }

        auto error_str = std::string{"Unsupported type for RandomNumber: "} + py::extract<std::string>(py::str(type))() + " (" + __func__ + ")";
        throw std::runtime_error(error_str);
    }
}

BOOST_PYTHON_MODULE(_value_refs) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::def("EmpireShipsDestroyed", boost::python::raw_function(insert_empire_ships_destroyed));

    for (std::string_view name : {"ShipDesignsDestroyed",
                                  "ShipDesignsLost",
                                  "ShipDesignsInProduction",
                                  "ShipDesignsOwned",
                                  "ShipDesignsProduced",
                                  "ShipDesignsScrapped"})
    {
            const auto f_insert_ship_designs = [name](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_ship_designs(name, args, kw); };
            boost::python::def(name.data(), boost::python::raw_function(f_insert_ship_designs));
    }

    // free_variable_name : Double
    for (const char* variable : {"UniverseCentreX",
                                 "UniverseCentreY",
                                 "UniverseWidth"})
    {
        py::scope().attr(variable) = value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, variable));
    }

    const auto type_int = py::import("builtins").attr("int");
    const auto type_float = py::import("builtins").attr("float");

    py::def("RandomNumber", py::make_function(
        [type_int, type_float](const py::object& type, const py::object& min, const py::object& max) { return insert_random_number_operation(type_int, type_float, type, min, max); },
        py::default_call_policies(),
        boost::mpl::vector<py::object, const py::object&, const py::object&, const py::object&>()));
}

