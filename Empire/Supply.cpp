#include "Supply.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/connected_components.hpp>
#include "Empire.h"
#include "EmpireManager.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"


namespace {
    DeclareThreadSafeLogger(supply);

    const std::set<int> EMPTY_INT_SET;
    const std::set<std::set<int>> EMPTY_INT_SET_SET;
    const std::set<std::pair<int, int>> EMPTY_INT_PAIR_SET;
    const std::map<int, float> EMPTY_INT_FLOAT_MAP;
}

const std::set<std::pair<int, int>>& SupplyManager::SupplyStarlaneTraversals(int empire_id) const {
    auto it = m_supply_starlane_traversals.find(empire_id);
    if (it != m_supply_starlane_traversals.end())
        return it->second;
    return EMPTY_INT_PAIR_SET;
}

const std::set<std::pair<int, int>>& SupplyManager::SupplyObstructedStarlaneTraversals(int empire_id) const {
    auto it = m_supply_starlane_obstructed_traversals.find(empire_id);
    if (it != m_supply_starlane_obstructed_traversals.end())
        return it->second;
    return EMPTY_INT_PAIR_SET;
}

const std::set<int>& SupplyManager::FleetSupplyableSystemIDs(int empire_id) const {
    auto it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it != m_fleet_supplyable_system_ids.end())
        return it->second;
    return EMPTY_INT_SET;
}

std::vector<int> SupplyManager::FleetSupplyableSystemIDs(
    int empire_id, bool include_allies, const ScriptingContext& context) const
{
    auto& direct_sys = FleetSupplyableSystemIDs(empire_id);
    std::vector<int> retval(direct_sys.begin(), direct_sys.end());
    if (!include_allies)
        return retval;

    retval.reserve(context.ContextObjects().size<System>());
    // add supplyable systems of all allies
    for (auto& [other_empire_id, systems] : m_fleet_supplyable_system_ids) {
        if (other_empire_id == empire_id)
            continue;
        if (systems.empty())
            continue;
        if (context.ContextDiploStatus(empire_id, other_empire_id) != DiplomaticStatus::DIPLO_ALLIED)
            continue;
        retval.insert(retval.end(), systems.begin(), systems.end());
    }
    std::sort(retval.begin(), retval.end());
    auto unique_it = std::unique(retval.begin(), retval.end());
    retval.resize(std::distance(retval.begin(), unique_it));
    return retval;
}

int SupplyManager::EmpireThatCanSupplyAt(int system_id) const {
    for (auto& [empire_id, sys_ids] : m_fleet_supplyable_system_ids) {
        if (sys_ids.contains(system_id))
            return empire_id;
    }
    return ALL_EMPIRES;
}

const std::set<std::set<int>>& SupplyManager::ResourceSupplyGroups(int empire_id) const {
    auto it = m_resource_supply_groups.find(empire_id);
    if (it != m_resource_supply_groups.end())
        return it->second;
    return EMPTY_INT_SET_SET;
}

const std::map<int, float>& SupplyManager::PropagatedSupplyRanges(int empire_id) const {
    auto emp_it = m_empire_propagated_supply_ranges.find(empire_id);
    if (emp_it == m_empire_propagated_supply_ranges.end())
        return EMPTY_INT_FLOAT_MAP;
    return emp_it->second;
}

const std::map<int, float>& SupplyManager::PropagatedSupplyDistances(int empire_id) const {
    auto emp_it = m_empire_propagated_supply_distances.find(empire_id);
    if (emp_it == m_empire_propagated_supply_distances.end())
        return EMPTY_INT_FLOAT_MAP;
    return emp_it->second;
}

bool SupplyManager::SystemHasFleetSupply(int system_id, int empire_id) const {
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;
    auto it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it == m_fleet_supplyable_system_ids.end())
        return false;
    const auto& sys_set = it->second;
    return sys_set.contains(system_id);
}

bool SupplyManager::SystemHasFleetSupply(int system_id, int empire_id, bool include_allies,
                                         const DiploStatusMap& diplo_statuses) const
{
    if (!include_allies)
        return SystemHasFleetSupply(system_id, empire_id);
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;

    auto empire_ids = EmpireManager::GetEmpireIDsWithDiplomaticStatusWithEmpire(
        empire_id, DiplomaticStatus::DIPLO_ALLIED, diplo_statuses);
    empire_ids.insert(empire_id);

    for (auto id : empire_ids) {
        auto sys_set_it = m_fleet_supplyable_system_ids.find(id);
        if (sys_set_it == m_fleet_supplyable_system_ids.end())
            continue;
        const auto& sys_set = sys_set_it->second;
        if (sys_set.contains(system_id))
            return true;
    }

    return false;
}

std::string SupplyManager::Dump(const Universe& u, int empire_id) const {
    std::string retval;

    const ObjectMap& objects = u.Objects();

    try {
        for (const auto& [supplying_empire_id, supplyable_system_ids] : m_fleet_supplyable_system_ids) {
            if (empire_id != ALL_EMPIRES && supplying_empire_id != empire_id)
                continue;

            retval += "Supplyable systems for empire " + std::to_string(supplying_empire_id) + "\n";
            for (const auto& sys : objects.find<System>(supplyable_system_ids)) {
                if (!sys)
                    continue;
                retval += "\n" + sys->PublicName(empire_id, u) + " (" + std::to_string(sys->ID()) + ") ";

                retval += "\nTraversals from here to: ";

                for (const auto& [sys1_id, sys2_id] : m_supply_starlane_traversals.at(supplying_empire_id)) {
                    if (sys1_id == sys->ID()) {
                        auto obj = objects.get(sys2_id);
                        if (obj)
                            retval += obj->PublicName(empire_id, u) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Traversals to here from: ";
                for (const auto& [sys1_id, sys2_id] : m_supply_starlane_traversals.at(supplying_empire_id)) {
                    if (sys2_id == sys->ID()) {
                        auto obj = objects.get(sys1_id);
                        if (obj)
                            retval += obj->PublicName(empire_id, u) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals from here to: ";
                for (const auto& [sys1_id, sys2_id] : m_supply_starlane_obstructed_traversals.at(supplying_empire_id)) {
                    if (sys1_id == sys->ID()) {
                        if (auto obj = objects.get(sys2_id))
                            retval += obj->PublicName(empire_id, u) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals to here from: ";
                for (const auto& [sys1_id, sys2_id] : m_supply_starlane_obstructed_traversals.at(supplying_empire_id)) {
                    if (sys2_id == sys->ID()) {
                        if (auto obj = objects.get(sys1_id))
                            retval += obj->PublicName(empire_id, u) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

            }
            retval += "\n\n";
        }

        for (const auto& [supplying_empire_id, system_groups] : m_resource_supply_groups) {
            retval += "Supply groups for empire " + std::to_string(supplying_empire_id) + "\n";
            for (const auto& system_group : system_groups) {
                retval += "group: ";
                for (const auto& sys : objects.find<System>(system_group)) {
                    if (sys)
                        retval += "\n" + sys->PublicName(empire_id, u) + " (" + std::to_string(sys->ID()) + ") ";
                }
                retval += "\n";
            }
            retval += "\n\n";
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "SupplyManager::Dump caught exception: " << e.what();
    }

    return retval;
}

namespace {
    std::pair<float, float> EmpireTotalSupplyRangeSumInSystem(
        int empire_id, int system_id, const ObjectMap& objects)
    {
        if (empire_id == ALL_EMPIRES || system_id == INVALID_OBJECT_ID)
            return {0.0f, 0.0f};
        const auto sys = objects.get<System>(system_id);
        if (!sys)
            return {0.0f, 0.0f};

        float accumulator_current = 0.0f;
        float accumulator_max = 0.0f;

        for (auto* obj : objects.findRaw(sys->ObjectIDs())) {
            if (!obj || !obj->OwnedBy(empire_id))
                continue;
            if (const auto* m = obj->GetMeter(MeterType::METER_SUPPLY))
                accumulator_current += m->Current();
            if (const auto* m = obj->GetMeter(MeterType::METER_MAX_SUPPLY))
                accumulator_max += m->Current();
        }
        return {accumulator_current, accumulator_max};
    }

    float EmpireTotalSupplyRange(int empire_id, const ObjectMap& objects) {
        if (empire_id == ALL_EMPIRES)
            return 0.0f;

        auto is_owned = [empire_id](const UniverseObject* obj) noexcept { return obj->OwnedBy(empire_id); };

        float accumulator_current = 0.0f;
        for (auto* obj : objects.findRaw<Planet>(is_owned)) { // TODO: handle ships if they can have supply meters
            if (auto* m = obj->UniverseObject::GetMeter(MeterType::METER_SUPPLY))
                accumulator_current += m->Current();
        }

        return accumulator_current;
    }

    float DistanceBetweenObjects(int obj1_id, int obj2_id, const ObjectMap& objects) {
        auto const obj1 = objects.getRaw<const System>(obj1_id);
        if (!obj1)
            return 0.0f;
        auto const obj2 = objects.getRaw<const System>(obj2_id);
        if (!obj2)
            return 0.0f;
        const double dx = obj2->X() - obj1->X();
        const double dy = obj2->Y() - obj1->Y();
        return static_cast<float>(std::sqrt(dx*dx + dy*dy));
    }
}

void SupplyManager::Update(const ScriptingContext& context) {
    const auto& empires = context.Empires();
    const Universe& universe = context.ContextUniverse();
    const ObjectMap& objects = context.ContextObjects();

    if (this != &context.supply)
        WarnLogger() << "SupplyManager::Update passed a ScriptingContext with a different SupplyManager referenced in it";

    m_supply_starlane_traversals.clear();
    m_supply_starlane_obstructed_traversals.clear();
    m_fleet_supplyable_system_ids.clear();
    m_resource_supply_groups.clear();
    m_propagated_supply_ranges.clear();

    DebugLogger(supply) << "SupplyManager::Update";

    // for each empire, need to get a set of sets of systems that can exchange
    // resources.  some sets may be just one system, in which resources can be
    // exchanged between UniverseObjects producing or consuming them, but which
    // can't exchange with any other systems.

    // which systems can share resources depends on system supply ranges, which
    // systems have obstructions to supply propagation for each empire, and
    // the ranges and obstructions of other empires' supply, as only one empire
    // can supply each system or propagate along each starlane. one empire's
    // propagating supply can push back another's, if the pusher's range is
    // larger.

    // map from empire id to map from system id to range (in starlane jumps)
    // that supply can be propagated out of that system by that empire.
    std::map<int, std::map<int, float>> empire_system_supply_ranges;
    // map from empire id to which systems are obstructed for it for supply
    // propagation
    std::map<int, std::set<int>> empire_supply_unobstructed_systems;
    // map from empire id to map from system id to pair of sum of supply source
    // ranges and max ranges of objects owned by empire in that in system
    std::map<int, std::map<int, std::pair<float, float>>> empire_system_supply_range_sums;
    // map from empire id to total supply range sum of objects it owns
    std::map<int, float> empire_total_supply_range_sums;

    for (auto& [empire_id, empire] : empires) {
        empire_system_supply_ranges[empire_id] = empire->SystemSupplyRanges();
        empire_supply_unobstructed_systems[empire_id] = empire->SupplyUnobstructedSystems();

        TraceLogger(supply) << "Empire " << empire_id << " unobstructed systems: "
                            << [&empire_supply_unobstructed_systems, empire_id{empire_id}]()
        {
            std::stringstream ss;
            for (int system_id : empire_supply_unobstructed_systems[empire_id])
                ss << system_id << ", ";
            return ss.str();
        }();
    }
    for (auto& [empire_id, systems] : empire_system_supply_ranges) {
        for (const auto system_id : systems | range_keys) {
            empire_system_supply_range_sums[empire_id][system_id] =
                EmpireTotalSupplyRangeSumInSystem(empire_id, system_id, objects);
        }
        empire_total_supply_range_sums[empire_id] = EmpireTotalSupplyRange(empire_id, objects);
    }


    for (const auto empire_id : empires | range_keys) {
        const auto& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(empire_id);
        std::set<int> systems_containing_friendly_fleets;

        for (auto* fleet : objects.allRaw<Fleet>()) {
            int system_id = fleet->SystemID();
            if (system_id == INVALID_OBJECT_ID || known_destroyed_objects.contains(fleet->ID()))
                continue;

            if (fleet->CanDamageShips(context) && fleet->Obstructive() && fleet->OwnedBy(empire_id)) {
                if (fleet->NextSystemID() == INVALID_OBJECT_ID ||
                    fleet->NextSystemID() == fleet->SystemID())
                { systems_containing_friendly_fleets.insert(system_id); }
            }
        }

        std::set<int> systems_where_others_have_supply_sources_and_current_empire_doesnt;
        // add all systems where others have supply
        for (const auto& [supply_empire_id, sys_ranges] : empire_system_supply_ranges) {
            if (supply_empire_id == empire_id || supply_empire_id == ALL_EMPIRES)
                continue;

            for (const auto& [system_id, range] : sys_ranges) {
                if (range <= 0.0f)
                    continue;
                systems_where_others_have_supply_sources_and_current_empire_doesnt.insert(system_id);
            }
        }
        // remove systems were this empire has supply
        auto it = empire_system_supply_ranges.find(empire_id);
        if (it != empire_system_supply_ranges.end()) {
            for (const auto& [system_id, range] : it->second) {
                if (range <= 0.0f)
                    continue;
                systems_where_others_have_supply_sources_and_current_empire_doesnt.erase(system_id);
            }
        }

        // for systems where others have supply sources and this empire doesn't
        // and where this empire has no fleets, supply is obstructed
        for (int system_id : systems_where_others_have_supply_sources_and_current_empire_doesnt) {
            if (!systems_containing_friendly_fleets.contains(system_id))
                empire_supply_unobstructed_systems[empire_id].erase(system_id);
        }
    }


    // system connections each empire can see / use for supply propagation
    const auto to_known_lanes = [&universe](const auto& id_e) -> decltype(auto)
    { return std::pair(id_e.first, id_e.second->KnownStarlanes(universe)); };

    auto lanes_rng = empires | range_transform(to_known_lanes);
    boost::container::flat_map<int, Empire::LaneSet> empire_visible_starlanes{lanes_rng.begin(), lanes_rng.end()};

    boost::container::flat_set<int> systems_with_supply_in_them;
    systems_with_supply_in_them.reserve(objects.size<System>());

    // store (supply range in jumps, and distance to supply source) of all
    // unobstructed systems before propagation, and add to list of systems
    // to propagate from.
    std::map<int, std::map<int, std::pair<float, float>>> empire_propagating_supply_ranges;
    float max_range = 0.0f;

    for (auto& [empire_id, system_ranges] : empire_system_supply_ranges) {
        const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

        for (const auto& [system_id, system_supply_range] : system_ranges) {
            if (unobstructed_systems.contains(system_id)) {
                // stored: first -> source supply range.  second -> distance to source (0 for the source itself)
                empire_propagating_supply_ranges[empire_id][system_id] = {system_supply_range, 0.0f};
                if (system_supply_range > max_range)
                    max_range = system_supply_range;
                systems_with_supply_in_them.insert(system_id);
            }
        }
    }

    TraceLogger(supply) << "Propagating supply";
    // spread supply out from sources by "diffusion" like process, along unobstructed
    // starlanes, until the range is exhausted.
    for (float range_to_spread = max_range; range_to_spread >= 0; range_to_spread -= 1.0f) {
        TraceLogger(supply) << "Propagating at range " << range_to_spread;

        // update systems that have supply in them
        for (auto& supply_ranges : empire_propagating_supply_ranges | range_values) {
            static_assert(std::is_same_v<std::decay_t<decltype(supply_ranges)>,
                          std::map<int, std::pair<float, float>>>,
                          "make sure supply ranges are sorted for use with ordered_unique_range below");
            auto sys_ids_rng = supply_ranges | range_keys;
#if BOOST_VERSION > 107800
            systems_with_supply_in_them.insert(boost::container::ordered_unique_range,
                                               sys_ids_rng.begin(), sys_ids_rng.end());
#else
            std::decay_t<decltype(systems_with_supply_in_them)>::sequence_type scratch;
            scratch.reserve(supply_ranges.size());
            range_copy(sys_ids_rng, std::back_inserter(scratch));
            systems_with_supply_in_them.insert(scratch.begin(), scratch.end());
#endif
        }

        // resolve supply fights between multiple empires in one system.
        // pass over all empire-supplied systems, removing supply for all
        // but the empire with the highest supply range in each system
        for (const auto* sys : objects.findRaw<System>(systems_with_supply_in_them)) {
            TraceLogger(supply) << "Determining top supply empire in system " << sys->Name() << " (" << sys->ID() << ")";
            // sort empires by range in this system
            std::map<float, std::set<int>> empire_ranges_here;
            for (auto& [empire_id, system_ranges] : empire_propagating_supply_ranges) {
                auto empire_supply_it = system_ranges.find(sys->ID());
                // does this empire have any range in this system? if so, store it
                if (empire_supply_it == system_ranges.end())
                    continue;

                // stuff to break ties...
                float bonus = 0.0f;

                // empires with planets in system
                bool has_outpost = false, has_colony = false;
                for (auto* planet : objects.findRaw<Planet>(sys->PlanetIDs())) {
                    if (!planet || !planet->OwnedBy(empire_id))
                        continue;
                    if (!planet->SpeciesName().empty())
                        has_colony = true;
                    else
                        has_outpost = true;
                }
                if (has_colony)
                    bonus += 0.5f;
                else if (has_outpost)
                    bonus += 0.3f;

                // sum of all supply sources in this system
                bonus += empire_system_supply_range_sums[empire_id][sys->ID()].first / 1000.0f;
                // sum of max supply of sourses in this system
                bonus += empire_system_supply_range_sums[empire_id][sys->ID()].second / 100000.0f;
                bonus += empire_total_supply_range_sums[empire_id] / 100000000.0f;

                // distance to supply source from here
                float propagated_distance_to_supply_source = std::max(1.0f, empire_supply_it->second.second);
                bonus += propagated_distance_to_supply_source / 10000.0f;

                // store ids of empires indexed by adjusted propgated range, in order to sort by range
                float propagated_range = empire_supply_it->second.first;
                empire_ranges_here[propagated_range + bonus].insert(empire_id);
            }

            if (empire_ranges_here.empty()) {
                TraceLogger(supply) << " ... no empire has a range here";

            } else if (empire_ranges_here.size() == 1) {
                TraceLogger(supply) << " ... only empire here: " << *empire_ranges_here.begin()->second.begin()
                                    << " with range: " << empire_ranges_here.begin()->first;

            } else {
                TraceLogger(supply) << " ... ranges of empires: " << [&]() {
                    std::stringstream erss;
                    for (auto& [empire_range, empire_ids] : empire_ranges_here) {
                        erss << empire_range << " : (";
                        for (const auto& eid : empire_ids)
                            erss << eid << " ";
                        erss << ")   ";
                    }
                    return erss.str();
                }();
            }

            if (empire_ranges_here.empty())
                continue;   // no empire has supply here?
            if (empire_ranges_here.size() == 1 && empire_ranges_here.begin()->second.size() < 2)
                continue;   // only one empire has supply here

            // at least two empires have supply sources here...
            // check if one is stronger

            // remove supply for all empires except the top-ranged empire here,
            // or one of the empires at the top if all top empires are allies
            auto range_empire_it = empire_ranges_here.rbegin();
            int top_range_empire_id = ALL_EMPIRES;
            if (range_empire_it->second.size() == 1) {
                // if just one empire has the most range, it is the top empire
                top_range_empire_id = *(range_empire_it->second.begin());

            } else {
                // if all empires that share the top range are allies, pick one
                // to be the top empire
                TraceLogger(supply) << " ... top empires are allied!";
                const auto& top_empires = range_empire_it->second;
                bool any_non_allied_pair = false;
                for (auto id1 : top_empires) {
                    if (id1 == ALL_EMPIRES) continue;
                    for (auto id2 : top_empires) {
                        if (id2 == ALL_EMPIRES || id2 <= id1) continue;
                        if (context.ContextDiploStatus(id1, id2) != DiplomaticStatus::DIPLO_ALLIED) {
                            any_non_allied_pair = true;
                            break;
                        }
                    }
                }
                if (!any_non_allied_pair) {
                    // arbitrarily pick the lowest ID empire
                    top_range_empire_id = *(range_empire_it->second.begin());
                }
            }
            TraceLogger(supply) << " ... top ranged empire here: " << top_range_empire_id
                                << " with range: " << range_empire_it->first;


            // remove range entries and traversals for all but the top empire
            // (or all empires if there is no single top empire)
            for (auto& [empire_id, empire_ranges] : empire_propagating_supply_ranges) {
                if (empire_id == top_range_empire_id)
                    continue;   // this is the top empire, so leave as the sole empire supplying here

                // remove from range entry...
                empire_ranges.erase(sys->ID());

                TraceLogger(supply) << "... removed empire " << empire_id << " system " << sys->ID() << " supply.";

                // Remove from unobstructed systems
                empire_supply_unobstructed_systems[empire_id].erase(sys->ID());

                auto& lane_traversals = m_supply_starlane_traversals[empire_id];
                const auto lane_traversals_initial{lane_traversals};
                auto& obstructed_traversals = m_supply_starlane_obstructed_traversals[empire_id];
                const auto obstrcuted_traversals_initial{obstructed_traversals};

                // remove from traversals departing from or going to this system for this empire,
                // and set any traversals going to this system as obstructed
                for (const auto& lane : lane_traversals_initial) {
                    if (lane.first == sys->ID())
                        lane_traversals.erase({sys->ID(), lane.second});
                    if (lane.second == sys->ID()) {
                        lane_traversals.erase({lane.first, sys->ID()});
                        obstructed_traversals.emplace(lane.first, sys->ID());
                    }
                }

                // remove obstructed traverals departing from this system
                for (const auto& lane : obstrcuted_traversals_initial) {
                    if (lane.first == sys->ID())
                        obstructed_traversals.erase(lane);
                }
            }

            //// DEBUG
            for (auto& [empire_id, system_ranges] : empire_propagating_supply_ranges) {
                auto range_it = system_ranges.find(sys->ID());
                if (range_it != system_ranges.end())
                    TraceLogger(supply) << " ... after culling empires ranges at system " << sys->ID()
                                        << " : " << empire_id << " : " << range_it->second.first;
            }
            //// END DEBUG
        }

        if (range_to_spread <= 0.0f)
            break;

        // initialize next iteration with current supply distribution
        auto empire_propagating_supply_ranges_next{empire_propagating_supply_ranges};


        // for sources of supply of at least the minimum range for this
        // iteration that are in the current map, give adjacent systems one
        // less supply in the next iteration (unless as much or more is already
        // there)
        for (const auto& [empire_id, prev_sys_ranges] : empire_propagating_supply_ranges) {
            TraceLogger(supply) << ">-< Doing supply propagation for empire " << empire_id
                                << " >-<  at spread range: " << range_to_spread;
            const auto& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

            const auto e_it = empire_visible_starlanes.find(empire_id);
            if (e_it == empire_visible_starlanes.end())
                continue;
            const auto& visible_lanes{e_it->second};


            for (const auto& [system_id, range_and_dist] : prev_sys_ranges) {
                const auto& [range, distance_to_supply_source] = range_and_dist;
                TraceLogger(supply) << " ... for system " << system_id << " with range: " << range;

                // get lanes starting in system with id system_id
                const Empire::LaneEndpoints system_lane{system_id, system_id};
                static constexpr auto lane_starts_less = [](const auto lane1, const auto lane2)
                { return lane1.start < lane2.start; };
                const auto system_lanes_rng = range_equal(visible_lanes, system_lane, lane_starts_less);
                static constexpr auto to_lane_end = [](const auto lane) { return lane.end; };


                // does the source system have the correct supply range to propagate outwards in this iteration?
                if (std::floor(range) != range_to_spread)
                    continue;
                float range_after_one_more_jump = range - 1.0f; // what to set adjacent systems' ranges to (at least)

                TraceLogger(supply) <<
                    [](const auto distance_to_supply_source, const auto range, const auto empire_id,
                       const auto system_id, const auto system_lanes_rng)
                {
                    std::string retval = "Propagating from system " + std::to_string(system_id) + " to ";
                    for (const int lane_end_sys_id : system_lanes_rng | range_transform(to_lane_end))
                        retval.append(std::to_string(lane_end_sys_id)).append(" ");
                    retval.append("range: ").append(std::to_string(range))
                          .append(" and distance: ").append(std::to_string(distance_to_supply_source));
                    return retval;
                }(distance_to_supply_source, range, empire_id, system_id, system_lanes_rng);


                // attempt to propagate to all adjacent systems...
                for (const int lane_end_sys_id : system_lanes_rng | range_transform(to_lane_end)) {
                    // is propagation to the adjacent system obstructed?
                    if (!unobstructed_systems.contains(lane_end_sys_id)) {
                        // propagation obstructed!
                        TraceLogger(supply) << "Added obstructed traversal from " << system_id << " to "
                                            << lane_end_sys_id << " due to not being on unobstructed systems";
                        m_supply_starlane_obstructed_traversals[empire_id].emplace(system_id, lane_end_sys_id);
                        continue;
                    }
                    // propagation not obstructed.
                    TraceLogger(supply) << "Propagation from " << system_id << " to " << lane_end_sys_id << " is unobstructed";

                    // does another empire already have as much or more supply here from a previous iteration?
                    float other_empire_biggest_range = -10000.0f;   // arbitrary big numbeer
                    for (auto& [other_empire_id, prev_other_empire_sys_ranges] : empire_propagating_supply_ranges) {
                        if (other_empire_id == empire_id)
                            continue;
                        auto prev_other_empire_range_it = prev_other_empire_sys_ranges.find(lane_end_sys_id);
                        if (prev_other_empire_range_it == prev_other_empire_sys_ranges.end())
                            continue;
                        if (prev_other_empire_range_it->second.first > other_empire_biggest_range)
                            other_empire_biggest_range = prev_other_empire_range_it->second.first;
                    }

                    // if so, add a blocked traversal and continue
                    if (range_after_one_more_jump <= other_empire_biggest_range) {
                        m_supply_starlane_obstructed_traversals[empire_id].emplace(system_id, lane_end_sys_id);
                        TraceLogger(supply) << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id
                                            << " due to other empire biggest range being " << other_empire_biggest_range;
                        continue;
                    }

                    // otherwise, propagate into system...
                    const float lane_length = DistanceBetweenObjects(system_id, lane_end_sys_id, objects);
                    const float distance_to_supply_source_after_next_lane = lane_length + distance_to_supply_source;

                    TraceLogger(supply) << "Attempting to propagate into system: " << lane_end_sys_id << " the new range: "
                                        << range_after_one_more_jump << " and distance: " << distance_to_supply_source_after_next_lane;

                    // if propagating supply would increase the range of the adjacent system,
                    // or decrease the distance to the adjacent system from a supply source...
                    const auto prev_range_it = prev_sys_ranges.find(lane_end_sys_id);
                    if (prev_range_it == prev_sys_ranges.end()) {
                        empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id] =
                            {range_after_one_more_jump, distance_to_supply_source_after_next_lane};
                        //TraceLogger(supply) << " ... default case: no previous entry.";

                    } else {
                        //TraceLogger(supply) << " ... previous entry values: " << prev_range_it->second.first << " and " << prev_range_it->second.second;

                        if (range_after_one_more_jump > prev_range_it->second.first) {
                            empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id].first =
                                range_after_one_more_jump;
                            //TraceLogger(supply) << " ... range increased!";
                        }
                        if (distance_to_supply_source_after_next_lane < prev_range_it->second.second) {
                            empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id].second =
                                distance_to_supply_source_after_next_lane;
                            //TraceLogger(supply) << " ... distance decreased!";
                        }
                    }
                    // always record a traversal, so connectivity is calculated properly
                    m_supply_starlane_traversals[empire_id].emplace(system_id, lane_end_sys_id);
                    TraceLogger(supply) << "Added traversal from " << system_id << " to " << lane_end_sys_id;

                    // erase any previous obstructed traversal that just succeeded
                    if (m_supply_starlane_obstructed_traversals[empire_id].contains({system_id, lane_end_sys_id})) {
                        //TraceLogger(supply) << "Removed obstructed traversal from " << system_id << " to " << lane_end_sys_id;
                        m_supply_starlane_obstructed_traversals[empire_id].erase({system_id, lane_end_sys_id});
                    }
                    if (m_supply_starlane_obstructed_traversals[empire_id].contains({lane_end_sys_id, system_id})) {
                        //TraceLogger(supply) << "Removed obstructed traversal from " << lane_end_sys_id << " to " << system_id;
                        m_supply_starlane_obstructed_traversals[empire_id].erase({lane_end_sys_id, system_id});
                    }
                }
            }
        }

        // save propagated results for next iteration
        empire_propagating_supply_ranges = std::move(empire_propagating_supply_ranges_next);
    }


    TraceLogger(supply) << "SupplyManager::Update: after removing conflicts, empires can provide supply to the following system ids (and ranges in jumps):";
    for (const auto& [empire_id, supply_ranges] : empire_propagating_supply_ranges) {
        TraceLogger(supply) << " ... empire " << empire_id << ":  " << [&, rngs{supply_ranges}]() {
            std::stringstream ss;
            for (auto& [sys_id, jumps_distance] : rngs)
                ss << sys_id << " (" << jumps_distance.first << "),  ";
            return ss.str();
        }();

    }


    // record which systems are fleet supplyable by each empire (after resolving conflicts in each system)
    // empire_propagating_supply_ranges[empire_id][system_id] = {system_supply_range, distance to source};
    for (const auto& [empire_id, system_supply_ranges] : empire_propagating_supply_ranges) {
        for (auto& [system_id, range_distance] : system_supply_ranges) {
            if (range_distance.first < 0.0f)
                continue;   // negative supply doesn't count... zero does (it just reaches)
            m_fleet_supplyable_system_ids[empire_id].insert(system_id);

            // should be only one empire per system at this point, but use max just to be safe...
            m_propagated_supply_ranges[system_id] =
                std::max(range_distance.first, m_propagated_supply_ranges[system_id]);
            m_empire_propagated_supply_ranges[empire_id][system_id] =
                m_propagated_supply_ranges[system_id];

            // should be only one empire per system at this point, but use max just to be safe...
            m_propagated_supply_distances[system_id] =
                std::max(range_distance.second, m_propagated_supply_distances[system_id]);
            m_empire_propagated_supply_distances[empire_id][system_id] =
                m_propagated_supply_distances[system_id];
        }

        //TraceLogger(supply) << "For empire: " << empire_id << " system supply distances: ";
        //for (auto& entry : m_empire_propagated_supply_distances[empire_id]) {
        //    TraceLogger(supply) << entry.first << " : " << entry.second;
        //}
    }


    for (const auto& [empire_id, traversals] : m_supply_starlane_traversals) {
        TraceLogger(supply) << "Empire " << empire_id << " propagated supply traversals:";
        for (auto const& [a, b] : traversals)
            TraceLogger(supply) << " ... " << a << " to " << b;
    }
    for (const auto& [empire_id, traversals] : m_supply_starlane_obstructed_traversals) {
        TraceLogger(supply) << "Empire " << empire_id << " obstructed supply traversals:";
        for (auto const& [a, b] : traversals)
            TraceLogger(supply) << " ... " << a << " to " << b;
    }

    auto ally_merged_supply_starlane_traversals{m_supply_starlane_traversals};
    auto allies_of = [&context](int empire_id)
    { return context.GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_id, DiplomaticStatus::DIPLO_ALLIED); };


    // add connections into allied empire systems where traversals are
    // obstructed on one side by the ally's supply
    for (const auto& [supply_empire_id, empire_obstructed_traversals] : m_supply_starlane_obstructed_traversals) {
        auto& empire_supply_traversals = ally_merged_supply_starlane_traversals[supply_empire_id]; // output
        const auto& supply_empire_baseline_unobstructed_systems =
            empires.GetEmpire(supply_empire_id)->SupplyUnobstructedSystems();

        const auto allies_of_empire = allies_of(supply_empire_id);
        for (int ally_id : allies_of_empire) {
            const auto& ally_supplyable_systems = m_empire_propagated_supply_ranges[ally_id];
            const auto& ally_baseline_unobstructed_systems =
                empires.GetEmpire(ally_id)->SupplyUnobstructedSystems();
            auto& ally_supply_traversals = ally_merged_supply_starlane_traversals[ally_id]; // output

            // find cases where outer loop empire has an obstructed traversal from
            // system A to system B and inner loop empire (the ally) has supply in
            // system B
            for (auto const& [sys_A, sys_B] : empire_obstructed_traversals) {
                TraceLogger(supply) << "for empire " << supply_empire_id
                                    << " obstructed traversal from " << sys_A << " - " << sys_B;
                if (!ally_supplyable_systems.contains(sys_B))
                    continue;
                TraceLogger(supply) << " ... supplied by ally " << ally_id;
                if (ally_supplyable_systems.contains(sys_B)) {
                    TraceLogger(supply) << " ... " << sys_A
                                        << (supply_empire_baseline_unobstructed_systems.contains(sys_A) ? " un" : " ")
                                        << "obstructed for empire " << supply_empire_id;
                    TraceLogger(supply) << " ... " << sys_B
                                        << (supply_empire_baseline_unobstructed_systems.contains(sys_B) ? " un" : " ")
                                        << "obstructed for empire " << supply_empire_id;
                    if (supply_empire_baseline_unobstructed_systems.contains(sys_B) &&
                        supply_empire_baseline_unobstructed_systems.contains(sys_A))
                    {
                        TraceLogger(supply) << " ... ... adding empire " << supply_empire_id
                                            << " traversals for " << sys_A << " and " << sys_B;
                        empire_supply_traversals.emplace(sys_A, sys_B);
                        empire_supply_traversals.emplace(sys_B, sys_A);
                    }

                    TraceLogger(supply) << " ... " << sys_A
                                        << (ally_baseline_unobstructed_systems.contains(sys_A) ? " un" : " ")
                                        << "obstructed for empire " << ally_id;
                    TraceLogger(supply) << " ... " << sys_B
                                        << (ally_baseline_unobstructed_systems.contains(sys_B) ? " un" : " ")
                                        << "obstructed for empire " << ally_id;
                    if (ally_baseline_unobstructed_systems.contains(sys_B) &&
                        ally_baseline_unobstructed_systems.contains(sys_A))
                    {
                        TraceLogger(supply) << " ... ... adding empire " << ally_id
                                            << " traversals for " << sys_A << " and " << sys_B;
                        ally_supply_traversals.emplace(sys_A, sys_B);
                        ally_supply_traversals.emplace(sys_B, sys_A);
                    }

                }
            }
        }
    }


    for (const auto& [supply_empire_id, empire] : empires) {
        const auto allies_of_empire = allies_of(supply_empire_id);
        if (allies_of_empire.empty())
            continue;
        const auto& empire_directly_supplied = m_empire_propagated_supply_ranges[supply_empire_id];
        const auto& empire_obstructed_traversals = m_supply_starlane_obstructed_traversals[supply_empire_id];
        TraceLogger(supply) << " CHECK BORDERS of empire " << supply_empire_id << "";
        for (auto const& [sys_A, sys_B] : empire_obstructed_traversals) {
            //TraceLogger(supply) << "(?empire " << supply_empire_id << "?) " << sys_A << " - " << sys_B << " (empire ?)";
            if (!empire_directly_supplied.contains(sys_A))
                ErrorLogger(supply) << "Error: system " << sys_A << " is NOT directly supplied by empire " << supply_empire_id << ", but it should";
            // Find the supplier of B and find the intersection of allies between the B supplier and the A supplier
            for (const auto& [b_empire_id, b_empire] : empires) {
                //TraceLogger(supply) << "Check if (empire " << b_empire_id << ") " << " is supplier of " << sys_B;
                if (!m_empire_propagated_supply_ranges[b_empire_id].contains(sys_B))
                    continue;
                TraceLogger(supply) << " ... empire " << supply_empire_id << ") " << sys_A << " - " << sys_B << " (empire " << b_empire_id << ")";
                // all allies of a_empire which are allies of b_empire may use this traversal
                for (int a_ally_id : allies_of_empire) {
                    TraceLogger(supply) << " ... Check if (empire " << a_ally_id << ") " << " is allied to supplier of " << sys_B << " (empire " << b_empire_id << ")";
                    if (a_ally_id == b_empire_id)
                        continue;
                    if (empires.GetDiplomaticStatus(a_ally_id, b_empire_id) >= DiplomaticStatus::DIPLO_ALLIED) {
                        TraceLogger(supply) << " ... ... Empire " << a_ally_id << " may use (empire " << supply_empire_id << ")" << sys_A << " - " << sys_B << " (empire " << b_empire_id << ") as its allied with " << b_empire_id;
                        auto& a_ally_supply_traversals = ally_merged_supply_starlane_traversals[a_ally_id]; // output
                        a_ally_supply_traversals.emplace(sys_A, sys_B);
                        a_ally_supply_traversals.emplace(sys_B, sys_A);
                    }
                }
                break; // stop after finding the one supplier of B
            }
        }
        TraceLogger(supply) << " DONE CHECK BORDERS of empire " << supply_empire_id;
    }

    for (const auto& [empire_id, traversals] : ally_merged_supply_starlane_traversals) {
        TraceLogger(supply) << "Empire " << empire_id << " supply traversals after ally connections:";
        for (auto const& [a, b] : traversals)
            TraceLogger(supply) << " ... " << a << " to " << b;
    }


    // same for fleet supplyable system ids, as these are added to supplyable
    // groups in following code
    auto ally_merged_fleet_supplyable_system_ids{m_fleet_supplyable_system_ids};
    for (auto& [empire_id, supplyable] : ally_merged_fleet_supplyable_system_ids) {
        const auto ally_ids = context.GetEmpireIDsWithDiplomaticStatusWithEmpire(
            empire_id, DiplomaticStatus::DIPLO_ALLIED);
        for (int ally_id : ally_ids) {
            const auto& ally_supplyable_systems = m_fleet_supplyable_system_ids[ally_id];
            // copy ally supplyable systems
            supplyable.insert(ally_supplyable_systems.begin(), ally_supplyable_systems.end());
        }
    }
    for (const auto& [empire_id, supplyable] : ally_merged_fleet_supplyable_system_ids) {
        TraceLogger(supply) << "Empire " << empire_id << " supplyable systems after adding allies:";
        for (auto const& a : supplyable)
            TraceLogger(supply) << " ... " << a;
    }


    // iterate...
    bool something_changed = true;
    std::size_t limit = 50;
    while (something_changed && limit-- > 0) {
        const auto initial{ally_merged_supply_starlane_traversals};

        // add allied supply starlane traversals to empires' traversals, so that
        // allies can use eachothers' supply networks
        for (auto& [empire_id, traversals] : ally_merged_supply_starlane_traversals) {
            const auto ally_ids = allies_of(empire_id);

            for (auto& [prior_empire_id, prior_traversals] : initial) {
                if (!ally_ids.contains(prior_empire_id))
                    continue; // only copy traversals of allied empires
                const auto& prior_empire_direct_supplied_systems = m_empire_propagated_supply_ranges[prior_empire_id];

                for (auto& [sys_A, sys_B] : prior_traversals) {
                    if (prior_empire_direct_supplied_systems.contains(sys_A) &&
                        prior_empire_direct_supplied_systems.contains(sys_B))
                    { traversals.emplace(sys_A, sys_B);} // only copy traversals to and from systems the allied empire directly propagated
                }
            }
        }

        something_changed = (initial != ally_merged_supply_starlane_traversals);
    }
    for (const auto& [empire_id, traversals] : ally_merged_supply_starlane_traversals) {
        TraceLogger(supply) << "Empire " << empire_id << " supply traversals after merging allies "
                            << (50 - limit) << " times:";
        for (auto const& [a, b] : traversals)
            TraceLogger(supply) << " ... " << a << " to " << b;
    }


    // determine supply-connected groups of systems for each empire.
    // need to merge interconnected supply groups into as few sets of mutually-
    // supply-exchanging systems as possible.  This requires finding the
    // connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.
    for (const auto empire_id : empire_propagating_supply_ranges | range_keys) {
        // assemble all direct connections between systems from traversals
        std::map<int, std::set<int>> supply_groups_map;
        for (auto const& lane : ally_merged_supply_starlane_traversals[empire_id]) {
            supply_groups_map[lane.first].insert(lane.second);
            supply_groups_map[lane.second].insert(lane.first);
        }

        // also add connections from all fleet-supplyable systems to themselves, so that
        // any fleet supplyable system with no connection to another system can still
        // have resource sharing within itself
        for (int system_id : ally_merged_fleet_supplyable_system_ids[empire_id])
            supply_groups_map[system_id].insert(system_id);


        if (supply_groups_map.empty())
            continue;

        TraceLogger(supply) << "Empire " << empire_id << " supply groups map before merging:";
        for (auto const& q : supply_groups_map) {
            TraceLogger(supply) << " ... " << q.first << " to: " << [&]() {
                std::stringstream other_ids;
                for (auto const& r : q.second)
                    other_ids << r << ", ";
                return other_ids.str();
            }();
        }


        // create graph
        boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;

        // boost expects vertex labels to range from 0 to num vertices - 1, so need
        // to map from system id to graph id and back when accessing vertices
        std::vector<int> graph_id_to_sys_id;
        graph_id_to_sys_id.reserve(supply_groups_map.size());

        std::map<int, int> sys_id_to_graph_id;
        int graph_id = 0;
        for (auto& supply_group : supply_groups_map) {
            int sys_id = supply_group.first;
            boost::add_vertex(graph);   // should add with index = graph_id

            graph_id_to_sys_id.push_back(sys_id);
            sys_id_to_graph_id[sys_id] = graph_id;
            ++graph_id;
        }

        // add edges for all direct connections between systems
        // and add edges from fleet supplyable systems to themselves
        for (const auto& supply_group : supply_groups_map) {
            int start_graph_id = sys_id_to_graph_id[supply_group.first];
            for (int system_id : supply_group.second) {
                int end_graph_id = sys_id_to_graph_id[system_id];
                boost::add_edge(start_graph_id, end_graph_id, graph);
            }
        }

        // declare storage and fill with the component id (group id of connected systems)
        // for each graph vertex
        std::vector<int> components(boost::num_vertices(graph));
        boost::connected_components(graph, components.data());

        // convert results back from graph id to system id, and into desired output format
        // output: std::map<int, std::set<std::set<int>>>& m_resource_supply_groups

        // first, sort into a map from component id to set of system ids in component
        std::map<int, std::set<int>> component_sets_map;
        for (std::size_t comp_graph_id = 0; comp_graph_id != components.size(); ++comp_graph_id) {
            int label = components[comp_graph_id];
            int sys_id = graph_id_to_sys_id[comp_graph_id];
            component_sets_map[label].insert(sys_id);
        }

        // copy sets in map into set of sets
        for (auto& component_set : component_sets_map)
            m_resource_supply_groups[empire_id].insert(std::move(component_set.second));
    }

    for (const auto& [empire_id, group_sets] : m_resource_supply_groups) {
        DebugLogger(supply) << "Connected supply groups for empire " << empire_id << ":";
        for (const auto& group_set : group_sets) {
            DebugLogger(supply) << " ... " << [&]() {
                std::stringstream ss;
                for (const auto& sys : group_set)
                    ss << sys << ", ";
                return ss.str();
            }();
        }
    }
}
