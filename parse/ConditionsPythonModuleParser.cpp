#include "ValueRefsPythonModuleParser.h"

#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>

#include "ValueRefPythonParser.h"
#include "EnumPythonParser.h"
#include "ConditionPythonParser.h"

namespace py = boost::python;

namespace {
    using ValueRef::CloneUnique;
    using boost::python::extract;

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
    condition_wrapper insert_design_has_part_class_(const boost::python::tuple& args, const boost::python::dict& kw) {
        ShipPartClass name = extract<enum_wrapper<ShipPartClass>>(kw["name"])().value;

        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            low = pyobject_to_vref<int>(kw["low"]);
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            high = pyobject_to_vref<int>(kw["high"]);
        }

        return make_wrapped<Condition::DesignHasPartClass>(
            std::move(name),
            std::move(low),
            std::move(high));
    }
}

BOOST_PYTHON_MODULE(_conditions) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::def("DesignHasPartClass", boost::python::raw_function(insert_design_has_part_class_));
}

