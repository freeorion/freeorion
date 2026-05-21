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
#include "ConditionsPythonModuleParser.h"
#include "EffectsPythonModuleParser.h"
#include "ValueRefsPythonModuleParser.h"

#include <boost/algorithm/string.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/core/ref.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/python/class.hpp>
#include <boost/python/import.hpp>
#include <boost/python/object.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/operators.hpp>

#include <filesystem>
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

    constexpr bool STATIC_FALSE = false;

    template <typename T1, typename T2>
    using ResultValueRefNumericType = std::conditional_t<std::is_same_v<T1, double>
        || std::is_same_v<std::remove_cvref_t<T1>, value_ref_wrapper<double>>
        || std::is_same_v<T2, double>
        || std::is_same_v<std::remove_cvref_t<T2>, value_ref_wrapper<double>>
        , double, int>;

    template <typename T>
    struct is_wrapper : std::false_type {};
    template <typename T>
    struct is_wrapper<value_ref_wrapper<T>> : std::true_type {};

    template <typename T, typename Arg>
    auto any_type_to_vref(Arg&& arg) {
        if constexpr (is_wrapper<std::decay_t<Arg>>::value) {
            using NumericT = typename std::decay_t<Arg>::BaseT;
            if constexpr (std::is_same_v<T, NumericT>) {
                return ValueRef::CloneUnique(arg.value_ref);
            } else {
                return std::make_unique<ValueRef::StaticCast<NumericT, T>>(ValueRef::CloneUnique(arg.value_ref));
            }
        } else {
            return std::make_unique<ValueRef::Constant<T>>(static_cast<T>(arg));
        }
    }

    template <typename T1, typename T2>
    struct OperationCreator {
        using BaseT = ResultValueRefNumericType<T1, T2>;

        value_ref_wrapper<BaseT> operator()(ValueRef::OpType op, T1 lhs, T2 rhs) {
            return value_ref_wrapper<BaseT>(std::make_shared<ValueRef::Operation<BaseT>>(
                op,
                any_type_to_vref<BaseT>(std::forward<T1>(lhs)),
                any_type_to_vref<BaseT>(std::forward<T2>(rhs))
            ));
        }
    };

    template <typename T, typename Arg>
    void register_arithmetic_ops(py::class_<value_ref_wrapper<T>>& cl) {
        for (const auto& op : std::initializer_list<std::tuple<const char*, const char*, ValueRef::OpType>>{
            {"__add__", "__radd__", ValueRef::OpType::PLUS},
            {"__sub__", "__rsub__", ValueRef::OpType::MINUS},
            {"__mul__", "__rmul__", ValueRef::OpType::TIMES},
            {"__truediv__", "__rtruediv__", ValueRef::OpType::DIVIDE},
            {"__eq__", nullptr, ValueRef::OpType::COMPARE_EQUAL},
            {"__gt__", nullptr, ValueRef::OpType::COMPARE_GREATER_THAN},
            {"__ge__", nullptr, ValueRef::OpType::COMPARE_GREATER_THAN_OR_EQUAL},
            {"__lt__", nullptr, ValueRef::OpType::COMPARE_LESS_THAN},
            {"__le__", nullptr, ValueRef::OpType::COMPARE_LESS_THAN_OR_EQUAL},
            {"__ne__", nullptr, ValueRef::OpType::COMPARE_NOT_EQUAL}})
        {
            const auto op_type = std::get<2>(op);
            const char* reverse_op = std::get<1>(op);
            cl.def(std::get<0>(op), py::make_function(
                [op_type](const value_ref_wrapper<T>& lhs, Arg rhs){ return OperationCreator<const value_ref_wrapper<T>&, Arg>()(op_type, lhs, rhs); },
                py::default_call_policies(),
                boost::mpl::vector<
                    value_ref_wrapper<typename OperationCreator<const value_ref_wrapper<T>&, Arg>::BaseT>,
                    const value_ref_wrapper<T>&,
                    Arg
                >()
            ));
            if (reverse_op != nullptr) {
                cl.def(reverse_op, py::make_function(
                    [op_type](const value_ref_wrapper<T>& rhs, Arg lhs){ return OperationCreator<Arg, const value_ref_wrapper<T>&>()(op_type, lhs, rhs); },
                    py::default_call_policies(),
                    boost::mpl::vector<
                        value_ref_wrapper<typename OperationCreator<const value_ref_wrapper<T>&, Arg>::BaseT>,
                        const value_ref_wrapper<T>&,
                        Arg
                    >()
                ));
            }
            cl.def(std::get<0>(op), py::make_function(
                [op_type](const value_ref_wrapper<T>& lhs, const value_ref_wrapper<Arg>& rhs){ return OperationCreator<const value_ref_wrapper<T>&, const value_ref_wrapper<Arg>&>()(op_type, lhs, rhs); },
                py::default_call_policies(),
                boost::mpl::vector<
                    value_ref_wrapper<typename OperationCreator<const value_ref_wrapper<T>&, const value_ref_wrapper<Arg>&>::BaseT>,
                    const value_ref_wrapper<T>&,
                    const value_ref_wrapper<Arg>&
                >()
            ));
        }
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

PythonTypes::PythonTypes() {
    auto builtings = py::import("builtins");
    type_int = builtings.attr("int");
    type_float = builtings.attr("float");
    type_bool = builtings.attr("bool");
    type_str = builtings.attr("str");
}

PythonTypes::~PythonTypes() {
    type_int = py::object();
    type_float = py::object();
    type_bool = py::object();
    type_str = py::object();
}

BOOST_PYTHON_MODULE(freeorion_loader) {
    py::register_exception_translator<import_error>(&translate);

    py::class_<PythonParser, py::bases<>, PythonParser, boost::noncopyable>("PythonParser", py::no_init)
        .def("find_spec", &PythonParser::find_spec)
        .def("create_module", &PythonParser::create_module)
        .def("exec_module", &PythonParser::exec_module);

    py::class_<module_spec>("PythonParserSpec", py::no_init)
        .def_readonly("name", &module_spec::fullname)
        .def_readonly("_uninitialized_submodules", &module_spec::uninitialized_submodules)
        .add_property("loader", py::make_function(+[](const module_spec& self) -> const PythonParser& { return self.parser; }, py::return_value_policy<py::reference_existing_object>()))
        .def_readonly("submodule_search_locations", &module_spec::path)
        .def_readonly("has_location", STATIC_FALSE)
        .def_readonly("cached", STATIC_FALSE)
        .def_readonly("parent", &module_spec::parent);
}

PythonParser::PythonParser(PythonCommon& _python, const std::filesystem::path& modules_dir) :
    m_python(_python),
    m_modules_dir(modules_dir)
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

    if (!m_python.InitErrorHandler()) {
        ErrorLogger() << "Python error handler isn't initialized!";
        throw std::runtime_error("Python error handler isn't initialized!");
    }

    try {
        LoadModule(&PyInit_freeorion_loader);
        m_populate_globals_func = [](py::dict& dict) {
            RegisterGlobalsEffects(dict);
            RegisterGlobalsConditions(dict);
            RegisterGlobalsValueRefs(dict);
            RegisterGlobalsSources(dict);
            RegisterGlobalsEnums(dict);
        };

        // Use wrappers to not collide with types in server and AI
        auto value_ref_wrapper_double_class = py::class_<value_ref_wrapper<double>>("ValueRefDouble", py::no_init)
            .def("__call__", &value_ref_wrapper<double>::call)
            .def(py::self_ns::pow(py::self_ns::self, double()))
            .def(py::self_ns::pow(double(), py::self_ns::self))
            .def(py::self_ns::pow(py::self_ns::self, py::self_ns::self))
            .def(py::self_ns::self & py::self_ns::self)
            .def(py::self_ns::self | py::self_ns::self)
            .def(~py::self_ns::self)
            .def(-py::self_ns::self);
        register_arithmetic_ops<double, double>(value_ref_wrapper_double_class);
        register_arithmetic_ops<double, int>(value_ref_wrapper_double_class);
        auto value_ref_wrapper_int_class = py::class_<value_ref_wrapper<int>>("ValueRefInt", py::no_init)
            .def(py::self_ns::self & py::self_ns::self)
            .def(py::self_ns::self | py::self_ns::self)
            .def(~py::self_ns::self)
            .def(py::self_ns::pow(py::self_ns::self, double()));
        register_arithmetic_ops<int, double>(value_ref_wrapper_int_class);
        register_arithmetic_ops<int, int>(value_ref_wrapper_int_class);
        py::class_<value_ref_wrapper<std::string>>("ValueRefString", py::no_init)
            .def(py::self_ns::self + std::string())
            .def(std::string() + py::self_ns::self);
        py::class_<value_ref_wrapper<std::vector<std::string>>>("ValueRefVectorString", py::no_init);
        py::class_<value_ref_wrapper<Visibility>>("ValueRefVisibility", py::no_init);
        py::class_<value_ref_wrapper<PlanetType>>("ValueRefPlanetType", py::no_init)
            .def(py::self_ns::self != py::self_ns::self);
        py::class_<value_ref_wrapper< ::PlanetEnvironment>>("ValueRefPlanetEnvironment", py::no_init);
        py::class_<value_ref_wrapper<PlanetSize>>("ValueRefPlanetSize", py::no_init)
            .def(py::self_ns::self != py::self_ns::self);
        py::class_<value_ref_wrapper< ::StarType>>("ValueRefStarType", py::no_init);
        py::class_<condition_wrapper>("Condition", py::no_init)
            .def(py::self_ns::self & py::self_ns::self)
            .def(py::self_ns::self & py::other<value_ref_wrapper<double>>())
            .def(py::other<value_ref_wrapper<double>>() & py::self_ns::self)
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
        py::class_<enum_wrapper<ShipPartClass>>("__ShipPartClass", py::no_init);
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
                [property] (const variable_wrapper& w) { return w.get_property<int>(std::string{property}); },
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
                [property](const variable_wrapper& w) { return w.get_property<double>(std::string{property}); },
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
                [property](const variable_wrapper& w) { return w.get_property<std::string>(std::string{property}); },
                py::default_call_policies(),
                boost::mpl::vector<value_ref_wrapper<std::string>, const variable_wrapper&>()));
        }

        for (std::string_view property : {"PlanetType",
                                          "OriginalType",
                                          "NextCloserToOriginalPlanetType",
                                          "NextBestPlanetType",
                                          "NextBetterPlanetType",
                                          "ClockwiseNextPlanetType",
                                          "CounterClockwiseNextPlanetType"})
        {
            py_variable_wrapper.add_property(property.data(), py::make_function(
                [property](const variable_wrapper& w) { return w.get_property<PlanetType>(std::string{property}); },
                py::default_call_policies(),
                boost::mpl::vector<value_ref_wrapper<PlanetType>, const variable_wrapper&>()));
        }

        for (std::string_view property : {"PlanetEnvironment"}) {
            py_variable_wrapper.add_property(property.data(), py::make_function(
                [property](const variable_wrapper& w) { return w.get_property< ::PlanetEnvironment>(std::string{property}); },
                py::default_call_policies(),
                boost::mpl::vector<value_ref_wrapper< ::PlanetEnvironment>, const variable_wrapper&>()));
        }

        for (std::string_view property : {"planetsize",
                                          "NextLargerPlanetSize",
                                          "NextSmallerPlanetSize"})
        {
            py_variable_wrapper.add_property(property.data(), py::make_function(
                [property](const variable_wrapper& w) { return w.get_property<PlanetSize>(std::string{property}); },
                py::default_call_policies(),
                boost::mpl::vector<value_ref_wrapper<PlanetSize>, const variable_wrapper&>()));
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
        m_meta_path->append(boost::cref(*this));
        m_meta_path_len = static_cast<int>(py::len(*m_meta_path));
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
        m_meta_path = boost::none;
    } catch (const py::error_already_set&) {
        ErrorLogger() << "Python parser destructor throw exception";
        m_python.HandleErrorAlreadySet();
    }

    Py_EndInterpreter(m_parser_thread_state);
    PyThreadState_Swap(m_main_thread_state);
}

bool PythonParser::ParseFileCommon(const std::filesystem::path& path,
                                   const boost::python::dict& globals,
                                   std::string& filename, std::string& file_contents) const
{
    filename = PathToString(path);

    bool read_success = ReadFile(path, file_contents);
    if (!read_success) {
        ErrorLogger() << "Unable to open data file " << filename;
        return false;
    }

    try {
        PythonCommon::CompileEval(file_contents.c_str(), path, globals);
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

py::object PythonParser::LoadModule(PyObject* (*init_function)()) const {
    PyObject *py_module = init_function();
    if (py_module) {
        const char* module_name = PyModule_GetName(py_module);
        DebugLogger() << "Injecting parser module " << module_name;
        py::object module{py::handle<>(py_module)};
        py::extract<py::dict>(py::import("sys").attr("modules"))()[std::string{"focs."} + module_name] = module;
        return module;
    }
    return py::object();
}

void PythonParser::UnloadModule(py::object module) const {
    const char* module_name = PyModule_GetName(module.ptr());
    py::import("sys").attr("modules").attr("pop")(std::string{"focs."} + module_name);
}

void PythonParser::LoadConditionsModule() const
{ (void)LoadModule(&PyInit__conditions); } // marked [[nodiscard]] but result not needed in this case

void PythonParser::LoadValueRefsModule() const
{ (void)LoadModule(&PyInit__value_refs); } // marked [[nodiscard]] but result not needed in this case

void PythonParser::LoadEffectsModule() const
{ (void)LoadModule(&PyInit__effects_new); } // marked [[nodiscard]] but result not needed in this case

py::object PythonParser::find_spec(const std::string& fullname, const py::object& path, const py::object& target) const {
    auto module_path(m_modules_dir);
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
        else if (fullname == "_typing") {
            return py::object();
        } else {
            ErrorLogger() << "Couldn't find file for module spec " << fullname;
            throw import_error("Couldn't find file for module spec " + fullname);
        }
    }
}

py::object PythonParser::create_module(const module_spec& spec)
{ return py::object(); }

py::object PythonParser::exec_module(py::object& module) {
    std::string fullname = py::extract<std::string>(module.attr("__name__"));

    py::dict globals = py::extract<py::dict>(module.attr("__dict__"));

    auto module_path(m_modules_dir);
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
                ErrorLogger() << "Unable to open data file " << PathToString(module_path);
                throw import_error("Unreadable module " + fullname);
            }

            // store globals content in module namespace
            // it is required so functions in the same module will see each other
            // and still import will work
            DebugLogger() << "Executing module file " << PathToString(module_path);
            try {
                if (m_populate_globals_func) {
                    m_populate_globals_func(globals);
                }

                PythonCommon::CompileEval(file_contents.c_str(), module_path, globals);
            } catch (const boost::python::error_already_set&) {
                m_python.HandleErrorAlreadySet();
                ErrorLogger() << "Unable to parse module file " << PathToString(module_path);
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
