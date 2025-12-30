#include "EffectsPythonModuleParser.h"

#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>

#include "EffectPythonParser.h"
#include "ValueRefPythonParser.h"

#include "../universe/Effects.h"

namespace py = boost::python;

namespace {
    effect_wrapper insert_move_in_orbit(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<double>> speed = pyobject_to_vref_or_cast<double, int>(kw["speed"]);

        if (kw.has_key("focus")) {
            auto focus = py::extract<condition_wrapper>(kw["focus"])();
            return effect_wrapper(std::make_shared<Effect::MoveInOrbit>(std::move(speed),
                ValueRef::CloneUnique(focus.condition)));
        } else if (kw.has_key("x") && kw.has_key("y")) {
            std::unique_ptr<ValueRef::ValueRef<double>> x = pyobject_to_vref_or_cast<double, int>(kw["x"]);
            std::unique_ptr<ValueRef::ValueRef<double>> y = pyobject_to_vref_or_cast<double, int>(kw["y"]);
            return effect_wrapper(std::make_shared<Effect::MoveInOrbit>(std::move(speed),
                std::move(x),
                std::move(y)));
        }

        throw std::runtime_error(std::string("Not implemented in ") + __func__);
    }
}

BOOST_PYTHON_MODULE(_effects_new) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::def("MoveInOrbit", boost::python::raw_function(insert_move_in_orbit));
}


