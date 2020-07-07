#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "../universe/Building.h"
#include "../universe/BuildingType.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../universe/Enums.h"
#include "../universe/Field.h"
#include "../universe/FieldType.h"
#include "../universe/Fleet.h"
#include "../universe/Pathfinder.h"
#include "../universe/Planet.h"
#include "../universe/PopCenter.h"
#include "../universe/ResourceCenter.h"
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
#include "../universe/UniverseObjectVisitors.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"

namespace py = boost::python;


#if defined(_MSC_VER)
#  if (_MSC_VER == 1900)
namespace boost {
    template<>
    const volatile UniverseObject*  get_pointer(const volatile UniverseObject* p) { return p; }
    template<>
    const volatile Fleet*           get_pointer(const volatile Fleet* p) { return p; }
    template<>
    const volatile Ship*            get_pointer(const volatile Ship* p) { return p; }
    template<>
    const volatile Planet*          get_pointer(const volatile Planet* p) { return p; }
    template<>
    const volatile System*          get_pointer(const volatile System* p) { return p; }
    template<>
    const volatile Field*           get_pointer(const volatile Field* p) { return p; }
    template<>
    const volatile Building*        get_pointer(const volatile Building* p) { return p; }
    template<>
    const volatile Universe*        get_pointer<const volatile Universe>(const volatile Universe* p) { return p; }
}
#  endif
#endif

namespace {
    void                    DumpObjects(const Universe& universe)
    { DebugLogger() << universe.Objects().Dump(); }
    std::string             ObjectDump(const UniverseObject& obj)
    { return obj.Dump(0); }

    // We're returning the result of operator-> here so that python doesn't
    // need to deal with std::shared_ptr class.
    // Please don't use this trick elsewhere to grab a raw UniverseObject*!
    const UniverseObject*   GetUniverseObjectP(const Universe& universe, int id)
    { return ::Objects().get<UniverseObject>(id).operator->(); }
    const Ship*             GetShipP(const Universe& universe, int id)
    { return ::Objects().get<Ship>(id).operator->(); }
    const Fleet*            GetFleetP(const Universe& universe, int id)
    { return ::Objects().get<Fleet>(id).operator->(); }
    const Planet*           GetPlanetP(const Universe& universe, int id)
    { return ::Objects().get<Planet>(id).operator->(); }
    const System*           GetSystemP(const Universe& universe, int id)
    { return ::Objects().get<System>(id).operator->(); }
    const Field*            GetFieldP(const Universe& universe, int id)
    { return ::Objects().get<Field>(id).operator->();  }
    const Building*         GetBuildingP(const Universe& universe, int id)
    { return ::Objects().get<Building>(id).operator->(); }

    template<typename T>
    std::vector<int> ObjectIDs(const Universe& universe)
    {
        std::vector<int> result;
        result.reserve(universe.Objects().size<T>());
        for (const auto& obj : universe.Objects().all<T>())
            result.push_back(obj->ID());
        return result;
    }

    std::vector<std::string>SpeciesFoci(const Species& species) {
        std::vector<std::string> retval;
        retval.reserve(species.Foci().size());
        for (const FocusType& focus : species.Foci())
            retval.push_back(focus.Name());
        return retval;
    }

    void UpdateMetersWrapper(const Universe& universe, const py::object& objIter) {
        py::stl_input_iterator<int> begin(objIter), end;
        std::vector<int> objvec(begin, end);
        GetUniverse().UpdateMeterEstimates(objvec);
    }

    //void (Universe::*UpdateMeterEstimatesVoidFunc)(void) = &Universe::UpdateMeterEstimates;

    double LinearDistance(const Universe& universe, int system1_id, int system2_id) {
        double retval = universe.GetPathfinder()->LinearDistance(system1_id, system2_id);
        return retval;
    }
    std::function<double (const Universe&, int, int)> LinearDistanceFunc = &LinearDistance;

    int JumpDistanceBetweenObjects(const Universe& universe, int object1_id, int object2_id) {
        return universe.GetPathfinder()->JumpDistanceBetweenObjects(object1_id, object2_id);
    }
    std::function<int (const Universe&, int, int)> JumpDistanceFunc = &JumpDistanceBetweenObjects;

    std::vector<int> ShortestPath(const Universe& universe, int start_sys, int end_sys, int empire_id) {
        std::vector<int> retval;
        std::pair<std::list<int>, int> path = universe.GetPathfinder()->ShortestPath(start_sys, end_sys, empire_id);
        std::copy(path.first.begin(), path.first.end(), std::back_inserter(retval));
        return retval;
    }
    std::function<std::vector<int> (const Universe&, int, int, int)> ShortestPathFunc = &ShortestPath;

    std::vector<int> ShortestNonHostilePath(const Universe& universe, int start_sys, int end_sys, int empire_id) {
        std::vector<int> retval;
        auto fleet_pred = std::make_shared<HostileVisitor>(empire_id);
        std::pair<std::list<int>, int> path = universe.GetPathfinder()->ShortestPath(start_sys, end_sys, empire_id, fleet_pred);
        std::copy(path.first.begin(), path.first.end(), std::back_inserter(retval));
        return retval;
    }
    std::function<std::vector<int> (const Universe&, int, int, int)> ShortestNonHostilePathFunc = &ShortestNonHostilePath;

    double ShortestPathDistance(const Universe& universe, int object1_id, int object2_id) {
        return universe.GetPathfinder()->ShortestPathDistance(object1_id, object2_id);
    }
    std::function<double (const Universe&, int, int)> ShortestPathDistanceFunc = &ShortestPathDistance;

    std::vector<int> LeastJumpsPath(const Universe& universe, int start_sys, int end_sys, int empire_id) {
        std::vector<int> retval;
        std::pair<std::list<int>, int> path = universe.GetPathfinder()->LeastJumpsPath(start_sys, end_sys, empire_id);
        std::copy(path.first.begin(), path.first.end(), std::back_inserter(retval));
        return retval;
    }
    std::function<std::vector<int> (const Universe&, int, int, int)> LeastJumpsFunc = &LeastJumpsPath;

    bool SystemsConnectedP(const Universe& universe, int system1_id, int system2_id, int empire_id=ALL_EMPIRES) {
        //DebugLogger() << "SystemsConnected!(" << system1_id << ", " << system2_id << ")";
        bool retval = universe.GetPathfinder()->SystemsConnected(system1_id, system2_id, empire_id);
        //DebugLogger() << "SystemsConnected! retval: " << retval;
        return retval;
    }
    std::function<bool (const Universe&, int, int, int)> SystemsConnectedFunc = &SystemsConnectedP;

    bool SystemHasVisibleStarlanesP(const Universe& universe, int system_id, int empire_id = ALL_EMPIRES) {
        return universe.GetPathfinder()->SystemHasVisibleStarlanes(system_id, empire_id);
    }
    std::function<bool (const Universe&, int, int)> SystemHasVisibleStarlanesFunc = &SystemHasVisibleStarlanesP;

    std::vector<int> ImmediateNeighborsP(const Universe& universe, int system1_id, int empire_id = ALL_EMPIRES) {
        std::vector<int> retval;
        for (const auto& entry : universe.GetPathfinder()->ImmediateNeighbors(system1_id, empire_id))
        { retval.push_back(entry.second); }
        return retval;
    }
    std::function<std::vector<int> (const Universe&, int, int)> ImmediateNeighborsFunc = &ImmediateNeighborsP;

    std::map<int, double> SystemNeighborsMapP(const Universe& universe, int system1_id, int empire_id = ALL_EMPIRES) {
        std::map<int, double> retval;
        for (const auto& entry : universe.GetPathfinder()->ImmediateNeighbors(system1_id, empire_id))
        { retval[entry.second] = entry.first; }
        return retval;
    }
    std::function<std::map<int, double> (const Universe&, int, int)> SystemNeighborsMapFunc = &SystemNeighborsMapP;

    const Meter*            (UniverseObject::*ObjectGetMeter)(MeterType) const =                &UniverseObject::GetMeter;

    std::map<MeterType, Meter> ObjectMetersP(const UniverseObject& object)
    { return std::map<MeterType, Meter>{object.Meters().begin(), object.Meters().end()}; }
    std::function<std::map<MeterType, Meter> (const UniverseObject&)> ObjectMetersFunc = &ObjectMetersP;

    std::vector<std::string> ObjectSpecials(const UniverseObject& object) {
        std::vector<std::string> retval;
        retval.reserve(object.Specials().size());
        for (const auto& special : object.Specials())
        { retval.push_back(special.first); }
        return retval;
    }

    const Meter*            (Ship::*ShipGetPartMeter)(MeterType, const std::string&) const =    &Ship::GetPartMeter;
    const auto&             (Ship::*ShipPartMeters)(void) const =                               &Ship::PartMeters;

    float ObjectCurrentMeterValueP(const UniverseObject& o, MeterType meter_type) {
        if (auto* m = o.GetMeter(meter_type))
            return m->Current();
        return 0.0f;
    }
    std::function<float(const UniverseObject&, MeterType)> ObjectCurrentMeterValueFunc = &ObjectCurrentMeterValueP;

    float ObjectInitialMeterValueP(const UniverseObject& o, MeterType meter_type) {
        if (auto* m = o.GetMeter(meter_type))
            return m->Initial();
        return 0.0f;
    }
    std::function<float(const UniverseObject&, MeterType)> ObjectInitialMeterValueFunc = &ObjectInitialMeterValueP;

    const ShipHull* ShipDesignHullP(const ShipDesign& design)
    { return GetShipHull(design.Hull()); }
    std::function<const ShipHull*(const ShipDesign& ship)> ShipDesignHullFunc = &ShipDesignHullP;

    const std::string& ShipDesignName(const ShipDesign& ship_design)
    { return ship_design.Name(false); }
    std::function<const std::string& (const ShipDesign&)> ShipDesignNameFunc = &ShipDesignName;

    const std::string& ShipDesignDescription(const ShipDesign& ship_design)
    { return ship_design.Description(false); }
    std::function<const std::string& (const ShipDesign&)> ShipDesignDescriptionFunc = &ShipDesignDescription;

    bool                    (*ValidDesignHullAndParts)(const std::string& hull,
                                                       const std::vector<std::string>& parts) = &ShipDesign::ValidDesign;

    const std::vector<std::string>& (ShipDesign::*PartsVoid)(void) const =                      &ShipDesign::Parts;
    // The following (PartsSlotType) is not currently used, but left as an example for this kind of wrapper
    //std::vector<std::string>        (ShipDesign::*PartsSlotType)(ShipSlotType) const =          &ShipDesign::Parts;

    std::vector<int> AttackStatsP(const ShipDesign& ship_design) {
        std::vector<int> results;
        results.reserve(ship_design.Parts().size());
        for (const std::string& part_name : ship_design.Parts()) {
            const ShipPart* part = GetShipPart(part_name);
            if (part && part->Class() == PC_DIRECT_WEAPON)  // TODO: handle other weapon classes when they are implemented
                results.push_back(part->Capacity());
        }
        return results;
    }
    std::function<std::vector<int> (const ShipDesign&)> AttackStatsFunc = &AttackStatsP;

    std::vector<ShipSlotType> HullSlots(const ShipHull& hull) {
        std::vector<ShipSlotType> retval;
        for (const ShipHull::Slot& slot : hull.Slots())
            retval.push_back(slot.type);
        return retval;
    }
    std::function<std::vector<ShipSlotType> (const ShipHull&)> HullSlotsFunc = &HullSlots;

    unsigned int            (ShipHull::*NumSlotsTotal)(void) const =                            &ShipHull::NumSlots;
    unsigned int            (ShipHull::*NumSlotsOfSlotType)(ShipSlotType) const =               &ShipHull::NumSlots;

    bool ObjectInField(const Field& field, const UniverseObject& obj)
    { return field.InField(obj.X(), obj.Y()); }
    bool                    (Field::*LocationInField)(double x, double y) const =               &Field::InField;

    float                   (Special::*SpecialInitialCapacityOnObject)(int) const =             &Special::InitialCapacity;

    bool EnqueueLocationTest(const BuildingType& building_type, int empire_id, int loc_id)
    { return building_type.EnqueueLocation(empire_id, loc_id);}

    bool HullProductionLocation(const ShipHull& hull, int location_id) {
        auto location = Objects().get(location_id);
        if (!location) {
            ErrorLogger() << "UniverseWrapper::HullProductionLocation Could not find location with id " << location_id;
            return false;
        }
        ScriptingContext location_as_source_context(location, location);
        return hull.Location()->Eval(location_as_source_context, location);
    }

    bool ShipPartProductionLocation(const ShipPart& part_type, int location_id) {
        auto location = Objects().get(location_id);
        if (!location) {
            ErrorLogger() << "UniverseWrapper::PartTypeProductionLocation Could not find location with id " << location_id;
            return false;
        }
        ScriptingContext location_as_source_context(location, location);
        return part_type.Location()->Eval(location_as_source_context, location);
    }

    bool RuleExistsAnyType(const GameRules& rules, const std::string& name)
    { return rules.RuleExists(name); }
    bool RuleExistsWithType(const GameRules& rules, const std::string& name, GameRules::Type type)
    { return rules.RuleExists(name, type); }
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
        py::class_<std::map<int, double>>("IntDblMap")
            .def(py::map_indexing_suite<std::map<int, double>, true>())
        ;
        py::class_<std::map<int, float>>("IntFltMap")
            .def(py::map_indexing_suite<std::map<int, float>, true>())
        ;
        py::class_<std::map<Visibility, int>>("VisibilityIntMap")
            .def(py::map_indexing_suite<std::map<Visibility, int>, true>())
        ;
        py::class_<std::vector<ShipSlotType>>("ShipSlotVec")
            .def(py::vector_indexing_suite<std::vector<ShipSlotType>, true>())
        ;
        py::class_<std::map<std::string, std::string>>("StringsMap")
            .def(py::map_indexing_suite<std::map<std::string, std::string>, true>())
        ;

        py::class_<std::map<MeterType, Meter>>("MeterTypeMeterMap")
            .def(py::map_indexing_suite<std::map<MeterType, Meter>, true>())
        ;

        // typedef std::map<std::pair<MeterType, std::string>, Meter> PartMeterMap;
        py::class_<std::pair<MeterType, std::string>>("MeterTypeStringPair")
            .add_property("meterType",  &std::pair<MeterType, std::string>::first)
            .add_property("string",     &std::pair<MeterType, std::string>::second)
        ;
        py::class_<Ship::PartMeterMap>("ShipPartMeterMap")
            .def(py::map_indexing_suite<Ship::PartMeterMap>())
        ;

        py::class_<std::map<std::string, std::map<int, std::map<int, double>>>>("StatRecordsMap")
            .def(py::map_indexing_suite<std::map<std::string, std::map<int, std::map<int, double>>>, true>())
        ;
        py::class_<std::map<int, std::map<int, double>>>("IntIntDblMapMap")
            .def(py::map_indexing_suite<std::map<int, std::map<int, double>>, true>())
        ;

        ///////////////////////////
        //   Effect Accounting   //
        ///////////////////////////
        py::class_<Effect::EffectCause>("EffectCause")
            .add_property("causeType",          &Effect::AccountingInfo::cause_type)
            .def_readonly("specificCause",      &Effect::AccountingInfo::specific_cause)
            .def_readonly("customLabel",        &Effect::AccountingInfo::custom_label)
        ;
        py::class_<Effect::AccountingInfo, py::bases<Effect::EffectCause>>("AccountingInfo")
            .add_property("sourceID",           &Effect::AccountingInfo::source_id)
            .add_property("meterChange",        &Effect::AccountingInfo::meter_change)
            .add_property("meterRunningTotal",  &Effect::AccountingInfo::running_meter_total)
        ;
        py::class_<std::vector<Effect::AccountingInfo>>("AccountingInfoVec")
            .def(py::vector_indexing_suite<std::vector<Effect::AccountingInfo>, true>())
        ;
        py::class_<std::map<MeterType, std::vector<Effect::AccountingInfo>>>("MeterTypeAccountingInfoVecMap")
            .def(py::map_indexing_suite<std::map<MeterType, std::vector<Effect::AccountingInfo>>, true>())
        ;
        py::class_<Effect::AccountingMap>("TargetIDAccountingMapMap")
            .def(py::map_indexing_suite<Effect::AccountingMap, true>())
        ;

        ///////////////
        //   Meter   //
        ///////////////
        py::class_<Meter, boost::noncopyable>("meter", py::no_init)
            .add_property("current",            &Meter::Current)
            .add_property("initial",            &Meter::Initial)
            .def("dump",                        &Meter::Dump,                       py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;

        ////////////////////
        //    Universe    //
        ////////////////////
        py::class_<Universe, boost::noncopyable>("universe", py::no_init)
            .def("getObject",                   make_function(GetUniverseObjectP,   py::return_value_policy<py::reference_existing_object>()))
            .def("getFleet",                    make_function(GetFleetP,            py::return_value_policy<py::reference_existing_object>()))
            .def("getShip",                     make_function(GetShipP,             py::return_value_policy<py::reference_existing_object>()))
            .def("getPlanet",                   make_function(GetPlanetP,           py::return_value_policy<py::reference_existing_object>()))
            .def("getSystem",                   make_function(GetSystemP,           py::return_value_policy<py::reference_existing_object>()))
            .def("getField",                    make_function(GetFieldP,            py::return_value_policy<py::reference_existing_object>()))
            .def("getBuilding",                 make_function(GetBuildingP,         py::return_value_policy<py::reference_existing_object>()))
            .def("getGenericShipDesign",        &Universe::GetGenericShipDesign,    py::return_value_policy<py::reference_existing_object>(), "Returns the ship design (ShipDesign) with the indicated name (string).")

            .add_property("allObjectIDs",       make_function(ObjectIDs<UniverseObject>,py::return_value_policy<py::return_by_value>()))
            .add_property("fleetIDs",           make_function(ObjectIDs<Fleet>,         py::return_value_policy<py::return_by_value>()))
            .add_property("systemIDs",          make_function(ObjectIDs<System>,        py::return_value_policy<py::return_by_value>()))
            .add_property("fieldIDs",           make_function(ObjectIDs<Field>,         py::return_value_policy<py::return_by_value>()))
            .add_property("planetIDs",          make_function(ObjectIDs<Planet>,        py::return_value_policy<py::return_by_value>()))
            .add_property("shipIDs",            make_function(ObjectIDs<Ship>,          py::return_value_policy<py::return_by_value>()))
            .add_property("buildingIDs",        make_function(ObjectIDs<Building>,      py::return_value_policy<py::return_by_value>()))
            .def("destroyedObjectIDs",          make_function(&Universe::EmpireKnownDestroyedObjectIDs,
                                                              py::return_value_policy<py::return_by_value>()))

            .def("systemHasStarlane",           make_function(
                                                    SystemHasVisibleStarlanesFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<bool, const Universe&, int, int>()
                                                ))

            .def("updateMeterEstimates",        &UpdateMetersWrapper)
            .add_property("effectAccounting",   make_function(&Universe::GetEffectAccountingMap,
                                                                                    py::return_value_policy<py::reference_existing_object>()))

            .def("linearDistance",              make_function(
                                                    LinearDistanceFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<double, const Universe&, int, int>()
                                                ))

            .def("jumpDistance",                make_function(
                                                    JumpDistanceFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<int, const Universe&, int, int>()
                                                ),
                                                "If two system ids are passed or both objects are within a system, "
                                                "return the jump distance between the two systems. If one object "
                                                "(e.g. a fleet) is on a starlane, then calculate the jump distance "
                                                "from both ends of the starlane to the target system and "
                                                "return the smaller one."
                                                )

            .def("shortestPath",                make_function(
                                                    ShortestPathFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe&, int, int, int>()
                                                ))

            .def("shortestNonHostilePath",      make_function(
                                                    ShortestNonHostilePathFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe&, int, int, int>()
                                                ),
                                                "Shortest sequence of System ids and distance from System (number1) to "
                                                "System (number2) with no hostile Fleets as determined by visibility "
                                                "of Empire (number3).  (number3) must be a valid empire."
                                                )

            .def("shortestPathDistance",        make_function(
                                                    ShortestPathDistanceFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<double, const Universe&, int, int>()
                                                ))

            .def("leastJumpsPath",              make_function(
                                                    LeastJumpsFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe&, int, int, int>()
                                                ))

            .def("systemsConnected",            make_function(
                                                    SystemsConnectedFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<bool, const Universe&, int, int, int>()
                                                ))

            .def("getImmediateNeighbors",       make_function(
                                                    ImmediateNeighborsFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe&, int, int>()
                                                ))

            .def("getSystemNeighborsMap",       make_function(
                                                    SystemNeighborsMapFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<std::map<int, double>, const Universe&, int, int>()
                                                ))

            .def("getVisibilityTurnsMap",       make_function(
                                                    &Universe::GetObjectVisibilityTurnMapByEmpire,
                                                    py::return_value_policy<py::return_by_value>()
                                                ))

            .def("getVisibility",               make_function(
                                                    &Universe::GetObjectVisibilityByEmpire,
                                                    py::return_value_policy<py::return_by_value>()
                                                ))

            // Indexed by stat name (string), contains a map indexed by empire id,
            // contains a map from turn number (int) to stat value (double).
            .def("statRecords",                 make_function(
                                                    &Universe::GetStatRecords,
                                                    py::return_value_policy<py::reference_existing_object>()),
                                                "Empire statistics recorded by the server each turn. Indexed first by "
                                                "staistic name (string), then by empire id (int), then by turn "
                                                "number (int), pointing to the statisic value (double)."
                                                )

            .def("dump",                        &DumpObjects)
        ;

        ////////////////////
        // UniverseObject //
        ////////////////////
        py::class_<UniverseObject, boost::noncopyable>("universeObject", py::no_init)
            .add_property("id",                 &UniverseObject::ID)
            .add_property("name",               make_function(&UniverseObject::Name,        py::return_value_policy<py::copy_const_reference>()))
            .add_property("x",                  &UniverseObject::X)
            .add_property("y",                  &UniverseObject::Y)
            .add_property("systemID",           &UniverseObject::SystemID)
            .add_property("unowned",            &UniverseObject::Unowned)
            .add_property("owner",              &UniverseObject::Owner)
            .def("ownedBy",                     &UniverseObject::OwnedBy)
            .add_property("creationTurn",       &UniverseObject::CreationTurn)
            .add_property("ageInTurns",         &UniverseObject::AgeInTurns)
            .add_property("specials",           make_function(ObjectSpecials,               py::return_value_policy<py::return_by_value>()))
            .def("hasSpecial",                  &UniverseObject::HasSpecial)
            .def("specialAddedOnTurn",          &UniverseObject::SpecialAddedOnTurn)
            .def("contains",                    &UniverseObject::Contains)
            .def("containedBy",                 &UniverseObject::ContainedBy)
            .add_property("containedObjects",   make_function(&UniverseObject::ContainedObjectIDs,  py::return_value_policy<py::return_by_value>()))
            .add_property("containerObject",    &UniverseObject::ContainerObjectID)
            .def("currentMeterValue",           make_function(
                                                    ObjectCurrentMeterValueFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<float, const UniverseObject&, MeterType>()
                                                ))
            .def("initialMeterValue",           make_function(
                                                    ObjectInitialMeterValueFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<float, const UniverseObject&, MeterType>()
                                                ))
            .add_property("tags",               make_function(&UniverseObject::Tags,        py::return_value_policy<py::return_by_value>()))
            .def("hasTag",                      &UniverseObject::HasTag)
            .add_property("meters",             make_function(
                                                    ObjectMetersFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<std::map<MeterType, Meter>, const UniverseObject&>()
                                                ))
            .def("getMeter",                    make_function(ObjectGetMeter,               py::return_internal_reference<>()))
            .def("dump",                        make_function(&ObjectDump,                  py::return_value_policy<py::return_by_value>()), "Returns string with debug information.")
        ;

        ///////////////////
        //     Fleet     //
        ///////////////////
        py::class_<Fleet, py::bases<UniverseObject>, boost::noncopyable>("fleet", py::no_init)
            .add_property("fuel",                       &Fleet::Fuel)
            .add_property("maxFuel",                    &Fleet::MaxFuel)
            .add_property("finalDestinationID",         &Fleet::FinalDestinationID)
            .add_property("previousSystemID",           &Fleet::PreviousSystemID)
            .add_property("nextSystemID",               &Fleet::NextSystemID)
            .add_property("aggressive",                 &Fleet::Aggressive)
            .add_property("speed",                      &Fleet::Speed)
            .add_property("canChangeDirectionEnRoute",  &Fleet::CanChangeDirectionEnRoute)
            .add_property("hasMonsters",                &Fleet::HasMonsters)
            .add_property("hasArmedShips",              &Fleet::HasArmedShips)
            .add_property("hasFighterShips",            &Fleet::HasFighterShips)
            .add_property("hasColonyShips",             &Fleet::HasColonyShips)
            .add_property("hasOutpostShips",            &Fleet::HasOutpostShips)
            .add_property("hasTroopShips",              &Fleet::HasTroopShips)
            .add_property("numShips",                   &Fleet::NumShips)
            .add_property("empty",                      &Fleet::Empty)
            .add_property("shipIDs",                    make_function(&Fleet::ShipIDs,      py::return_internal_reference<>()))
        ;

        //////////////////
        //     Ship     //
        //////////////////
        py::class_<Ship, py::bases<UniverseObject>, boost::noncopyable>("ship", py::no_init)
            .add_property("design",                 make_function(&Ship::Design,            py::return_value_policy<py::reference_existing_object>()))
            .add_property("designID",               &Ship::DesignID)
            .add_property("fleetID",                &Ship::FleetID)
            .add_property("producedByEmpireID",     &Ship::ProducedByEmpireID)
            .add_property("arrivedOnTurn",          &Ship::ArrivedOnTurn)
            .add_property("lastResuppliedOnTurn",   &Ship::LastResuppliedOnTurn)
            .add_property("lastTurnActiveInCombat", &Ship::LastTurnActiveInCombat)
            .add_property("isMonster",              &Ship::IsMonster)
            .add_property("isArmed",                &Ship::IsArmed)
            .add_property("hasFighters",            &Ship::HasFighters)
            .add_property("canColonize",            &Ship::CanColonize)
            .add_property("canInvade",              &Ship::HasTroops)
            .add_property("canBombard",             &Ship::CanBombard)
            .add_property("speciesName",            make_function(&Ship::SpeciesName,       py::return_value_policy<py::copy_const_reference>()))
            .add_property("speed",                  &Ship::Speed)
            .add_property("colonyCapacity",         &Ship::ColonyCapacity)
            .add_property("troopCapacity",          &Ship::TroopCapacity)
            .add_property("orderedScrapped",        &Ship::OrderedScrapped)
            .add_property("orderedColonizePlanet",  &Ship::OrderedColonizePlanet)
            .add_property("orderedInvadePlanet",    &Ship::OrderedInvadePlanet)
            .def("initialPartMeterValue",           &Ship::InitialPartMeterValue)
            .def("currentPartMeterValue",           &Ship::CurrentPartMeterValue)
            .add_property("partMeters",             make_function(ShipPartMeters,           py::return_internal_reference<>()))
            .def("getMeter",                        make_function(ShipGetPartMeter,         py::return_internal_reference<>()))
        ;

        //////////////////
        //  ShipDesign  //
        //////////////////
        py::class_<ShipDesign, boost::noncopyable>("shipDesign", py::no_init)
            .add_property("id",                 make_function(&ShipDesign::ID,              py::return_value_policy<py::return_by_value>()))
            .add_property("name",               make_function(
                                                    ShipDesignNameFunc,
                                                    py::return_value_policy<py::copy_const_reference>(),
                                                    boost::mpl::vector<const std::string&, const ShipDesign&>()
                                                ))
            .add_property("description",        make_function(
                                                    ShipDesignDescriptionFunc,
                                                    py::return_value_policy<py::copy_const_reference>(),
                                                    boost::mpl::vector<const std::string&, const ShipDesign&>()
                                                ))
            .add_property("designedOnTurn",     make_function(&ShipDesign::DesignedOnTurn,  py::return_value_policy<py::return_by_value>()))
            .add_property("speed",              make_function(&ShipDesign::Speed,           py::return_value_policy<py::return_by_value>()))
            .add_property("structure",          make_function(&ShipDesign::Structure,       py::return_value_policy<py::return_by_value>()))
            .add_property("shields",            make_function(&ShipDesign::Shields,         py::return_value_policy<py::return_by_value>()))
            .add_property("fuel",               make_function(&ShipDesign::Fuel,            py::return_value_policy<py::return_by_value>()))
            .add_property("detection",          make_function(&ShipDesign::Detection,       py::return_value_policy<py::return_by_value>()))
            .add_property("colonyCapacity",     make_function(&ShipDesign::ColonyCapacity,  py::return_value_policy<py::return_by_value>()))
            .add_property("troopCapacity",      make_function(&ShipDesign::TroopCapacity,   py::return_value_policy<py::return_by_value>()))
            .add_property("stealth",            make_function(&ShipDesign::Stealth,         py::return_value_policy<py::return_by_value>()))
            .add_property("industryGeneration", make_function(&ShipDesign::IndustryGeneration,  py::return_value_policy<py::return_by_value>()))
            .add_property("researchGeneration", make_function(&ShipDesign::ResearchGeneration,  py::return_value_policy<py::return_by_value>()))
            .add_property("influenceGeneration",make_function(&ShipDesign::InfluenceGeneration, py::return_value_policy<py::return_by_value>()))
            .add_property("defense",            make_function(&ShipDesign::Defense,         py::return_value_policy<py::return_by_value>()))
            .add_property("attack",             make_function(&ShipDesign::Attack,          py::return_value_policy<py::return_by_value>()))
            .add_property("canColonize",        make_function(&ShipDesign::CanColonize,     py::return_value_policy<py::return_by_value>()))
            .add_property("canInvade",          make_function(&ShipDesign::HasTroops,       py::return_value_policy<py::return_by_value>()))
            .add_property("isArmed",            make_function(&ShipDesign::IsArmed,         py::return_value_policy<py::return_by_value>()))
            .add_property("hasFighters",        make_function(&ShipDesign::HasFighters,     py::return_value_policy<py::return_by_value>()))
            .add_property("hasDirectWeapons",   make_function(&ShipDesign::HasDirectWeapons,py::return_value_policy<py::return_by_value>()))
            .add_property("isMonster",          make_function(&ShipDesign::IsMonster,       py::return_value_policy<py::return_by_value>()))
            .def("productionCost",              &ShipDesign::ProductionCost)
            .def("productionTime",              &ShipDesign::ProductionTime)
            .def("perTurnCost",                 &ShipDesign::PerTurnCost)
            .add_property("costTimeLocationInvariant",
                                                &ShipDesign::ProductionCostTimeLocationInvariant)
            .add_property("hull",               make_function(&ShipDesign::Hull,            py::return_value_policy<py::return_by_value>()))
            .add_property("ship_hull",          make_function(
                                                    ShipDesignHullFunc,
                                                    py::return_value_policy<py::reference_existing_object>(),
                                                    boost::mpl::vector<const ShipHull*, const ShipDesign&>()
                                                ))
            .add_property("parts",              make_function(PartsVoid,                    py::return_internal_reference<>()))
            .add_property("attackStats",        make_function(
                                                    AttackStatsFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const ShipDesign&>()
                                                ))

            .def("productionLocationForEmpire", &ShipDesign::ProductionLocation)
            .def("dump",                        &ShipDesign::Dump,                          py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;
        py::def("validShipDesign",                  ValidDesignHullAndParts, "Returns true (boolean) if the passed hull (string) and parts (StringVec) make up a valid ship design, and false (boolean) otherwise. Valid ship designs don't have any parts in slots that can't accept that type of part, and contain only hulls and parts that exist (and may also need to contain the correct number of parts - this needs to be verified).");
        py::def("getShipDesign",                    &GetShipDesign,                             py::return_value_policy<py::reference_existing_object>(), "Returns the ship design (ShipDesign) with the indicated id number (int).");
        py::def("getPredefinedShipDesign",          &GetPredefinedShipDesign,                   py::return_value_policy<py::reference_existing_object>(), "Returns the ship design (ShipDesign) with the indicated name (string).");


        py::class_<ShipPart, boost::noncopyable>("shipPart", py::no_init)
            .add_property("name",               make_function(&ShipPart::Name,              py::return_value_policy<py::copy_const_reference>()))
            .add_property("partClass",          &ShipPart::Class)
            .add_property("capacity",           &ShipPart::Capacity)
            .add_property("secondaryStat",      &ShipPart::SecondaryStat)
            .add_property("mountableSlotTypes", make_function(&ShipPart::MountableSlotTypes,py::return_value_policy<py::return_by_value>()))
            .def("productionCost",              &ShipPart::ProductionCost)
            .def("productionTime",              &ShipPart::ProductionTime)
            .def("canMountInSlotType",          &ShipPart::CanMountInSlotType)
            .add_property("costTimeLocationInvariant",
                                                &ShipPart::ProductionCostTimeLocationInvariant)
            .def("productionLocation",          &ShipPartProductionLocation, "Returns the result of Location condition (bool) in passed location_id (int)")
        ;
        py::def("getShipPart",                      &GetShipPart,                               py::return_value_policy<py::reference_existing_object>(), "Returns the ShipPart with the indicated name (string).");

        py::class_<ShipHull, boost::noncopyable>("shipHull", py::no_init)
            .add_property("name",               make_function(&ShipHull::Name,              py::return_value_policy<py::copy_const_reference>()))
            .add_property("numSlots",           make_function(NumSlotsTotal,                py::return_value_policy<py::return_by_value>()))
            .add_property("structure",          &ShipHull::Structure)
            .add_property("stealth",            &ShipHull::Stealth)
            .add_property("fuel",               &ShipHull::Fuel)
            .add_property("starlaneSpeed",      &ShipHull::Speed) // TODO: Remove this after transition period
            .add_property("speed",              &ShipHull::Speed)
            .def("numSlotsOfSlotType",          NumSlotsOfSlotType)
            .add_property("slots",              make_function(
                                                    HullSlotsFunc,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    boost::mpl::vector<std::vector<ShipSlotType>, const ShipHull&>()
                                                ))
            .def("productionCost",              &ShipHull::ProductionCost)
            .def("productionTime",              &ShipHull::ProductionTime)
            .add_property("costTimeLocationInvariant",
                                                &ShipHull::ProductionCostTimeLocationInvariant)
            .def("hasTag",                      &ShipHull::HasTag)
            .def("productionLocation",          &HullProductionLocation, "Returns the result of Location condition (bool) in passed location_id (int)")
        ;
        py::def("getShipHull",                      &GetShipHull,                               py::return_value_policy<py::reference_existing_object>(), "Returns the ship hull with the indicated name (string).");

        //////////////////
        //   Building   //
        //////////////////
        py::class_<Building, py::bases<UniverseObject>, boost::noncopyable>("building", py::no_init)
            .add_property("buildingTypeName",   make_function(&Building::BuildingTypeName,  py::return_value_policy<py::copy_const_reference>()))
            .add_property("planetID",           make_function(&Building::PlanetID,          py::return_value_policy<py::return_by_value>()))
            .add_property("producedByEmpireID", &Building::ProducedByEmpireID)
            .add_property("orderedScrapped",    &Building::OrderedScrapped)
        ;

        //////////////////
        // BuildingType //
        //////////////////
        py::class_<BuildingType, boost::noncopyable>("buildingType", py::no_init)
            .add_property("name",               make_function(&BuildingType::Name,          py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",        make_function(&BuildingType::Description,   py::return_value_policy<py::copy_const_reference>()))
            .def("productionCost",              &BuildingType::ProductionCost)
            .def("productionTime",              &BuildingType::ProductionTime)
            .def("perTurnCost",                 &BuildingType::PerTurnCost)
            .def("captureResult",               &BuildingType::GetCaptureResult)
            .def("canBeProduced",               &BuildingType::ProductionLocation)  //(int empire_id, int location_id)
            .def("canBeEnqueued",               &EnqueueLocationTest)  //(int empire_id, int location_id)
            .add_property("costTimeLocationInvariant",
                                                &BuildingType::ProductionCostTimeLocationInvariant)
            .def("dump",                        &BuildingType::Dump,                        py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;
        py::def("getBuildingType",                  &GetBuildingType,                           py::return_value_policy<py::reference_existing_object>(), "Returns the building type (BuildingType) with the indicated name (string).");
        ////////////////////
        // ResourceCenter //
        ////////////////////
        py::class_<ResourceCenter, boost::noncopyable>("resourceCenter", py::no_init)
            .add_property("focus",                  make_function(&ResourceCenter::Focus,   py::return_value_policy<py::copy_const_reference>()))
            .add_property("turnsSinceFocusChange" , &ResourceCenter::TurnsSinceFocusChange)
            .add_property("availableFoci",          &ResourceCenter::AvailableFoci)
        ;

        ///////////////////
        //   PopCenter   //
        ///////////////////
        py::class_<PopCenter, boost::noncopyable>("popCenter", py::no_init)
            .add_property("speciesName",        make_function(&PopCenter::SpeciesName,      py::return_value_policy<py::copy_const_reference>()))
        ;

        //////////////////
        //    Planet    //
        //////////////////
        py::class_<Planet, py::bases<UniverseObject, PopCenter, ResourceCenter>, boost::noncopyable>("planet", py::no_init)
            .add_property("size",                           &Planet::Size)
            .add_property("type",                           &Planet::Type)
            .add_property("originalType",                   &Planet::OriginalType)
            .add_property("distanceFromOriginalType",       &Planet::DistanceFromOriginalType)
            .def("environmentForSpecies",                   &Planet::EnvironmentForSpecies)
            .def("nextBetterPlanetTypeForSpecies",          &Planet::NextBetterPlanetTypeForSpecies)
            .add_property("clockwiseNextPlanetType",        &Planet::ClockwiseNextPlanetType)
            .add_property("counterClockwiseNextPlanetType", &Planet::CounterClockwiseNextPlanetType)
            .add_property("nextLargerPlanetSize",           &Planet::NextLargerPlanetSize)
            .add_property("nextSmallerPlanetSize",          &Planet::NextSmallerPlanetSize)
            .add_property("OrbitalPeriod",                  &Planet::OrbitalPeriod)
            .add_property("InitialOrbitalPosition",         &Planet::InitialOrbitalPosition)
            .def("OrbitalPositionOnTurn",                   &Planet::OrbitalPositionOnTurn)
            .add_property("RotationalPeriod",               &Planet::RotationalPeriod)
            .add_property("LastTurnAttackedByShip",         &Planet::LastTurnAttackedByShip)
            .add_property("LastTurnConquered",              &Planet::LastTurnConquered)
            .add_property("buildingIDs",                    make_function(&Planet::BuildingIDs,     py::return_internal_reference<>()))
            .add_property("habitableSize",                  &Planet::HabitableSize)
        ;

        //////////////////
        //    System    //
        //////////////////
        py::class_<System, py::bases<UniverseObject>, boost::noncopyable>("system", py::no_init)
            .add_property("starType",           &System::GetStarType)
            .add_property("numStarlanes",       &System::NumStarlanes)
            .add_property("numWormholes",       &System::NumWormholes, "Currently unused.")
            .def("HasStarlaneToSystemID",       &System::HasStarlaneTo)
            .def("HasWormholeToSystemID",       &System::HasWormholeTo, "Currently unused.")
            .add_property("starlanesWormholes", make_function(&System::StarlanesWormholes,  py::return_value_policy<py::return_by_value>()), "Currently unused.")
            .add_property("planetIDs",          make_function(&System::PlanetIDs,           py::return_value_policy<py::return_by_value>()))
            .add_property("buildingIDs",        make_function(&System::BuildingIDs,         py::return_value_policy<py::return_by_value>()))
            .add_property("fleetIDs",           make_function(&System::FleetIDs,            py::return_value_policy<py::return_by_value>()))
            .add_property("shipIDs",            make_function(&System::ShipIDs,             py::return_value_policy<py::return_by_value>()))
            .add_property("fieldIDs",           make_function(&System::FieldIDs,            py::return_value_policy<py::return_by_value>()))
            .add_property("lastTurnBattleHere", &System::LastTurnBattleHere)
        ;

        //////////////////
        //    Field     //
        //////////////////
        py::class_<Field, py::bases<UniverseObject>, boost::noncopyable>("field", py::no_init)
            .add_property("fieldTypeName",      make_function(&Field::FieldTypeName,        py::return_value_policy<py::copy_const_reference>()))
            .def("inField",                     &ObjectInField)
            .def("inField",                     LocationInField)
        ;

        //////////////////
        //   FieldType  //
        //////////////////
        py::class_<FieldType, boost::noncopyable>("fieldType", py::no_init)
            .add_property("name",               make_function(&FieldType::Name,             py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",        make_function(&FieldType::Description,      py::return_value_policy<py::copy_const_reference>()))
            .def("dump",                        &FieldType::Dump,                           py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;
        py::def("getFieldType",                     &GetFieldType,                           py::return_value_policy<py::reference_existing_object>());


        /////////////////
        //   Special   //
        /////////////////
        py::class_<Special, boost::noncopyable>("special", py::no_init)
            .add_property("name",               make_function(&Special::Name,           py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",        &Special::Description)
            .add_property("spawnrate",          make_function(&Special::SpawnRate,      py::return_value_policy<py::return_by_value>()))
            .add_property("spawnlimit",         make_function(&Special::SpawnLimit,     py::return_value_policy<py::return_by_value>()))
            .def("dump",                        &Special::Dump,                         py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
            .def("initialCapacity",             SpecialInitialCapacityOnObject)
        ;
        py::def("getSpecial",                       &GetSpecial,                            py::return_value_policy<py::reference_existing_object>(), "Returns the special (Special) with the indicated name (string).");

        /////////////////
        //   Species   //
        /////////////////
        py::class_<Species, boost::noncopyable>("species", py::no_init)
            .add_property("name",               make_function(&Species::Name,           py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",        make_function(&Species::Description,    py::return_value_policy<py::copy_const_reference>()))
            .add_property("homeworlds",         make_function(&Species::Homeworlds,     py::return_value_policy<py::copy_const_reference>()))
            .add_property("foci",               &SpeciesFoci)
            .add_property("preferredFocus",     make_function(&Species::PreferredFocus, py::return_value_policy<py::copy_const_reference>()))
            .add_property("canColonize",        make_function(&Species::CanColonize,    py::return_value_policy<py::return_by_value>()))
            .add_property("canProduceShips",    make_function(&Species::CanProduceShips,py::return_value_policy<py::return_by_value>()))
            .add_property("tags",               make_function(&Species::Tags,           py::return_value_policy<py::return_by_value>()))
            // TODO: const std::vector<FocusType>& Species::Foci()
            .def("getPlanetEnvironment",        &Species::GetPlanetEnvironment)
            .def("dump",                        &Species::Dump,                         py::return_value_policy<py::return_by_value>(), "Returns string with debug information, use '0' as argument.")
        ;
        py::def("getSpecies",                       &GetSpecies,                            py::return_value_policy<py::reference_existing_object>(), "Returns the species (Species) with the indicated name (string).");
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
            .def("getRulesAsStrings",           make_function(&GameRules::GetRulesAsStrings,    py::return_value_policy<py::return_by_value>()))
            .def("ruleExists",                  RuleExistsAnyType)
            .def("ruleExistsWithType",          RuleExistsWithType)
            .def("getDescription",              make_function(&GameRules::GetDescription,       py::return_value_policy<py::copy_const_reference>()))
            .def("getToggle",                   &GameRules::Get<bool>)
            .def("getInt",                      &GameRules::Get<int>)
            .def("getDouble",                   &GameRules::Get<double>)
            .def("getString",                   make_function(&GameRules::Get<std::string>,     py::return_value_policy<py::return_by_value>()));
        py::def("getGameRules",                     &GetGameRules,                                  py::return_value_policy<py::reference_existing_object>(), "Returns the game rules manager, which can be used to look up the names (string) of rules are defined with what type (boolean / toggle, int, double, string), and what values the rules have in the current game.");
    }
}
