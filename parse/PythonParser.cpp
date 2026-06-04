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
#include "SourcesPythonModuleParser.h"
#include "EnumsPythonModuleParser.h"

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

PythonParser::PythonParser(PythonCommon& _python) :
    m_python(_python)
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
        m_python.SetPopulateGlobalsFunc([](py::dict& globals) {});
        m_python.InitModuleLoader();

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

PythonParser::~PythonParser()
{
    m_python.SetPopulateGlobalsFunc(std::function<void(py::dict&)>{});
    // To correctly init it again between sub-interpreters border
    m_python.FinalizeModuleLoader();

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

void PythonParser::LoadSourcesModule() const
{ (void)LoadModule(&PyInit__sources); } // marked [[nodiscard]] but result not needed in this case

void PythonParser::LoadEnumsModule() const
{ (void)LoadModule(&PyInit__enums); } // marked [[nodiscard]] but result not needed in this case
