#include "PythonParser.h"

#include "../universe/Species.h"
#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"
#include "../universe/ValueRefs.h"
#include "../universe/Conditions.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/PythonCommon.h"
#include "Parse.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include <boost/algorithm/string.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/core/ref.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/python/class.hpp>
#include <boost/python/import.hpp>
#include <boost/python/object.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/operators.hpp>
#include <memory>
#include <string>

namespace py = boost::python;

namespace {
    struct import_error : std::runtime_error {
        import_error(const std::string& what) : std::runtime_error(what) {}
    };

    void translate(import_error const& e) {
        PyErr_SetString(PyExc_ImportError, e.what());
    }
}

struct module_spec {
    module_spec(const std::string& name, const std::string& parent_, const PythonParser& parser_) :
        fullname(name),
        parent(parent_),
        parser(parser_)
    {}

    py::list path;
    py::list uninitialized_submodules;
    std::string fullname;
    std::string parent;
    const PythonParser& parser;
};

PythonParser::PythonParser(PythonCommon& _python, const boost::filesystem::path& scripting_dir) :
    m_python(_python),
    m_scripting_dir(scripting_dir)
{
    if (!m_python.IsPythonRunning()) {
        ErrorLogger() << "Python parse given non-initialized python!";
        throw std::runtime_error("Python isn't initialized");
    }

    m_main_thread_state = PyThreadState_Get();
    m_parser_thread_state = Py_NewInterpreter();

    if (!m_main_thread_state && !m_parser_thread_state) {
        ErrorLogger() << "Python parser sub-interpreter isn't initialized!";
        throw std::runtime_error("Python sub-interpreter isn't initialized");
    }

    try {
        type_int = py::import("builtins").attr("int");
        type_float = py::import("builtins").attr("float");
        type_bool = py::import("builtins").attr("bool");
        type_str = py::import("builtins").attr("str");
        py::import("builtins").attr("parser_context") = true;

        py::register_exception_translator<import_error>(&translate);

        py::class_<PythonParser, py::bases<>, PythonParser, boost::noncopyable>("PythonParser", py::no_init)
            .def("find_spec", &PythonParser::find_spec)
            .def("create_module", &PythonParser::create_module)
            .def("exec_module", &PythonParser::exec_module);

        py::class_<module_spec>("PythonParserSpec", py::no_init)
            .def_readonly("name", &module_spec::fullname)
            .def_readonly("_uninitialized_submodules", &module_spec::uninitialized_submodules)
            .add_static_property("loader", py::make_getter(*this, py::return_value_policy<py::reference_existing_object>()))
            .def_readonly("submodule_search_locations", &module_spec::path)
            .def_readonly("has_location", false)
            .def_readonly("cached", false)
            .def_readonly("parent", &module_spec::parent);

        // Use wrappers to not collide with types in server and AI
        py::class_<value_ref_wrapper<int>>("ValueRefInt", py::no_init)
            .def(int() * py::self_ns::self)
            .def(double() * py::self_ns::self)
            .def(py::self_ns::self / int())
            .def(py::self_ns::self - int())
            .def(int() - py::self_ns::self)
            .def(py::self_ns::self + py::self_ns::self)
            .def(py::self_ns::self + int())
            .def(double() + py::self_ns::self)
            .def(py::self_ns::self < py::self_ns::self)
            .def(py::self_ns::self < int())
            .def(py::self_ns::self <= int())
            .def(py::self_ns::self > int())
            .def(py::self_ns::self >= int())
            .def(py::self_ns::self >= py::self_ns::self)
            .def(py::self_ns::self == py::self_ns::self)
            .def(double() - py::self_ns::self)
            .def(py::self_ns::self == int())
            .def(py::self_ns::self != int())
            .def(py::self_ns::self | py::self_ns::self)
            .def(py::self_ns::pow(py::self_ns::self, double()));
        py::class_<value_ref_wrapper<double>>("ValueRefDouble", py::no_init)
            .def("__call__", &value_ref_wrapper<double>::call)
            .def(int() * py::self_ns::self)
            .def(py::other<value_ref_wrapper<int>>() * py::self_ns::self)
            .def(py::self_ns::self * py::other<value_ref_wrapper<int>>())
            .def(py::self_ns::self * double())
            .def(py::self_ns::self * py::self_ns::self)
            .def(double() * py::self_ns::self)
            .def(py::self_ns::self / py::self_ns::self)
            .def(py::self_ns::self / int())
            .def(py::self_ns::self / double())
            .def(py::self_ns::self + int())
            .def(py::self_ns::self + double())
            .def(py::self_ns::self + py::self_ns::self)
            .def(py::self_ns::self + py::other<value_ref_wrapper<int>>())
            .def(py::other<value_ref_wrapper<int>>() + py::self_ns::self)
            .def(py::self_ns::self - double())
            .def(py::self_ns::self - py::self_ns::self)
            .def(py::self_ns::self - py::other<value_ref_wrapper<int>>())
            .def(int() + py::self_ns::self)
            .def(int() - py::self_ns::self)
            .def(py::self_ns::self - int())
            .def(double() <= py::self_ns::self)
            .def(py::self_ns::self <= double())
            .def(py::self_ns::self <= py::self_ns::self)
            .def(py::self_ns::self >= int())
            .def(py::self_ns::self > py::self_ns::self)
            .def(py::self_ns::self >= py::self_ns::self)
            .def(py::self_ns::self < py::self_ns::self)
            .def(double() < py::self_ns::self)
            .def(py::self_ns::self < double())
            .def(py::self_ns::self != int())
            .def(py::self_ns::pow(py::self_ns::self, double()))
            .def(py::self_ns::pow(double(), py::self_ns::self))
            .def(py::self_ns::pow(py::self_ns::self, py::self_ns::self))
            .def(py::self_ns::self & py::self_ns::self)
            .def(-py::self_ns::self);
        py::class_<value_ref_wrapper<std::string>>("ValueRefString", py::no_init)
            .def(py::self_ns::self + std::string())
            .def(std::string() + py::self_ns::self);
        py::class_<value_ref_wrapper<Visibility>>("ValueRefVisibility", py::no_init);
        py::class_<condition_wrapper>("Condition", py::no_init)
            .def(py::self_ns::self & py::self_ns::self)
            .def(py::self_ns::self & py::other<value_ref_wrapper<double>>())
            .def(py::self_ns::self & py::other<value_ref_wrapper<int>>())
            .def(py::other<value_ref_wrapper<int>>() & py::self_ns::self)
            .def(py::self_ns::self | py::self_ns::self)
            .def(py::self_ns::self | py::other<value_ref_wrapper<int>>())
            .def(~py::self_ns::self);
        py::class_<effect_wrapper>("Effect", py::no_init);
        py::class_<effect_group_wrapper>("EffectsGroup", py::no_init);
        py::class_<enum_wrapper<UnlockableItemType>>("__UnlockableItemType", py::no_init);
        py::class_<enum_wrapper<EmpireAffiliationType>>("__EmpireAffiliationType", py::no_init);
        py::class_<enum_wrapper<MeterType>>("__MeterType", py::no_init);
        py::class_<enum_wrapper<ResourceType>>("__ResourceType", py::no_init);
        py::class_<enum_wrapper< ::PlanetEnvironment>>("__PlanetEnvironment", py::no_init);
        py::class_<enum_wrapper<PlanetSize>>("__PlanetSize", py::no_init);
        py::class_<enum_wrapper<PlanetType>>("__PlanetType", py::no_init)
            .def(py::self_ns::self == py::self_ns::self)
            .def("__hash__", py::make_function(std::hash<enum_wrapper<PlanetType>>{},
                py::default_call_policies(),
                boost::mpl::vector<std::size_t, const enum_wrapper<PlanetType>&>()));
        py::class_<enum_wrapper< ::StarType>>("__StarType", py::no_init);
        py::class_<enum_wrapper<ValueRef::StatisticType>>("__StatisticType", py::no_init);
        py::class_<enum_wrapper<Condition::ContentType>>("__LocationContentType", py::no_init);
        py::class_<enum_wrapper<BuildType>>("__BuildType", py::no_init);
        py::class_<enum_wrapper<Visibility>>("__Visibility", py::no_init);
        py::class_<enum_wrapper<CaptureResult>>("__CaptureResult", py::no_init);
        py::class_<unlockable_item_wrapper>("UnlockableItem", py::no_init);
        py::class_<FocusType>("__FocusType", py::no_init);
        auto py_variable_wrapper = py::class_<variable_wrapper>("__Variable", py::no_init);

        for (std::string_view property : {"Owner",
                                          "OwnerBeforeLastConquered",
                                          "SupplyingEmpire",
                                          "ID",
                                          "CreationTurn",
                                          "Age",
                                          "ProducedByEmpireID",
                                          "ArrivedOnTurn",
                                          "DesignID",
                                          "FleetID",
                                          "PlanetID",
                                          "SystemID",
                                          "ContainerID",
                                          "FinalDestinationID",
                                          "NextSystemID",
                                          "NearestSystemID",
                                          "PreviousSystemID",
                                          "PreviousToFinalDestinationID",
                                          "NumShips",
                                          "NumStarlanes",
                                          "LastTurnActiveInBattle",
                                          "LastTurnAnnexed",
                                          "LastTurnAttackedByShip",
                                          "LastTurnBattleHere",
                                          "LastTurnColonized",
                                          "LastTurnConquered",
                                          "LastTurnMoveOrdered",
                                          "LastTurnResupplied",
                                          "Orbit",
                                          "TurnsSinceAnnexation",
                                          "TurnsSinceColonization",
                                          "TurnsSinceFocusChange",
                                          "TurnsSinceLastConquered",
                                          "ETA",
                                          "LaunchedFrom",
                                          "OrderedColonizePlanetID",
                                          "OwnerBeforeLastConquered",
                                          "LastInvadedByEmpire",
                                          "LastColonizedByEmpire"})
        {
            py_variable_wrapper.add_property(property.data(), py::make_function(
                [property] (const variable_wrapper& w) { return w.get_int_property(std::string{property}); },
                py::default_call_policies(),
                boost::mpl::vector<value_ref_wrapper<int>, const variable_wrapper&>()));
        }

        for (std::string_view property : {"Industry",
                                          "TargetIndustry",
                                          "Research",
                                          "TargetResearch",
                                          "Influence",
                                          "TargetInfluence",
                                          "Construction",
                                          "TargetConstruction",
                                          "Population",
                                          "TargetPopulation",
                                          "TargetHappiness",
                                          "Happiness",
                                          "MaxFuel",
                                          "Fuel",
                                          "MaxShield",
                                          "Shield",
                                          "MaxDefense",
                                          "Defense",
                                          "MaxTroops",
                                          "Troops",
                                          "RebelTroops",
                                          "MaxStructure",
                                          "Structure",
                                          "MaxSupply",
                                          "Supply",
                                          "MaxStockpile",
                                          "Stockpile",
                                          "Stealth",
                                          "Detection",
                                          "Speed",
                                          "X",
                                          "Y",
                                          "SizeAsDouble",
                                          "HabitableSize",
                                          "Size",
                                          "DistanceFromOriginalType",
                                          "DestroyFightersPerBattleMax",
                                          "DamageStructurePerBattleMax",
                                          "PropagatedSupplyRange"})
        {
            py_variable_wrapper.add_property(property.data(), py::make_function(
                [property](const variable_wrapper& w) { return w.get_double_property(std::string{property}); },
                py::default_call_policies(),
                boost::mpl::vector<value_ref_wrapper<double>, const variable_wrapper&>()));
        }

        for (std::string_view property : {"Name",
                                          "Species",
                                          "BuildingType",
                                          "FieldType",
                                          "Focus",
                                          "DefaultFocus",
                                          "Hull"})
        {
            py_variable_wrapper.add_property(property.data(), py::make_function(
                [property](const variable_wrapper& w) { return w.get_string_property(std::string{property}); },
                py::default_call_policies(),
                boost::mpl::vector<value_ref_wrapper<std::string>, const variable_wrapper&>()));
        }

        for (std::string_view container : {"Planet",
                                           "System",
                                           "Fleet"})
        {
             py_variable_wrapper.add_property(container.data(), py::make_function(
                [container](const variable_wrapper& w) { return w.get_variable_property(container); },
                py::default_call_policies(),
                boost::mpl::vector<variable_wrapper, const variable_wrapper&>()));
        }

        py::implicitly_convertible<value_ref_wrapper<double>, condition_wrapper>();
        py::implicitly_convertible<value_ref_wrapper<int>, condition_wrapper>();

        m_meta_path = py::extract<py::list>(py::import("sys").attr("meta_path"))();
        const auto meta_path_len = py::len(*m_meta_path);
        for (std::decay_t<decltype(meta_path_len)> i = 0; i < meta_path_len; ++i)
            m_meta_path->pop();

        m_meta_path->append(boost::cref(*this));
        m_meta_path_len = static_cast<int>(py::len(*m_meta_path));

        py::import("sys").attr("modules") = py::dict();
    } catch (const boost::python::error_already_set&) {
        m_python.HandleErrorAlreadySet();
        if (!m_python.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python.Initialize())
                ErrorLogger() << "Python interpreter successfully restarted.";
            else
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
        }
        throw std::runtime_error("Python parser failed to initialize");
    }
}

PythonParser::~PythonParser() {
    try {
        m_meta_path->pop(m_meta_path_len - 1);
        type_int = py::object();
        type_float = py::object();
        type_bool = py::object();
        type_str = py::object();
        m_meta_path = boost::none;
    } catch (const py::error_already_set&) {
        ErrorLogger() << "Python parser destructor throw exception";
        m_python.HandleErrorAlreadySet();
    }

    Py_EndInterpreter(m_parser_thread_state);
    PyThreadState_Swap(m_main_thread_state);
}

bool PythonParser::ParseFileCommon(const boost::filesystem::path& path,
                                   const boost::python::dict& globals,
                                   std::string& filename, std::string& file_contents) const
{
    filename = path.string();

    bool read_success = ReadFile(path, file_contents);
    if (!read_success) {
        ErrorLogger() << "Unable to open data file " << filename;
        return false;
    }

    try {
        py::exec(file_contents.c_str(), globals);
    } catch (const boost::python::error_already_set&) {
        m_python.HandleErrorAlreadySet();
        ErrorLogger() << "Unable to parse data file " << filename;
        if (!m_python.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
            }
        }
        return false;
    }

    return true;
}

py::object PythonParser::find_spec(const std::string& fullname, const py::object& path, const py::object& target) const {
    auto module_path(m_scripting_dir);
    std::string parent;
    std::string current;
    for (auto it = boost::algorithm::make_split_iterator(fullname, boost::algorithm::token_finder(boost::algorithm::is_any_of(".")));
         it != boost::algorithm::split_iterator<std::string::const_iterator>(); ++it)
    {
        module_path = module_path / boost::copy_range<std::string>(*it);
        if (!current.empty()) {
            if (parent.empty())
                parent = std::move(current);
            else
                parent = parent + "." + current;
        }
        current = boost::copy_range<std::string>(*it);
    }

    if (IsExistingDir(module_path)) {
        return py::object(module_spec(fullname, parent, *this));
    } else {
        module_path.replace_extension("py");
        if (IsExistingFile(module_path))
            return py::object(module_spec(fullname, parent, *this));
        else {
            ErrorLogger() << "Couldn't find file for module spec " << fullname;
            throw import_error("Couldn't find file for module spec " + fullname);
        }
    }
}

py::object PythonParser::create_module(const module_spec& spec)
{ return py::object(); }

py::object PythonParser::exec_module(py::object& module) {
    std::string fullname = py::extract<std::string>(module.attr("__name__"));

    py::dict m_dict = py::extract<py::dict>(module.attr("__dict__"));

    auto module_path(m_scripting_dir);
    for (auto it = boost::algorithm::make_split_iterator(fullname, boost::algorithm::token_finder(boost::algorithm::is_any_of(".")));
         it != boost::algorithm::split_iterator<std::string::iterator>(); ++it)
    { module_path = module_path / boost::copy_range<std::string>(*it); }

    if (IsExistingDir(module_path)) {
        return py::object();
    } else {
        module_path.replace_extension("py");
        if (IsExistingFile(module_path)) {
            std::string file_contents;
            bool read_success = ReadFile(module_path, file_contents);
            if (!read_success) {
                ErrorLogger() << "Unable to open data file " << module_path.string();
                throw import_error("Unreadable module " + fullname);
            }

            // store globals content in module namespace
            // it is required so functions in the same module will see each other
            // and still import will work
            DebugLogger() << "Executing module file " << module_path.string();
            try {
#if PY_VERSION_HEX < 0x03080000
                m_dict["__builtins__"] = boost::python::import("builtins");
#endif
                RegisterGlobalsEffects(m_dict);
                RegisterGlobalsConditions(m_dict);
                RegisterGlobalsValueRefs(m_dict, *this);
                RegisterGlobalsSources(m_dict);
                RegisterGlobalsEnums(m_dict);

                py::exec(file_contents.c_str(), m_dict, m_dict);
            } catch (const boost::python::error_already_set&) {
                m_python.HandleErrorAlreadySet();
                ErrorLogger() << "Unable to parse module file " << module_path.string();
                if (!m_python.IsPythonRunning()) {
                    ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
                    if (m_python.Initialize()) {
                        ErrorLogger() << "Python interpreter successfully restarted.";
                    } else {
                        ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
                    }
                }
                throw import_error("Cannot execute module " + fullname);
            }

            return py::object();
        } else {
            throw import_error("Module not existed " + fullname);
        }
    }
}

