#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"
#include "../Empire/Diplomacy.h"
#include "../Empire/Government.h"
#include "../Empire/InfluenceQueue.h"
#include "../universe/UniverseObject.h"
#include "../universe/UnlockableItem.h"
#include "../universe/Planet.h"
#include "../universe/ScriptingContext.h"
#include "../universe/Tech.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
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

    auto obstructedStarlanes(const Empire& empire) -> std::vector<std::pair<int, int>>
    {
        const auto& laneset = IApp::GetApp()->GetSupplyManager().SupplyObstructedStarlaneTraversals(empire.EmpireID());
        static_assert(!std::is_same_v<std::decay_t<decltype(laneset)>, std::vector<std::pair<int, int>>>); // if are the same, don't need to explicitly construct the return value...
        try {
            return {laneset.begin(), laneset.end()};
        } catch (...) {
            return {};
        }
    }

    auto jumpsToSuppliedSystem(const Empire& empire) -> std::map<int, int>
    {
        const ScriptingContext& context = IApp::GetApp()->GetContext();

        std::map<int, int> retval;
        const auto empire_starlanes = empire.KnownStarlanes(context.ContextUniverse());
        std::deque<int> propagating_list;

        for (int system_id : context.supply.FleetSupplyableSystemIDs(empire.EmpireID(), true, context)) {
            retval[system_id] = 0;
            propagating_list.push_back(system_id);
        }

        // get lanes starting in system with id system_id
        static constexpr auto lane_starts_less = [](const auto lane1, const auto lane2) { return lane1.start < lane2.start; };
        static constexpr auto to_lane_end = [](const auto lane) { return lane.end; };


        // iteratively propagate supply out from supplied systems, to determine
        // how many jumps away from supply each unsupplied system is...
        while (!propagating_list.empty()) {
            // get next system and distance from the list
            const int from_sys_id = propagating_list.front();
            propagating_list.pop_front();
            const int from_sys_dist = retval[from_sys_id];

            // get lanes originating in this system
            const Empire::LaneEndpoints system_lane{from_sys_id, from_sys_id};
            const auto system_lanes_rng = range_equal(empire_starlanes, system_lane, lane_starts_less);

            // propagate to any not-already-counted adjacent system
            for (const int lane_end_system_id : system_lanes_rng | range_transform(to_lane_end)) {
                if (retval.contains(lane_end_system_id))
                    continue; // system already processed
                // system not yet processed; add it to list to propagate from, and set its range to one more than this system
                propagating_list.push_back(lane_end_system_id);
                retval.emplace(lane_end_system_id, from_sys_dist - 1); // negative values used to indicate jumps to nearest supply for historical compatibility reasons
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
        std::map<std::set<int>, float> planets_with_available_pp;
        const auto& objects = IApp::GetApp()->GetContext().ContextObjects();

        // filter industry pool output to get just planet IDs
        for (auto& [object_ids, PP] : empire.GetIndustryPool().Output()) {
            std::set<int> planet_ids;
            for (const auto* planet : objects.findRaw<Planet>(object_ids)) {
                if (planet)
                    planet_ids.insert(planet->ID());
            }
            if (!planet_ids.empty())
                planets_with_available_pp.emplace(std::move(planet_ids), PP);
        }
        return planets_with_available_pp;
    }

    auto PlanetsWithAllocatedPP(const Empire& empire) -> std::map<std::set<int>, float>
    {
        const auto& objects = IApp::GetApp()->GetContext().ContextObjects();
        const auto& prod_queue = empire.GetProductionQueue();
        std::map<std::set<int>, float> planets_with_allocated_pp;
        for (const auto& objects_pp : prod_queue.AllocatedPP()) {
            std::set<int> planets;
            for (const auto* planet : objects.findRaw<Planet>(objects_pp.first)) {
                if (planet)
                    planets.insert(planet->ID());
            }
            if (!planets.empty())
                planets_with_allocated_pp[planets] = objects_pp.second;
        }
        return planets_with_allocated_pp;
    }

    auto PlanetsWithWastedPP(const Empire& empire) -> std::set<std::set<int>>
    {
        const auto& objects = IApp::GetApp()->GetContext().ContextObjects();
        const ProductionQueue& prod_queue = empire.GetProductionQueue();
        std::set<std::set<int>> planets_with_wasted_pp;
        for (const auto& object_ids : prod_queue.ObjectsWithWastedPP(empire.GetIndustryPool())) {
            std::set<int> planet_ids;
            for (const auto* planet : objects.findRaw<Planet>(object_ids)) {
                if (planet)
                    planet_ids.insert(planet->ID());
            }
            if (!planet_ids.empty())
                planets_with_wasted_pp.insert(std::move(planet_ids));
        }
        return planets_with_wasted_pp;
    }

    auto ResearchedTechNames(const Empire& empire) -> std::set<std::string>
    {
        std::set<std::string> out;
        const auto& rt = empire.ResearchedTechs();
        std::transform(rt.begin(), rt.end(), std::inserter(out, out.end()),
                       [](const auto& t) { return t.first; });
        return out;
    }

    auto ViewVecToStringVec(const std::vector<std::string_view>& in) -> std::vector<std::string>
    {
        std::vector<std::string> out;
        out.reserve(in.size());
        std::transform(in.begin(), in.end(), std::back_inserter(out),
                       [](auto view) { return std::string{view}; });
        return out;
    }

    template <typename C>
    auto ViewMapToStringMap(const std::map<std::string_view, int, C>& in) -> std::map<std::string, int>
    {
        std::map<std::string, int> out;
        std::transform(in.begin(), in.end(), std::inserter(out, out.end()),
                       [](auto view_int) { return std::pair{std::string{view_int.first}, view_int.second}; });
        return out;
    }
    auto ViewVecToStringMap(const std::vector<std::pair<std::string_view, int>>& in) -> std::map<std::string, int>
    {
        std::map<std::string, int> out;
        std::transform(in.begin(), in.end(), std::inserter(out, out.end()),
                       [](auto view_int) { return std::pair{std::string{view_int.first}, view_int.second}; });
        return out;
    }

    auto MapFlatSetFloatToMapSetFloat(const auto& in) -> std::map<std::set<int>, float>
    {
        std::map<std::set<int>, float> out;
        std::transform(in.begin(), in.end(), std::inserter(out, out.end()),
                       [](auto set_float)
                       { return std::pair{std::set<int>{set_float.first.begin(), set_float.first.end()},
                                          set_float.second}; });
        return out;
    }

    template <typename T, typename AoC>
    std::vector<T> ToVec(const boost::container::flat_set<T, AoC>& in)
    { return std::vector<T>(in.begin(), in.end()); }
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
        py::to_python_converter<std::pair<int, int>, PairToTupleConverter<int, int>>();
        py::to_python_converter<std::pair<float, int>, PairToTupleConverter<float, int>>();

        py::class_<std::map<std::pair<int, int>, int>>("IntIntPairIntMap")
            .def(py::map_indexing_suite<std::map<std::pair<int, int>, int>, true>())
        ;

        py::class_<std::vector<std::pair<int, int>>>("IntIntPairVec")
            .def(py::vector_indexing_suite<std::vector<std::pair<int, int>>, true>())
        ;

        py::class_<std::vector<UnlockableItem>>("UnlockableItemVec")
            .def(py::vector_indexing_suite<std::vector<UnlockableItem>, true>())
        ;

        ::FreeOrionPython::SetWrapper<std::set<std::set<int>>>::Wrap("IntSetSet");
        ::FreeOrionPython::SetWrapper<std::set<int>>::Wrap("IntSet");
        ::FreeOrionPython::SetWrapper<std::set<std::string>>::Wrap("StringSet");

        py::class_<std::map<std::string, int>>("StringIntMap")
            .def(py::map_indexing_suite<std::map<std::string, int>, true>())
        ;

        py::class_<std::map<int, std::string>>("IntStringMap")
            .def(py::map_indexing_suite<std::map<int, std::string>, true>())
        ;

        py::class_<std::map<std::string, std::map<int, std::string>>>("StringIntStringMapMap")
            .def(py::map_indexing_suite<std::map<std::string, std::map<int, std::string>>, true>())
        ;

        py::class_<std::map<int, float>>("IntFloatMap")
            .def(py::map_indexing_suite<std::map<int, float>, true>())
        ;

        py::class_<std::map<int, float>>("IntIntMap")
            .def(py::map_indexing_suite<std::map<int, float>, true>())
        ;

        py::class_<std::map<std::set<int>, float>>("IntSetFloatMap")
            .def(py::map_indexing_suite<std::map<std::set<int>, float>, true>())
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
            .add_property("availableBuildingTypes", +[](const Empire& empire) { return ToVec(empire.AvailableBuildingTypes()); })

            .add_property("totalShipsOwned",        make_function(&Empire::TotalShipsOwned,         py::return_value_policy<py::return_by_value>()))
            .def("shipDesignAvailable",             +[](const Empire& empire, int id) -> bool { return empire.ShipDesignAvailable(id, IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("allShipDesigns",         make_function(&Empire::ShipDesigns,             py::return_value_policy<py::return_by_value>()))
            .add_property("availableShipDesigns",   +[](const Empire& empire) -> std::set<int> { auto temp{empire.AvailableShipDesigns(IApp::GetApp()->GetContext().ContextUniverse())}; return {temp.begin(), temp.end()}; })


            .add_property("availableShipParts",     +[](const Empire& empire) { return ToVec(empire.AvailableShipParts()); })
            .add_property("availableShipHulls",     +[](const Empire& empire) { return ToVec(empire.AvailableShipHulls()); })

            .add_property("productionQueue",        make_function(&Empire::GetProductionQueue,      py::return_internal_reference<>()))
            .def("productionCostAndTime",           +[](const Empire& empire, const ProductionQueue::Element& element) -> std::pair<float, int> { return element.ProductionCostAndTime(IApp::GetApp()->GetContext()); },
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
            .def("researchProgress",                +[](const Empire& e, const std::string& tech) { return e.ResearchProgress(tech, IApp::GetApp()->GetContext()); })
            .add_property("researchQueue",          make_function(&Empire::GetResearchQueue,        py::return_internal_reference<>()))

            .def("policyAdopted",                   +[](const Empire& e, const std::string& policy) { return e.PolicyAdopted(policy); })
            .def("turnPolicyAdopted",               +[](const Empire& e, const std::string& policy) { return e.TurnPolicyAdopted(policy); })
            .def("slotPolicyAdoptedIn",             +[](const Empire& e, const std::string& policy) { return e.SlotPolicyAdoptedIn(policy); })

            .add_property("adoptedPolicies",        make_function(
                                                        +[](const Empire& e)
                                                        { return ViewVecToStringVec(e.AdoptedPolicies()); },
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))
            .add_property("categoriesSlotPolicies", make_function(
                                                        +[](const Empire& e) -> std::map<std::string, std::map<int, std::string>> {
                                                            std::map<std::string, std::map<int, std::string>> retval;
                                                            for (auto& [cat, slots_policies] : e.CategoriesSlotsPoliciesAdopted())
                                                                for (auto& [slot, policy] : slots_policies)
                                                                    retval[std::string{cat}].emplace(slot, policy);
                                                            return retval;
                                                        },
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))
            .add_property("turnsPoliciesAdopted",   make_function(
                                                        +[](const Empire& e)
                                                        { return ViewMapToStringMap(e.TurnsPoliciesAdopted()); },
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))

            .add_property("availablePolicies",      make_function(
                                                        +[](const Empire& e)
                                                        {
                                                            const auto& ap = e.AvailablePolicies();
                                                            return std::set<std::string>(ap.begin(), ap.end());
                                                        },
                                                        py::return_value_policy<py::return_by_value>()))

            .def("policyAvailable",                 +[](const Empire& e, const std::string& policy) { return e.PolicyAvailable(policy); })

            .def("policyPrereqsAndExclusionsOK",    +[](const Empire& e, const std::string& policy) { return e.PolicyPrereqsAndExclusionsOK(policy, IApp::GetApp()->CurrentTurn()); })

            .add_property("totalPolicySlots",       make_function(
                                                        +[](const Empire& e) -> std::map<std::string, int>
                                                        { return ViewVecToStringMap(e.TotalPolicySlots()); },
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))
            .add_property("emptyPolicySlots",       make_function(
                                                        +[](const Empire& e) -> std::map<std::string, int>
                                                        { return ViewVecToStringMap(e.EmptyPolicySlots()); },
                                                        py::return_value_policy<py::return_by_value>()
                                                    ))

            .def("canBuild",                        +[](const Empire& empire, BuildType build_type, const std::string& name, int location) -> bool { return empire.ProducibleItem(build_type, name, location, IApp::GetApp()->GetContext()); })
            .def("canBuild",                        +[](const Empire& empire, BuildType build_type, int design, int location) -> bool { return empire.ProducibleItem(build_type, design, location, IApp::GetApp()->GetContext()); })

            .def("hasExploredSystem",               &Empire::HasExploredSystem)
            .add_property("exploredSystemIDs",      +[](const Empire& empire) { return ToVec(empire.ExploredSystems()); })

            .add_property("eliminated",             &Empire::Eliminated)
            .add_property("won",                    &Empire::Won)

            .add_property("productionPoints",       make_function(&Empire::ProductionPoints,        py::return_value_policy<py::return_by_value>()))
            .def("resourceStockpile",               &Empire::ResourceStockpile)
            .def("resourceProduction",              &Empire::ResourceOutput)
            .def("resourceAvailable",               &Empire::ResourceAvailable)

            .def("population",                      &Empire::Population)

            .def("preservedLaneTravel",             &Empire::PreservedLaneTravel)
            .add_property("fleetSupplyableSystemIDs",   make_function(
                                                            +[](const Empire& empire) -> const std::set<int>& { return IApp::GetApp()->GetSupplyManager().FleetSupplyableSystemIDs(empire.EmpireID()); },
                                                            py::return_value_policy<py::copy_const_reference>()
                                                        ))
            .add_property("supplyUnobstructedSystems",  make_function(&Empire::SupplyUnobstructedSystems,   py::return_internal_reference<>()))
            .add_property("systemSupplyRanges",         make_function(&Empire::SystemSupplyRanges,          py::return_internal_reference<>()))
            .add_property("resourceSupplyGroups",       make_function(
                                                             +[](const Empire& empire) -> const std::set<std::set<int>>& { return IApp::GetApp()->GetSupplyManager().ResourceSupplyGroups(empire.EmpireID()); },
                                                             py::return_value_policy<py::copy_const_reference>()
                                                        ))

            .def("obstructedStarlanes",             obstructedStarlanes,
                                                    py::return_value_policy<py::return_by_value>())
            .def("supplyProjections",               jumpsToSuppliedSystem,
                                                    py::return_value_policy<py::return_by_value>(),
                                                    "Returns the (negative) number of jumps (int) away each known system ID (int) is from this empire's supply network. 0 in dicates systems that are fleet supplied. -1 indicates a system that is 1 jump away from a supplied system. -4 indicates a system that is 4 jumps from a supply connection.")
            .def("getMeter",                        +[](const Empire& empire, const std::string& name) -> const Meter* { return empire.GetMeter(name); },
                                                    py::return_internal_reference<>(),
                                                    "Returns the empire meter with the indicated name (string).")
            .add_property("lastTurnReceived",       &Empire::LastTurnReceived);
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
            .add_property("removed",                &ProductionQueue::Element::to_be_removed)
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

            .add_property("allocatedPP",            +[](const ProductionQueue& p) -> std::map<std::set<int>, float> { return MapFlatSetFloatToMapSetFloat(p.AllocatedPP()); })
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
            .def("researchCost",                    +[](const Tech& t, int empire_id) { return t.ResearchCost(empire_id, IApp::GetApp()->GetContext()); })
            .def("perTurnCost",                     +[](const Tech& t, int empire_id) { return t.PerTurnCost(empire_id, IApp::GetApp()->GetContext()); })
            .def("researchTime",                    +[](const Tech& t, int empire_id) { return t.ResearchTime(empire_id, IApp::GetApp()->GetContext()); })
            .add_property("prerequisites",          make_function(&Tech::Prerequisites,     py::return_internal_reference<>()))
            .add_property("unlockedTechs",          make_function(&Tech::UnlockedTechs,     py::return_internal_reference<>()))
            .add_property("unlockedItems",          make_function(&Tech::UnlockedItems,     py::return_internal_reference<>()))
            .def("recursivePrerequisites",          +[](const Tech& tech, int empire_id) -> std::vector<std::string> { return GetTechManager().RecursivePrereqs(tech.Name(), empire_id, IApp::GetApp()->GetContext()); },
                                                    py::return_value_policy<py::return_by_value>())
        ;

        def("getTech",                              +[](const std::string& name) -> const Tech* { return GetTech(name); },
                                                    py::return_value_policy<py::reference_existing_object>(),
                                                    "Returns the tech (Tech) with the indicated name (string).");

        def("getTechCategories",
            +[]() -> std::vector<std::string> { return ViewVecToStringVec(GetTechManager().CategoryNames()); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the names of all tech categories (StringVec).");

        def("techs",
            +[]() -> std::vector<std::string> { return ViewVecToStringVec(GetTechManager().TechNames()); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the names of all techs (StringVec).");

        def("techsInCategory",
            +[](const std::string& category) -> std::vector<std::string> { return ViewVecToStringVec(GetTechManager().TechNames(category)); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the names of all techs (StringVec) in the indicated tech category name (string).");

        py::class_<UnlockableItem>("UnlockableItem", py::init<UnlockableItemType, const std::string&>())
            .add_property("type",               &UnlockableItem::type)
            .add_property("name",               &UnlockableItem::name)
        ;

        //////////////////
        //    Policy    //
        //////////////////
        py::class_<Policy, boost::noncopyable>("policy", py::no_init)
            .add_property("name",                   make_function(&Policy::Name,                py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",            make_function(&Policy::Description,         py::return_value_policy<py::copy_const_reference>()))
            .add_property("shortDescription",       make_function(&Policy::ShortDescription,    py::return_value_policy<py::copy_const_reference>()))
            .add_property("category",               make_function(&Policy::Category,            py::return_value_policy<py::copy_const_reference>()))
            .def("adoptionCost",                    +[](const Policy& p)                       { return p.AdoptionCost(IApp::GetApp()->EmpireID(), IApp::GetApp()->GetContext()); })
            .def("adoptionCost",                    +[](const Policy& p, const Empire& empire) { return p.AdoptionCost(empire.EmpireID(), IApp::GetApp()->GetContext()); })
            .def("adoptionCost",                    +[](const Policy& p, int empire_id)        { return p.AdoptionCost(empire_id, IApp::GetApp()->GetContext()); })
        ;

        def("getPolicy",
            +[](const std::string& name) { return GetPolicy(name); },
            py::return_value_policy<py::reference_existing_object>(),
            "Returns the policy (Policy) with the indicated name (string).");

        def("policyCategories",
            +[]() -> std::vector<std::string> { return ViewVecToStringVec(GetPolicyManager().PolicyCategories()); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the names of all policy categories (StringVec).");

        def("policies",
            +[]() -> std::vector<std::string> { return ViewVecToStringVec(GetPolicyManager().PolicyNames()); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the names of all policies (StringVec).");

        def("policiesInCategory",
            +[](const std::string& category) -> std::vector<std::string> { return ViewVecToStringVec(GetPolicyManager().PolicyNames(category)); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the names of all policies (StringVec) in the"
            " indicated policy category name (string).");

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
            .add_property("status",  &DiplomaticStatusUpdateInfo::diplo_status)
            .add_property("empire1", &DiplomaticStatusUpdateInfo::empire1_id)
            .add_property("empire2", &DiplomaticStatusUpdateInfo::empire2_id)
        ;
    }
}
