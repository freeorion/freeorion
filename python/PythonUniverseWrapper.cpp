#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Building.h"
#include "../universe/ResourceCenter.h"
#include "../universe/PopCenter.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Field.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"

#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>

namespace {
    void                    DumpObjects(const Universe& universe)
    { Logger().debugStream() << universe.Objects().Dump(); }

    // We're returning the result of operator-> here so that python doesn't
    // need to deal with our TemporaryPtr class.
    // Please don't use this trick elsewhere to grab a raw UniverseObject*!
    const UniverseObject*   GetUniverseObjectP(const Universe& universe, int id)
    { return ::GetUniverseObject(id).operator->(); }
    const Ship*             GetShipP(const Universe& universe, int id)
    { return ::GetShip(id).operator->(); }
    const Fleet*            GetFleetP(const Universe& universe, int id)
    { return ::GetFleet(id).operator->(); }
    const Planet*           GetPlanetP(const Universe& universe, int id)
    { return ::GetPlanet(id).operator->(); }
    const System*           GetSystemP(const Universe& universe, int id)
    { return ::GetSystem(id).operator->(); }
    const Field*            GetFieldP(const Universe& universe, int id)
    { return ::GetField(id).operator->();  }
    const Building*         GetBuildingP(const Universe& universe, int id)
    { return ::GetBuilding(id).operator->(); }

    std::vector<int>        ObjectIDs(const Universe& universe)
    { return Objects().FindObjectIDs(); }
    std::vector<int>        FleetIDs(const Universe& universe)
    { return Objects().FindObjectIDs<Fleet>(); }
    std::vector<int>        SystemIDs(const Universe& universe)
    { return Objects().FindObjectIDs<System>(); }
    std::vector<int>        FieldIDs(const Universe& universe)
    { return Objects().FindObjectIDs<Field>(); }
    std::vector<int>        PlanetIDs(const Universe& universe)
    { return Objects().FindObjectIDs<Planet>(); }
    std::vector<int>        ShipIDs(const Universe& universe)
    { return Objects().FindObjectIDs<Ship>(); }
    std::vector<int>        BuildingIDs(const Universe& universe)
    { return Objects().FindObjectIDs<Building>(); }

    std::vector<std::string>SpeciesFoci(const Species& species) {
        std::vector<std::string> retval;
        const std::vector<FocusType>& foci = species.Foci();
        for (std::vector<FocusType>::const_iterator f_it = foci.begin(); f_it != foci.end(); f_it++ )
            retval.push_back(f_it->Name());
        return retval;
    }

    void                    UpdateMetersWrapper(const Universe& universe, boost::python::list objList) {
        std::vector<int> objvec;
        int const numObjects = boost::python::len(objList);
        for (int i = 0; i < numObjects; i++)
            objvec.push_back(boost::python::extract<int>(objList[i]));
        GetUniverse().UpdateMeterEstimates(objvec);
    }

    //void                    (Universe::*UpdateMeterEstimatesVoidFunc)(void) =                   &Universe::UpdateMeterEstimates;

    double                  LinearDistance(const Universe& universe, int system1_id, int system2_id) {
        double retval = 9999999.9;  // arbitrary large value
        try {
            retval = universe.LinearDistance(system1_id, system2_id);
        } catch (...) {
        }
        return retval;
    }
    boost::function<double(const Universe&, int, int)> LinearDistanceFunc =                     &LinearDistance;

    int                     JumpDistance(const Universe& universe, int system1_id, int system2_id) {
        try {
            return universe.JumpDistance(system1_id, system2_id);
        } catch (...) {
        }
        return -1;
    }
    boost::function<int(const Universe&, int, int)> JumpDistanceFunc =                          &JumpDistance;

    std::vector<int>        ShortestPath(const Universe& universe, int start_sys, int end_sys, int empire_id) {
        std::vector<int> retval;
        try {
            std::pair<std::list<int>, int> path = universe.ShortestPath(start_sys, end_sys, empire_id);
            std::copy(path.first.begin(), path.first.end(), std::back_inserter(retval));
        } catch (...) {
        }
        return retval;
    }
    boost::function<std::vector<int>(const Universe&, int, int, int)> ShortestPathFunc =        &ShortestPath;

    std::vector<int>        LeastJumpsPath(const Universe& universe, int start_sys, int end_sys, int empire_id) {
        std::vector<int> retval;
        try {
            std::pair<std::list<int>, int> path = universe.LeastJumpsPath(start_sys, end_sys, empire_id);
            std::copy(path.first.begin(), path.first.end(), std::back_inserter(retval));
        } catch (...) {
        }
        return retval;
    }
    boost::function<std::vector<int>(const Universe&, int, int, int)> LeastJumpsFunc =          &LeastJumpsPath;

    bool                    SystemsConnectedP(const Universe& universe, int system1_id, int system2_id, int empire_id=ALL_EMPIRES) {
        //Logger().debugStream() << "SystemsConnected!(" << system1_id << ", " << system2_id << ")";
        try {
            bool retval = universe.SystemsConnected(system1_id, system2_id, empire_id);
            //Logger().debugStream() << "SystemsConnected! retval: " << retval;
            return retval;
        } catch (...) {
        }
        return false;
    }
    boost::function<bool(const Universe&, int, int, int)> SystemsConnectedFunc =                &SystemsConnectedP;

    std::vector<int>        ImmediateNeighborsP(const Universe& universe, int system1_id, int empire_id = ALL_EMPIRES) {
        std::multimap<double, int> lanemap;
        std::vector<int> retval;
        try {
            lanemap = universe.ImmediateNeighbors(system1_id, empire_id);
            for (std::multimap<double, int>::const_iterator it = lanemap.begin(); it != lanemap.end(); ++it)
            { retval.push_back(it->second); }
            return retval;
        } catch (...) {
        }
        return retval;
    }
    boost::function<std::vector<int> (const Universe&, int, int)> ImmediateNeighborsFunc =      &ImmediateNeighborsP;

    std::map<int,double>    SystemNeighborsMapP(const Universe& universe, int system1_id, int empire_id = ALL_EMPIRES) {
        std::multimap<double, int> lanemap;
        std::map<int,double> retval;
        try {
            lanemap = universe.ImmediateNeighbors(system1_id, empire_id);
            for (std::multimap<double, int>::const_iterator it = lanemap.begin(); it != lanemap.end(); ++it)
            { retval[it->second] = it->first; }
            return retval;
        } catch (...) {
        }
        return retval;
    }
    boost::function<std::map<int,double> (const Universe&, int, int)> SystemNeighborsMapFunc =      &SystemNeighborsMapP;

    int                     VisibilityP(const Universe& universe, int object_id, int empire_id = ALL_EMPIRES) {
        int retval;
        //std::vector<int> retval;
        try {
            retval = universe.GetObjectVisibilityByEmpire(object_id, empire_id);
            return retval;
        } catch (...) {
        }
        return INVALID_VISIBILITY;
    }
    boost::function<int (const Universe&, int, int)> VisibilityFunc =                           &VisibilityP;

    std::vector<int>        VisibilityTurnsP(const Universe& universe, int object_id, int empire_id = ALL_EMPIRES) {
        Universe::VisibilityTurnMap  vismap;
        std::vector<int> retval;
        //std::vector<int> retval;
        try {
            vismap = universe.GetObjectVisibilityTurnMapByEmpire(object_id, empire_id);
            retval.push_back(vismap[VIS_NO_VISIBILITY]);
            retval.push_back(vismap[VIS_BASIC_VISIBILITY]);
            retval.push_back(vismap[VIS_PARTIAL_VISIBILITY]);
            retval.push_back(vismap[VIS_FULL_VISIBILITY]);
            return retval;
        } catch (...) {
        }
        return retval;
    }
    boost::function<std::vector<int> (const Universe&, int, int)> VisibilityTurnsFunc =         &VisibilityTurnsP;

    const Meter*            (UniverseObject::*ObjectGetMeter)(MeterType) const =                &UniverseObject::GetMeter;
    const std::map<MeterType, Meter>&
                            (UniverseObject::*ObjectMeters)(void) const =                       &UniverseObject::Meters;

    std::vector<std::string> ObjectSpecials(const UniverseObject& object) {
        std::vector<std::string> retval;
        for (std::map<std::string, int>::const_iterator it = object.Specials().begin();
             it != object.Specials().end(); ++it)
        { retval.push_back(it->first); }
        return retval;
    }

    const Meter*            (Ship::*ShipGetPartMeter)(MeterType, const std::string&) const =    &Ship::GetPartMeter;
    const Ship::PartMeterMap&
                            (Ship::*ShipPartMeters)(void) const =                               &Ship::PartMeters;

    bool                    (*ValidDesignHullAndParts)(const std::string& hull,
                                                       const std::vector<std::string>& parts) = &ShipDesign::ValidDesign;
    bool                    (*ValidDesignDesign)(const ShipDesign&) =                           &ShipDesign::ValidDesign;

    std::vector<int>        DirectFireStatsP(const ShipDesign& ship_design) {
        const std::vector<std::string>& partslist = ship_design.Parts();
        std::vector<int> results;
        for (std::vector<std::string>::const_iterator part_it = partslist.begin(); part_it!=partslist.end(); part_it++){
            const PartType* part = GetPartType(*part_it);
            if (part && part->Class() == PC_SHORT_RANGE) { // TODO: handle other weapon classes when they are implemented
                const DirectFireStats& stats = boost::get<DirectFireStats>(part->Stats());
                results.push_back(stats.m_damage);
            }
        }
        return results;
    }
    boost::function<std::vector<int> (const ShipDesign&)> DirectFireStatsFunc =                 &DirectFireStatsP;

    const std::vector<std::string>& (ShipDesign::*PartsVoid)(void) const =                      &ShipDesign::Parts;
    std::vector<std::string>        (ShipDesign::*PartsSlotType)(ShipSlotType) const =          &ShipDesign::Parts;

    unsigned int            (HullType::*NumSlotsTotal)(void) const =                            &HullType::NumSlots;
    unsigned int            (HullType::*NumSlotsOfSlotType)(ShipSlotType) const =               &HullType::NumSlots;

    bool                    ObjectInField(const Field& field, const UniverseObject& obj)
    { return field.InField(obj.X(), obj.Y()); }
    bool                    (Field::*LocationInField)(double x, double y) const =               &Field::InField;
}

namespace FreeOrionPython {
    using boost::python::def;
    using boost::python::class_;
    using boost::python::bases;
    using boost::python::no_init;
    using boost::noncopyable;
    using boost::python::return_value_policy;
    using boost::python::copy_const_reference;
    using boost::python::reference_existing_object;
    using boost::python::return_by_value;
    using boost::python::return_internal_reference;

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
        class_<std::map<int, int> >("IntIntMap")
            .def(boost::python::map_indexing_suite<std::map<int, int>, true >())
        ;
        class_<std::map<int, double> >("IntDblMap")
            .def(boost::python::map_indexing_suite<std::map<int, double>,true >())
        ;
        class_<std::map<int, Visibility> >("IntVisibilityMap")
            .def(boost::python::map_indexing_suite<std::map<int, Visibility>,true >())
        ;
        class_<std::map<Visibility,int> >("VisibilityIntMap")
            .def(boost::python::map_indexing_suite<std::map<Visibility, int>, true>())
        ;

        ////////////////////
        //    Universe    //
        ////////////////////
        class_<Universe, noncopyable>("universe", no_init)
            .def("getObject",                   make_function(GetUniverseObjectP,   return_value_policy<reference_existing_object>()))
            .def("getFleet",                    make_function(GetFleetP,            return_value_policy<reference_existing_object>()))
            .def("getShip",                     make_function(GetShipP,             return_value_policy<reference_existing_object>()))
            .def("getPlanet",                   make_function(GetPlanetP,           return_value_policy<reference_existing_object>()))
            .def("getSystem",                   make_function(GetSystemP,           return_value_policy<reference_existing_object>()))
            .def("getField",                    make_function(GetFieldP,            return_value_policy<reference_existing_object>()))
            .def("getBuilding",                 make_function(GetBuildingP,         return_value_policy<reference_existing_object>()))

            .add_property("allObjectIDs",       make_function(ObjectIDs,            return_value_policy<return_by_value>()))
            .add_property("fleetIDs",           make_function(FleetIDs,             return_value_policy<return_by_value>()))
            .add_property("systemIDs",          make_function(SystemIDs,            return_value_policy<return_by_value>()))
            .add_property("fieldIDs",           make_function(FieldIDs,             return_value_policy<return_by_value>()))
            .add_property("planetIDs",          make_function(PlanetIDs,            return_value_policy<return_by_value>()))
            .add_property("shipIDs",            make_function(ShipIDs,              return_value_policy<return_by_value>()))
            .add_property("buildingIDs",        make_function(BuildingIDs,          return_value_policy<return_by_value>()))
            .def("destroyedObjectIDs",          make_function(&Universe::EmpireKnownDestroyedObjectIDs,
                                                                                    return_value_policy<return_by_value>()))

            .def("systemHasStarlane",           &Universe::SystemHasVisibleStarlanes)

            .def("updateMeterEstimates",        &UpdateMetersWrapper)

            .def("linearDistance",              make_function(
                                                    LinearDistanceFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<double, const Universe&, int, int>()
                                                ))

            .def("jumpDistance",                make_function(
                                                    JumpDistanceFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<int, const Universe&, int, int>()
                                                ))

            .def("shortestPath",                make_function(
                                                    ShortestPathFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe&, int, int, int>()
                                                ))

            .def("leastJumpsPath",              make_function(
                                                    LeastJumpsFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe&, int, int, int>()
                                                ))

            .def("systemsConnected",            make_function(
                                                    SystemsConnectedFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<bool, const Universe&, int, int, int>()
                                                ))

            .def("getImmediateNeighbors",       make_function(
                                                    ImmediateNeighborsFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe&, int, int>()
                                                ))

            .def("getSystemNeighborsMap",       make_function(
                                                    SystemNeighborsMapFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::map<int, double>, const Universe&, int, int>()
                                                ))
            .def("getVisibilityMap",            make_function(
                                                    &Universe::GetObjectVisibilityByEmpire,
                                                    return_value_policy<return_by_value>()
                                                ))

            .def("getVisibilityTurnsMap",       make_function(
                                                    &Universe::GetObjectVisibilityTurnMapByEmpire,
                                                    return_value_policy<return_by_value>()
                                                ))

            .def("getVisibilityTurns",          make_function(
                                                    VisibilityTurnsFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe&, int, int>()
                                                ))

            .def("getVisibility",               make_function(
                                                    VisibilityFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<int, const Universe&, int, int>()
                                                ))
            .def("getVisibilityMap",            make_function(
                                                    &Universe::GetObjectVisibilityByEmpire,
                                                    return_value_policy<return_by_value>()
                                                ))

            .def("dump",                        &DumpObjects)
        ;

        ////////////////////
        // UniverseObject //
        ////////////////////
        class_<UniverseObject, noncopyable>("universeObject", no_init)
            .add_property("id",                 &UniverseObject::ID)
            .add_property("name",               make_function(&UniverseObject::Name,        return_value_policy<copy_const_reference>()))
            .add_property("x",                  &UniverseObject::X)
            .add_property("y",                  &UniverseObject::Y)
            .add_property("systemID",           &UniverseObject::SystemID)
            .add_property("unowned",            &UniverseObject::Unowned)
            .add_property("owner",              &UniverseObject::Owner)
            .def("ownedBy",                     &UniverseObject::OwnedBy)
            .add_property("creationTurn",       &UniverseObject::CreationTurn)
            .add_property("ageInTurns",         &UniverseObject::AgeInTurns)
            .add_property("specials",           make_function(ObjectSpecials,               return_value_policy<return_by_value>()))
            .def("hasSpecial",                  &UniverseObject::HasSpecial)
            .def("specialAddedOnTurn",          &UniverseObject::SpecialAddedOnTurn)
            .def("contains",                    &UniverseObject::Contains)
            .def("containedBy",                 &UniverseObject::ContainedBy)
            .add_property("containedObjects",   make_function(&UniverseObject::ContainedObjectIDs,  return_value_policy<return_by_value>()))
            .add_property("containerObject",    &UniverseObject::ContainerObjectID)
            .def("currentMeterValue",           &UniverseObject::CurrentMeterValue)
            .def("initialMeterValue",           &UniverseObject::InitialMeterValue)
            .def("nextTurnCurrentMeterValue",   &UniverseObject::NextTurnCurrentMeterValue)
            .add_property("tags",               make_function(&UniverseObject::Tags,        return_value_policy<return_by_value>()))
            .def("hasTag",                      &UniverseObject::HasTag)
            .add_property("meters",             make_function(ObjectMeters,                 return_internal_reference<>()))
            .def("getMeter",                    make_function(ObjectGetMeter,               return_internal_reference<>()))
        ;

        ///////////////////
        //     Fleet     //
        ///////////////////
        class_<Fleet, bases<UniverseObject>, noncopyable>("fleet", no_init)
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
            .add_property("hasColonyShips",             &Fleet::HasColonyShips)
            .add_property("hasOutpostShips",            &Fleet::HasOutpostShips)
            .add_property("hasTroopShips",              &Fleet::HasTroopShips)
            .add_property("numShips",                   &Fleet::NumShips)
            .add_property("empty",                      &Fleet::Empty)
            .add_property("shipIDs",                    make_function(&Fleet::ShipIDs,      return_internal_reference<>()))
        ;

        //////////////////
        //     Ship     //
        //////////////////
        class_<Ship, bases<UniverseObject>, noncopyable>("ship", no_init)
            .add_property("design",                 make_function(&Ship::Design,            return_value_policy<reference_existing_object>()))
            .add_property("designID",               &Ship::DesignID)
            .add_property("fleetID",                &Ship::FleetID)
            .add_property("producedByEmpireID",     &Ship::ProducedByEmpireID)
            .add_property("isMonster",              &Ship::IsMonster)
            .add_property("isArmed",                &Ship::IsArmed)
            .add_property("canColonize",            &Ship::CanColonize)
            .add_property("canInvade",              &Ship::HasTroops)
            .add_property("speciesName",            make_function(&Ship::SpeciesName,       return_value_policy<copy_const_reference>()))
            .add_property("speed",                  &Ship::Speed)
            .add_property("orderedScrapped",        &Ship::OrderedScrapped)
            .add_property("orderedColonizePlanet",  &Ship::OrderedColonizePlanet)
            .add_property("orderedInvadePlanet",    &Ship::OrderedInvadePlanet)
            .def("initialPartMeterValue",           &Ship::InitialPartMeterValue)
            .def("currentPartMeterValue",           &Ship::CurrentPartMeterValue)
            .add_property("partMeters",             make_function(ShipPartMeters,           return_internal_reference<>()))
            .def("getMeter",                        make_function(ShipGetPartMeter,         return_internal_reference<>()))
        ;

        //////////////////
        //  ShipDesign  //
        //////////////////
        class_<ShipDesign, noncopyable>("shipDesign", no_init)
            .add_property("id",                 make_function(&ShipDesign::ID,              return_value_policy<return_by_value>()))
            .def("name",                        make_function(&ShipDesign::Name,            return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&ShipDesign::Description,     return_value_policy<copy_const_reference>()))
            .add_property("designedOnTurn",     make_function(&ShipDesign::DesignedOnTurn,  return_value_policy<return_by_value>()))
            .add_property("battleSpeed",        make_function(&ShipDesign::BattleSpeed,     return_value_policy<return_by_value>()))
            .add_property("starlaneSpeed",      make_function(&ShipDesign::StarlaneSpeed,   return_value_policy<return_by_value>()))
            .add_property("structure",          make_function(&ShipDesign::Structure,       return_value_policy<return_by_value>()))//since defense does not include Hull
            .add_property("shields",            make_function(&ShipDesign::Shields,         return_value_policy<return_by_value>()))
            .add_property("defense",            make_function(&ShipDesign::Defense,         return_value_policy<return_by_value>()))
            .add_property("attack",             make_function(&ShipDesign::Attack,          return_value_policy<return_by_value>()))
            .add_property("canColonize",        make_function(&ShipDesign::CanColonize,     return_value_policy<return_by_value>()))
            .add_property("canInvade",          make_function(&ShipDesign::HasTroops,       return_value_policy<return_by_value>()))
            .add_property("isArmed",            make_function(&ShipDesign::IsArmed,         return_value_policy<return_by_value>()))
            .add_property("isMonster",          make_function(&ShipDesign::IsMonster,       return_value_policy<return_by_value>()))
            .def("productionCost",              &ShipDesign::ProductionCost)
            .def("productionTime",              &ShipDesign::ProductionTime)
            .def("perTurnCost",                 &ShipDesign::PerTurnCost)
            .add_property("hull",               make_function(&ShipDesign::Hull,            return_value_policy<return_by_value>()))
            .add_property("parts",              make_function(PartsVoid,                    return_internal_reference<>()))
            .add_property("directFireStats",    make_function(
                                                    DirectFireStatsFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const ShipDesign&>()
                                                ))

            .def("partsInSlotType",             PartsSlotType,                              return_value_policy<return_by_value>())
            .def("productionLocationForEmpire", &ShipDesign::ProductionLocation)
        ;
        def("validShipDesign",                  ValidDesignHullAndParts);
        def("validShipDesign",                  ValidDesignDesign);
        def("getShipDesign",                    &GetShipDesign,                             return_value_policy<reference_existing_object>());

        class_<PartType, noncopyable>("partType", no_init)
            .add_property("name",               make_function(&PartType::Name,              return_value_policy<copy_const_reference>()))
            .add_property("class",              &PartType::Class)
            .def("productionCost",              &PartType::ProductionCost)
            .def("productionTime",              &PartType::ProductionTime)
            .def("canMountInSlotType",          &PartType::CanMountInSlotType)
        ;
        def("getPartType",                      &GetPartType,                               return_value_policy<reference_existing_object>());

        class_<HullType, noncopyable>("hullType", no_init)
            .add_property("name",               make_function(&HullType::Name,              return_value_policy<copy_const_reference>()))
            .add_property("numSlots",           make_function(NumSlotsTotal,                return_value_policy<return_by_value>()))
            .def("numSlotsOfSlotType",          NumSlotsOfSlotType)
            .add_property("slots",              make_function(&HullType::Slots,             return_internal_reference<>()))
            .def("productionCost",              &HullType::ProductionCost)
            .def("productionTime",              &HullType::ProductionTime)
        ;
        def("getHullType",                      &GetHullType,                               return_value_policy<reference_existing_object>());

        //////////////////
        //   Building   //
        //////////////////
        class_<Building, bases<UniverseObject>, noncopyable>("building", no_init)
            .add_property("buildingTypeName",   make_function(&Building::BuildingTypeName,  return_value_policy<copy_const_reference>()))
            .add_property("planetID",           make_function(&Building::PlanetID,          return_value_policy<return_by_value>()))
            .add_property("producedByEmpireID", &Building::ProducedByEmpireID)
            .add_property("orderedScrapped",    &Building::OrderedScrapped)
        ;

        //////////////////
        // BuildingType //
        //////////////////
        class_<BuildingType, noncopyable>("buildingType", no_init)
            .add_property("name",               make_function(&BuildingType::Name,          return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&BuildingType::Description,   return_value_policy<copy_const_reference>()))
            .def("productionCost",              &BuildingType::ProductionCost)
            .def("productionTime",              &BuildingType::ProductionTime)
            .def("perTurnCost",                 &BuildingType::PerTurnCost)
            .def("captureResult",               &BuildingType::GetCaptureResult)
            .def("canBeProduced",               &BuildingType::ProductionLocation)  //(int empire_id, int location_id)
        ;
        def("getBuildingType",                  &GetBuildingType,                           return_value_policy<reference_existing_object>());
        ////////////////////
        // ResourceCenter //
        ////////////////////
        class_<ResourceCenter, noncopyable>("resourceCenter", no_init)
            .add_property("focus",                  make_function(&ResourceCenter::Focus,       return_value_policy<copy_const_reference>()))
            .add_property("turnsSinceFocusChange" , &ResourceCenter::TurnsSinceFocusChange)
            .add_property("availableFoci",          &ResourceCenter::AvailableFoci)
        ;

        ///////////////////
        //   PopCenter   //
        ///////////////////
        class_<PopCenter, noncopyable>("popCenter", no_init)
            .add_property("speciesName",        make_function(&PopCenter::SpeciesName,      return_value_policy<copy_const_reference>()))
            .add_property("nextTurnPopGrowth",  &PopCenter::NextTurnPopGrowth)
        ;

        //////////////////
        //    Planet    //
        //////////////////
        class_<Planet, bases<UniverseObject, PopCenter, ResourceCenter>, noncopyable>("planet", no_init)
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
            .add_property("AxialTilt",                      &Planet::AxialTilt)
            .add_property("buildingIDs",                    make_function(&Planet::BuildingIDs,     return_internal_reference<>()))
        ;

        //////////////////
        //    System    //
        //////////////////
        class_<System, bases<UniverseObject>, noncopyable>("system", no_init)
            .add_property("starType",           &System::GetStarType)
            .add_property("numStarlanes",       &System::NumStarlanes)
            .add_property("numWormholes",       &System::NumWormholes)
            .def("HasStarlaneToSystemID",       &System::HasStarlaneTo)
            .def("HasWormholeToSystemID",       &System::HasWormholeTo)
            .add_property("starlanesWormholes", make_function(&System::StarlanesWormholes,  return_value_policy<return_by_value>()))
            .add_property("planetIDs",          make_function(&System::PlanetIDs,           return_value_policy<return_by_value>()))
            .add_property("buildingIDs",        make_function(&System::BuildingIDs,         return_value_policy<return_by_value>()))
            .add_property("fleetIDs",           make_function(&System::FleetIDs,            return_value_policy<return_by_value>()))
            .add_property("shipIDs",            make_function(&System::ShipIDs,             return_value_policy<return_by_value>()))
            .add_property("fieldIDs",           make_function(&System::FieldIDs,            return_value_policy<return_by_value>()))
            .add_property("lastTurnBattleHere", &System::LastTurnBattleHere)
        ;

        //////////////////
        //    Field     //
        //////////////////
        class_<Field, bases<UniverseObject>, noncopyable>("field", no_init)
            .add_property("fieldTypeName",      make_function(&Field::FieldTypeName,        return_value_policy<copy_const_reference>()))
            .def("inField",                     &ObjectInField)
            .def("inField",                     LocationInField)
        ;

        //////////////////
        //   FieldType  //
        //////////////////
        class_<FieldType, noncopyable>("fieldType", no_init)
            .add_property("name",               make_function(&FieldType::Name,             return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&FieldType::Description,      return_value_policy<copy_const_reference>()))
        ;
        def("getFieldType",                     &GetFieldType,                           return_value_policy<reference_existing_object>());


        /////////////////
        //   Special   //
        /////////////////
        class_<Special, noncopyable>("special", no_init)
            .add_property("name",               make_function(&Special::Name,           return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&Special::Description,    return_value_policy<copy_const_reference>()))
        ;
        def("getSpecial",                       &GetSpecial,                            return_value_policy<reference_existing_object>());

        /////////////////
        //   Species   //
        /////////////////
        class_<Species, noncopyable>("species", no_init)
            .add_property("name",               make_function(&Species::Name,           return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&Species::Description,    return_value_policy<copy_const_reference>()))
            .add_property("homeworlds",         make_function(&Species::Homeworlds,     return_value_policy<copy_const_reference>()))
            .add_property("foci",               &SpeciesFoci)
            .add_property("preferredFocus",     make_function(&Species::PreferredFocus, return_value_policy<copy_const_reference>()))
            .add_property("canColonize",        make_function(&Species::CanColonize,    return_value_policy<return_by_value>()))
            .add_property("canProduceShips",    make_function(&Species::CanProduceShips,return_value_policy<return_by_value>()))
            .add_property("tags",               make_function(&Species::Tags,           return_value_policy<return_by_value>()))
            // TODO: const std::vector<FocusType>& Species::Foci()
            .def("getPlanetEnvironment",        &Species::GetPlanetEnvironment)
        ;
        def("getSpecies",                       &GetSpecies,                            return_value_policy<reference_existing_object>());
    }

    void WrapGalaxySetupData() {
        class_<GalaxySetupData>("galaxySetupData")
            .def_readonly ("seed",              &GalaxySetupData::m_seed)
            .def_readwrite("size",              &GalaxySetupData::m_size)
            .def_readwrite("shape",             &GalaxySetupData::m_shape)
            .def_readonly ("age",               &GalaxySetupData::m_age)
            .def_readonly ("starlaneFrequency", &GalaxySetupData::m_starlane_freq)
            .def_readonly ("planetDensity",     &GalaxySetupData::m_planet_density)
            .def_readonly ("specialsFrequency", &GalaxySetupData::m_specials_freq)
            .def_readonly ("monsterFrequency",  &GalaxySetupData::m_monster_freq)
            .def_readonly ("nativeFrequency",   &GalaxySetupData::m_native_freq)
            .def_readonly ("maxAIAgression",    &GalaxySetupData::m_ai_aggr);
    }
}
