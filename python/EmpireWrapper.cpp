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

    constexpr auto to_int_value_pair = [](const std::pair<UniverseObjectID, UniverseObjectID> in) noexcept -> std::pair<int, int>
    { return {Value(in.first), Value(in.second)}; };

    auto obstructedStarlanes(const Empire& empire) -> std::vector<std::pair<int, int>>
    {
        const auto& laneset = IApp::GetApp()->GetSupplyManager().SupplyObstructedStarlaneTraversals(empire.GetEmpireID());
        static_assert(!std::is_same_v<std::decay_t<decltype(laneset)>, std::vector<std::pair<int, int>>>); // if are the same, don't need to explicitly construct the return value...
        try {
            return laneset | range_transform(to_int_value_pair) | range_to_vec;
        } catch (...) {
            return {};
        }
    }

    auto jumpsToSuppliedSystem(const Empire& empire) -> std::map<int, int>
    {
        const ScriptingContext& context = IApp::GetApp()->GetContext();

        std::map<int, int> retval;
        const auto empire_starlanes = empire.KnownStarlanes(context.ContextUniverse());
        std::deque<UniverseObjectID> propagating_list;

        for (auto system_id : context.supply.FleetSupplyableSystemIDs(empire.GetEmpireID(), true, context)) {
            retval[Value(system_id)] = 0;
            propagating_list.push_back(system_id);
        }

        // get lanes starting in system with id system_id
        static constexpr auto lane_starts_less = [](const auto lane1, const auto lane2) { return lane1.start < lane2.start; };
        static constexpr auto to_lane_end = [](const auto lane) { return lane.end; };


        // iteratively propagate supply out from supplied systems, to determine
        // how many jumps away from supply each unsupplied system is...
        while (!propagating_list.empty()) {
            // get next system and distance from the list
            const auto from_sys_id = propagating_list.front();
            propagating_list.pop_front();
            const auto from_sys_dist = retval[Value(from_sys_id)];

            // get lanes originating in this system
            const Empire::LaneEndpoints system_lane{from_sys_id, from_sys_id};
            const auto system_lanes_rng = range_equal(empire_starlanes, system_lane, lane_starts_less);

            // propagate to any not-already-counted adjacent system
            for (const auto lane_end_system_id : system_lanes_rng | range_transform(to_lane_end)) {
                if (retval.contains(Value(lane_end_system_id)))
                    continue; // system already processed
                // system not yet processed; add it to list to propagate from, and set its range to one more than this system
                propagating_list.push_back(lane_end_system_id);
                retval.emplace(Value(lane_end_system_id), from_sys_dist - 1); // negative values used to indicate jumps to nearest supply for historical compatibility reasons
            }
        }

        //// DEBUG
        //DebugLogger() << "jumpsToSuppliedSystem results for empire, " << empire.Name() << " (" << empire.GetEmpireID() << ") :";
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
                    planet_ids.insert(Value(planet->ID()));
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
                    planets.insert(Value(planet->ID()));
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
                    planet_ids.insert(Value(planet->ID()));
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
    { return in | range_transform([](auto& sv) { return std::string{sv}; }) | range_to_vec; }

    constexpr auto to_str_int = [](auto view_int) { return std::pair{std::string{view_int.first}, view_int.second}; };

    template <typename C>
    auto ViewMapToStringMap(const std::map<std::string_view, int, C>& in) -> std::map<std::string, int>
    { return in | range_transform(to_str_int) | range_to_map; }

    auto ViewVecToStringMap(const std::vector<std::pair<std::string_view, int>>& in) -> std::map<std::string, int>
    { return in | range_transform(to_str_int) | range_to_map; }

    constexpr auto to_int_value = [](auto in) noexcept -> int { return Value(in); };

    constexpr auto to_int_set_float = [](auto&& in) -> std::pair<std::set<int>, float>
    { return {in.first | range_transform(to_int_value) | range_to_set, in.second}; };

    auto MapFlatSetFloatToMapSetFloat(const auto& in) -> std::map<std::set<int>, float>
    { return in | range_transform(to_int_set_float) | range_to_map; }

    constexpr auto to_int_float = [](auto&& in) -> std::pair<int, float>
    { return {to_int_value(in.first), in.second}; };

    auto MapIDFloatToMapIntFloat(const auto& in) -> std::map<int, float>
    { return in | range_transform(to_int_float) | range_to_map; }

    template <typename T, typename AoC>
    std::vector<T> ToVec(const boost::container::flat_set<T, AoC>& in)
    { return in | range_to_vec; }

    auto ToIntVec(const auto& in)
    { return in | range_transform(to_int_value) | range_to_vec; }
        
    auto ToIntSet(const auto& in)
    { return in | range_transform(to_int_value) | range_to_set; }

    constexpr auto to_int_set = [](auto in) -> std::set<int> { return ToIntSet(in); };

    auto ToIntSetSet(const std::set<std::set<UniverseObjectID>>& in)
    { return in | range_transform(to_int_set) | range_to_set; }

    auto ToStringSet(auto&& in) {
        std::set<std::string> retval;
        for (auto& s : in) {
            if constexpr (std::is_lvalue_reference_v<decltype(in)>)
                retval.insert(s);
            else
                retval.insert(std::move(s));
        }
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

            .add_property("empireID",               +[](const Empire& empire) noexcept -> int { return Value(empire.GetEmpireID()); })
            .add_property("capitalID",              +[](const Empire& empire) noexcept -> int { return Value(empire.CapitalID()); })

            .add_property("colour",                 +[](const Empire& empire) { EmpireColor color = empire.Color(); return py::make_tuple(std::get<0>(color), std::get<1>(color), std::get<2>(color), std::get<3>(color)); })

            .def("buildingTypeAvailable",           &Empire::BuildingTypeAvailable)
            .add_property("availableBuildingTypes", +[](const Empire& empire) { return ToVec(empire.AvailableBuildingTypes()); })

            .add_property("totalShipsOwned",        make_function(&Empire::TotalShipsOwned,         py::return_value_policy<py::return_by_value>()))
            .def("shipDesignAvailable",             +[](const Empire& empire, int id) -> bool { return empire.ShipDesignAvailable(id, IApp::GetApp()->GetContext().ContextUniverse()); })
            .add_property("allShipDesigns",         make_function(&Empire::ShipDesigns,             py::return_value_policy<py::return_by_value>()))
            .add_property("availableShipDesigns",   +[](const Empire& empire) -> std::set<int> { auto temp{empire.AvailableShipDesigns(IApp::GetApp()->GetContext().ContextUniverse())}; return {temp.begin(), temp.end()}; })


            .add_property("availableShipParts",     +[](const Empire& empire) -> std::vector<std::string> { return ToVec(empire.AvailableShipParts()); })
            .add_property("availableShipHulls",     +[](const Empire& empire) -> std::vector<std::string> { return ToVec(empire.AvailableShipHulls()); })

            .add_property("productionQueue",        make_function(&Empire::GetProductionQueue,      py::return_internal_reference<>()))
            .def("productionCostAndTime",           +[](const Empire& empire, const ProductionQueue::Element& element) -> std::pair<float, int> { return element.ProductionCostAndTime(IApp::GetApp()->GetContext()); })
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
            .def("researchProgress",                +[](const Empire& e, const std::string& tech) -> float { return e.ResearchProgress(tech, IApp::GetApp()->GetContext()); })
            .add_property("researchQueue",          make_function(&Empire::GetResearchQueue,        py::return_internal_reference<>()))

            .def("policyAdopted",                   +[](const Empire& e, const std::string& policy) { return e.PolicyAdopted(policy); })
            .def("turnPolicyAdopted",               +[](const Empire& e, const std::string& policy) { return e.TurnPolicyAdopted(policy); })
            .def("slotPolicyAdoptedIn",             +[](const Empire& e, const std::string& policy) { return e.SlotPolicyAdoptedIn(policy); })

            .add_property("adoptedPolicies",        +[](const Empire& e) -> std::vector<std::string> { return ViewVecToStringVec(e.AdoptedPolicies()); })
            .add_property("categoriesSlotPolicies", +[](const Empire& e) -> std::map<std::string, std::map<int, std::string>> {
                                                            std::map<std::string, std::map<int, std::string>> retval;
                                                            for (auto& [cat, slots_policies] : e.CategoriesSlotsPoliciesAdopted())
                                                                for (auto& [slot, policy] : slots_policies)
                                                                    retval[std::string{cat}].emplace(slot, policy);
                                                            return retval;
                                                    })
            .add_property("turnsPoliciesAdopted",   +[](const Empire& e) -> std::map<std::string, int> { return ViewMapToStringMap(e.TurnsPoliciesAdopted()); })

            .add_property("availablePolicies",      +[](const Empire& e) -> std::set<std::string> { return ToStringSet(e.AvailablePolicies()); })

            .def("policyAvailable",                 +[](const Empire& e, const std::string& policy) -> bool { return e.PolicyAvailable(policy); })

            .def("policyPrereqsAndExclusionsOK",    +[](const Empire& e, const std::string& policy) -> bool { return e.PolicyPrereqsAndExclusionsOK(policy, IApp::GetApp()->CurrentTurn()); })

            .add_property("totalPolicySlots",       +[](const Empire& e) -> std::map<std::string, int> { return ViewVecToStringMap(e.TotalPolicySlots()); })
            .add_property("emptyPolicySlots",       +[](const Empire& e) -> std::map<std::string, int> { return ViewVecToStringMap(e.EmptyPolicySlots()); })

            .def("canBuild",                        +[](const Empire& empire, BuildType build_type, const std::string& name, int location) -> bool { return empire.ProducibleItem(build_type, name, UniverseObjectID{location}, IApp::GetApp()->GetContext()); })
            .def("canBuild",                        +[](const Empire& empire, BuildType build_type, int design, int location) -> bool { return empire.ProducibleItem(build_type, design, UniverseObjectID{location}, IApp::GetApp()->GetContext()); })

            .def("hasExploredSystem",               +[](const Empire& empire, int sys_id) -> bool { return empire.HasExploredSystem(UniverseObjectID{sys_id}); })
            .add_property("exploredSystemIDs",      +[](const Empire& empire) -> std::vector<int> { return ToIntVec(empire.ExploredSystems()); })

            .add_property("eliminated",             &Empire::Eliminated)
            .add_property("won",                    &Empire::Won)

            .add_property("productionPoints",       &Empire::ProductionPoints)
            .def("resourceStockpile",               &Empire::ResourceStockpile)
            .def("resourceProduction",              &Empire::ResourceOutput)
            .def("resourceAvailable",               &Empire::ResourceAvailable)

            .def("population",                      &Empire::Population)

            .def("preservedLaneTravel",             +[](const Empire& empire, int sys_id_1, int sys_id_2) -> bool { return empire.PreservedLaneTravel(UniverseObjectID{sys_id_1}, UniverseObjectID{sys_id_2}); })

            .add_property("fleetSupplyableSystemIDs",   +[](const Empire& empire) -> std::set<int> { return ToIntSet(IApp::GetApp()->GetSupplyManager().FleetSupplyableSystemIDs(empire.GetEmpireID())); })
            .add_property("supplyUnobstructedSystems",  +[](const Empire& empire) -> std::set<int> { return ToIntSet(empire.SupplyUnobstructedSystems()); })
            .add_property("systemSupplyRanges",         +[](const Empire& empire) -> std::map<int, float> { return MapIDFloatToMapIntFloat(empire.SystemSupplyRanges()); })
            .add_property("resourceSupplyGroups",       +[](const Empire& empire) -> std::set<std::set<int>> { return ToIntSetSet(IApp::GetApp()->GetSupplyManager().ResourceSupplyGroups(empire.GetEmpireID())); })

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
            .add_property("designID",               +[](const ProductionQueue::Element& element) -> int { return element.item.design_id; })
            .add_property("buildType",              +[](const ProductionQueue::Element& element) -> BuildType { return element.item.build_type; })
            .add_property("locationID",             +[](const ProductionQueue::Element& element) noexcept -> int { return Value(element.location); })
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
            .add_property("empireID",               +[](const ProductionQueue& q) noexcept -> int { return Value(q.GetEmpireID()); })

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
            .def("__contains__",                    +[](const ResearchQueue& q, const ResearchQueue::Element& e) -> bool { return q.InQueue(e.name); },
                                                    py::return_value_policy<py::return_by_value>())
            .add_property("totalSpent",             &ResearchQueue::TotalRPsSpent)
            .add_property("empireID",               +[](const ResearchQueue& q) -> int { return Value(q.GetEmpireID()); })
        ;

        //////////////////
        //     Tech     //
        //////////////////
        py::class_<Tech, boost::noncopyable>("tech", py::no_init)
            .add_property("name",                   make_function(&Tech::Name,              py::return_value_policy<py::copy_const_reference>()))
            .add_property("description",            make_function(&Tech::Description,       py::return_value_policy<py::copy_const_reference>()))
            .add_property("shortDescription",       make_function(&Tech::ShortDescription,  py::return_value_policy<py::copy_const_reference>()))
            .add_property("category",               make_function(&Tech::Category,          py::return_value_policy<py::copy_const_reference>()))
            .def("researchCost",                    +[](const Tech& t, int empire_id) { return t.ResearchCost(EmpireID{empire_id}, IApp::GetApp()->GetContext()); })
            .def("perTurnCost",                     +[](const Tech& t, int empire_id) { return t.PerTurnCost(EmpireID{empire_id}, IApp::GetApp()->GetContext()); })
            .def("researchTime",                    +[](const Tech& t, int empire_id) { return t.ResearchTime(EmpireID{empire_id}, IApp::GetApp()->GetContext()); })
            .add_property("prerequisites",          make_function(&Tech::Prerequisites,     py::return_internal_reference<>()))
            .add_property("unlockedTechs",          make_function(&Tech::UnlockedTechs,     py::return_internal_reference<>()))
            .add_property("unlockedItems",          make_function(&Tech::UnlockedItems,     py::return_internal_reference<>()))
            .def("recursivePrerequisites",          +[](const Tech& tech, int empire_id) -> std::vector<std::string> { return GetTechManager().RecursivePrereqs(tech.Name(), EmpireID{empire_id}, IApp::GetApp()->GetContext()); },
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
            .def("adoptionCost",                    +[](const Policy& p)                       { return p.AdoptionCost(IApp::GetApp()->GetEmpireID(), IApp::GetApp()->GetContext()); })
            .def("adoptionCost",                    +[](const Policy& p, const Empire& empire) { return p.AdoptionCost(empire.GetEmpireID(), IApp::GetApp()->GetContext()); })
            .def("adoptionCost",                    +[](const Policy& p, int empire_id)        { return p.AdoptionCost(EmpireID{empire_id}, IApp::GetApp()->GetContext()); })
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
            +[]() -> std::vector<std::string> { return GetPolicyManager().PolicyNamesCopies(); },
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
            .def("__init__", py::make_constructor(+[](int sender, int recipient, DiplomaticMessage::Type dmt) { return new DiplomaticMessage{EmpireID{sender}, EmpireID{recipient}, dmt}; }))
            .add_property("type",      &DiplomaticMessage::GetType)
            .add_property("recipient", +[](const DiplomaticMessage& dm) noexcept -> int { return Value(dm.RecipientEmpireID()); })
            .add_property("sender",    +[](const DiplomaticMessage& dm) noexcept -> int { return Value(dm.SenderEmpireID()); })
        ;

        ////////////////////////////
        // DiplomaticStatusUpdate //
        ////////////////////////////
        py::class_<DiplomaticStatusUpdateInfo>("diplomaticStatusUpdate")
            .def("__init__", py::make_constructor(+[](int empire1, int empire2, DiplomaticStatus status) { return new DiplomaticStatusUpdateInfo{EmpireID{empire1}, EmpireID{empire2}, status}; }))
            .add_property("status", &DiplomaticStatusUpdateInfo::diplo_status)
            .add_property("empire1", +[](const DiplomaticStatusUpdateInfo& dsui) noexcept { return Value(dsui.empire1_id); })
            .add_property("empire2", +[](const DiplomaticStatusUpdateInfo& dsui) noexcept { return Value(dsui.empire2_id); })
        ;
    }
}
