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

#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>

namespace {
    const UniverseObject*   GetObjectA(const Universe& universe, int id) {
        return universe.Objects().Object(id);
    }
    const Fleet*            GetFleet(const Universe& universe, int id) {
        return universe.Objects().Object<Fleet>(id);
    }
    const Ship*             GetShip(const Universe& universe, int id) {
        return universe.Objects().Object<Ship>(id);
    }
    const Planet*           GetPlanet(const Universe& universe, int id) {
        return universe.Objects().Object<Planet>(id);
    }
    const System*           GetSystem(const Universe& universe, int id) {
        return universe.Objects().Object<System>(id);
    }
    const Building*         GetBuilding(const Universe& universe, int id) {
        return universe.Objects().Object<Building>(id);
    }

    std::vector<int>        ObjectIDs(const Universe& universe) {
        return universe.Objects().FindObjectIDs<UniverseObject>();
    }
    std::vector<int>        FleetIDs(const Universe& universe) {
        return universe.Objects().FindObjectIDs<Fleet>();
    }
    std::vector<int>        SystemIDs(const Universe& universe) {
        return universe.Objects().FindObjectIDs<System>();
    }
    std::vector<int>        PlanetIDs(const Universe& universe) {
        return universe.Objects().FindObjectIDs<Planet>();
    }
    std::vector<int>        ShipIDs(const Universe& universe) {
        return universe.Objects().FindObjectIDs<Ship>();
    }
    std::vector<int>        BuildingIDs(const Universe& universe) {
        return universe.Objects().FindObjectIDs<Building>();
    }

    void                    (Universe::*UpdateMeterEstimatesVoidFunc)(void) =                   &Universe::UpdateMeterEstimates;

    std::vector<int>        LeastJumpsPath(const Universe* universe, int start_sys, int end_sys, int empire_id) {
        std::pair<std::list<int>, int> path = universe->LeastJumpsPath(start_sys, end_sys, empire_id);
        std::vector<int> retval;
        std::copy(path.first.begin(), path.first.end(), std::back_inserter(retval));
        return retval;
    }

    boost::function<std::vector<int>(const Universe*, int, int, int)> LeastJumpsFunc =          &LeastJumpsPath;

    std::vector<int>        ShortestPath(const Universe* universe, int start_sys, int end_sys, int empire_id) {
        std::pair<std::list<int>, int> path = universe->ShortestPath(start_sys, end_sys, empire_id);
        std::vector<int> retval;
        std::copy(path.first.begin(), path.first.end(), std::back_inserter(retval));
        return retval;
    }

    boost::function<std::vector<int>(const Universe*, int, int, int)> ShortestPathFunc =        &ShortestPath;

    const Meter*            (UniverseObject::*ObjectGetMeter)(MeterType) const =                &UniverseObject::GetMeter;

    boost::function<double(const UniverseObject*, MeterType)> InitialCurrentMeterValueFromObject =
        boost::bind(&Meter::InitialCurrent, boost::bind(ObjectGetMeter, _1, _2));

    boost::function<double(const UniverseObject*, MeterType)> InitialMaxMeterValueFromObject =
        boost::bind(&Meter::InitialMax, boost::bind(ObjectGetMeter, _1, _2));

    boost::function<double(const UniverseObject*, MeterType)> ProjectedMaxMeterValueFromObject =
        boost::bind(&Meter::Max, boost::bind(ObjectGetMeter, _1, _2));

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
            .def("getObject",                   make_function(GetObjectA,   return_value_policy<reference_existing_object>()))
            .def("getFleet",                    make_function(GetFleet,     return_value_policy<reference_existing_object>()))
            .def("getShip",                     make_function(GetShip,      return_value_policy<reference_existing_object>()))
            .def("getPlanet",                   make_function(GetPlanet,    return_value_policy<reference_existing_object>()))
            .def("getSystem",                   make_function(GetSystem,    return_value_policy<reference_existing_object>()))
            .def("getBuilding",                 make_function(GetBuilding,  return_value_policy<reference_existing_object>()))

            .add_property("allObjectIDs",       make_function(ObjectIDs,    return_value_policy<return_by_value>()))
            .add_property("fleetIDs",           make_function(FleetIDs,     return_value_policy<return_by_value>()))
            .add_property("systemIDs",          make_function(SystemIDs,    return_value_policy<return_by_value>()))
            .add_property("planetIDs",          make_function(PlanetIDs,    return_value_policy<return_by_value>()))
            .add_property("shipIDs",            make_function(ShipIDs,      return_value_policy<return_by_value>()))
            .add_property("buildingIDs",        make_function(BuildingIDs,  return_value_policy<return_by_value>()))

            .def("systemHasStarlane",           &Universe::SystemHasVisibleStarlanes)
            .def("systemsConnected",            &Universe::SystemsConnected)

            // put as part of universe class so one doesn't need a UniverseObject object in python to access these
            .def_readonly("invalidObjectID",    &UniverseObject::INVALID_OBJECT_ID)
            .def_readonly("invalidObjectAge",   &UniverseObject::INVALID_OBJECT_AGE)

            .def("updateMeterEstimates",        UpdateMeterEstimatesVoidFunc)

            .def("leastJumpsPath",              make_function(
                                                    LeastJumpsFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe*, int, int, int>()
                                                ))

            .def("shortestPath",                make_function(
                                                    ShortestPathFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<int>, const Universe*, int, int, int>()
                                                ))
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
            .add_property("owners",             make_function(&UniverseObject::Owners,      return_internal_reference<>()))
            .def("ownedBy",                     &UniverseObject::OwnedBy)
            .def("whollyOwnedBy",               &UniverseObject::WhollyOwnedBy)
            .add_property("creationTurn",       &UniverseObject::CreationTurn)
            .add_property("ageInTurns",         &UniverseObject::AgeInTurns)
            .add_property("specials",           make_function(&UniverseObject::Specials,    return_internal_reference<>()))
            .def("Contains",                    &UniverseObject::Contains)
            .def("ContainedBy",                 &UniverseObject::ContainedBy)
            .def("MeterPoints",                 &UniverseObject::MeterPoints)           // actual amount of something represented by meter.  eg. population or resource production that isn't equal to the meter value
            .def("ProjectedMeterPoints",        &UniverseObject::ProjectedMeterPoints)
            .def("CurrentMeter",                make_function(
                                                    boost::bind(InitialCurrentMeterValueFromObject, _1, _2),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<double, const UniverseObject*, MeterType>()
                                                ))
            .def("ProjectedCurrentMeter",       &UniverseObject::ProjectedCurrentMeter)
            .def("MaxMeter",                    make_function(
                                                    boost::bind(InitialMaxMeterValueFromObject, _1, _2),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<double, const UniverseObject*, MeterType>()
                                                ))
            .def("ProjectedMaxMeter",           make_function(
                                                    boost::bind(ProjectedMaxMeterValueFromObject, _1, _2),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<double, const UniverseObject*, MeterType>()
                                                ))
            //.def("GetMeter",                    ObjectGetMeter,                             return_value_policy<reference_existing_object>())
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
            .add_property("hasArmedShips",              &Fleet::HasArmedShips)
            .add_property("hasColonyShips",             &Fleet::HasColonyShips)
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
            .add_property("getFleet",           make_function(&Ship::GetFleet,              return_value_policy<reference_existing_object>()))
            .add_property("isArmed",            &Ship::IsArmed)
            .add_property("canColonize",        &Ship::CanColonize)
            .add_property("speed",              &Ship::Speed)
        ;

        //////////////////
        //  ShipDesign  //
        //////////////////
        class_<ShipDesign, noncopyable>("shipDesign", no_init)
            .add_property("id",                 make_function(&ShipDesign::ID,                  return_value_policy<return_by_value>()))
            .def("name",                        make_function(&ShipDesign::Name,                return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&ShipDesign::Description,         return_value_policy<copy_const_reference>()))
            .add_property("designedByEmpireID", make_function(&ShipDesign::DesignedByEmpire,    return_value_policy<return_by_value>()))
            .add_property("designedOnTurn",     make_function(&ShipDesign::DesignedOnTurn,      return_value_policy<return_by_value>()))
            .add_property("battleSpeed",        make_function(&ShipDesign::BattleSpeed,         return_value_policy<return_by_value>()))
            .add_property("starlaneSpeed",      make_function(&ShipDesign::StarlaneSpeed,       return_value_policy<return_by_value>()))
            .add_property("defense",            make_function(&ShipDesign::Defense,             return_value_policy<return_by_value>()))
            .add_property("attack",             make_function(&ShipDesign::Attack,              return_value_policy<return_by_value>()))
            .add_property("canColonize",        make_function(&ShipDesign::CanColonize,         return_value_policy<return_by_value>()))
            .add_property("cost",               make_function(&ShipDesign::Cost,                return_value_policy<return_by_value>()))
            .add_property("buildTime",          make_function(&ShipDesign::BuildTime,           return_value_policy<return_by_value>()))
            .add_property("hull",               make_function(&ShipDesign::Hull,                return_value_policy<return_by_value>()))
            .add_property("parts",              make_function(PartsVoid,                        return_internal_reference<>()))
            .def("partsInSlotType",             PartsSlotType,                                  return_value_policy<return_by_value>())
            //.add_property("graphic",            make_function(&ShipDesign::Graphic,             return_value_policy<copy_const_reference>()))
            //.add_property("model",              make_function(&ShipDesign::Model,               return_value_policy<copy_const_reference>()))
            .def("productionLocationForEmpire", &ShipDesign::ProductionLocation)
        ;
        def("validShipDesign",                  ValidDesignHullAndParts);
        def("validShipDesign",                  ValidDesignDesign);
        def("getShipDesign",                    &GetShipDesign,                                 return_value_policy<reference_existing_object>());

        class_<PartType, noncopyable>("partType", no_init)
            .add_property("name",               make_function(&PartType::Name,                  return_value_policy<copy_const_reference>()))
            .def("canMountInSlotType",          &PartType::CanMountInSlotType)
        ;
        def("getPartType",                      &GetPartType,                                   return_value_policy<reference_existing_object>());

        class_<HullType, noncopyable>("hullType", no_init)
            .add_property("name",               make_function(&HullType::Name,                  return_value_policy<copy_const_reference>()))
            .add_property("numSlots",           make_function(NumSlotsTotal,                    return_value_policy<return_by_value>()))
            .def("numSlotsOfSlotType",          NumSlotsOfSlotType)
            .add_property("slots",              make_function(&HullType::Slots,                 return_internal_reference<>()))
        ;
        def("getHullType",                      &GetHullType,                                   return_value_policy<reference_existing_object>());


        //////////////////
        //   Building   //
        //////////////////
        class_<Building, bases<UniverseObject>, noncopyable>("building", no_init)
            .def("getBuildingType",             &Building::GetBuildingType,                     return_value_policy<reference_existing_object>())
//            .add_property("operating",          &Building::Operating)
            .def("getPlanet",                   &Building::GetPlanet,                           return_value_policy<reference_existing_object>())
        ;

        //////////////////
        // BuildingType //
        //////////////////
        class_<BuildingType, noncopyable>("buildingType", no_init)
            .add_property("name",               make_function(&BuildingType::Name,              return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&BuildingType::Description,       return_value_policy<copy_const_reference>()))
            .add_property("buildCost",          &BuildingType::BuildCost)
            .add_property("buildTime",          &BuildingType::BuildTime)
            .add_property("maintenanceCost",    &BuildingType::MaintenanceCost)
            .def("captureResult",               &BuildingType::GetCaptureResult)
        ;
        def("getBuildingType",                  &GetBuildingType,                               return_value_policy<reference_existing_object>());


        ////////////////////
        // ResourceCenter //
        ////////////////////
        class_<ResourceCenter, noncopyable>("resourceCenter", no_init)
            .add_property("primaryFocus",       &ResourceCenter::PrimaryFocus)
            .add_property("secondaryFocus",     &ResourceCenter::SecondaryFocus)
        ;

        ///////////////////
        //   PopCenter   //
        ///////////////////
        class_<PopCenter, noncopyable>("popCenter", no_init)
            .add_property("inhabitants",        &PopCenter::Inhabitants)
            .add_property("allocatedFood",      &PopCenter::AllocatedFood)
        ;

        //////////////////
        //    Planet    //
        //////////////////
        class_<Planet, bases<UniverseObject, PopCenter, ResourceCenter>, noncopyable>("planet", no_init)
            .add_property("size",               &Planet::Size)
            .add_property("type",               &Planet::Type)
            .add_property("buildingIDs",        make_function(&Planet::Buildings,               return_internal_reference<>()))
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
    }
}
