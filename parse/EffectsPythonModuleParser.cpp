#include "EffectsPythonModuleParser.h"

#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>

#include "EnumPythonParser.h"
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

        auto error_str = std::string{"Either focus or x, y should be set for MoveInOrbit ("} + __func__ + ")";
        throw std::runtime_error(error_str);
    }

    effect_wrapper insert_create_planet(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<PlanetType>> type = pyobject_to_vref_enum<PlanetType>(kw["type"]);
        std::unique_ptr<ValueRef::ValueRef<PlanetSize>> size = pyobject_to_vref_enum<PlanetSize>(kw["planetsize"]);

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name"))
            name = pyobject_to_vref<std::string>(kw["name"]);

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        if (kw.has_key("initial_effects")) {
            boost::python::stl_input_iterator<effect_wrapper> effects_begin(kw["initial_effects"]), effects_end;
            for (auto it = effects_begin; it != effects_end; ++ it)
                effects.push_back(ValueRef::CloneUnique(it->effect));
        }

        return effect_wrapper(std::make_shared<Effect::CreatePlanet>(
            std::move(type),
            std::move(size),
            std::move(name),
            std::move(effects)));
    }
}

BOOST_PYTHON_MODULE(_effects_new) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::def("MoveInOrbit", boost::python::raw_function(insert_move_in_orbit));
    boost::python::def("CreatePlanet", boost::python::raw_function(insert_create_planet));
}


