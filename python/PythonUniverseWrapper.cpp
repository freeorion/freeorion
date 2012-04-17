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
#include "../universe/Special.h"
#include "../universe/Species.h"

#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>

namespace {
    void                    DumpObjects(const Universe& universe)
    { Logger().debugStream() << universe.Objects().Dump(); }
    const UniverseObject*   GetUniverseObjectP(const Universe& universe, int id) {
        //Logger().debugStream() << "GetUniverseObjectP(universe, " << id << ")";
        const UniverseObject* retval = universe.Objects().Object(id);
        if (!retval)
            retval = ::GetUniverseObject(id);
        return retval;
    }
    const Fleet*            GetFleetP(const Universe& universe, int id) {
        //Logger().debugStream() << "GetFleetP(universe, " << id << ")";
        const Fleet* retval = universe.Objects().Object<Fleet>(id);
        if (!retval)
            retval = ::GetFleet(id);
        return retval;
    }
    const Ship*             GetShipP(const Universe& universe, int id) {
        //Logger().debugStream() << "GetShipP(universe, " << id << ")";
        const Ship* retval = universe.Objects().Object<Ship>(id);
        if (!retval)
            retval = ::GetShip(id);
        return retval;
    }
    const Planet*           GetPlanetP(const Universe& universe, int id) {
        //Logger().debugStream() << "GetPlanetP(universe, " << id << ")";
        const Planet* retval = universe.Objects().Object<Planet>(id);
        if (!retval)
            retval = ::GetPlanet(id);
        return retval;
    }
    const System*           GetSystemP(const Universe& universe, int id) {
        //Logger().debugStream() << "GetSystemP(universe, " << id << ")";
        const System* retval = universe.Objects().Object<System>(id);
        if (!retval)
            retval = ::GetSystem(id);
        return retval;
    }
    const Building*         GetBuildingP(const Universe& universe, int id) {
        //Logger().debugStream() << "GetBuildingP(universe, " << id << ")";
        const Building* retval = universe.Objects().Object<Building>(id);
        if (!retval)
            retval = ::GetBuilding(id);
        return retval;
    }

    std::vector<int>        ObjectIDs(const Universe& universe)
    { return universe.Objects().FindObjectIDs(); }
    std::vector<int>        FleetIDs(const Universe& universe)
    { return universe.Objects().FindObjectIDs<Fleet>(); }
    std::vector<int>        SystemIDs(const Universe& universe)
    { return Objects().FindObjectIDs<System>(); }
    std::vector<int>        PlanetIDs(const Universe& universe)
    { return Objects().FindObjectIDs<Planet>(); }
    std::vector<int>        ShipIDs(const Universe& universe)
    { return universe.Objects().FindObjectIDs<Ship>(); }
    std::vector<int>        BuildingIDs(const Universe& universe)
    { return Objects().FindObjectIDs<Building>(); }

    void                    (Universe::*UpdateMeterEstimatesVoidFunc)(void) =                   &Universe::UpdateMeterEstimates;

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

    bool                    SystemsConnectedP(const Universe& universe, int system1_id, int system2_id) {
        //Logger().debugStream() << "SystemsConnected!(" << system1_id << ", " << system2_id << ")";
        try {
            bool retval = universe.SystemsConnected(system1_id, system2_id);
            //Logger().debugStream() << "SystemsConnected! retval: " << retval;
            return retval;
        } catch (...) {
        }
        return false;
    }
    boost::function<bool(const Universe&, int, int)> SystemsConnectedFunc =                     &SystemsConnectedP;

    const Meter*            (UniverseObject::*ObjectGetMeter)(MeterType) const =                &UniverseObject::GetMeter;

    std::vector<std::string>    ObjectSpecials(const UniverseObject& object) {
        std::vector<std::string> retval;
        for (std::map<std::string, int>::const_iterator it = object.Specials().begin();
             it != object.Specials().end(); ++it)
        { retval.push_back(it->first); }
        return retval;
    }

    boost::function<double(const UniverseObject*, MeterType)> InitialCurrentMeterValueFromObject =
        boost::bind(&Meter::Initial, boost::bind(ObjectGetMeter, _1, _2));

    bool                    (*ValidDesignHullAndParts)(const std::string& hull,
                                                       const std::vector<std::string>& parts) = &ShipDesign::ValidDesign;
    bool                    (*ValidDesignDesign)(const ShipDesign&) =                           &ShipDesign::ValidDesign;

    const std::vector<std::string>& (ShipDesign::*PartsVoid)(void) const =                      &ShipDesign::Parts;
    std::vector<std::string>        (ShipDesign::*PartsSlotType)(ShipSlotType) const =          &ShipDesign::Parts;

    unsigned int            (HullType::*NumSlotsTotal)(void) const =                            &HullType::NumSlots;
    unsigned int            (HullType::*NumSlotsOfSlotType)(ShipSlotType) const =               &HullType::NumSlots;
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
        ////////////////////
        //    Universe    //
        ////////////////////
        class_<Universe, noncopyable>("universe", no_init)
            .def("getObject",                   make_function(GetUniverseObjectP,   return_value_policy<reference_existing_object>()))
            .def("getFleet",                    make_function(GetFleetP,            return_value_policy<reference_existing_object>()))
            .def("getShip",                     make_function(GetShipP,             return_value_policy<reference_existing_object>()))
            .def("getPlanet",                   make_function(GetPlanetP,           return_value_policy<reference_existing_object>()))
            .def("getSystem",                   make_function(GetSystemP,           return_value_policy<reference_existing_object>()))
            .def("getBuilding",                 make_function(GetBuildingP,         return_value_policy<reference_existing_object>()))

            .add_property("allObjectIDs",       make_function(ObjectIDs,            return_value_policy<return_by_value>()))
            .add_property("fleetIDs",           make_function(FleetIDs,             return_value_policy<return_by_value>()))
            .add_property("systemIDs",          make_function(SystemIDs,            return_value_policy<return_by_value>()))
            .add_property("planetIDs",          make_function(PlanetIDs,            return_value_policy<return_by_value>()))
            .add_property("shipIDs",            make_function(ShipIDs,              return_value_policy<return_by_value>()))
            .add_property("buildingIDs",        make_function(BuildingIDs,          return_value_policy<return_by_value>()))

            .def("systemHasStarlane",           &Universe::SystemHasVisibleStarlanes)

            .def("updateMeterEstimates",        UpdateMeterEstimatesVoidFunc)

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
                                                    boost::mpl::vector<bool, const Universe&, int, int>()
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
            .def("currentMeterValue",           &UniverseObject::CurrentMeterValue)
            .def("initialMeterValue",           &UniverseObject::InitialMeterValue)
            .def("nextTurnCurrentMeterValue",   &UniverseObject::NextTurnCurrentMeterValue)
            .add_property("tags",               make_function(&UniverseObject::Tags,        return_value_policy<return_by_value>()))
            .def("hasTag",                      &UniverseObject::HasTag)
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
            .add_property("speed",                      &Fleet::Speed)
            .add_property("canChangeDirectionEnRoute",  &Fleet::CanChangeDirectionEnRoute)
            .add_property("hasMonsters",                &Fleet::HasMonsters)
            .add_property("hasArmedShips",              &Fleet::HasArmedShips)
            .add_property("hasColonyShips",             &Fleet::HasColonyShips)
            .add_property("hasTroopShips",              &Fleet::HasTroopShips)
            .add_property("numShips",                   &Fleet::NumShips)
            .add_property("empty",                      &Fleet::Empty)
            .add_property("shipIDs",                    make_function(&Fleet::ShipIDs,      return_internal_reference<>()))
        ;

        //////////////////
        //     Ship     //
        //////////////////
        class_<Ship, bases<UniverseObject>, noncopyable>("ship", no_init)
            .add_property("design",             make_function(&Ship::Design,                return_value_policy<reference_existing_object>()))
            .add_property("designID",           &Ship::DesignID)
            .add_property("fleetID",            &Ship::FleetID)
            .add_property("isMonster",          &Ship::IsMonster)
            .add_property("isArmed",            &Ship::IsArmed)
            .add_property("canColonize",        &Ship::CanColonize)
            .add_property("canInvade",          &Ship::HasTroops)
            .add_property("speed",              &Ship::Speed)
        ;

        //////////////////
        //  ShipDesign  //
        //////////////////
        class_<ShipDesign, noncopyable>("shipDesign", no_init)
            .add_property("id",                 make_function(&ShipDesign::ID,              return_value_policy<return_by_value>()))
            .def("name",                        make_function(&ShipDesign::Name,            return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&ShipDesign::Description,     return_value_policy<copy_const_reference>()))
            .add_property("designedByEmpireID", make_function(&ShipDesign::DesignedByEmpire,return_value_policy<return_by_value>()))
            .add_property("designedOnTurn",     make_function(&ShipDesign::DesignedOnTurn,  return_value_policy<return_by_value>()))
            .add_property("battleSpeed",        make_function(&ShipDesign::BattleSpeed,     return_value_policy<return_by_value>()))
            .add_property("starlaneSpeed",      make_function(&ShipDesign::StarlaneSpeed,   return_value_policy<return_by_value>()))
            .add_property("defense",            make_function(&ShipDesign::Defense,         return_value_policy<return_by_value>()))
            .add_property("attack",             make_function(&ShipDesign::Attack,          return_value_policy<return_by_value>()))
            .add_property("canColonize",        make_function(&ShipDesign::CanColonize,     return_value_policy<return_by_value>()))
            .add_property("canInvade",          make_function(&ShipDesign::HasTroops,       return_value_policy<return_by_value>()))
            .add_property("isArmed",            make_function(&ShipDesign::IsArmed,         return_value_policy<return_by_value>()))
            .add_property("isMonster",          make_function(&ShipDesign::IsMonster,       return_value_policy<return_by_value>()))
            .add_property("productionCost",     make_function(&ShipDesign::ProductionCost,  return_value_policy<return_by_value>()))
            .add_property("productionTime",     make_function(&ShipDesign::ProductionTime,  return_value_policy<return_by_value>()))
            .add_property("perTurnCost",        make_function(&ShipDesign::PerTurnCost,     return_value_policy<return_by_value>()))
            .add_property("hull",               make_function(&ShipDesign::Hull,            return_value_policy<return_by_value>()))
            .add_property("parts",              make_function(PartsVoid,                    return_internal_reference<>()))
            .def("partsInSlotType",             PartsSlotType,                              return_value_policy<return_by_value>())
            .def("productionLocationForEmpire", &ShipDesign::ProductionLocation)
        ;
        def("validShipDesign",                  ValidDesignHullAndParts);
        def("validShipDesign",                  ValidDesignDesign);
        def("getShipDesign",                    &GetShipDesign,                             return_value_policy<reference_existing_object>());

        class_<PartType, noncopyable>("partType", no_init)
            .add_property("name",               make_function(&PartType::Name,              return_value_policy<copy_const_reference>()))
            .add_property("class",              &PartType::Class)
            .add_property("productionCost",     make_function(&PartType::ProductionCost,    return_value_policy<return_by_value>()))
            .add_property("productionTime",     make_function(&PartType::ProductionTime,    return_value_policy<return_by_value>()))
            .def("canMountInSlotType",          &PartType::CanMountInSlotType)
        ;
        def("getPartType",                      &GetPartType,                               return_value_policy<reference_existing_object>());

        class_<HullType, noncopyable>("hullType", no_init)
            .add_property("name",               make_function(&HullType::Name,              return_value_policy<copy_const_reference>()))
            .add_property("numSlots",           make_function(NumSlotsTotal,                return_value_policy<return_by_value>()))
            .def("numSlotsOfSlotType",          NumSlotsOfSlotType)
            .add_property("slots",              make_function(&HullType::Slots,             return_internal_reference<>()))
            .add_property("productionCost",     make_function(&HullType::ProductionCost,    return_value_policy<return_by_value>()))
            .add_property("productionTime",     make_function(&HullType::ProductionTime,    return_value_policy<return_by_value>()))
        ;
        def("getHullType",                      &GetHullType,                               return_value_policy<reference_existing_object>());


        //////////////////
        //   Building   //
        //////////////////
        class_<Building, bases<UniverseObject>, noncopyable>("building", no_init)
            .add_property("buildingTypeName",   make_function(&Building::BuildingTypeName,  return_value_policy<copy_const_reference>()))
            //.add_property("operating",          &Building::Operating)
            .add_property("planetID",           make_function(&Building::PlanetID,          return_value_policy<return_by_value>()))
        ;

        //////////////////
        // BuildingType //
        //////////////////
        class_<BuildingType, noncopyable>("buildingType", no_init)
            .add_property("name",               make_function(&BuildingType::Name,          return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&BuildingType::Description,   return_value_policy<copy_const_reference>()))
            .add_property("productionCost",     &BuildingType::ProductionCost)
            .add_property("productionTime",     &BuildingType::ProductionTime)
            .add_property("maintenanceCost",    &BuildingType::MaintenanceCost)
            .add_property("perTurnCost",        &BuildingType::PerTurnCost)
            .def("captureResult",               &BuildingType::GetCaptureResult)
        ;
        def("getBuildingType",                  &GetBuildingType,                           return_value_policy<reference_existing_object>());


        ////////////////////
        // ResourceCenter //
        ////////////////////
        class_<ResourceCenter, noncopyable>("resourceCenter", no_init)
            .add_property("focus",              make_function(&ResourceCenter::Focus,       return_value_policy<copy_const_reference>()))
            .add_property("availableFoci",      &ResourceCenter::AvailableFoci)
        ;

        ///////////////////
        //   PopCenter   //
        ///////////////////
        class_<PopCenter, noncopyable>("popCenter", no_init)
            .add_property("speciesName",        make_function(&PopCenter::SpeciesName,      return_value_policy<copy_const_reference>()))
        ;

        //////////////////
        //    Planet    //
        //////////////////
        class_<Planet, bases<UniverseObject, PopCenter, ResourceCenter>, noncopyable>("planet", no_init)
            .add_property("size",               &Planet::Size)
            .add_property("type",               &Planet::Type)
            .add_property("buildingIDs",        make_function(&Planet::Buildings,           return_internal_reference<>()))
        ;

        //////////////////
        //    System    //
        //////////////////
        class_<System, bases<UniverseObject>, noncopyable>("system", no_init)
            .add_property("starType",           &System::GetStarType)
            .add_property("numOrbits",          &System::Orbits)
            .add_property("numStarlanes",       &System::NumStarlanes)
            .add_property("numWormholes",       &System::NumWormholes)
            .def("HasStarlaneToSystemID",       &System::HasStarlaneTo)
            .def("HasWormholeToSystemID",       &System::HasWormholeTo)
            .add_property("allObjectIDs",       make_function(&System::FindObjectIDs<UniverseObject>,   return_value_policy<return_by_value>()))
            .add_property("planetIDs",          make_function(&System::FindObjectIDs<Planet>,           return_value_policy<return_by_value>()))
            .add_property("fleetIDs",           make_function(&System::FindObjectIDs<Fleet>,            return_value_policy<return_by_value>()))
        ;

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
            // TODO: const std::vector<FocusType>& Species::Foci()
            .def("getPlanetEnvironment",        &Species::GetPlanetEnvironment)
        ;
        def("getSpecies",                       &GetSpecies,                            return_value_policy<reference_existing_object>());
    }
}
