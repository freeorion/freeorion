#include "../util/boost_fix.h"
#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "../Empire/EmpireManager.h"
#include "../universe/Building.h"
#include "../universe/BuildingType.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../universe/Field.h"
#include "../universe/FieldType.h"
#include "../universe/Fleet.h"
#include "../universe/NamedValueRefManager.h"
#include "../universe/Pathfinder.h"
#include "../universe/Planet.h"
#include "../universe/ScriptingContext.h"
#include "../universe/ShipDesign.h"
#include "../universe/Ship.h"
#include "../universe/ShipHull.h"
#include "../universe/ShipPart.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"

namespace py = boost::python;


namespace {
    constexpr auto to_string = [](std::string_view sv) { return std::string{sv}; };
    constexpr auto to_value = [](const auto& strong_t) noexcept { return Value(strong_t); };

    template <typename T>
    std::vector<T> ToVec(std::vector<T>) = delete;

    auto ToVec(auto&& in)
    { return in | range_to_vec; }

    std::vector<int> ToIntValueVec(auto&& in)
    { return in | range_transform(to_value) | range_to_vec; }

    template <typename T>
    auto ObjectIDs(const Universe& universe) -> std::vector<int>
    { return universe.Objects().allWithIDs<T>() | range_keys | range_transform(to_value) | range_to_vec; }


    auto ObjectTagsAsStringVec(const UniverseObject& o) -> std::vector<std::string>
    {
        UniverseObject::TagVecs tags = o.Tags(IApp::GetApp()->GetContext());
        std::vector<std::string> result;
        result.reserve(tags.size());
        std::transform(tags.first.begin(), tags.first.end(), std::back_inserter(result), to_string);
        std::transform(tags.second.begin(), tags.second.end(), std::back_inserter(result), to_string);
        return result;
    }

    enum class SpeciesInfo : uint8_t {
        TAGS,
        LIKES,
        DISLIKES
    };
    auto SpeciesTags(const Species& species, SpeciesInfo what_info) -> std::vector<std::string>
    {
        const auto& tags = [what_info, &species]() -> const auto& {
            static CONSTEXPR_VEC const std::vector<std::string_view> EMPTY_VEC_STRING_VEC;
            switch (what_info) {
            case SpeciesInfo::TAGS:     return species.Tags();
            case SpeciesInfo::LIKES:    return species.Likes();
            case SpeciesInfo::DISLIKES: return species.Dislikes();
            default:                    return EMPTY_VEC_STRING_VEC;
            }
        }();

        return tags | range_transform(to_string) | range_to_vec;
    }

    auto SpeciesFoci(const Species& species) -> std::vector<std::string>
    {
        static constexpr auto to_name = [](const auto& f) noexcept -> const auto& { return f.Name(); };
        return species.Foci() | range_transform(to_name) | range_to_vec;
    }

    auto SpeciesHomeworlds(const Species& species) -> std::set<int>
    {
        const auto& species_homeworlds{IApp::GetApp()->GetSpeciesManager().GetSpeciesHomeworldsMap()};
        auto it = species_homeworlds.find(species.Name());
        if (it == species_homeworlds.end())
            return {};
        return it->second | range_transform(to_value) | range_to_set;
    }

    void UpdateMetersWrapper(Universe& universe, const py::object&)
    { universe.UpdateMeterEstimates(IApp::GetApp()->GetContext()); }

    using AccountingMapMappedType = Effect::AccountingMap::mapped_type;
    using IntIndexedAccountingMap = std::unordered_map<int, AccountingMapMappedType>;

    IntIndexedAccountingMap GetAccountingMapIntIndexed(const Universe& u) {
        IntIndexedAccountingMap retval;
        for (const auto& [obj_id, am] : u.GetEffectAccountingMap())
            retval.emplace(Value(obj_id), am);
        return retval;
    }

    auto ShortestPath(const Universe& universe, int start_sys, int end_sys, EmpireID empire_id) -> std::vector<int>
    {
        auto path{universe.GetPathfinder().ShortestPath(UniverseObjectID{start_sys}, UniverseObjectID{end_sys},
                                                        EmpireID{empire_id}).first};
        static_assert(std::is_same_v<std::vector<UniverseObjectID>, decltype(path)>);
        return path | range_transform(to_value) | range_to_vec;
    }

    auto ShortestNonHostilePath(const Universe& universe, int start_sys, int end_sys, EmpireID empire_id) -> std::vector<int>
    {
        const auto& empires = IApp::GetApp()->Empires();
        const auto is_hostile = [empire_id, &empires](const UniverseObject* obj)
        { return !obj || obj->HostileToEmpire(EmpireID(empire_id), empires); };

        auto path = universe.GetPathfinder().ShortestPath(
            UniverseObjectID{start_sys}, UniverseObjectID{end_sys}, is_hostile, universe.EmpireKnownObjects(empire_id)).first;
        static_assert(std::is_same_v<std::vector<UniverseObjectID>, decltype(path)>);
        return path | range_transform(to_value) | range_to_vec;
    }

    auto LeastJumpsPath(const Universe& universe, int start_sys, int end_sys, int empire_id) -> std::vector<int>
    {
        auto path = universe.GetPathfinder().LeastJumpsPath(UniverseObjectID{start_sys}, UniverseObjectID{end_sys},
                                                            EmpireID{empire_id}).first;
        static_assert(std::is_same_v<std::vector<UniverseObjectID>, decltype(path)>);
        return path | range_transform(to_value) | range_to_vec;
    }

    auto ImmediateNeighbors(const Universe& universe, int system1_id, int empire_id) -> std::vector<int>
    {
        auto neighbours = universe.GetPathfinder().ImmediateNeighbors(UniverseObjectID{system1_id}, EmpireID{empire_id});
        return neighbours | range_values | range_transform(to_value) | range_to_vec;
    }

    auto SystemNeighborsMap(const Universe& universe, int system1_id, int empire_id) -> std::map<int, double>
    {
        const auto neighbours{universe.GetPathfinder().ImmediateNeighbors(UniverseObjectID{system1_id}, EmpireID{empire_id})};
        static constexpr auto swap_kvp = [](const auto& kvp) noexcept { return std::pair{Value(kvp.second), kvp.first}; };
        return neighbours | range_transform(swap_kvp) | range_to_map;
    }

    auto GetObjEmpireVisTurns(const Universe& universe, int object_id, int empire_id) -> std::map<Visibility, int>
    {
        std::map<Visibility, int> retval;
        const auto& ovt = universe.GetObjectVisibilityTurnsByEmpire(UniverseObjectID{object_id}, EmpireID{empire_id});
        if (!ovt.empty()) {
            for (Visibility vis : {Visibility::VIS_BASIC_VISIBILITY, Visibility::VIS_PARTIAL_VISIBILITY, Visibility::VIS_FULL_VISIBILITY}) {
                if (ovt[vis] != INVALID_GAME_TURN)
                    retval.emplace(vis, ovt[vis]);
            }
        }
        return retval;
    }

    auto ObjectSpecials(const UniverseObject& object) -> std::vector<std::string>
    { return object.Specials() | range_keys | range_to_vec; }

    auto ObjectCurrentMeterValue(const UniverseObject& o, MeterType meter_type) -> float
    {
        if (auto* m = o.GetMeter(meter_type))
            return m->Current();
        return 0.0f;
    }

    auto ObjectInitialMeterValue(const UniverseObject& o, MeterType meter_type) -> float
    {
        if (auto* m = o.GetMeter(meter_type))
            return m->Initial();
        return 0.0f;
    }

    auto AttackStats(const ShipDesign& ship_design) -> std::vector<int>
    {
        static constexpr auto get_part = [](std::string_view name) -> const auto* { return GetShipPart(name); };
        static constexpr auto is_pcdw = [](const auto* p) noexcept { return p && p->Class() == ShipPartClass::PC_DIRECT_WEAPON; }; // TODO: handle other weapon classes when they are implemented
        static constexpr auto to_cap = [](const auto* p) { return static_cast<int>(p->Capacity()); };
        return ship_design.Parts() | range_transform(get_part) | range_filter(is_pcdw)
            | range_transform(to_cap) | range_to_vec;
    }

    auto HullSlots(const ShipHull& hull) -> std::vector<ShipSlotType>
    {
        static constexpr auto to_slot_type = [](const auto& slot) noexcept { return slot.type; };
        return hull.Slots() | range_transform(to_slot_type) | range_to_vec;
    }

    auto HullProductionLocation(const ShipHull& hull, int location_id) -> bool
    {
        ScriptingContext& context = IApp::GetApp()->GetContext();

        UniverseObject* location = context.ContextObjects().getRaw(UniverseObjectID{location_id}); // intentionally getting a mutable pointer to construct context with
        if (!location) {
            ErrorLogger() << "UniverseWrapper::HullProductionLocation Could not find location with id " << location_id;
            return false;
        }

        const ScriptingContext location_as_source_context{context, ScriptingContext::Source{}, location,
                                                          ScriptingContext::Target{}, location};
        return hull.Location()->EvalOne(location_as_source_context, location);
    }

    auto ShipPartProductionLocation(const ShipPart& part_type, int location_id) -> bool
    {
        ScriptingContext& context = IApp::GetApp()->GetContext();

        UniverseObject* location = context.ContextObjects().getRaw(UniverseObjectID{location_id});
        if (!location) {
            ErrorLogger() << "UniverseWrapper::PartTypeProductionLocation Could not find location with id " << location_id;
            return false;
        }
        const ScriptingContext location_as_source_context{context, ScriptingContext::Source{}, location,
                                                          ScriptingContext::Target{}, location};
        return part_type.Location()->EvalOne(location_as_source_context, location);
    }

    template <typename X>
    struct PairToTupleConverter {
        static PyObject* convert(X pair)
        { return py::incref(py::make_tuple(pair.first, pair.second).ptr()); }
    };

    std::vector<std::string> ExtractList(const py::list& py_string_list) {
        std::vector<std::string> retval;
        retval.reserve(len(py_string_list));
        for (int i = 0; i < len(py_string_list); i++)
            retval.push_back(py::extract<std::string>(py_string_list[i]));
        return retval;
    }

    std::vector<std::string> ViewsToStrings(const std::vector<std::string_view>& svs)
    { return svs | range_transform(to_string) | range_to_vec; }
}

namespace FreeOrionPython {
    /**
     * CallPolicies:
     *
     * return_value_policy<copy_const_reference>        when returning a relatively small object, such as a string,
     *                                                  that is returned by const reference or pointer
     *
     * return_value_policy<return_by_value>             when returning either a simple data type or a temporary object
     *                                                  in a function that will go out of scope after being returned
     *
     * return_internal_reference<>                      when returning an object or data that is a member of the object
     *                                                  on which the function is called (and shares its lifetime)
     *
     * return_value_policy<reference_existing_object>   when returning an object from a non-member function, or a
     *                                                  member function where the returned object's lifetime is not
     *                                                  fixed to the lifetime of the object on which the function is
     *                                                  called
     */
    void WrapUniverseClasses() {
        ////////////////////////
        // Container Wrappers //
        ////////////////////////
        py::class_<std::map<int, int>>("IntIntMap")
            .def(py::map_indexing_suite<std::map<int, int>, true>())
        ;
        py::class_<std::map<int, double>>("IntDoubleMap")
            .def(py::map_indexing_suite<std::map<int, double>, true>())
        ;
        py::class_<std::map<int, float>>("IntFloatMap")
            .def(py::map_indexing_suite<std::map<int, float>, true>())
        ;
        py::class_<std::map<Visibility, int>>("VisibilityIntMap")
            .def(py::map_indexing_suite<std::map<Visibility, int>, true>())
        ;
        py::class_<std::vector<ShipSlotType>>("ShipSlotTypeVec")
            .def(py::vector_indexing_suite<std::vector<ShipSlotType>, true>())
        ;
        py::class_<std::map<std::string, std::string>>("StringStringMap")
            .def(py::map_indexing_suite<std::map<std::string, std::string>, true>())
        ;

        py::class_<std::map<MeterType, Meter>>("MeterTypeMeterMap")
            .def(py::map_indexing_suite<std::map<MeterType, Meter>, true>())
        ;

        py::to_python_converter<std::pair<MeterType, std::string>,
                                PairToTupleConverter<std::pair<MeterType, std::string>>>();
        py::to_python_converter<std::pair<std::string, MeterType>,
                                PairToTupleConverter<std::pair<std::string, MeterType>>>();

        py::class_<Ship::PartMeterMap>("StringMeterTypePairMeterMap")
            .def(py::map_indexing_suite<Ship::PartMeterMap>())
        ;

        py::class_<std::map<std::string, std::map<int, std::map<int, double>>>>("StringIntIntDoubleMapMapMap")
            .def(py::map_indexing_suite<std::map<std::string, std::map<int, std::map<int, double>>>, true>())
        ;
        py::class_<std::map<int, std::map<int, double>>>("IntIntDoubleMapMap")
            .def(py::map_indexing_suite<std::map<int, std::map<int, double>>, true>())
        ;

        ///////////////////////////
        //   Effect Accounting   //
        ///////////////////////////
        py::class_<Effect::AccountingInfo>("AccountingInfo")
            .add_property("causeType",          &Effect::AccountingInfo::cause_type)
            .def_readonly("specificCause",      &Effect::AccountingInfo::specific_cause)
            .def_readonly("customLabel",        &Effect::AccountingInfo::custom_label)
            .add_property("sourceID",           +[](const Effect::AccountingInfo& ai) -> int { return Value(ai.source_id); })
            .add_property("meterChange",        &Effect::AccountingInfo::meter_change)
            .add_property("meterRunningTotal",  &Effect::AccountingInfo::running_meter_total)
        ;
        py::class_<std::vector<Effect::AccountingInfo>>("AccountingInfoVec")
            .def(py::vector_indexing_suite<std::vector<Effect::AccountingInfo>, true>())
        ;
        py::class_<Effect::AccountingMap::mapped_type::value_type>("MeterTypeAccountingInfoVecPair")
            .add_property("meterType",          &Effect::AccountingMap::mapped_type::value_type::first)
        ;
        py::to_python_converter<Effect::AccountingMap::mapped_type::value_type,
            PairToTupleConverter<Effect::AccountingMap::mapped_type::value_type>>();
        py::class_<Effect::AccountingMap::mapped_type>("MeterTypeAccountingInfoVecMap")
            .def(py::map_indexing_suite<Effect::AccountingMap::mapped_type, true>())
        ;
        py::to_python_converter<IntIndexedAccountingMap::value_type,
            PairToTupleConverter<IntIndexedAccountingMap::value_type>>();
        py::class_<IntIndexedAccountingMap>("IntMeterTypeAccountingInfoVecMapMap")
            .def(py::map_indexing_suite<IntIndexedAccountingMap, true>())
        ;

        //////////////////////////////////////////////
        //   Content Named Global Constant Values   //
        //////////////////////////////////////////////
        py::def("namedRealDefined",
                +[](const std::string& name) -> bool { return GetValueRef<double>(name, true); },
                "Returns true/false (boolean) whether there is a defined double-valued "
                "scripted constant with name (string).");
        py::def("namedIntDefined",
                +[](const std::string& name) -> bool { return GetValueRef<int>(name, true); },
                "Returns true/false (boolean) whether there is a defined int-valued "
                "scripted constant with name (string).");
        py::def("getNamedReal",
                +[](const std::string& name) -> double {
                    if (const auto ref = GetValueRef<double>(name, true))
                        return ref->Eval();
                    return 0.0;
                },
                "Returns the named real value of the scripted constant with name (string). "
                "If no such named constant exists, returns 0.0.");
        py::def("getNamedInt",
                +[](const std::string& name) -> int {
                    if (const auto ref = GetValueRef<int>(name, true))
                        return ref->Eval();
                    return 0;
                },
                "Returns the named integer value of the scripted constant with name (string). "
                "If no such named constant exists, returns 0.");

        ///////////////
        //   Meter   //
        ///////////////
        py::class_<Meter, boost::noncopyable>("meter", py::no_init)
            .add_property("current",            +[](const Meter& m) -> float { return m.Current(); })
            .add_property("initial",            +[](const Meter& m) -> float { return m.Initial(); })
            .def("dump",                        +[](const Meter& m) -> std::string { return m.Dump(0).data(); },
                                                py::return_value_policy<py::return_by_value>(),
                                                "Returns string with debug information.")
        ;

        ////////////////////
        //    Universe    //
        ////////////////////
        py::class_<Universe, boost::noncopyable>("universe", py::no_init)
            .def("getObject",                   +[](const Universe& u, int id) -> const UniverseObject* { return u.Objects().getRaw<const UniverseObject>(UniverseObjectID{id}); },
                                                py::return_value_policy<py::reference_existing_object>())
            .def("getFleet",                    +[](const Universe& u, int id) -> const Fleet* { return u.Objects().getRaw<const Fleet>(UniverseObjectID{id}); },
                                                py::return_value_policy<py::reference_existing_object>())
            .def("getShip",                     +[](const Universe& u, int id) -> const Ship* { return u.Objects().getRaw<const Ship>(UniverseObjectID{id}); },
                                                py::return_value_policy<py::reference_existing_object>())
            .def("getPlanet",                   +[](const Universe& u, int id) -> const Planet* { return u.Objects().getRaw<const Planet>(UniverseObjectID{id}); },
                                                py::return_value_policy<py::reference_existing_object>())
            .def("getSystem",                   +[](const Universe& u, int id) -> const System* { return u.Objects().getRaw<const System>(UniverseObjectID{id}); },
                                                py::return_value_policy<py::reference_existing_object>())
            .def("getField",                    +[](const Universe& u, int id) -> const Field* { return u.Objects().getRaw<const Field>(UniverseObjectID{id}); },
                                                py::return_value_policy<py::reference_existing_object>())
            .def("getBuilding",                 +[](const Universe& u, int id) -> const Building* { return u.Objects().getRaw<const Building>(UniverseObjectID{id}); },
                                                py::return_value_policy<py::reference_existing_object>())
            .def("getGenericShipDesign",        +[](const Universe& u, const std::string& name) -> const ShipDesign* { return u.GetGenericShipDesign(name); },
                                                py::return_value_policy<py::reference_existing_object>(),
                                                "Returns the ship design (ShipDesign) with the indicated name (string).")

            .add_property("fleetIDs",           make_function(ObjectIDs<Fleet>,         py::return_value_policy<py::return_by_value>()))
            .add_property("systemIDs",          make_function(ObjectIDs<System>,        py::return_value_policy<py::return_by_value>()))
            .add_property("fieldIDs",           make_function(ObjectIDs<Field>,         py::return_value_policy<py::return_by_value>()))
            .add_property("planetIDs",          make_function(ObjectIDs<Planet>,        py::return_value_policy<py::return_by_value>()))
            .add_property("shipIDs",            make_function(ObjectIDs<Ship>,          py::return_value_policy<py::return_by_value>()))
            .add_property("buildingIDs",        make_function(ObjectIDs<Building>,      py::return_value_policy<py::return_by_value>()))
            .def("destroyedObjectIDs",          +[](const Universe& u, int id) -> std::set<int> {
                                                    const auto& ekdoi{u.EmpireKnownDestroyedObjectIDs(static_cast<EmpireID>(id))};
                                                    std::set<int> retval;
                                                    for (auto obj_id : ekdoi)
                                                        retval.insert(Value(obj_id));
                                                    return retval;
                                                },
                                                py::return_value_policy<py::return_by_value>())

            .def("systemHasStarlane",           +[](const Universe& u, int system_id, EmpireID empire_id) -> bool { return u.GetPathfinder().SystemHasVisibleStarlanes(static_cast<UniverseObjectID>(system_id), u.EmpireKnownObjects(static_cast<EmpireID>(empire_id))); },
                                                py::return_value_policy<py::return_by_value>())

            .def("updateMeterEstimates",        UpdateMetersWrapper)
            .add_property("effectAccounting",   GetAccountingMapIntIndexed)

            .def("linearDistance",              +[](const Universe& u, int system1_id, int system2_id) -> double { return u.GetPathfinder().LinearDistance(static_cast<UniverseObjectID>(system1_id), static_cast<UniverseObjectID>(system2_id), u.Objects()); },
                                                py::return_value_policy<py::return_by_value>())

            .def("jumpDistance",                +[](const Universe& u, int object1_id, int object2_id) -> int { return u.GetPathfinder().JumpDistanceBetweenObjects(static_cast<UniverseObjectID>(object1_id), static_cast<UniverseObjectID>(object2_id), u.Objects()); },
                                                py::return_value_policy<py::return_by_value>(),
                                                "If two system ids are passed or both objects are within a system, "
                                                "return the jump distance between the two systems. If one object "
                                                "(e.g. a fleet) is on a starlane, then calculate the jump distance "
                                                "from both ends of the starlane to the target system and "
                                                "return the smaller one.")

            .def("shortestPath",                ShortestPath,
                                                py::return_value_policy<py::return_by_value>())

            .def("shortestNonHostilePath",      ShortestNonHostilePath,
                                                py::return_value_policy<py::return_by_value>(),
                                                "Shortest sequence of System ids and distance from System (number1) to "
                                                "System (number2) with no hostile Fleets as determined by visibility "
                                                "of Empire (number3).  (number3) must be a valid empire.")

            .def("shortestPathDistance",        +[](const Universe& u, int object1_id, int object2_id) -> double { return u.GetPathfinder().ShortestPathDistance(UniverseObjectID{object1_id}, UniverseObjectID{object2_id}, u.Objects()); },
                                                py::return_value_policy<py::return_by_value>())

            .def("leastJumpsPath",              LeastJumpsPath,
                                                py::return_value_policy<py::return_by_value>())

            .def("systemsConnected",            +[](const Universe& u, int system1_id, int system2_id, EmpireID empire_id) -> bool { return u.GetPathfinder().SystemsConnected(UniverseObjectID{system1_id}, UniverseObjectID{system2_id}, EmpireID{empire_id}); },
                                                py::return_value_policy<py::return_by_value>())

            .def("getImmediateNeighbors",       ImmediateNeighbors,
                                                py::return_value_policy<py::return_by_value>())

            .def("getSystemNeighborsMap",       SystemNeighborsMap,
                                                py::return_value_policy<py::return_by_value>())

            .def("getVisibilityTurnsMap",       GetObjEmpireVisTurns,
                                                py::return_value_policy<py::return_by_value>())

            .def("getVisibility",               &Universe::GetObjectVisibilityByEmpire,
                                                py::return_value_policy<py::return_by_value>())

            // Indexed by stat name (string), contains a map indexed by empire id,
            // contains a map from turn number (int) to stat value (double).
            .def("statRecords",                 &Universe::GetStatRecords,
                                                py::return_value_policy<py::reference_existing_object>(),
                                                "Empire statistics recorded by the server each turn. Indexed first by "
                                                "staistic name (string), then by empire id (int), then by turn "
                                                "number (int), pointing to the statisic value (double).")

            .def("dump",                        +[](const Universe& u) { DebugLogger() << u.Objects().Dump(); })
        ;

        ////////////////////
        // UniverseObject //
        ////////////////////
        py::class_<UniverseObject, boost::noncopyable>("universeObject", py::no_init)
            .add_property("id",                 +[](const UniverseObject& o) noexcept { return Value(o.ID()); })
            .add_property("name",               make_function(&UniverseObject::Name, py::return_value_policy<py::copy_const_reference>()))
            .add_property("x",                  &UniverseObject::X)
            .add_property("y",                  &UniverseObject::Y)
            .add_property("systemID",           +[](const UniverseObject& o) noexcept { return Value(o.SystemID()); })
            .add_property("unowned",            &UniverseObject::Unowned)
            .add_property("owner",              +[](const UniverseObject& o) noexcept { return Value(o.Owner()); })
            .def("ownedBy",                     +[](const UniverseObject& o, int eid) { return o.OwnedBy(EmpireID{eid}); })
            .add_property("creationTurn",       &UniverseObject::CreationTurn)
            .add_property("ageInTurns",         +[](const UniverseObject& o) { return o.AgeInTurns(IApp::GetApp()->CurrentTurn()); })
            .add_property("specials",           make_function(ObjectSpecials,        py::return_value_policy<py::return_by_value>()))
            .def("hasSpecial",                  &UniverseObject::HasSpecial)
            .def("specialAddedOnTurn",          &UniverseObject::SpecialAddedOnTurn)
            .def("contains",                    +[](const UniverseObject& o, int oid) { return o.Contains(UniverseObjectID{oid}); })
            .def("containedBy",                 +[](const UniverseObject& o, int oid) { return o.ContainedBy(UniverseObjectID{oid}); })
            .add_property("containedObjects",   +[](const UniverseObject& o) { return ToIntValueVec(o.ContainedObjectIDs()); })
            .add_property("containerObject",    +[](const UniverseObject& o) { return Value(o.ContainerObjectID()); })
            .def("currentMeterValue",           ObjectCurrentMeterValue,             py::return_value_policy<py::return_by_value>())
            .def("initialMeterValue",           ObjectInitialMeterValue,             py::return_value_policy<py::return_by_value>())
            .add_property("tags",               make_function(ObjectTagsAsStringVec, py::return_value_policy<py::return_by_value>()))
            .def("hasTag",                      +[](const UniverseObject& obj, const std::string& tag) -> bool { return obj.HasTag(tag, IApp::GetApp()->GetContext()); })
            .add_property("meters",             make_function(
                                                    +[](const UniverseObject& o) -> std::map<MeterType, Meter> { return {o.Meters().begin(), o.Meters().end()}; },
                                                    py::return_value_policy<py::return_by_value>()
                                                ))
            .def("getMeter",                    +[](const UniverseObject& o, MeterType type) -> const Meter* { return o.GetMeter(type); },
                                                py::return_internal_reference<>())
            .def("dump",                        +[](const UniverseObject& o) -> std::string { return o.Dump(); },
                                                py::return_value_policy<py::return_by_value>(),
                                                "Returns string with debug information.")
        ;

        ///////////////////
        //     Fleet     //
        ///////////////////
        py::class_<Fleet, py::bases<UniverseObject>, boost::noncopyable>("fleet", py::no_init)
            .add_property("fuel",                      +[](const Fleet& fleet) -> float { return fleet.Fuel(IApp::GetApp()->GetContext().ContextObjects()); })
            .add_property("maxFuel",                   +[](const Fleet& fleet) -> float { return fleet.MaxFuel(IApp::GetApp()->GetContext().ContextObjects()); })
            .add_property("finalDestinationID",        +[](const Fleet& fleet) -> int { return Value(fleet.FinalDestinationID()); })
            .add_property("previousSystemID",          +[](const Fleet& fleet) -> int { return Value(fleet.PreviousSystemID()); })
            .add_property("nextSystemID",              +[](const Fleet& fleet) -> int { return Value(fleet.NextSystemID()); })
            .add_property("route",                     +[](const Fleet& fleet) -> std::vector<int> { return ToIntValueVec(fleet.TravelRoute()); })
            .add_property("aggressive",                &Fleet::Aggressive)
            .add_property("obstructive",               &Fleet::Obstructive)
            .add_property("aggression",                &Fleet::Aggression)
            .add_property("speed",                     +[](const Fleet& fleet) -> float { return fleet.Speed(IApp::GetApp()->GetContext().ContextObjects()); })
            .add_property("canChangeDirectionEnRoute", &Fleet::CanChangeDirectionEnRoute)
            .add_property("hasMonsters",               +[](const Fleet& fleet) -> bool { return fleet.HasMonsters(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("hasArmedShips",             +[](const Fleet& fleet) -> bool { return fleet.HasArmedShips(IApp::GetApp()->GetContext()); })
            .add_property("hasFighterShips",           +[](const Fleet& fleet) -> bool { return fleet.HasFighterShips(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("hasColonyShips",            +[](const Fleet& fleet) -> bool { return fleet.HasColonyShips(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("hasOutpostShips",           +[](const Fleet& fleet) -> bool { return fleet.HasOutpostShips(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("hasTroopShips",             +[](const Fleet& fleet) -> bool { return fleet.HasTroopShips(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("numShips",                  &Fleet::NumShips)
            .add_property("empty",                     &Fleet::Empty)
            .add_property("shipIDs",                   +[](const Fleet& fleet) -> std::vector<int> { return ToIntValueVec(fleet.ShipIDs()); })
        ;

        //////////////////
        //     Ship     //
        //////////////////
        py::class_<Ship, py::bases<UniverseObject>, boost::noncopyable>("ship", py::no_init)
            .add_property("design",                 make_function(
                                                        +[](const Ship& ship) -> const ShipDesign* { return IApp::GetApp()->GetContext().ContextUniverse().GetShipDesign(ship.DesignID()); },
                                                        py::return_value_policy<py::reference_existing_object>()
                                                    ))
            .add_property("designID",               &Ship::DesignID)
            .add_property("fleetID",                +[](const Ship& ship) -> int { return Value(ship.FleetID()); })
            .add_property("producedByEmpireID",     +[](const Ship& ship) -> int { return Value(ship.ProducedByEmpireID()); })
            .add_property("arrivedOnTurn",          &Ship::ArrivedOnTurn)
            .add_property("lastResuppliedOnTurn",   &Ship::LastResuppliedOnTurn)
            .add_property("lastTurnActiveInCombat", &Ship::LastTurnActiveInCombat)
            .add_property("isMonster",              +[](const Ship& ship) -> bool { return ship.IsMonster(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("isArmed",                +[](const Ship& ship) -> bool { return ship.IsArmed(IApp::GetApp()->GetContext()); })
            .add_property("hasFighters",            +[](const Ship& ship) -> bool { return ship.HasFighters(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("canColonize",            +[](const Ship& ship) -> bool { const auto& context = IApp::GetApp()->GetContext(); return ship.CanColonize(context.ContextUniverse(), context.species); })
            .add_property("canInvade",              +[](const Ship& ship) -> bool { return ship.HasTroops(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("canBombard",             +[](const Ship& ship) -> bool { return ship.CanBombard(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("speciesName",            make_function(&Ship::SpeciesName,       py::return_value_policy<py::copy_const_reference>()))
            .add_property("speed",                  &Ship::Speed)
            .add_property("colonyCapacity",         +[](const Ship& ship) -> float { return ship.ColonyCapacity(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("troopCapacity",          +[](const Ship& ship) -> float { return ship.TroopCapacity(IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("orderedScrapped",        &Ship::OrderedScrapped)
            .add_property("orderedColonizePlanet",  +[](const Ship& ship) -> int { return Value(ship.OrderedColonizePlanet()); })
            .add_property("orderedInvadePlanet",    +[](const Ship& ship) -> int { return Value(ship.OrderedInvadePlanet()); })
            .def("initialPartMeterValue",           &Ship::InitialPartMeterValue)
            .def("currentPartMeterValue",           &Ship::CurrentPartMeterValue)
            .add_property("partMeters",             make_function(
                                                        +[](const Ship& ship) -> const Ship::PartMeterMap& { return ship.PartMeters(); },
                                                        py::return_internal_reference<>()
                                                    ))
            .def("getMeter",                        +[](const Ship& ship, MeterType type, const std::string& part_name) -> const Meter* { return ship.GetPartMeter(type, part_name); },
                                                    py::return_internal_reference<>())
        ;

        //////////////////
        //  ShipDesign  //
        //////////////////
        py::class_<ShipDesign, boost::noncopyable>("shipDesign", py::no_init)
            .add_property("id",                 make_function(&ShipDesign::ID,              py::return_value_policy<py::return_by_value>()))
            .add_property("name",               make_function(
                                                    +[](const ShipDesign& ship_design) -> const std::string& { return ship_design.Name(false); },
                                                    py::return_value_policy<py::copy_const_reference>()
                                                ))
            .add_property("description",        make_function(
                                                    +[](const ShipDesign& ship_design) -> const std::string& { return ship_design.Description(false); },
                                                    py::return_value_policy<py::copy_const_reference>()
                                                ))
            .add_property("designedOnTurn",     &ShipDesign::DesignedOnTurn)
            .add_property("speed",              &ShipDesign::Speed)
            .add_property("structure",          &ShipDesign::Structure)
            .add_property("shields",            &ShipDesign::Shields)
            .add_property("fuel",               &ShipDesign::Fuel)
            .add_property("detection",          &ShipDesign::Detection)
            .add_property("colonyCapacity",     &ShipDesign::ColonyCapacity)
            .add_property("troopCapacity",      &ShipDesign::TroopCapacity)
            .add_property("stealth",            &ShipDesign::Stealth)
            .add_property("industryGeneration", &ShipDesign::IndustryGeneration)
            .add_property("researchGeneration", &ShipDesign::ResearchGeneration)
            .add_property("influenceGeneration",&ShipDesign::InfluenceGeneration)
            .add_property("defense",            &ShipDesign::Defense)
            .add_property("attack",             &ShipDesign::Attack)
            .add_property("canColonize",        &ShipDesign::CanColonize)
            .add_property("canInvade",          &ShipDesign::HasTroops)
            .add_property("isArmed",            &ShipDesign::IsArmed)
            .add_property("hasFighters",        &ShipDesign::HasFighters)
            .add_property("hasDirectWeapons",   &ShipDesign::HasDirectWeapons)
            .add_property("isMonster",          &ShipDesign::IsMonster)
            .def("productionCost",              +[](const ShipDesign& ship_design, int empire_id, int location_id) -> float { return ship_design.ProductionCost(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .def("productionTime",              +[](const ShipDesign& ship_design, int empire_id, int location_id) -> int { return ship_design.ProductionTime(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .def("perTurnCost",                 +[](const ShipDesign& ship_design, int empire_id, int location_id) -> float { return ship_design.PerTurnCost(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .add_property("costTimeLocationInvariant",
                                                &ShipDesign::ProductionCostTimeLocationInvariant)
            .add_property("hull",               make_function(&ShipDesign::Hull,            py::return_value_policy<py::return_by_value>()))
            .add_property("ship_hull",          make_function(
                                                    +[](const ShipDesign& design) -> const ShipHull* { return GetShipHull(design.Hull()); },
                                                    py::return_value_policy<py::reference_existing_object>()
                                                ))
            .add_property("parts",              make_function(
                                                    +[](const ShipDesign& design) -> const std::vector<std::string>& { return design.Parts(); },
                                                    py::return_internal_reference<>()
                                                ))
            .add_property("attackStats",        make_function(
                                                    AttackStats,
                                                    py::return_value_policy<py::return_by_value>()
                                                ))

            .def("productionLocationForEmpire", +[](const ShipDesign& ship_design, int empire_id, int location_id) { return ship_design.ProductionLocation(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .def("dump",                        &ShipDesign::Dump,                          py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;
        py::def("validShipDesign",
                +[](const std::string& hull, const py::list& parts) -> bool { return ShipDesign::ValidDesign(hull, ExtractList(parts)); },
                "Returns true (boolean) if the passed hull (string) and parts"
                " (list of string) make up a valid ship design, and false (boolean)"
                " otherwise. Valid ship designs don't have any parts in slots"
                " that can't accept that type of part, and contain only hulls"
                " and parts that exist (and may also need to contain the"
                " correct number of parts - this needs to be verified).");
        py::def("getShipDesign",                +[](int id) -> const ShipDesign* { return IApp::GetApp()->GetContext().ContextUniverse().GetShipDesign(id); },
                py::return_value_policy<py::reference_existing_object>(),
                "Returns the ship design (ShipDesign) with the indicated id number (int).");
        py::def("getPredefinedShipDesign",      +[](const std::string& name) -> const ShipDesign* { return IApp::GetApp()->GetContext().ContextUniverse().GetGenericShipDesign(name); },
                py::return_value_policy<py::reference_existing_object>(),
                "Returns the ship design (ShipDesign) with the indicated name (string).");

        py::class_<ShipPart, boost::noncopyable>("shipPart", py::no_init)
            .add_property("name",               make_function(&ShipPart::Name,              py::return_value_policy<py::copy_const_reference>()))
            .add_property("partClass",          &ShipPart::Class)
            .add_property("capacity",           &ShipPart::Capacity)
            .add_property("secondaryStat",      &ShipPart::SecondaryStat)
            .add_property("mountableSlotTypes", make_function(&ShipPart::MountableSlotTypes,py::return_value_policy<py::return_by_value>()))
            .def("productionCost",              +[](const ShipPart& ship_part, int empire_id, int location_id, int design_id) -> float { return ship_part.ProductionCost(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext(), design_id); })
            .def("productionTime",              +[](const ShipPart& ship_part, int empire_id, int location_id, int design_id) -> int { return ship_part.ProductionTime(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext(), design_id); })
            .def("canMountInSlotType",          &ShipPart::CanMountInSlotType)
            .add_property("costTimeLocationInvariant",
                                                &ShipPart::ProductionCostTimeLocationInvariant)
            .def("hasTag",                      +[](const ShipPart& part, const std::string& tag) -> bool { return part.HasTag(tag); })
            .def("productionLocation",          &ShipPartProductionLocation, "Returns the result of Location condition (bool) in passed location_id (int)")
        ;
        py::def("getShipPart",                  +[](const std::string& name) -> const ShipPart* { return GetShipPart(name); }, py::return_value_policy<py::reference_existing_object>(), "Returns the ShipPart with the indicated name (string).");

        py::class_<ShipHull, boost::noncopyable>("shipHull", py::no_init)
            .add_property("name",               make_function(&ShipHull::Name,              py::return_value_policy<py::copy_const_reference>()))
            .add_property("numSlots",           make_function(
                                                    +[](const ShipHull& hull) -> unsigned int { return hull.NumSlots(); },
                                                    py::return_value_policy<py::return_by_value>()
                                                ))
            .add_property("structure",          &ShipHull::Structure)
            .add_property("stealth",            &ShipHull::Stealth)
            .add_property("fuel",               &ShipHull::Fuel)
            .add_property("speed",              &ShipHull::Speed)
            .add_property("detection",          &ShipHull::Detection)
            .def("numSlotsOfSlotType",          +[](const ShipHull& hull, ShipSlotType slot_type) -> unsigned int { return hull.NumSlots(slot_type); })
            .add_property("slots",              make_function(
                                                    HullSlots,
                                                    py::return_value_policy<py::return_by_value>()
                                                ))
            .def("productionCost",              +[](const ShipHull& ship_hull, int empire_id, int location_id, int design_id) -> float { return ship_hull.ProductionCost(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext(), design_id); })
            .def("productionTime",              +[](const ShipHull& ship_hull, int empire_id, int location_id, int design_id) -> int { return ship_hull.ProductionTime(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext(), design_id); })
            .add_property("costTimeLocationInvariant",
                                                &ShipHull::ProductionCostTimeLocationInvariant)
            .def("hasTag",                      +[](const ShipHull& hull, const std::string& tag) -> bool { return hull.HasTag(tag); })
            .def("productionLocation",          &HullProductionLocation, "Returns the result of Location condition (bool) in passed location_id (int)")
        ;
        py::def("getShipHull",                  +[](const std::string& name) { return GetShipHull(name); },
                                                py::return_value_policy<py::reference_existing_object>(),
                                                "Returns the ship hull with the indicated name (string).");

        //////////////////
        //   Building   //
        //////////////////
        py::class_<Building, py::bases<UniverseObject>, boost::noncopyable>("building", py::no_init)
            .add_property("buildingTypeName",   make_function(&Building::BuildingTypeName,  py::return_value_policy<py::copy_const_reference>()))
            .add_property("planetID",           +[](const Building& building) -> int { return Value(building.PlanetID()); })
            .add_property("producedByEmpireID", +[](const Building& building) -> int { return Value(building.ProducedByEmpireID()); })
            .add_property("orderedScrapped",    &Building::OrderedScrapped)
        ;

        //////////////////
        // BuildingType //
        //////////////////
        py::class_<BuildingType, boost::noncopyable>("buildingType", py::no_init)
            .add_property("name",               make_function(&BuildingType::Name,          py::return_value_policy<py::return_by_value>()))
            .add_property("description",        make_function(&BuildingType::Description,   py::return_value_policy<py::copy_const_reference>()))
            .def("productionCost",              +[](const BuildingType& bt, int empire_id, int location_id) -> float { return bt.ProductionCost(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .def("productionTime",              +[](const BuildingType& bt, int empire_id, int location_id) -> int { return bt.ProductionTime(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .def("perTurnCost",                 +[](const BuildingType& bt, int empire_id, int location_id) -> float { return bt.PerTurnCost(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .def("captureResult",               +[](const BuildingType& bt, int empire1_id, int empire2_id, int object_id) { return bt.GetCaptureResult(EmpireID{empire1_id}, EmpireID{empire2_id}, UniverseObjectID{object_id}, false); })
            .def("canBeProduced",               +[](const BuildingType& building_type, int empire_id, int location_id) -> bool { return building_type.ProductionLocation(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .def("canBeEnqueued",               +[](const BuildingType& building_type, int empire_id, int location_id) -> bool { return building_type.EnqueueLocation(EmpireID{empire_id}, UniverseObjectID{location_id}, IApp::GetApp()->GetContext()); })
            .add_property("costTimeLocationInvariant",
                                                &BuildingType::ProductionCostTimeLocationInvariant)
            .def("dump",                        &BuildingType::Dump,                        py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;
        py::def("getBuildingType",              +[](const std::string& name) { return GetBuildingType(name); },
                                                py::return_value_policy<py::reference_existing_object>(),
                                                "Returns the building type (BuildingType) with the indicated name (string).");

        //////////////////
        //    Planet    //
        //////////////////
        py::class_<Planet, py::bases<UniverseObject>, boost::noncopyable>("planet", py::no_init)
            .add_property("speciesName",                    make_function(&Planet::SpeciesName, py::return_value_policy<py::copy_const_reference>()))
            .add_property("focus",                          make_function(&Planet::Focus,       py::return_value_policy<py::copy_const_reference>()))
            .add_property("turnsSinceFocusChange",          +[](const Planet& rc) { return rc.TurnsSinceFocusChange(IApp::GetApp()->CurrentTurn()); })
            .add_property("availableFoci",                  +[](const Planet& rc) -> std::vector<std::string> { return ViewsToStrings(rc.AvailableFoci(IApp::GetApp()->GetContext())); })
            .add_property("size",                           &Planet::Size)
            .add_property("type",                           &Planet::Type)
            .add_property("originalType",                   &Planet::OriginalType)
            .add_property("distanceFromOriginalType",       &Planet::DistanceFromOriginalType)
            .def("environmentForSpecies",                   +[](const Planet& planet, const std::string& species) { return planet.EnvironmentForSpecies(IApp::GetApp()->GetContext().species, species); })
            .def("nextBetterPlanetTypeForSpecies",          &Planet::NextBetterPlanetTypeForSpecies)
            .add_property("clockwiseNextPlanetType",        &Planet::ClockwiseNextPlanetType)
            .add_property("counterClockwiseNextPlanetType", &Planet::CounterClockwiseNextPlanetType)
            .add_property("nextLargerPlanetSize",           &Planet::NextLargerPlanetSize)
            .add_property("nextSmallerPlanetSize",          &Planet::NextSmallerPlanetSize)
            .def("OrbitalPositionOnTurn",                   &Planet::OrbitalPositionOnTurn)
            .add_property("lastTurnAttackedByShip",         &Planet::LastTurnAttackedByShip)
            .add_property("lastTurnColonized",              &Planet::LastTurnColonized)
            .add_property("lastTurnConquered",              &Planet::LastTurnConquered)
            .add_property("ownerBeforeLastConquered",       +[](const Planet& p) -> int { return Value(p.OwnerBeforeLastConquered()); })
            .add_property("lastInvadedByEmpire",            +[](const Planet& p) -> int { return Value(p.LastInvadedByEmpire()); })
            .add_property("lastColonizedByEmpire",          +[](const Planet& p) -> int { return Value(p.LastColonizedByEmpire()); })
            .add_property("buildingIDs",                    +[](const Planet& planet) -> std::vector<int> { return ToIntValueVec(planet.BuildingIDs()); })
            .add_property("habitableSize",                  &Planet::HabitableSize)
        ;

        //////////////////
        //    System    //
        //////////////////
        py::class_<System, py::bases<UniverseObject>, boost::noncopyable>("system", py::no_init)
            .add_property("starType",           &System::GetStarType)
            .add_property("numStarlanes",       &System::NumStarlanes, "Number of starlanes connecting to this sytsem")
            .def("HasStarlaneToSystemID",       +[](const System& system, int dest_id) -> bool { return system.HasStarlaneTo(UniverseObjectID{dest_id}); }, "true if the passed in ID (int) is the ID of a system this system has a starlane connection with")
            .add_property("starlanesWormholes", +[](const System& system) -> std::vector<int> { return ToIntValueVec(system.Starlanes()); }, "[deprecated] use starlanes")
            .add_property("starlanes",          +[](const System& system) -> std::vector<int> { return ToIntValueVec(system.Starlanes()); }, "IDs of systems to which this system has starlane connections")
            .add_property("planetIDs",          +[](const System& system) -> std::vector<int> { return ToIntValueVec(system.PlanetIDs()); }, "IDs of planets in this system")
            .add_property("buildingIDs",        +[](const System& system) -> std::vector<int> { return ToIntValueVec(system.BuildingIDs()); }, "IDs of buildings in this system")
            .add_property("fleetIDs",           +[](const System& system) -> std::vector<int> { return ToIntValueVec(system.FleetIDs()); }, "IDs of fleets in this system")
            .add_property("shipIDs",            +[](const System& system) -> std::vector<int> { return ToIntValueVec(system.ShipIDs()); }, "IDs of ships in this system")
            .add_property("fieldIDs",           +[](const System& system) -> std::vector<int> { return ToIntValueVec(system.FieldIDs()); }, "IDs of fields in this system. Other fields may enclose this system but not be contained within this system.")
            .add_property("lastTurnBattleHere", &System::LastTurnBattleHere)
        ;

        //////////////////
        //    Field     //
        //////////////////
        py::class_<Field, py::bases<UniverseObject>, boost::noncopyable>("field", py::no_init)
            .add_property("fieldTypeName",      make_function(&Field::FieldTypeName,        py::return_value_policy<py::copy_const_reference>()))
            .def("inField",                     +[](const Field& field, const UniverseObject& obj) -> bool { return field.InField(obj.X(), obj.Y()); })
            .def("inField",                     +[](const Field& field, double x, double y) -> bool { return field.InField(x, y); })
        ;

        //////////////////
        //   FieldType  //
        //////////////////
        py::class_<FieldType, boost::noncopyable>("fieldType", py::no_init)
            .add_property("name",               make_function(&FieldType::Name,             py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",        make_function(&FieldType::Description,      py::return_value_policy<py::copy_const_reference>()))
            .def("dump",                        &FieldType::Dump,                           py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;
        py::def("getFieldType",                 +[](const std::string& name) { return GetFieldType(name); },
                                                py::return_value_policy<py::reference_existing_object>());


        /////////////////
        //   Special   //
        /////////////////
        py::class_<Special, boost::noncopyable>("special", py::no_init)
            .add_property("name",               make_function(&Special::Name,           py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",        &Special::Description)
            .add_property("spawnrate",          make_function(&Special::SpawnRate,      py::return_value_policy<py::return_by_value>()))
            .add_property("spawnlimit",         make_function(&Special::SpawnLimit,     py::return_value_policy<py::return_by_value>()))
            .def("dump",                        &Special::Dump,                         py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
            .def("initialCapacity",             +[](const Special& special, int obj_id) -> float { return special.InitialCapacity(UniverseObjectID{obj_id}, IApp::GetApp()->GetContext()); })
        ;
        py::def("getSpecial",                   +[](const std::string& name) { return ::GetSpecial(name); },
                                                py::return_value_policy<py::reference_existing_object>(),
                                                "Returns the special (Special) with the indicated name (string).");

        /////////////////
        //   Species   //
        /////////////////
        py::class_<Species, boost::noncopyable>("species", py::no_init)
            .add_property("name",               make_function(&Species::Name,           py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",        make_function(&Species::Description,    py::return_value_policy<py::copy_const_reference>()))
            .add_property("homeworlds",         &SpeciesHomeworlds) // TODO: SpeciesManager::SpeciesShipsDestroyed, GetSpeciesEmpireOpinionsMap, GetSpeciesSpeciesOpinionsMap
            .add_property("foci",               &SpeciesFoci)
            .add_property("preferredFocus",     make_function(&Species::DefaultFocus,   py::return_value_policy<py::copy_const_reference>()))
            .add_property("canColonize",        make_function(&Species::CanColonize,    py::return_value_policy<py::return_by_value>()))
            .add_property("canProduceShips",    make_function(&Species::CanProduceShips,py::return_value_policy<py::return_by_value>()))
            .add_property("native",             &Species::Native)
            .add_property("tags",               +[](const Species& s) { return SpeciesTags(s, SpeciesInfo::TAGS); })
            .add_property("spawnrate",          make_function(&Species::SpawnRate,      py::return_value_policy<py::return_by_value>()))
            .add_property("spawnlimit",         make_function(&Species::SpawnLimit,     py::return_value_policy<py::return_by_value>()))
            .add_property("likes",              +[](const Species& s) { return SpeciesTags(s, SpeciesInfo::LIKES); })
            .add_property("dislikes",           +[](const Species& s) { return SpeciesTags(s, SpeciesInfo::DISLIKES); })
            .def("getPlanetEnvironment",        &Species::GetPlanetEnvironment)
            .def("dump",                        &Species::Dump,                         py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;
        py::def("getSpecies",                   +[](const std::string& name) { return IApp::GetApp()->GetContext().species.GetSpecies(name); },
                                                py::return_value_policy<py::reference_existing_object>(),
                                                "Returns the species (Species) with the indicated name (string).");
    }

    void WrapGalaxySetupData() {
        py::class_<GalaxySetupData>("GalaxySetupData")
            .add_property("seed",               make_function(&GalaxySetupData::GetSeed,            py::return_value_policy<py::return_by_value>()))
            .add_property("size",               make_function(&GalaxySetupData::GetSize,            py::return_value_policy<py::return_by_value>()))
            .add_property("shape",              make_function(&GalaxySetupData::GetShape,           py::return_value_policy<py::return_by_value>()))
            .add_property("age",                make_function(&GalaxySetupData::GetAge,             py::return_value_policy<py::return_by_value>()))
            .add_property("starlaneFrequency",  make_function(&GalaxySetupData::GetStarlaneFreq,    py::return_value_policy<py::return_by_value>()))
            .add_property("planetDensity",      make_function(&GalaxySetupData::GetPlanetDensity,   py::return_value_policy<py::return_by_value>()))
            .add_property("specialsFrequency",  make_function(&GalaxySetupData::GetSpecialsFreq,    py::return_value_policy<py::return_by_value>()))
            .add_property("monsterFrequency",   make_function(&GalaxySetupData::GetMonsterFreq,     py::return_value_policy<py::return_by_value>()))
            .add_property("nativeFrequency",    make_function(&GalaxySetupData::GetNativeFreq,      py::return_value_policy<py::return_by_value>()))
            .add_property("maxAIAggression",    make_function(&GalaxySetupData::GetAggression,      py::return_value_policy<py::return_by_value>()))
            .add_property("gameUID",            make_function(&GalaxySetupData::GetGameUID,         py::return_value_policy<py::return_by_value>()),
                                                &GalaxySetupData::SetGameUID);

        py::class_<GameRules, boost::noncopyable>("GameRules", py::no_init)
            .add_property("empty",              make_function(&GameRules::Empty,                py::return_value_policy<py::return_by_value>()))
            .def("getRulesAsStrings",           &GameRules::GetRulesAsStrings,
                                                py::return_value_policy<py::return_by_value>())
            .def("ruleExists",                  +[](GameRules& rules, const std::string& name) -> bool { return rules.RuleExists(name); })
            .def("ruleExistsWithType",          +[](GameRules& rules, const std::string& name, GameRule::Type type) -> bool { return rules.RuleExists(name, type); })
            .def("getDescription",              &GameRules::GetDescription,
                                                py::return_value_policy<py::copy_const_reference>())
            .def("getToggle",                   &GameRules::Get<bool>)
            .def("getInt",                      &GameRules::Get<int>)
            .def("getDouble",                   &GameRules::Get<double>)
            .def("getString",                   &GameRules::Get<std::string>,
                                                py::return_value_policy<py::return_by_value>());

        py::def("getGameRules",
                &GetGameRules,
                py::return_value_policy<py::reference_existing_object>(),
                "Returns the game rules manager, which can be used to look up"
                " the names (string) of rules are defined with what type"
                " (boolean / toggle, int, double, string), and what values the"
                " rules have in the current game.");
    }
}
