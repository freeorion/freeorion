#include "ValueRefsPythonModuleParser.h"

#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>

#include "ValueRefPythonParser.h"

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

    value_ref_wrapper<int> insert_ship_designs_lost(const boost::python::tuple& args, const boost::python::dict& kw) {
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
            "ShipDesignsLost",
            std::move(empire),
            nullptr,
            nullptr,
            nullptr,
            nullptr
        )); 
    }

    value_ref_wrapper<int> insert_ship_designs_produced(const boost::python::tuple& args, const boost::python::dict& kw) {
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
            "ShipDesignsProduced",
            std::move(empire),
            nullptr,
            nullptr,
            nullptr,
            nullptr
        )); 
    }

    value_ref_wrapper<int> insert_ship_designs_scrapped(const boost::python::tuple& args, const boost::python::dict& kw) {
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
            "ShipDesignsScrapped",
            std::move(empire),
            nullptr,
            nullptr,
            nullptr,
            nullptr
        )); 
    }
}

BOOST_PYTHON_MODULE(_value_refs) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::def("EmpireShipsDestroyed", boost::python::raw_function(insert_empire_ships_destroyed));
    boost::python::def("ShipDesignsLost", boost::python::raw_function(insert_ship_designs_lost));
    boost::python::def("ShipDesignsProduced", boost::python::raw_function(insert_ship_designs_produced));
    boost::python::def("ShipDesignsScrapped", boost::python::raw_function(insert_ship_designs_scrapped));
}

