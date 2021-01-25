#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"
#include "../Empire/Diplomacy.h"
#include "../Empire/Government.h"
#include "../Empire/InfluenceQueue.h"
#include "../universe/UniverseObject.h"
#include "../universe/UnlockableItem.h"
#include "../universe/Planet.h"
#include "../universe/Tech.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/SitRepEntry.h"
#include "SetWrapper.h"

#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/to_python_converter.hpp>

#include <iterator>
#include <memory>

namespace py = boost::python;


namespace {
    template<typename T1, typename T2>
    struct PairToTupleConverter {
        static PyObject* convert(const std::pair<T1, T2>& pair) {
            return py::incref(py::make_tuple(pair.first, pair.second).ptr());
        }
    };

    typedef PairToTupleConverter<int, int> IntIntPairConverter;

    typedef PairToTupleConverter<float, int> FloatIntPairConverter;


    auto GetSitRep(const Empire& empire, int index) -> const SitRepEntry&
    {
        static SitRepEntry EMPTY_ENTRY;
        if (index < 0 || index >= empire.NumSitRepEntries())
            return EMPTY_ENTRY;
        return *std::next(empire.SitRepBegin(), index);
    }

    auto obstructedStarlanes(const Empire& empire) -> std::vector<std::pair<int, int>>
    {
        const std::set<std::pair<int, int>>& laneset = GetSupplyManager().SupplyObstructedStarlaneTraversals(empire.EmpireID());
        std::vector<std::pair<int, int>> retval;
        try {
            for (const std::pair<int, int>& lane : laneset)
            { retval.push_back(lane); }
        } catch (...) {
        }
        return retval;
    }

    auto jumpsToSuppliedSystem(const Empire& empire) -> std::map<int, int>
    {
        std::map<int, int> retval;
        const std::map<int, std::set<int>>& empire_starlanes = empire.KnownStarlanes();
        std::list<int> propagating_list;

        for (int system_id : GetSupplyManager().FleetSupplyableSystemIDs(empire.EmpireID(), true)) {
            retval[system_id] = 0;
            propagating_list.push_back(system_id);
        }

        // iteratively propagate supply out from supplied systems, to determine
        // how many jumps away from supply each unsupplied system is...
        while (!propagating_list.empty()) {
            // get next system and distance from the list
            int from_sys_id = propagating_list.front();
            propagating_list.pop_front();
            int from_sys_dist = retval[from_sys_id];

            // get lanes connected to this system
            auto lane_set_it = empire_starlanes.find(from_sys_id);
            if (lane_set_it == empire_starlanes.end())
                continue;   // no lanes to propagate from for this supply source
            auto& lane_ends = lane_set_it->second;

            // propagate to any not-already-counted adjacent system
            for (int lane_end_system_id : lane_ends) {
                if (retval.count(lane_end_system_id))
                    continue;   // system already processed
                // system not yet processed; add it to list to propagate from, and set its range to one more than this system
                propagating_list.push_back(lane_end_system_id);
                retval[lane_end_system_id] = from_sys_dist - 1;   // negative values used to indicate jumps to nearest supply for historical compatibility reasons
            }
        }

        //// DEBUG
        //DebugLogger() << "jumpsToSuppliedSystem results for empire, " << empire.Name() << " (" << empire.EmpireID() << ") :";
        //for (const auto& system_jumps : retval) {
        //    DebugLogger() << "sys " << system_jumps.first << "  range: " << system_jumps.second;
        //}
        //// END DEBUG

        return retval;
    }

    auto PlanetsWithAvailablePP(const Empire& empire) -> std::map<std::set<int>, float>
    {
        const auto& industry_pool = empire.GetResourcePool(ResourceType::RE_INDUSTRY);
        const ProductionQueue& prod_queue = empire.GetProductionQueue();
        std::map<std::set<int>, float> planets_with_available_pp;
        for (const auto& objects_pp : prod_queue.AvailablePP(industry_pool)) {
            std::set<int> planets;
            for (const auto& planet : Objects().find<Planet>(objects_pp.first)) {
                if (!planet)
                    continue;
                planets.insert(planet->ID());
            }
            if (!planets.empty())
                planets_with_available_pp[planets] = objects_pp.second;
        }
        return planets_with_available_pp;
    }

    auto PlanetsWithAllocatedPP(const Empire& empire) -> std::map<std::set<int>, float>
    {
        const auto& prod_queue = empire.GetProductionQueue();
        std::map<std::set<int>, float> planets_with_allocated_pp;
        for (const auto& objects_pp : prod_queue.AllocatedPP()) {
            std::set<int> planets;
            for (const auto& planet : Objects().find<Planet>(objects_pp.first)) {
                if (!planet)
                    continue;
                planets.insert(planet->ID());
            }
            if (!planets.empty())
                planets_with_allocated_pp[planets] = objects_pp.second;
        }
        return planets_with_allocated_pp;
    }

    auto PlanetsWithWastedPP(const Empire& empire) -> std::set<std::set<int>>
    {
        const auto& industry_pool = empire.GetResourcePool(ResourceType::RE_INDUSTRY);
        const ProductionQueue& prod_queue = empire.GetProductionQueue();
        std::set<std::set<int>> planets_with_wasted_pp;
        for (const auto& objects : prod_queue.ObjectsWithWastedPP(industry_pool)) {
                 std::set<int> planets;
                 for (const auto& planet : Objects().find<Planet>(objects)) {
                    if (!planet)
                        continue;
                    planets.insert(planet->ID());
                 }
                 if (!planets.empty())
                     planets_with_wasted_pp.insert(planets);
             }
             return planets_with_wasted_pp;
    }

    auto ResearchedTechNames(const Empire& empire) -> std::set<std::string>
    {
        std::set<std::string> retval;
        for (const auto& entry : empire.ResearchedTechs())
            retval.insert(entry.first);
        return retval;
    }
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
    void WrapEmpire() {
        py::to_python_converter<std::pair<int, int>, IntIntPairConverter>();

        py::to_python_converter<std::pair<float, int>, FloatIntPairConverter>();

        py::class_<std::map<std::pair<int, int>, int>>("PairIntInt_IntMap")
            .def(py::map_indexing_suite<std::map<std::pair<int, int>, int>, true>())
        ;

        py::class_<std::vector<std::pair<int, int>>>("IntPairVec")
            .def(py::vector_indexing_suite<std::vector<std::pair<int, int>>, true>())
        ;

        py::class_<std::vector<UnlockableItem>>("UnlockableItemVec")
            .def(py::vector_indexing_suite<std::vector<UnlockableItem>, true>())
        ;

        py::class_<ResourcePool, std::shared_ptr<ResourcePool>, boost::noncopyable>("resPool", py::no_init);

        FreeOrionPython::SetWrapper<std::set<int>>::Wrap("IntSetSet");

        py::class_<std::map<std::set<int>, float>>("resPoolMap")
            .def(py::map_indexing_suite<std::map<std::set<int>, float>, true>())
        ;

        py::class_<std::map<std::string, int>>("StringIntMap")
            .def(py::map_indexing_suite<std::map<std::string, int>, true>())
        ;

        py::class_<std::map<int, std::string>>("IntStringMap")
            .def(py::map_indexing_suite<std::map<int, std::string>, true>())
        ;

        py::class_<std::map<std::string, std::map<int, std::string>>>("String_IntStringMap_Map")
            .def(py::map_indexing_suite<std::map<std::string, std::map<int, std::string>>, true>())
        ;

        ///////////////////
        //     Empire    //
        ///////////////////
        py::class_<Empire, boost::noncopyable>("empire", py::no_init)
            .add_property("name",                   make_function(&Empire::Name,                    py::return_value_policy<py::copy_const_reference>()))
            .add_property("playerName",             make_function(&Empire::PlayerName,              py::return_value_policy<py::copy_const_reference>()))

            .add_property("empireID",               &Empire::EmpireID)
            .add_property("capitalID",              &Empire::CapitalID)

            .add_property("colour",                 +[](const Empire& empire) { EmpireColor color = empire.Color(); return py::make_tuple(std::get<0>(color), std::get<1>(color), std::get<2>(color), std::get<3>(color)); })

            .def("buildingTypeAvailable",           &Empire::BuildingTypeAvailable)
            .add_property("availableBuildingTypes", make_function(&Empire::AvailableBuildingTypes,  py::return_internal_reference<>()))
            .def("shipDesignAvailable",             (bool (Empire::*)(int) const)&Empire::ShipDesignAvailable)
            .add_property("allShipDesigns",         make_function(&Empire::ShipDesigns,             py::return_value_policy<py::return_by_value>()))
            .add_property("availableShipDesigns",   make_function(&Empire::AvailableShipDesigns,    py::return_value_policy<py::return_by_value>()))
            .add_property("availableShipParts",     make_function(&Empire::AvailableShipParts,      py::return_value_policy<py::copy_const_reference>()))
            .add_property("availableShipHulls",     make_function(&Empire::AvailableShipHulls,      py::return_value_policy<py::copy_const_reference>()))
            .add_property("productionQueue",        make_function(&Empire::GetProductionQueue,      py::return_internal_reference<>()))
            .def("productionCostAndTime",           +[](const Empire& empire, const ProductionQueue::Element& element) -> std::pair<float, int> { return element.ProductionCostAndTime(); },
                                                    py::return_value_policy<py::return_by_value>())
            .add_property("planetsWithAvailablePP", make_function(
                                                        PlanetsWithAvailablePP,
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))
            .add_property("planetsWithAllocatedPP", make_function(
                                                        PlanetsWithAllocatedPP,
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))
            .add_property("planetsWithWastedPP",    make_function(
                                                        PlanetsWithWastedPP,
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))

            .def("techResearched",                  &Empire::TechResearched)
            .add_property("availableTechs",         make_function(
                                                        ResearchedTechNames,
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))
            .def("getTechStatus",                   &Empire::GetTechStatus)
            .def("researchProgress",                &Empire::ResearchProgress)
            .add_property("researchQueue",          make_function(&Empire::GetResearchQueue,        py::return_internal_reference<>()))

            .def("policyAdopted",                   &Empire::PolicyAdopted)
            .def("turnPolicyAdopted",               &Empire::TurnPolicyAdopted)
            .def("slotPolicyAdoptedIn",             &Empire::SlotPolicyAdoptedIn)
            .add_property("adoptedPolicies",        make_function(&Empire::AdoptedPolicies,                 py::return_value_policy<py::return_by_value>()))
            .add_property("categoriesSlotPolicies", make_function(&Empire::CategoriesSlotsPoliciesAdopted,  py::return_value_policy<py::return_by_value>()))
            .add_property("turnsPoliciesAdopted",   make_function(&Empire::TurnsPoliciesAdopted,            py::return_value_policy<py::return_by_value>()))
            .add_property("availablePolicies",      make_function(&Empire::AvailablePolicies,               py::return_value_policy<py::copy_const_reference>()))
            .def("policyAvailable",                 &Empire::PolicyAvailable)
            .add_property("totalPolicySlots",       make_function(&Empire::TotalPolicySlots,                py::return_value_policy<py::return_by_value>()))
            .add_property("emptyPolicySlots",       make_function(&Empire::EmptyPolicySlots,                py::return_value_policy<py::return_by_value>()))

            .def("canBuild",                        +[](const Empire& empire, BuildType build_type, const std::string& name, int location) -> bool { return empire.ProducibleItem(build_type, name, location); })
            .def("canBuild",                        +[](const Empire& empire, BuildType build_type, int design, int location) -> bool { return empire.ProducibleItem(build_type, design, location); })

            .def("hasExploredSystem",               &Empire::HasExploredSystem)
            .add_property("exploredSystemIDs",      make_function(&Empire::ExploredSystems,         py::return_internal_reference<>()))

            .add_property("eliminated",             &Empire::Eliminated)
            .add_property("won",                    &Empire::Won)

            .add_property("productionPoints",       make_function(&Empire::ProductionPoints,        py::return_value_policy<py::return_by_value>()))
            .def("resourceStockpile",               &Empire::ResourceStockpile)
            .def("resourceProduction",              &Empire::ResourceOutput)
            .def("resourceAvailable",               &Empire::ResourceAvailable)
            .def("getResourcePool",                 &Empire::GetResourcePool)

            .def("population",                      &Empire::Population)

            .def("preservedLaneTravel",             &Empire::PreservedLaneTravel)
            .add_property("fleetSupplyableSystemIDs",   make_function(
                                                            +[](const Empire& empire) -> const std::set<int>& { return GetSupplyManager().FleetSupplyableSystemIDs(empire.EmpireID()); },
                                                            py::return_value_policy<py::copy_const_reference>()
                                                        ))
            .add_property("supplyUnobstructedSystems",  make_function(&Empire::SupplyUnobstructedSystems,   py::return_internal_reference<>()))
            .add_property("systemSupplyRanges",         make_function(&Empire::SystemSupplyRanges,          py::return_internal_reference<>()))

            .def("numSitReps",                      &Empire::NumSitRepEntries)
            .def("getSitRep",                       GetSitRep,
                                                    py::return_internal_reference<>())
            .def("obstructedStarlanes",             obstructedStarlanes,
                                                    py::return_value_policy<py::return_by_value>())
            .def("supplyProjections",               jumpsToSuppliedSystem,
                                                    py::return_value_policy<py::return_by_value>())
            .def("getMeter",                        +[](const Empire& empire, const std::string& name) -> const Meter* { return empire.GetMeter(name); },
                                                    py::return_internal_reference<>(),
                                                    "Returns the empire meter with the indicated name (string).")
        ;

        //////////////////////
        // Production Queue //
        //////////////////////
        py::class_<ProductionQueue::Element>("productionQueueElement", py::no_init)
            .add_property("name",                   make_function(
                                                        +[](const ProductionQueue::Element& element) -> const std::string& { return element.item.name; },
                                                        py::return_value_policy<py::copy_const_reference>()
                                                    ))
            .add_property("designID",               make_function(
                                                        +[](const ProductionQueue::Element& element) -> int { return element.item.design_id; },
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))
            .add_property("buildType",              make_function(
                                                        +[](const ProductionQueue::Element& element) -> BuildType { return element.item.build_type; },
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))
            .add_property("locationID",             &ProductionQueue::Element::location)
            .add_property("allocation",             &ProductionQueue::Element::allocated_pp)
            .add_property("progress",               &ProductionQueue::Element::progress)
            .add_property("turnsLeft",              &ProductionQueue::Element::turns_left_to_completion)
            .add_property("remaining",              &ProductionQueue::Element::remaining)
            .add_property("blocksize",              &ProductionQueue::Element::blocksize)
            .add_property("paused",                 &ProductionQueue::Element::paused)
            .add_property("allowedStockpile",       &ProductionQueue::Element::allowed_imperial_stockpile_use)
            ;

        py::class_<ProductionQueue, boost::noncopyable>("productionQueue", py::no_init)
            .def("__iter__",                        py::iterator<ProductionQueue>())  // ProductionQueue provides STL container-like interface to contained queue
            .def("__getitem__",                     +[](const ProductionQueue& queue, int index) -> const ProductionQueue::Element& { return queue[index]; },
                                                    py::return_internal_reference<>())
            .def("__len__",                         &ProductionQueue::size)
            .add_property("size",                   &ProductionQueue::size)
            .add_property("empty",                  &ProductionQueue::empty)
            .add_property("totalSpent",             &ProductionQueue::TotalPPsSpent)
            .add_property("empireID",               &ProductionQueue::EmpireID)
            .def("availablePP",                     &ProductionQueue::AvailablePP,
                                                    py::return_value_policy<py::return_by_value>())
            .add_property("allocatedPP",            make_function(&ProductionQueue::AllocatedPP,        py::return_internal_reference<>()))
            .def("objectsWithWastedPP",             &ProductionQueue::ObjectsWithWastedPP,
                                                    py::return_value_policy<py::return_by_value>())
            ;

        ////////////////////
        // Research Queue //
        ////////////////////
        py::class_<ResearchQueue::Element>("researchQueueElement", py::no_init)
            .def_readonly("tech",                   &ResearchQueue::Element::name)
            .def_readonly("allocation",             &ResearchQueue::Element::allocated_rp)
            .def_readonly("turnsLeft",              &ResearchQueue::Element::turns_left)
        ;

        py::class_<ResearchQueue, boost::noncopyable>("researchQueue", py::no_init)
            .def("__iter__",                        py::iterator<ResearchQueue>())  // ResearchQueue provides STL container-like interface to contained queue
            .def("__getitem__",                     &ResearchQueue::operator[],                         py::return_internal_reference<>())
            .def("__len__",                         &ResearchQueue::size)
            .add_property("size",                   &ResearchQueue::size)
            .add_property("empty",                  &ResearchQueue::empty)
            .def("inQueue",                         &ResearchQueue::InQueue)
            .def("__contains__",                    +[](const ResearchQueue* queue, const ResearchQueue::Element& element) -> bool { return queue->InQueue(element.name); },
                                                    py::return_value_policy<py::return_by_value>())
            .add_property("totalSpent",             &ResearchQueue::TotalRPsSpent)
            .add_property("empireID",               &ResearchQueue::EmpireID)
        ;

        //////////////////
        //     Tech     //
        //////////////////
        py::class_<Tech, boost::noncopyable>("tech", py::no_init)
            .add_property("name",                   make_function(&Tech::Name,              py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",            make_function(&Tech::Description,       py::return_value_policy<py::copy_const_reference>()))
            .add_property("shortDescription",       make_function(&Tech::ShortDescription,  py::return_value_policy<py::copy_const_reference>()))
            .add_property("category",               make_function(&Tech::Category,          py::return_value_policy<py::copy_const_reference>()))
            .def("researchCost",                    &Tech::ResearchCost)
            .def("perTurnCost",                     &Tech::PerTurnCost)
            .def("researchTime",                    &Tech::ResearchTime)
            .add_property("prerequisites",          make_function(&Tech::Prerequisites,     py::return_internal_reference<>()))
            .add_property("unlockedTechs",          make_function(&Tech::UnlockedTechs,     py::return_internal_reference<>()))
            .add_property("unlockedItems",          make_function(&Tech::UnlockedItems,     py::return_internal_reference<>()))
            .def("recursivePrerequisites",          +[](const Tech& tech, int empire_id) -> std::vector<std::string> { return GetTechManager().RecursivePrereqs(tech.Name(), empire_id); },
                                                    py::return_value_policy<py::return_by_value>())
        ;

        def("getTech",                              &GetTech,                               py::return_value_policy<py::reference_existing_object>(), "Returns the tech (Tech) with the indicated name (string).");
        def("getTechCategories",                    &TechManager::CategoryNames,            py::return_value_policy<py::return_by_value>(), "Returns the names of all tech categories (StringVec).");

        py::def("techs",
                +[]() -> std::vector<std::string> { return GetTechManager().TechNames(); },
                py::return_value_policy<py::return_by_value>(),
                "Returns the names of all techs (StringVec).");

        py::def("techsInCategory",
                +[](const std::string& category) -> std::vector<std::string> { return GetTechManager().TechNames(category); },
                py::return_value_policy<py::return_by_value>(),
                "Returns the names of all techs (StringVec) in the indicated"
                " tech category name (string).");

        py::class_<UnlockableItem>("UnlockableItem", py::init<UnlockableItemType, const std::string&>())
            .add_property("type",               &UnlockableItem::type)
            .add_property("name",               &UnlockableItem::name)
        ;

        ///////////////////////
        //  Influence Queue  //
        ///////////////////////
        py::class_<InfluenceQueue::Element>("influenceQueueElement", py::no_init)
            .def_readonly("name",                   &InfluenceQueue::Element::name)
            .def_readonly("allocation",             &InfluenceQueue::Element::allocated_ip)
        ;
        py::class_<InfluenceQueue, boost::noncopyable>("influenceQueue", py::no_init)
            .def("__iter__",                        py::iterator<InfluenceQueue>())  // InfluenceQueue provides STL container-like interface to contained queue
            .def("__getitem__",                     +[](const InfluenceQueue& queue, int index) -> const InfluenceQueue::Element& { return queue[index]; },
                                                    py::return_internal_reference<>())
            .def("__len__",                         &InfluenceQueue::size)
            .add_property("size",                   &InfluenceQueue::size)
            .add_property("empty",                  &InfluenceQueue::empty)
            .def("inQueue",                         &InfluenceQueue::InQueue)
            .def("__contains__",                    +[](const InfluenceQueue* queue, const InfluenceQueue::Element& element) -> bool { return queue->InQueue(element.name); },
                                                    py::return_value_policy<py::return_by_value>())
            .add_property("totalSpent",             &InfluenceQueue::TotalIPsSpent)
            .add_property("empireID",               &InfluenceQueue::EmpireID)

            .add_property("allocatedStockpileIP",   &InfluenceQueue::AllocatedStockpileIP)
            .add_property("expectedNewStockpile",   &InfluenceQueue::ExpectedNewStockpileAmount)
        ;

        //////////////////
        //    Policy    //
        //////////////////
        py::class_<Policy, boost::noncopyable>("policy", py::no_init)
            .add_property("name",                   make_function(&Policy::Name,                py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",            make_function(&Policy::Description,         py::return_value_policy<py::copy_const_reference>()))
            .add_property("shortDescription",       make_function(&Policy::ShortDescription,    py::return_value_policy<py::copy_const_reference>()))
            .add_property("category",               make_function(&Policy::Category,            py::return_value_policy<py::copy_const_reference>()))
            .def("adoptionCost",                    &Policy::AdoptionCost)
        ;

        def("getPolicy",                            &GetPolicy,                                 py::return_value_policy<py::reference_existing_object>(), "Returns the policy (Policy) with the indicated name (string).");
        def("getPolicyCategories",                  &PolicyManager::PolicyCategories,           py::return_value_policy<py::return_by_value>(), "Returns the names of all policy categories (StringVec).");

        py::def("policies",
                +[]() -> std::vector<std::string> { return GetPolicyManager().PolicyNames(); },
               py::return_value_policy<py::return_by_value>(),
               "Returns the names of all policies (StringVec).");

        py::def("policiesInCategory",
                +[](const std::string& category) -> std::vector<std::string> { return GetPolicyManager().PolicyNames(category); },
                py::return_value_policy<py::return_by_value>(),
                "Returns the names of all policies (StringVec) in the"
                " indicated policy category name (string).");

        ///////////////////
        //  SitRepEntry  //
        ///////////////////
        py::class_<SitRepEntry, boost::noncopyable>("sitrep", py::no_init)
            .add_property("typeString",         make_function(&SitRepEntry::GetTemplateString,  py::return_value_policy<py::copy_const_reference>()))
            .def("getDataString",               &SitRepEntry::GetDataString,
                                                py::return_value_policy<py::copy_const_reference>())
            .def("getDataIDNumber",             &SitRepEntry::GetDataIDNumber)
            .add_property("getTags",            make_function(&SitRepEntry::GetVariableTags,    py::return_value_policy<py::return_by_value>()))
            .add_property("getTurn",            &SitRepEntry::GetTurn)
        ;

        ///////////////////////
        // DiplomaticMessage //
        ///////////////////////
        py::class_<DiplomaticMessage>("diplomaticMessage")
            .def(py::init<int, int, DiplomaticMessage::Type>())
            .add_property("type",      &DiplomaticMessage::GetType)
            .add_property("recipient", &DiplomaticMessage::RecipientEmpireID)
            .add_property("sender",    &DiplomaticMessage::SenderEmpireID)
        ;

        ////////////////////////////
        // DiplomaticStatusUpdate //
        ////////////////////////////
        py::class_<DiplomaticStatusUpdateInfo>("diplomaticStatusUpdate")
            .def(py::init<int, int, DiplomaticStatus>())
            .add_property("status",             &DiplomaticStatusUpdateInfo::diplo_status)
            .add_property("empire1",            &DiplomaticStatusUpdateInfo::empire1_id)
            .add_property("empire2",            &DiplomaticStatusUpdateInfo::empire2_id)
        ;
    }
}
