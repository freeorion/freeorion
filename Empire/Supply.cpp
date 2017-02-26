#include "Supply.h"

#include "EmpireManager.h"
#include "Empire.h"
#include "../universe/Universe.h"
#include "../universe/Pathfinder.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Enums.h"
#include "../util/AppInterface.h"
#include "../util/Serialize.h"
#include "../util/Logger.h"
#include "../util/ScopedTimer.h"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include <algorithm>
#include <iomanip>


std::string SupplySystemBonusTupleString(const SupplySystemBonusTuple& x) {
    std::stringstream ss;
    ss << std::setprecision(2) << std::get<ssbBonus>(x) << ":(" << std::get<ssbVisibilityBonus>(x) << "+"
       << std::get<ssbShipBonus>(x) << "+" << std::get<ssbColonyBonus>(x) << ")";
    return ss.str();
}

std::string SupplySystemEmpireTupleString(const SupplySystemEmpireTuple& x) {
    std::stringstream ss;
    ss << std::setprecision(2) << "source = " << std::get<sseSource>(x) << " range = " << std::get<sseRange>(x) << " bonus = "
       << SupplySystemBonusTupleString(std::get<sseBonus>(x)) << " dist = " << std::get<sseDistance>(x)
       << " stealth = "<< std::get<sseStealth>(x);
    return ss.str();
}

class SupplyManager::SupplyManagerImpl {
public:
    /** \name Structors */ //@{
    SupplyManagerImpl();
    SupplyManagerImpl& operator=(const SupplyManagerImpl& rhs);
    //@}

    /** \name Accessors */ //@{
    /** Returns set of directed starlane traversals along which supply can flow,
      * along with their stealth.  Results are pairs of system ids of start and
      * end system of traversal. */
    const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>&   SupplyStarlaneTraversals() const;
    const std::unordered_map<std::pair<int, int>, float>&                            SupplyStarlaneTraversals(int empire_id) const;

    /** Returns set of directed starlane traversals along which supply could
      * flow for this empire along with its stealth, but which can't due to some
      * obstruction in one of the systems. */
    const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>&   SupplyObstructedStarlaneTraversals() const;
    const std::unordered_map<std::pair<int, int> ,float>&                            SupplyObstructedStarlaneTraversals(int empire_id) const;

    /** Returns set of system ids where fleets can be supplied by this empire
      * (as determined by object supply meters and rules of supply propagation
      * and blockade). */
    const std::unordered_map<int, std::unordered_set<int> >&          FleetSupplyableSystemIDs() const;
    const std::unordered_set<int>&                                    FleetSupplyableSystemIDs(int empire_id) const;
    std::unordered_set<int>                                           FleetSupplyableSystemIDs(int empire_id, bool include_allies) const;

    /** Returns set of sets of systems that can share industry (systems in
      * separate groups are blockaded or otherwise separated). */
    const std::unordered_map<int, std::vector<std::shared_ptr<const std::unordered_set<int>>>>& ResourceSupplyGroups() const;
    const std::vector<std::shared_ptr<const std::unordered_set<int>>>&                          ResourceSupplyGroups(int empire_id) const;

    /** Returns the range from each system that the empire with id \a empire_id
      * can propagate supply.*/
    const std::unordered_map<int, float>&                             PropagatedSupplyRanges(int empire_id) const;

    /** Returns the distance from each system for the empire with id
      * \a empire_id to the closest source of supply without passing
      * through obstructed systems for that empire. */
    const std::unordered_map<int, float>&                             PropagatedSupplyDistances(int empire_id) const;

    /** Returns the stealth of the supply in each system for the empire with id
      * \a empire_id if the empire has siupply in system. */
    const std::unordered_map<int, float>&                             SupplyStealth(int empire_id) const;

    /** Returns true if system with id \a system_id is fleet supplyable or in
      * one of the resource supply groups for empire with id \a empire_id */
    bool        SystemHasFleetSupply(int system_id, int empire_id) const;
    bool        SystemHasFleetSupply(int system_id, int empire_id, bool include_allies) const;

    /** Return an ordered map from empire id to each system supply and detailed bonuses. */
    boost::optional<const SupplyManager::system_iterator> SystemSupply(const int system_id) const;

    std::string Dump(int empire_id = ALL_EMPIRES) const;
    //@}

    /** \name Mutators */ //@{
    /** Calculates systems at which fleets of empires can be supplied, and
      * groups of systems that can exchange resources, and the starlane
      * traversals used to do so. */
    void    Update();
    void    UpdateOld();

    /** Part of the Update function.*/
    std::map<int, std::map<int, std::pair<float, float>>> PropagateSupplyAlongUnobstructedStarlanes(
        const std::unordered_map<int, std::unordered_map<int, float>>& empire_system_supply_ranges,
        std::map<int, std::set<int>>& empire_supply_unobstructed_systems);

    void DetermineSupplyConnectedSystemGroups();
    //@}

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

private:
    /** ordered pairs of system ids between which a starlane runs that can be
        used to convey resources between systems. indexed first by empire id. */
    std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>  m_supply_starlane_traversals;

    /** ordered pairs of system ids between which a starlane could be used to
        convey resources between system, but is not because something is
        obstructing the resource flow.  That is, the resource flow isn't limited
        by range, but by something blocking its flow. */
    std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>  m_supply_starlane_obstructed_traversals;

    /** ids of systems where fleets can be resupplied. indexed by empire id. */
    std::unordered_map<int, std::unordered_set<int>> m_fleet_supplyable_system_ids;

    /** sets of system ids that are connected by supply lines and are able to
        share resources between systems or between objects in systems. indexed
        by empire id. */
    std::unordered_map<int, std::vector<std::shared_ptr<const std::unordered_set<int>>>>  m_resource_supply_groups;

    /** for whichever empire can propagate supply into this system, what is the
        additional range from this system that empire can propagate supply */
    std::unordered_map<int, float>                            m_propagated_supply_ranges;

    /** for each empire, what systems it can propagate supply into, and how many
      * further supply jumps it could propagate past this system, if not blocked
      * from doing so by supply obstructions. */
    std::unordered_map<int, std::unordered_map<int, float>>             m_empire_propagated_supply_ranges;

    /** for whichever empire can propagate supply into this system, how far
      * that system is from the closest source of supply for that empire, along
      * possible supply propgation connections (ie. not though a supply
      * obstructed system for that empire) */
    std::unordered_map<int, float>                            m_propagated_supply_distances;

    /** for each empire, what systems it can propagate supply into, and how far
      * that system is from the closest source of supply for that empire, along
      * possible supply propgation connections (ie. not though a supply
      * obstructed system) */
    std::unordered_map<int, std::unordered_map<int, float>>             m_empire_propagated_supply_distances;

    // For each empire and each system the highest stealth of supply in that system.
    std::unordered_map<int, std::unordered_map<int, float>> m_empire_to_system_to_stealth;

    // For each system a sorted map from empire to system merit and all components.
    // TODO consider assembling this from smaller maps for individual components
    std::unordered_map<int, std::map<int, std::vector<SupplySystemEmpireTuple>>> m_system_to_empire_to_final_supply;
};


SupplyManager::SupplyManagerImpl::SupplyManagerImpl() :
    m_supply_starlane_traversals(),
    m_supply_starlane_obstructed_traversals(),
    m_fleet_supplyable_system_ids(),
    m_resource_supply_groups()
{}

SupplyManager::SupplyManagerImpl& SupplyManager::SupplyManagerImpl::operator=(const SupplyManagerImpl& rhs) {
    m_supply_starlane_traversals =              rhs.m_supply_starlane_traversals;
    m_supply_starlane_obstructed_traversals =   rhs.m_supply_starlane_obstructed_traversals;
    m_fleet_supplyable_system_ids =             rhs.m_fleet_supplyable_system_ids;
    m_resource_supply_groups =                  rhs.m_resource_supply_groups;
    return *this;
}

namespace {
    // TODO remove these and replace with boost::optional which doesn't result in cross thread
    // reference to right here.
    static const std::set<int> EMPTY_INT_SET;
    static const std::set<std::set<int>> EMPTY_INT_SET_SET;
    static const std::unordered_set<int> EMPTY_INT_UNORDERED_SET;
    static const std::vector<std::shared_ptr<const std::unordered_set<int>>> EMPTY_INT_VECTOR_SHARED_UNORDERED_SET;
    static const std::set<std::pair<int, int>> EMPTY_INT_PAIR_SET;
    static const std::map<int, float> EMPTY_INT_FLOAT_MAP;
    static const std::unordered_map<int, float> EMPTY_INT_FLOAT_UNORDERED_MAP;
    static const std::unordered_map<std::pair<int, int>, float> EMPTY_MAP_PAIR_INT_TO_FLOAT;
}

const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>& SupplyManager::SupplyManagerImpl::SupplyStarlaneTraversals() const
{ return m_supply_starlane_traversals; }

const std::unordered_map<std::pair<int, int>, float>& SupplyManager::SupplyManagerImpl::SupplyStarlaneTraversals(int empire_id) const {
    const auto it = m_supply_starlane_traversals.find(empire_id);
    if (it != m_supply_starlane_traversals.end())
        return it->second;
    return EMPTY_MAP_PAIR_INT_TO_FLOAT;
}

const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>& SupplyManager::SupplyManagerImpl::SupplyObstructedStarlaneTraversals() const
{ return m_supply_starlane_obstructed_traversals; }

const std::unordered_map<std::pair<int, int>, float>& SupplyManager::SupplyManagerImpl::SupplyObstructedStarlaneTraversals(int empire_id) const {
    const auto it = m_supply_starlane_obstructed_traversals.find(empire_id);
    if (it != m_supply_starlane_obstructed_traversals.end())
        return it->second;
    return EMPTY_MAP_PAIR_INT_TO_FLOAT;
}

const std::unordered_map<int, std::unordered_set<int>>& SupplyManager::SupplyManagerImpl::FleetSupplyableSystemIDs() const
{ return m_fleet_supplyable_system_ids; }

const std::unordered_set<int>& SupplyManager::SupplyManagerImpl::FleetSupplyableSystemIDs(int empire_id) const {
    const auto& it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it != m_fleet_supplyable_system_ids.end())
        return it->second;
    return EMPTY_INT_UNORDERED_SET;
}

std::unordered_set<int> SupplyManager::SupplyManagerImpl::FleetSupplyableSystemIDs(int empire_id, bool include_allies) const {
    std::unordered_set<int> retval = FleetSupplyableSystemIDs(empire_id);
    if (!include_allies)
        return retval;

    // add supplyable systems of all allies
    for (auto empire_id_sys_id_set : m_fleet_supplyable_system_ids) {
        int other_empire_id = empire_id_sys_id_set.first;
        const auto& systems = empire_id_sys_id_set.second;
        if (systems.empty() || ((empire_id != other_empire_id)
                                && Empires().GetDiplomaticStatus(empire_id, other_empire_id) != DIPLO_PEACE))
            continue;
        retval.insert(systems.begin(), systems.end());
    }
    return retval;
}

const std::unordered_map<int, std::vector<std::shared_ptr<const std::unordered_set<int>>>>& SupplyManager::SupplyManagerImpl::ResourceSupplyGroups() const
{ return m_resource_supply_groups; }

const std::vector<std::shared_ptr<const std::unordered_set<int>>>& SupplyManager::SupplyManagerImpl::ResourceSupplyGroups(int empire_id) const {
    const auto& it = m_resource_supply_groups.find(empire_id);
    if (it != m_resource_supply_groups.end())
        return it->second;
    return EMPTY_INT_VECTOR_SHARED_UNORDERED_SET;
}

const std::unordered_map<int, float>& SupplyManager::SupplyManagerImpl::PropagatedSupplyRanges(int empire_id) const
{
    auto emp_it = m_empire_propagated_supply_ranges.find(empire_id);
    if (emp_it == m_empire_propagated_supply_ranges.end())
        return EMPTY_INT_FLOAT_UNORDERED_MAP;
    return emp_it->second;
}

const std::unordered_map<int, float>& SupplyManager::SupplyManagerImpl::PropagatedSupplyDistances(int empire_id) const {
    auto emp_it = m_empire_propagated_supply_distances.find(empire_id);
    if (emp_it == m_empire_propagated_supply_distances.end())
        return EMPTY_INT_FLOAT_UNORDERED_MAP;
    return emp_it->second;
}

const std::unordered_map<int, float>& SupplyManager::SupplyManagerImpl::SupplyStealth(int empire_id) const {
    auto emp_it = m_empire_to_system_to_stealth.find(empire_id);
    if (emp_it == m_empire_to_system_to_stealth.end()) {
        ErrorLogger() << "Supply stealth received an unexpected empire id = " << empire_id <<  " size = " << m_empire_to_system_to_stealth.size();
        return EMPTY_INT_FLOAT_UNORDERED_MAP;
    }
    return emp_it->second;
}

bool SupplyManager::SupplyManagerImpl::SystemHasFleetSupply(int system_id, int empire_id) const {
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;
    const auto& it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it == m_fleet_supplyable_system_ids.end())
        return false;
    const auto& sys_set = it->second;
    if (sys_set.find(system_id) != sys_set.end())
        return true;
    return false;
}

bool SupplyManager::SupplyManagerImpl::SystemHasFleetSupply(int system_id, int empire_id, bool include_allies) const {
    if (!include_allies)
        return SystemHasFleetSupply(system_id, empire_id);
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;

    std::set<int> empire_ids{empire_id};
    for (auto e_pair : Empires()) {
        if (e_pair.first == empire_id || Empires().GetDiplomaticStatus(empire_id, e_pair.first) == DIPLO_PEACE)
            empire_ids.insert(e_pair.first);
    }

    for (auto id : empire_ids) {
        auto sys_set_it = m_fleet_supplyable_system_ids.find(id);
        if (sys_set_it == m_fleet_supplyable_system_ids.end())
            continue;
        auto sys_set = sys_set_it->second;
        if (sys_set.find(system_id) != sys_set.end())
            return true;
    }

    return false;
}

boost::optional<const SupplyManager::system_iterator>
SupplyManager::SupplyManagerImpl::SystemSupply(const int system_id) const {
    const auto &retval =  m_system_to_empire_to_final_supply.find(system_id);
    if (retval == m_system_to_empire_to_final_supply.end())
        return boost::none;
    return boost::optional<const SupplyManager::system_iterator>(retval);
}

std::string SupplyManager::SupplyManagerImpl::Dump(int empire_id) const {
    std::string retval;

    try {
        for (const auto& empire_supply : m_fleet_supplyable_system_ids) {
            if (empire_id != ALL_EMPIRES && empire_supply.first != empire_id)
                continue;
            retval += "Supplyable systems for empire " + std::to_string(empire_supply.first) + "\n";
            for (int system_id : empire_supply.second) {
                std::shared_ptr<const System> sys = GetSystem(system_id);
                if (!sys)
                    continue;
                retval += "\n" + sys->PublicName(empire_id) + " (" + std::to_string(sys->ID()) + ") ";

                retval += "\nTraversals from here to: ";

                for (const auto& trav : m_supply_starlane_traversals.at(empire_supply.first)) {
                    if (trav.first.first == sys->ID()) {
                        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(trav.first.second);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Traversals to here from: ";
                for (const auto& trav : m_supply_starlane_traversals.at(empire_supply.first)) {
                    if (trav.first.second == sys->ID()) {
                        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(trav.first.first);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals from here to: ";
                for (const auto& trav : m_supply_starlane_obstructed_traversals.at(empire_supply.first)) {
                    if (trav.first.first == sys->ID()) {
                        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(trav.first.second);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals to here from: ";
                for (const auto& trav : m_supply_starlane_obstructed_traversals.at(empire_supply.first)) {
                    if (trav.first.second == sys->ID()) {
                        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(trav.first.first);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

            }
            retval += "\n\n";
        }

        for (const auto& empire_supply : m_resource_supply_groups) {
            retval += "Supply groups for empire " + std::to_string(empire_supply.first) + "\n";
            for (const auto& system_group : empire_supply.second) {
                retval += "group: ";
                for (int system_id : *system_group) {
                    std::shared_ptr<const System> sys = GetSystem(system_id);
                    if (!sys)
                        continue;
                    retval += "\n" + sys->PublicName(empire_id) + " (" + std::to_string(sys->ID()) + ") ";
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
    float EmpireTotalSupplyRangeSumInSystem(int empire_id, int system_id) {
        return 0.0f;
    }

    float DistanceBetweenObjects(int obj1_id, int obj2_id) {
        std::shared_ptr<const System> obj1 = GetSystem(obj1_id);
        if (!obj1)
            return 0.0f;
        std::shared_ptr<const System> obj2 = GetSystem(obj2_id);
        if (!obj2)
            return 0.0f;
        double dx = obj2->X() - obj1->X();
        double dy = obj2->Y() - obj1->Y();
        return static_cast<float>(std::sqrt(dx*dx + dy*dy));
    }

    /** Return a map from empire id to detection strength.*/
    float EmpireDetectionStrength(const int empire_id) {
        const auto empire = Empires().GetEmpire(empire_id);
        if (!empire)
            return 0.0f;

        const auto meter = empire->GetMeter("METER_DETECTION_STRENGTH");
        return meter ? meter->Current() : 0.0f;
    }

    template<typename T> void verifyType(int id, const std::string& msg = "somewhere") {
        std::shared_ptr<const T> ptr = Objects().Object<T>(id);
        if (!ptr) {
            ErrorLogger() << "Expected T " << msg;
        }
    }
}

namespace {
    /** Parts of the Update function that don't depend on SupplyManager.*/

    /** Return a bonus value applied to supply to reduce likelyhood of ties. */
    /** Return the portion of the supply bonus known from the empire and system.*/
    SupplySystemBonusTuple ComputeSystemSupplyBonuses(const int empire_id, const int sys_id)
    {
        // empires with visibility into system
        Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(sys_id, empire_id);
        float visibility_bonus = 0.0f;
        if (vis >= VIS_PARTIAL_VISIBILITY)
            visibility_bonus = 0.02f;
        else if (vis == VIS_BASIC_VISIBILITY)
            visibility_bonus = 0.01f;

        // empires with ships / planets in system (that haven't already made it obstructed for another empire)
        bool has_ship = false, has_outpost = false, has_colony = false;
        if (std::shared_ptr<const System> sys = GetSystem(sys_id)) {

            for (auto ship : Objects().FindObjects(sys->ShipIDs())) {
                if (ship && ship->OwnedBy(empire_id)) {
                    has_ship = true;
                    break;
                }
            }

            for (auto obj : Objects().FindObjects(sys->PlanetIDs())) {
                std::shared_ptr<Planet> planet = std::dynamic_pointer_cast<Planet>(obj);
                if (!planet || !planet->OwnedBy(empire_id))
                    continue;

                if (!planet->SpeciesName().empty()) {
                    has_colony = true;
                    break;
                }

                has_outpost = true;
            }
        }
        float ship_bonus = (has_ship) ? 0.1f : 0.0f;
        float colony_bonus = 0.0f;
        if (has_colony)
            colony_bonus += 0.5f;
        else if (has_outpost)
            colony_bonus += 0.3f;

        float bonus = visibility_bonus + ship_bonus + colony_bonus;

        SupplySystemBonusTuple retval = std::make_tuple(bonus, visibility_bonus, ship_bonus, colony_bonus);
        DebugLogger() << "System bonuses for system " << sys_id << " empire " << empire_id << " = "
                      << SupplySystemBonusTupleString(retval);
        return retval;
    }

    /** Return the supply bonus accounting for the propagted range and distance.*/
    float ComputeSupplyBonuses(const int empire_id, const int sys_id, std::pair<float, float> range_and_distance,
                               std::map<int, std::map<int, float>>& empire_system_supply_range_sums)
    {
        // stuff to break ties...
        auto system_bonuses = ComputeSystemSupplyBonuses(empire_id, sys_id);
        float bonus = std::get<ssbBonus>(system_bonuses);

        // sum of all supply sources in this system
        bonus += empire_system_supply_range_sums[empire_id][sys_id] / 1000.0f;

        // distance to supply source from here
        float propagated_distance_to_supply_source = std::max(1.0f, range_and_distance.second);
        bonus += 0.0001f / propagated_distance_to_supply_source;

        // todo: other bonuses?
        return bonus;
    }

    /** SupplyMerit is the merit of a single supply source in a system.  In contested systems,
        higher merit sources will win against lower merit sources.

        A merit can be < zero merit if it's distance portion is positive, or system_bonus is negative
    */
    class SupplyMerit {
        public:
        // Default constructor.
        SupplyMerit() :
            m_range(0.0f),
            m_system_bonus(std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f)),
            m_distance(0.0f)
        {}

        // Constructor for a supply source merit.
        SupplyMerit(float _range) : SupplyMerit()
        { m_range = _range; }

        // Constructor for system bonus merit.
        SupplyMerit(const SupplySystemBonusTuple& _bonus) : SupplyMerit()
        { m_system_bonus = _bonus; }

        /** Localize changes the system bonus to the system bonues of \p local. */
        SupplyMerit& Localize(const SupplyMerit& local) {
            m_system_bonus = local.m_system_bonus;
            return *this;
        }

        /** Return a new SupplyMerit based on this merit reduced by one jump of
            distance \p _dist.*/
        SupplyMerit& OneJumpLessMerit(float _dist = 0.0f) {
            m_range    -= 1.0f;
            m_distance += _dist;
            return *this;
        }

        friend bool operator<(const SupplyMerit& l, const SupplyMerit& r) {
            // All merits with negative range are equal
            if (l.m_range < 0 && r.m_range < 0)
                return false;

            // Note:: distance is reverse sorted to penalize distance sources.
            return (  std::tie(l.m_range, l.m_system_bonus, r.m_distance)
                    < std::tie(r.m_range, r.m_system_bonus, l.m_distance));
        }

        friend bool operator==(const SupplyMerit& l, const SupplyMerit& r) {
            // All merits with negative range are equal
            if (l.m_range < 0 && r.m_range < 0)
                return true;
            return (   std::tie(l.m_range, l.m_system_bonus, l.m_distance)
                    == std::tie(r.m_range, r.m_system_bonus, r.m_distance));
        }

        friend std::ostream& operator<<(std::ostream& os, const SupplyMerit& x) {
            os << '(' << x.m_range << '/' << std::get<ssbBonus>(x.m_system_bonus) << '/' << x.m_distance << ')';
            return os;
        }

        float Range() const
        {return m_range;}

        const SupplySystemBonusTuple& Bonus() const
        {return m_system_bonus;}

        float Distance() const
        {return m_distance;}

        private:
        // Number of jumps that the supply can propagate.
        float m_range;
        // A per system bonus value;
        SupplySystemBonusTuple m_system_bonus;
        // The distance along the jumps that this supply source has propagated,
        float m_distance;
    };

    // Implement the remaining relational operators.
    using namespace std::rel_ops;

    /** Return the SupplyMerit of a single universe object. This external "constructor" wrapper
        that returns none on failure avoids having to throw on error in the real constrctor.*/
    boost::optional<SupplyMerit> CalculateObjectSupplyMerit(const std::shared_ptr<const UniverseObject>& obj) {
        // Check is it owned with a valid id and a supply meter.
        if ((obj->SystemID() == INVALID_OBJECT_ID)
            || obj->Unowned()
            || !obj->GetMeter(METER_SUPPLY))
        { return boost::none; }

        // TODO: Why is this NextTurn Supply?
        float supply_range = obj->NextTurnCurrentMeterValue(METER_SUPPLY);
        return SupplyMerit(supply_range);
    }

    /** Return the SupplyMerits of all empires present in a single system. This
        external "constructor" wrapper that returns none on failure avoids
        having to throw on error in the real constrctor.*/
    boost::optional<std::map<int, SupplyMerit>> CalculateSystemSupplyMerit(const System& sys) {
        // Check is it owned.
        if (sys.Unowned())
            return boost::none;

        boost::optional<std::map<int, SupplyMerit>> retval;

        auto system_id = sys.SystemID();
        for (const auto& id_and_empire : Empires()) {
            auto system_bonuses = ComputeSystemSupplyBonuses(id_and_empire.second->EmpireID(), system_id);
            auto bonus = std::get<ssbBonus>(system_bonuses);
            if (bonus > 0.0f) {
                if (!retval)
                    retval = std::map<int, SupplyMerit>();
                retval->insert({id_and_empire.second->EmpireID(), system_bonuses});
            }
        }

        return retval;
    }

    using SupplyPODTuple = std::tuple<SupplyMerit, float, int, int>;
    enum SupplySystemPODFields {ssMerit, ssStealth, ssSource, ssEmpireID};

    /** SupplySystemPOD contains all of the data needs to resolve one systems supply conflicts. */
    struct SupplySystemPOD {
        // TODO resort and add source id?
        boost::optional<std::set<SupplyPODTuple>>   merit_stealth_supply_empire;
        boost::optional<std::unordered_set<int>>    contesting_empires;
        boost::optional<std::unordered_set<int>>    blockading_fleets_empire_ids;
        boost::optional<int>                        blockading_colonies_empire_id;
        boost::optional<std::map<int, SupplyMerit>> system_bonuses;
    };


    /** Return a map from empire id to the stealth and supply of a supply generating object.*/
    boost::optional<std::set<SupplyPODTuple>> CalculateMeritStealthSupplyEmpire(const System& system) {
        boost::optional<std::set<SupplyPODTuple>> empire_to_stealth_supply = boost::none;

        // as of this writing, only planets can generate supply propagation
        for (auto obj : Objects().FindObjects(system.PlanetIDs())) {
            auto merit = CalculateObjectSupplyMerit(obj);
            if(!merit)
                continue;

            float stealth = obj->GetMeter(METER_STEALTH) ? obj->CurrentMeterValue(METER_STEALTH) : 0;
            if (!empire_to_stealth_supply)
                empire_to_stealth_supply = std::set<SupplyPODTuple>();
            (*empire_to_stealth_supply).insert({*merit, stealth, obj->ID(), obj->Owner()});
        }

        return empire_to_stealth_supply;
    }

    // /** Return a map from empire id to the stealth and supply of a supply generating object.*/
    // boost::optional<std::unordered_map<int, std::set<std::pair<float, SupplyMerit>>>>
    // CalculateEmpireToStealthSupply(const System& system) {
    //     boost::optional<std::unordered_map<int, std::set<std::pair<float, SupplyMerit>>>>
    //         empire_to_stealth_supply = boost::none;

    //     // as of this writing, only planets can generate supply propagation
    //     for (auto obj : Objects().FindObjects(system.PlanetIDs())) {
    //         auto merit = CalculateSupplyMerit(obj);
    //         if(!merit)
    //             continue;

    //         float stealth = obj->GetMeter(METER_STEALTH) ? obj->CurrentMeterValue(METER_STEALTH) : 0;
    //         if (!empire_to_stealth_supply)
    //             empire_to_stealth_supply = std::unordered_map<int, std::set<std::pair<float, SupplyMerit>>>();
    //         (*empire_to_stealth_supply)[obj->Owner()].insert({stealth, *merit});
    //     }

    //     return empire_to_stealth_supply;
    // }

    // /** Return a map from system id to empire id to the stealth and supply of a supply generating object.*/
    // std::unordered_map<int, std::unordered_map<int, std::set<std::pair<float, SupplyMerit>>>> CalculateSystemToEmpireToStealthSupply() {
    //     std::unordered_map<int, std::unordered_map<int, std::set<std::pair<float, SupplyMerit>>>> system_to_empire_to_stealth_supply;

    //     for (auto system_it : Objects().ExistingSystems()) {
    //         if (auto system = std::dynamic_pointer_cast<const System>(system_it.second)) {
    //             if (auto stealth_supply = CalculateEmpireToStealthSupply(*system))
    //                 system_to_empire_to_stealth_supply[system->SystemID()] = *stealth_supply;
    //         }
    //     }

    //     return system_to_empire_to_stealth_supply;
    // }

    /** Return sets of contestings and blockading fleet's empire ids.

        The fleets determined to be blocking by this function may be different than
        blocking fleets determined by an empire, because a blockading fleet may not
        be visible to a given empire.
    */
    std::pair<boost::optional<std::unordered_set<int>>, boost::optional<std::unordered_set<int>>> CalculateBlockadingFleetsEmpireIDs(const System& system)
    {
        boost::optional<std::unordered_set<int>> contesting_fleets_empire_ids;
        boost::optional<std::unordered_set<int>> blockading_fleets_empire_ids;

        // For a fleet to be blockading or contesting it must be in an aggresive stance, armed with
        // direct weapons or fighters, present in a system.  If more than one mutually hostile fleet is
        // in a system the fleet that arrived first (and its allies) are blockading and other fleets
        // are contesting the blockade.  First arrival is determined by unrestricted starlane access.

        // Unrestricted lane access (i.e, (fleet->ArrivalStarlane() == system->ID()) ) is used as a proxy for
        // order of arrival -- if an enemy has unrestricted lane access and you don't, they must have arrived
        // before you, or be in cahoots with someone who did.

        // TODO: the previous message about unrestricted lane access is repeated in enough locations to warrant its
        // own function in the Fleet class.

        for (auto fleet_it : Objects().FindObjects(system.FleetIDs())) {
            auto fleet = std::dynamic_pointer_cast<const Fleet>(fleet_it);
            if (!fleet || fleet->SystemID() != system.SystemID())
                continue;

            auto is_aggressive = ((fleet->HasArmedShips() || fleet->HasFighterShips()) && fleet->Aggressive());
            auto is_stopped = fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID();
            auto is_unrestricted = fleet->ArrivalStarlane() == fleet->SystemID();
            auto is_contesting = is_aggressive && is_stopped;
            auto is_blockading = is_aggressive && is_stopped && is_unrestricted;

            if (is_contesting) {
                if (!contesting_fleets_empire_ids)
                    contesting_fleets_empire_ids = std::unordered_set<int>();
                contesting_fleets_empire_ids->insert(fleet->Owner());
            }

            if (is_blockading) {
                if (!blockading_fleets_empire_ids)
                    blockading_fleets_empire_ids = std::unordered_set<int>();
                blockading_fleets_empire_ids->insert(fleet->Owner());
            }
        }

        return {contesting_fleets_empire_ids, blockading_fleets_empire_ids};
    }

    /** Return a map from system to blockading fleet's empire id.

        The fleets determined to be blocking by this function may be different than
        blocking fleets determined by an empire, because a blockading fleet may not
        be visible to a given empire.
    */
    std::pair<std::unordered_map<int, std::unordered_set<int>>,
              std::unordered_map<int, std::unordered_set<int>>> CalculateSystemToBlockadingFleetsEmpireIDs()
    {
        std::unordered_map<int, std::unordered_set<int>> system_to_contesting_fleets_empire_ids;
        std::unordered_map<int, std::unordered_set<int>> system_to_blockading_fleets_empire_ids;

        for (auto system_it : Objects().ExistingSystems()) {
            auto system = std::dynamic_pointer_cast<const System>(system_it.second);

            if (!system)
                continue;

            boost::optional<std::unordered_set<int>> contesting_empires;
            boost::optional<std::unordered_set<int>> blockading_fleets_empire_ids;
            std::tie(contesting_empires, blockading_fleets_empire_ids) = CalculateBlockadingFleetsEmpireIDs(*system);

            if (contesting_empires && !contesting_empires->empty())
                system_to_contesting_fleets_empire_ids[system->SystemID()]= *contesting_empires;

            if (blockading_fleets_empire_ids && !blockading_fleets_empire_ids->empty())
                system_to_blockading_fleets_empire_ids[system->SystemID()] = *blockading_fleets_empire_ids;
        }
        return {system_to_contesting_fleets_empire_ids, system_to_blockading_fleets_empire_ids};
    }

    /** Return an optional empire id if any empires colonies can blockade supply.

        A colony can blockade supply if only one empire has supply in the system and no hostile empires
        are contesting the system.

        A colony's ability to blockade supply was added to relax requirements on the AI understanding
        supply mechanics.  It may be removed in future.

        The systems determined to be blocked by this function may be different than
        determined by an empire, because empires don't have complete visibility.
    */
    boost::optional<int> CalculateBlockadingColonyEmpireID(
        const System& system,
        const boost::optional<std::set<SupplyPODTuple>>& empire_to_stealth_supply,
        const boost::optional<std::unordered_set<int>>& contesting_fleets_empire_ids)
    {
        // No supply in system, so no blockading colony.
        if (!empire_to_stealth_supply)
            return boost::none;
        auto empire_id = std::get<ssEmpireID>(*empire_to_stealth_supply->begin());

        // Check that all the source have the same empire id.
        if (!std::all_of(empire_to_stealth_supply->begin(), empire_to_stealth_supply->end(),
                        [&](const SupplyPODTuple& stuff_empire){
                             return std::get<ssEmpireID>(stuff_empire) == empire_id;
                         })) {
            return boost::none;
        }

        // Are any hostile fleets contesting control of this system?
        if (!contesting_fleets_empire_ids)
            return empire_id;
        if (std::none_of(
                contesting_fleets_empire_ids->begin(), contesting_fleets_empire_ids->end(),
                [=](int fleet_owner) {
                    return ((fleet_owner != empire_id)
                            && (Empires().GetDiplomaticStatus(empire_id, fleet_owner) == DIPLO_WAR));
                }))
            return empire_id;

        return boost::none;
    }

    /** Return a map from system to blockading colony's empire id.

        A colony's ability to blockade supply was added to relax requirements on the AI understanding
        supply mechanics.  It may be removed in future.

        The systems determined to be blocked by this function may be different than
        determined by an empire, because empires don't have complete visibility.
    */
    std::unordered_map<int, std::unordered_set<int>> CalculateSystemToBlockadingColonyEmpireIDs(
        const std::unordered_map<int, std::set<SupplyPODTuple>>& system_to_empire_to_stealth_supply,
        const std::unordered_map<int, std::unordered_set<int>>& system_to_contesting_fleets_empire_ids)
    {
        std::unordered_map<int, std::unordered_set<int>> system_to_blockading_colony_empire_ids;

        for (auto system_to_empire_to_stealth_supply_it : system_to_empire_to_stealth_supply) {
            auto system_id = system_to_empire_to_stealth_supply_it.first;
            auto system = std::dynamic_pointer_cast<const System>(Objects().Object(system_id));

            if (!system)
                continue;

            // Are any hostile fleets contesting control of this system?
            auto fleets_it = system_to_contesting_fleets_empire_ids.find(system_id);
            if (fleets_it == system_to_contesting_fleets_empire_ids.end())
                continue;

            auto maybe_blockader_id = CalculateBlockadingColonyEmpireID(
                *system, system_to_empire_to_stealth_supply_it.second,
                fleets_it->second);

            if (maybe_blockader_id)
                system_to_blockading_colony_empire_ids[system_id].insert(*maybe_blockader_id);
        }

        return system_to_blockading_colony_empire_ids;
    }

    /** Return a map from system id to all initial sources of supply. */
    // TODO change to vector
    std::unordered_map<int, std::set<SupplyPODTuple>> CalculateInitialSupplySources() {
        std::unordered_map<int, std::set<SupplyPODTuple>> system_to_supply_pod;

        for (auto system_it : Objects().ExistingSystems()) {
            auto system = std::dynamic_pointer_cast<const System>(system_it.second);
            if (!system)
                continue;

            if (auto stealth_supply = CalculateMeritStealthSupplyEmpire(*system))
                system_to_supply_pod[system->SystemID()] = *stealth_supply;
        }

        return system_to_supply_pod;
    }

    /** CalculateInitialSystemConditions returns a SupplySystemPOD for \p system populated with
        everything except the local system supply, blockading fleets, contesting fleets,
        blockading planets and local bonuses. */
    boost::optional<SupplySystemPOD> CalculateInitialSystemConditions(
        const System &system, const boost::optional<std::set<SupplyPODTuple>>& stealth_supply)
    {
        boost::optional<std::unordered_set<int>> contesting_empires;
        boost::optional<std::unordered_set<int>> blockading_fleets_empire_ids;
        std::tie(contesting_empires, blockading_fleets_empire_ids) = CalculateBlockadingFleetsEmpireIDs(system);

        auto blockading_colony = CalculateBlockadingColonyEmpireID(system, stealth_supply, contesting_empires);

        auto local_bonuses = CalculateSystemSupplyMerit(system);

        if (!contesting_empires && !blockading_fleets_empire_ids && !blockading_colony && !local_bonuses)
            return boost::none;

        return SupplySystemPOD({boost::optional<std::set<SupplyPODTuple>>(),
                    contesting_empires, blockading_fleets_empire_ids, blockading_colony, local_bonuses});
    }

    /** Return a map from system id to SupplySystemrPOD populated with
        everything except the local system supply, blockading fleets, contesting fleets,
        blockading planets and local bonuses. \p system_to_supply are the per system initial
        supply sources. */
    std::unordered_map<int, SupplySystemPOD> CalculateInitialSystemConditions(
        const std::unordered_map<int, std::set<SupplyPODTuple>>& system_to_supply)
    {
        std::unordered_map<int, SupplySystemPOD> system_to_supply_pod;

        for (auto system_it : Objects().ExistingSystems()) {
            auto system = std::dynamic_pointer_cast<const System>(system_it.second);
            if (!system)
                continue;

            // Find any local supply sources
            boost::optional<std::set<SupplyPODTuple>> maybe_local_supply;
            const auto& local_supply_it = system_to_supply.find(system->SystemID());
            if (local_supply_it != system_to_supply.end())
                maybe_local_supply = local_supply_it->second;

            if (auto pod = CalculateInitialSystemConditions(*system, maybe_local_supply))
                system_to_supply_pod[system->SystemID()] = *pod;
        }

        return system_to_supply_pod;
    }

    /**Localize (modify) the \p supply_sources with local factors in \p supply_pod. */
    void LocalizeSupply(std::set<SupplyPODTuple>& supply_sources, const SupplySystemPOD& supply_pod) {
        // Adjust merit for local bonuses if needed
        auto local_bonuses = supply_pod.system_bonuses;
        if (!local_bonuses)
            return;

        // List of merits that need to be adjusted
        std::vector<std::set<SupplyPODTuple>::const_iterator> erase_these;
        std::vector<SupplyPODTuple> add_these;
        for (auto supply_tuple_it = supply_sources.begin();
             supply_tuple_it != supply_sources.end(); ++supply_tuple_it)
        {
            const auto& local_empire = std::get<ssEmpireID>(*supply_tuple_it);
            const auto& bonus_it = local_bonuses->find(local_empire);
            if (bonus_it == local_bonuses->end())
                continue;

            auto local_supply_tuple = *supply_tuple_it;
            SupplyMerit& local_merit = std::get<ssMerit>(local_supply_tuple);
            local_merit.Localize(bonus_it->second);

            erase_these.push_back(supply_tuple_it);
            add_these.push_back(local_supply_tuple);

            DebugLogger() << "Localizing merit from " << std::get<ssMerit>(*supply_tuple_it)
                          << " to " << std::get<ssMerit>(local_supply_tuple)
                          << " with local bonus " << bonus_it->second;
        }

        // Adjust the set
        // TODO does this need to be a set?
        for (const auto& erase_it : erase_these)
            supply_sources.erase(erase_it);
        for(const auto& add_this : add_these)
            supply_sources.insert(add_this);
    }

    /** Modify \p system_to_supply to be localize by local bonus factors in \p system_to_supply_pod. */
    void LocalizeSupply(
        std::unordered_map<int, std::set<SupplyPODTuple>>& system_to_supply,
        const std::unordered_map<int, SupplySystemPOD>& system_to_supply_pod)
    {
        for (auto& system_and_supply : system_to_supply) {
            const auto&  system_to_supply_pod_it = system_to_supply_pod.find(system_and_supply.first);
            if (system_to_supply_pod_it == system_to_supply_pod.end())
                continue;

            LocalizeSupply(system_and_supply.second, system_to_supply_pod_it->second);
        }
    }

    boost::optional<SupplySystemPOD> CalculateInitialSupply(const System &system) {
        auto stealth_supply = CalculateMeritStealthSupplyEmpire(system);

        boost::optional<std::unordered_set<int>> contesting_empires;
        boost::optional<std::unordered_set<int>> blockading_fleets_empire_ids;
        std::tie(contesting_empires, blockading_fleets_empire_ids) = CalculateBlockadingFleetsEmpireIDs(system);

        auto blockading_colony = CalculateBlockadingColonyEmpireID(system, stealth_supply, contesting_empires);

        auto local_bonuses = CalculateSystemSupplyMerit(system);

        // Adjust merit for local bonuses if needed
        if (stealth_supply && local_bonuses) {
            // List of merits that need to be adjusted
            std::vector<std::set<SupplyPODTuple>::const_iterator> erase_these;
            std::vector<SupplyPODTuple> add_these;
            for (auto supply_tuple_it = stealth_supply->begin();
                 supply_tuple_it != stealth_supply->end(); ++supply_tuple_it)
            {
                const auto& local_empire = std::get<ssEmpireID>(*supply_tuple_it);
                const auto& bonus_it = local_bonuses->find(local_empire);
                if (bonus_it == local_bonuses->end())
                    continue;

                auto local_supply_tuple = *supply_tuple_it;
                SupplyMerit& local_merit = std::get<ssMerit>(local_supply_tuple);
                local_merit.Localize(bonus_it->second);

                erase_these.push_back(supply_tuple_it);
                add_these.push_back(local_supply_tuple);

                DebugLogger() << "Localizing merit in system "<< system.SystemID() << " from "
                              << std::get<ssMerit>(*supply_tuple_it) << " to "
                              << std::get<ssMerit>(local_supply_tuple) << " with local bonus "
                              << bonus_it->second;
            }

            // Adjust the set
            for (const auto& erase_it : erase_these)
                stealth_supply->erase(erase_it);
            for(const auto& add_this : add_these)
                stealth_supply->insert(add_this);
        }

        if (!stealth_supply && !contesting_empires && !blockading_fleets_empire_ids && !blockading_colony && !local_bonuses)
            return boost::none;

        return SupplySystemPOD({stealth_supply, contesting_empires, blockading_fleets_empire_ids,
                    blockading_colony, local_bonuses});
    }

    /** Return a map from system id to SupplySystemrPOD. */
    std::unordered_map<int, SupplySystemPOD> CalculateInitialSupply() {
        std::unordered_map<int, SupplySystemPOD> system_to_supply_pod;

        for (auto system_it : Objects().ExistingSystems()) {
            auto system = std::dynamic_pointer_cast<const System>(system_it.second);
            if (!system)
                continue;

            if (auto pod = CalculateInitialSupply(*system))
                system_to_supply_pod[system->SystemID()] = *pod;
        }

        return system_to_supply_pod;
    }

    /** SupplyTranches are sets of supply sources divided by merit into tranches starting from the
        maximum merit of one starlane hop reduction in merit magnitude.
    */
    class SupplyTranche {
        public:
        SupplyTranche(const SupplyMerit& threshold) :
            m_lower_threshold(threshold),
            m_system_to_source_to_merit_stealth_empire(),
            m_num_sources{0}
        {}

        void Add(int source_id, const SupplyMerit& merit, float stealth, int system_id, int empire_id) {
            if (merit < m_lower_threshold) {
                ErrorLogger() << "source " << merit << " lower than merit threshold " << m_lower_threshold <<". Skipping ...";
                return;
            }
            m_system_to_source_to_merit_stealth_empire[system_id].insert({source_id, {merit, stealth, empire_id}});
            ++m_num_sources;
            DebugLogger() << "Tranche::Add() sys = " << system_id << " source = " << source_id
                          << " stealth = "<< stealth<< " empire = " << empire_id << " merit = "
                          << merit <<" new size = " << m_num_sources << ".";
        }

        void Remove(int system_id, int source_id) {
            auto system_it = m_system_to_source_to_merit_stealth_empire.find(system_id);
            if (system_it == m_system_to_source_to_merit_stealth_empire.end())
                return;

            DebugLogger() << "Found and removing from system " << system_it->first << "source id " << source_id;
            system_it->second.erase(source_id);
            --m_num_sources;

            system_it = m_system_to_source_to_merit_stealth_empire.find(system_id);
            if (system_it == m_system_to_source_to_merit_stealth_empire.end())
                return;
            ErrorLogger() << "Found more?";
        }

        std::unordered_map<int, std::unordered_map<int, std::tuple<SupplyMerit, float, int>>>::iterator begin()
        {
            DebugLogger() << "Tranche.begin() called with " << m_num_sources << " in "
                          << m_system_to_source_to_merit_stealth_empire.size()
                          << " systems in tranche with threshold " << m_lower_threshold;

            return m_system_to_source_to_merit_stealth_empire.begin();
        }

        std::unordered_map<int, std::unordered_map<int, std::tuple<SupplyMerit, float, int>>>::iterator end()
        { return m_system_to_source_to_merit_stealth_empire.end(); }

        /** Return the threshold.*/
        const SupplyMerit& Threshold() const
        { return m_lower_threshold; }

        private:
        SupplyMerit m_lower_threshold;
        std::unordered_map<int, std::unordered_map<int, std::tuple<SupplyMerit, float, int>>>
        m_system_to_source_to_merit_stealth_empire;
        std::size_t m_num_sources;
    };

    class SupplyTranches {
        public:
        SupplyTranches(const std::unordered_map<int, SupplySystemPOD>& system_to_supply_pod) :
            m_tranches{},
            m_max_merit{SupplyMerit()}
        {
            // Sort by SupplyMerit
            std::set<std::pair<SupplyMerit, std::shared_ptr<UniverseObject>>> merit_and_source;
            for (const auto& system_id_and_pod : system_to_supply_pod) {
                if (!system_id_and_pod.second.merit_stealth_supply_empire)
                    continue;
                for (const auto& supply_tuple : *system_id_and_pod.second.merit_stealth_supply_empire) {
                    const auto& merit = std::get<ssMerit>(supply_tuple);
                    const auto& source_id = std::get<ssSource>(supply_tuple);
                    const auto& obj = Objects().Object(source_id);
                    if (!obj)
                        continue;
                    merit_and_source.insert({merit, obj});
                    DebugLogger() << " Tranche found merit " << merit << " for obj " << obj->ID();
                }
            }

            if (merit_and_source.empty()) {
                return;
            }

            // Divide into tranches of the minimum drop in merit across a single starlane.
            m_max_merit = merit_and_source.rbegin()->first;

            auto merit_source_it = merit_and_source.rbegin();
            auto merit_threshold = SupplyMerit(m_max_merit).OneJumpLessMerit();
            auto tranche = SupplyTranche(merit_threshold);

            SupplyMerit merit;
            std::shared_ptr<UniverseObject> source;

            DebugLogger() << "SupplyTranches() adding " << merit_and_source.size() << " supply sources to tranches.";
            while (merit_source_it != merit_and_source.rend()) {
                std::tie(merit, source) = *merit_source_it;

                if (merit <= merit_threshold) {
                    // Start a new tranche: Save the old, change the threshold, start a new tranche
                    m_tranches.insert({merit_threshold, tranche});
                    merit_threshold.OneJumpLessMerit();
                    tranche = SupplyTranche(merit_threshold);
                    DebugLogger() << "SupplyTranches() start new tranche with merit " << merit_threshold << ".";
                }

                // Add to the tranche
                float stealth = source->GetMeter(METER_STEALTH) ? source->CurrentMeterValue(METER_STEALTH) : 0;
                tranche.Add(source->ID(), merit, stealth, source->SystemID(), source->Owner());
                DebugLogger() << "SupplyTranches() add " << source->ID() << " with  merit " << merit
                              << " to tranche with threshold " << merit_threshold;

                ++merit_source_it;
            }

            m_tranches.insert({merit_threshold, tranche});

            DebugLogger() << "SupplyTranches() built " << m_tranches.size() << " tranches with data.";
            // Create the remaining tranches down to lower of zero merit or the minimum detected
            // merit reduced by one jump of merit reduction.
            auto zero_merit = SupplyMerit();
            const auto min_merit = SupplyMerit(std::min(zero_merit, merit_and_source.begin()->first)).OneJumpLessMerit();

            while (merit_threshold > min_merit) {
                merit_threshold.OneJumpLessMerit();
                m_tranches.insert({merit_threshold, SupplyTranche(merit_threshold)});
                DebugLogger() << "SupplyTranches() add empty tranche built " << merit_threshold << " threshold.";
            }
            DebugLogger() << "SupplyTranches() built " << m_tranches.size() << " tranches max merit "
                          << m_max_merit << " min merit " << m_tranches.begin()->first;
        }

        /** Add these sources to the correct tranche*/
        void Add(std::map<SupplyMerit, std::tuple<int, float, int, int>>& merit_to_source_stealth_system_empire) {
            for (auto&& merit_and_source_stealth_system_empire : merit_to_source_stealth_system_empire) {
                operator[](merit_and_source_stealth_system_empire.first).Add(
                    std::get<0>(merit_and_source_stealth_system_empire.second),
                    merit_and_source_stealth_system_empire.first,
                    std::get<1>(merit_and_source_stealth_system_empire.second),
                    std::get<2>(merit_and_source_stealth_system_empire.second),
                    std::get<3>(merit_and_source_stealth_system_empire.second));
            }
        }

        /** Return the tranche that would hold a source with \p merit.*/
        SupplyTranche& operator[](const SupplyMerit& merit) {
            auto it = m_tranches.lower_bound(merit);
            if (it != m_tranches.begin())
                --it;
            if (it == m_tranches.end()) {
                ErrorLogger() << "SupplyTranches()[" << merit << "] found end.";
                auto merit_minus_one = SupplyMerit(merit).OneJumpLessMerit();
                m_tranches.insert({merit_minus_one, SupplyTranche(merit_minus_one)});
                return m_tranches.begin()->second;

            }
                
            //DebugLogger() << "SupplyTranches()[" << merit << "] found " << it->first << " threshold. ";

            //Expected exit
            if (it->first >= SupplyMerit(merit).OneJumpLessMerit()) {
                //DebugLogger() << "SupplyTranches()[" << merit << "] returned " << it->first << " threshold. ";
                return it->second;
            }

            if (it == m_tranches.begin()) {
                ErrorLogger() << "SupplyTranches()[" << merit << "] Unable to find tranche. Creating a less than zero tranche.";
                auto lower_than_zero_merit = SupplyMerit(it->first).OneJumpLessMerit();
                m_tranches.insert({lower_than_zero_merit, SupplyTranche(lower_than_zero_merit)});
                return m_tranches.begin()->second;
            }

            ErrorLogger() << "SupplyTranches()[" << merit << "] found nothing.  Max merit is " << m_max_merit
                          << "Making a tranche for this merit. ";

            auto merit_threshold = SupplyMerit(merit).OneJumpLessMerit();
            m_tranches.insert({merit_threshold, SupplyTranche(merit_threshold)});
            return m_tranches.begin()->second;
        }

        std::size_t Size() const
        { return m_tranches.size(); }

        std::map<SupplyMerit, SupplyTranche>::iterator begin() { return m_tranches.begin(); }
        std::map<SupplyMerit, SupplyTranche>::iterator end()   { return m_tranches.end(); }

        void Remove(const int system_id, int source_id, const SupplyMerit& merit) {
            operator[](merit).Remove(system_id, source_id);
        }

        const SupplyMerit& MaxMerit() const
        {return m_max_merit;}

        private:
        std::map<SupplyMerit, SupplyTranche> m_tranches;
        SupplyMerit m_max_merit;


    };

    // boost::optional<SupplySystemPOD> CalculateInitialSupply(
    //     const System &system,
    //     const std::set<std::pair<SupplyMerit, float>>& merit_and_stealth)
    // {
    //     auto stealth_supply = CalculateEmpireToStealthSupply(system);

    //     boost::optional<std::unordered_set<int>> contesting_empires;
    //     boost::optional<std::unordered_set<int>> blockading_fleets_empire_ids;
    //     std::tie(contesting_empires, blockading_fleets_empire_ids) = CalculateBlockadingFleetsEmpireIDs(system);

    //     auto blockading_colony = CalculateBlockadingColonyEmpireID(system, stealth_supply, contesting_empires);

    //     if (!stealth_supply && !contesting_empires && !blockading_fleets_empire_ids && !blockading_colony)
    //         return boost::none;

    //     return SupplySystemPOD({stealth_supply, contesting_empires, blockading_fleets_empire_ids, blockading_colony});
    // }

    // // /** Return a map from system id to SupplySystemPOD. */
    // std::unordered_map<int, SupplySystemPOD> CalculateSupplySystemPODs(
    //     const std::set<std::pair<SupplyMerit, std::shared_ptr<UniverseObject>>>& merit_to_obj)
    // {
    //     std::unordered_map<int, std::set<std::pair<SupplyMerit, float>>> system_to_supply_merits;
    //     for (auto merit_and_obj : merit_to_obj) {
    //         const std::shared_ptr<UniverseObject>& obj = merit_and_obj.second;
    //         if (obj->SystemID() == INVALID_OBJECT_) {
    //             ErrorLogger() << "An supply source with id " << obj->ID() << " has an invalid system id.";
    //             continue;
    //         }

    //         const SupplMerit& merit = merit_and_obj.first;
    //         float stealth = obj->GetMeter(METER_STEALTH) ? obj->CurrentMeterValue(METER_STEALTH) : 0;

    //         system_to_supply_merits[obj->SystemID()].insert({merit, stealth});
    //     }

    //     std::unordered_map<int, SupplySystemPOD> system_to_supply_pod;

    //     for (auto system_id_and_merits : system_to_supply_merits) {
    //         auto system = std::dynamic_pointer_cast<const System>(system_id_and_merits.first);
    //         if (!system)
    //             continue;

    //         if (auto pod = CalculateInitialSupply(*system, system_id_and_merits.second))
    //             system_to_supply_pod[system->SystemID()] = *pod;
    //     }

    //     return system_to_supply_pod;
    // }

    /** Remove \p sys_id from unobstructed_systems, remove all startlanes that arrive or depart sys_id
        from lane_traversals and remove obstructed traversals that depart this system.*/
    void RemoveSystemFromTraversals(
        const int sys_id,
        std::unordered_map<std::pair<int, int>, float>& lane_traversals,
        std::unordered_map<std::pair<int, int>, float>& obstructed_traversals)
    {
        auto lane_traversals_initial = lane_traversals;
        auto obstructed_traversals_initial = obstructed_traversals;

        // remove from traversals departing from or going to this system for this empire,
        // and set any traversals going to this system as obstructed
        for (const auto& lane : lane_traversals_initial) {
            if (lane.first.first == sys_id) {
                lane_traversals.erase({sys_id, lane.first.second});
            }
            if (lane.first.second == sys_id) {
                lane_traversals.erase({lane.first.first, sys_id});
                obstructed_traversals.insert({{lane.first.first, sys_id}, 0.0f});
            }
        }

        // remove obstructed traverals departing from this system
        for (const auto& lane : obstructed_traversals_initial) {
            if (lane.first.first == sys_id)
                obstructed_traversals.erase(lane.first);
        }
    }

    /** Add an obstructed traversal.*/
    void AddObstructedTraversal(
        const int sys_id, const int end_sys_id,
        const std::set<int>& supply_unobstructed_systems,
        const std::unordered_map<std::pair<int, int>, float>& lane_traversals,
        std::unordered_map<std::pair<int, int>, float>& obstructed_traversals)
    {
        obstructed_traversals.insert({{sys_id, end_sys_id}, 0.0f});
    }

    /** Add a traversal */
    void AddTraversal(
        const int sys_id, const int end_sys_id,
        const std::set<int>& supply_unobstructed_systems,
        std::unordered_map<std::pair<int, int>, float>& lane_traversals,
        std::unordered_map<std::pair<int, int>, float>& obstructed_traversals)
    {

        //DebugLogger() << "Added traversal from " << sys_id << " to " << end_sys_id;
        // always record a traversal, so connectivity is calculated properly
        lane_traversals.insert({{sys_id, end_sys_id}, 0.0f});

        // erase any previous obstructed traversal that just succeeded
        if (obstructed_traversals.find({sys_id, end_sys_id}) !=
            obstructed_traversals.end())
        {
            //DebugLogger() << "Removed obstructed traversal from " << sys_id << " to " << end_sys_id;
            obstructed_traversals.erase({sys_id, end_sys_id});
        }
        if (obstructed_traversals.find({end_sys_id, sys_id}) !=
            obstructed_traversals.end())
        {
            //DebugLogger() << "Removed obstructed traversal from " << end_sys_id << " to " << sys_id;
            obstructed_traversals.erase({end_sys_id, sys_id});
        }
    }

    /** Resolve the supply conflicts in a single \p system between the \p candidates and other
        supply source in the system \p pod and return a pair of winners and losers.

        Winners are candidates who can propagate to other systems if they have more range.
        Losers are supply sources removed from the system by this contest.
    */
    std::pair<std::vector<SupplyPODTuple>,
              std::unordered_map<int, SupplyMerit>>
    ContestSystem(
        SupplySystemPOD& pod, int system_id,
        const std::unordered_map<int, std::tuple<SupplyMerit, float, int>>& _candidates
        )
    {
        // Supply resolution works as follows.  Each supply source has a (merit,
        // stealth).  Each empire has a detection stength.  A supply source with
        // greater or equal merit will remove another supply source if its
        // empire's detection strength is greater that the other's stealth and
        // the empires are hostile.

        if (_candidates.empty()) {
            ErrorLogger() << "Supply ContestSystem passed " << _candidates.size()
                          << " candidates into system " << system_id << ".";
            return {{}, {}};
        }

        DebugLogger() << " Contesting system "<<system_id<<" with " << _candidates.size() << " candidates.";

        // Massage the _candidates into a sorted set of candidates.
        std::set<SupplyPODTuple> candidates;
        for (auto&& source_and_merit_stealth_empire : _candidates) {
            candidates.insert({std::get<ssMerit>(source_and_merit_stealth_empire.second),
                        std::get<ssStealth>(source_and_merit_stealth_empire.second),
                        source_and_merit_stealth_empire.first,
                        std::get<ssSource>(source_and_merit_stealth_empire.second)});
        }

        // Sources that have been removed.
        std::unordered_map<int, SupplyMerit> losers;

        // Candidates that have not been removed.
        std::vector<SupplyPODTuple> winners;

        // There are no empires here.
        if (!pod.merit_stealth_supply_empire || pod.merit_stealth_supply_empire->empty()) {
            if (!candidates.empty())
                ErrorLogger() << "Supply ContestSystem passed " << candidates.size()
                              << " into empty system " << system_id << ".";
            return {{}, {}};
        }

        for (auto candidate_it = candidates.rbegin(); candidate_it != candidates.rend(); ++candidate_it) {
            auto& candidate = *candidate_it;
            const SupplyMerit& candidate_merit = std::get<ssMerit>(candidate);
            float candidate_stealth;
            int candidate_source_id, candidate_empire_id;
            std::tie(std::ignore, candidate_stealth, candidate_source_id, candidate_empire_id) = candidate;

            // Find this empire's detection strength
            auto candidate_detection = EmpireDetectionStrength(candidate_empire_id);

            if (pod.merit_stealth_supply_empire->find(candidate) == pod.merit_stealth_supply_empire->end()) {
                ErrorLogger() << "Supply ContestSystem passed a candidate" << candidate_source_id
                              << " not present in system " << system_id << ". Skipping...";
                continue;
            }

            DebugLogger() << " Contesting candidate " << candidate_source_id << " from empire "
                          << candidate_empire_id << " with merit " << candidate_merit << " and stealth "
                          << candidate_stealth << " detection " << candidate_detection << ".";

            // Conditionally add candidate to the keepers.  It might be removed if tied for range.
            bool is_keep_candidate = true;

            // A list of individual others to remove from the list.
            std::vector<std::set<SupplyPODTuple>::reverse_iterator> marked_for_removals;

            for (auto other_it = pod.merit_stealth_supply_empire->rbegin();
                 other_it != pod.merit_stealth_supply_empire->rend(); ++other_it)
            {
                const SupplyMerit& other_merit = std::get<ssMerit>(*other_it);
                float other_stealth;
                int other_source_id, other_empire_id;
                std::tie(std::ignore, other_stealth, other_source_id, other_empire_id) = *other_it;

                DebugLogger() << " against other " << other_source_id << " from empire "
                              << other_empire_id << " with merit " << other_merit << " and stealth "
                              << other_stealth << ".";

                // Same source looping back
                if (other_source_id == candidate_source_id){
                    // Same source looping back
                    if (other_merit > candidate_merit) {
                        DebugLogger() << " Lost to loopback ";
                        is_keep_candidate = false;
                        break;
                    }
                    continue;
                }

                // Same empire
                if (other_empire_id == candidate_empire_id) {
                    // Other's merit and stealth are bigger so remove the candidate
                    if ((other_merit >= candidate_merit) && (other_stealth >= candidate_stealth)) {
                        DebugLogger() << " Lost to same empire ";
                        is_keep_candidate = false;
                        break;
                    }

                    // Other has less merit and less stealth so it can never
                    // improve the candidate's empire's situation by having a
                    // stealth higher than an opponents detection.  Remove it.
                    if (other_merit <= candidate_merit && other_stealth <= candidate_stealth) {
                        DebugLogger() << " Won to same empire ";
                        marked_for_removals.push_back(other_it);
                    }
                    continue;
                }

                // If other is not hostile to candidate empire then skip it
                if (Empires().GetDiplomaticStatus(candidate_empire_id, other_empire_id) != DIPLO_WAR)
                    continue;

                // Other has greater or equal merit. So check if the other can remove the candidate.
                if (other_merit >= candidate_merit) {
                    auto other_detection = EmpireDetectionStrength(other_empire_id);

                    // If other can't detect the candidate, skip it.
                    if (candidate_stealth > other_detection) {
                        // Intentional, do nothing and fall through for the equal merit case
                    }

                    // If other has the same merit then remove the candidate
                    else if (other_merit == candidate_merit) {
                        DebugLogger() << " Lost to equal merit.  Should fall through ";
                        is_keep_candidate = false;
                        // Intentional, do nothing and fall through for the equal merit case
                    }

                    else {
                        // Detectable, hostile and have less merite so remove candidate.
                        DebugLogger() << " Lost to less merit. ";
                        is_keep_candidate = false;
                        continue;
                    }
                }

                // Other has less or equal merit.  So check if the candidate
                // removes the other.
                if (other_merit <= candidate_merit) {
                    // If other can't be detected then skip it
                    if (other_stealth > candidate_detection)
                        continue;

                    // If other has the same merit then remove both the candidate and the other
                    if (other_merit == candidate_merit) {
                        DebugLogger() << " Won to equal merit.  Should both be removed. ";
                        marked_for_removals.push_back(other_it);
                        continue;
                    }

                    // All remaining others are detectable, hostile and have less merit so remove them.
                    DebugLogger() << " Won.";
                    marked_for_removals.push_back(other_it);
                }

                // If the candidate was removed stop checking to see if it can remove smaller merits
                if (!is_keep_candidate)
                    break;
            }

            // Did the candidate win/lose?
            if (is_keep_candidate)
                winners.push_back(candidate);
            else
                losers.insert({candidate_source_id, candidate_merit});

            // Were any of the others marked for removal
            for (auto marked : marked_for_removals) {
                const SupplyMerit& marked_merit = std::get<ssMerit>(*marked);
                int marked_source_id, marked_empire_id;
                std::tie(std::ignore, std::ignore, marked_source_id, marked_empire_id) = *marked;

                losers.insert({marked_source_id, marked_merit});

                // Remove the collected **reverse** iterator.  Should be O(1).
                pod.merit_stealth_supply_empire->erase(std::next(marked).base());
            }

            // Reset
            is_keep_candidate = true;
            marked_for_removals.clear();
        }
        return {winners, losers};
    }

    /** Propagate the \p propagators from \p system_id through all starlanes visible to their
        empire in \p empire_to_system_to_starlanes. Return the newly added supply elements.*/
    boost::optional<std::map<SupplyMerit, std::tuple<int, float, int, int>>> AttemptToPropagate(
        std::unordered_map<int, SupplySystemPOD>& system_to_supply_pod,
        SupplyTranches& tranches,
        const int system_id,
        const std::vector<SupplyPODTuple>& propagators,
        std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>>& empire_to_system_to_starlanes)
    {
        // Return a set of winners that will propagate further and a set of losers that will not.
        boost::optional<std::map<SupplyMerit, std::tuple<int, float, int, int>>> retval;

        // For each propagator
        for (auto&& propagator : propagators) {

            const SupplyMerit& merit = std::get<ssMerit>(propagator);
            float stealth;
            int source_id, empire_id;
            std::tie(std::ignore, stealth, source_id, empire_id) = propagator;

            auto check_blockade_effectiveness = [&empire_id, &stealth](int blockading_empire_id) {
                // Blockade if hostile and can detect
                auto blockading_detection = EmpireDetectionStrength(blockading_empire_id);
                return ((empire_id != blockading_empire_id)
                        && (Empires().GetDiplomaticStatus(empire_id, blockading_empire_id) == DIPLO_WAR)
                        && (blockading_detection > stealth));
            };

            // Follow all known starlanes.
            const auto& endpoints = empire_to_system_to_starlanes[empire_id][system_id];
            for (auto&& end_system : endpoints) {
                auto& pod = system_to_supply_pod[end_system];

                // boost::optional<std::set<std::tuple<SupplyMerit, float, int, int>>> merit_stealth_supply_empire;
                // boost::optional<std::unordered_set<int>> contesting_empires;
                // boost::optional<std::unordered_set<int>> blockading_fleets_empire_ids;
                // boost::optional<int> blockading_colonies_empire_id;

                // Check for hostile blockading fleets that can detect this supply
                if (pod.blockading_fleets_empire_ids) {
                    if (std::any_of(
                            pod.blockading_fleets_empire_ids->begin(), pod.blockading_fleets_empire_ids->end(),
                            check_blockade_effectiveness))
                    {
                        // blockaded
                        continue;
                    }
                }

                // Check for hostile blockading colonies.
                // Note: This is a temporary(?) workaround to ease AI development
                if (bool(pod.blockading_colonies_empire_id)
                    && check_blockade_effectiveness(*pod.blockading_colonies_empire_id))
                {
                    continue;
                }

                // Reduce the merit by the length of this starlane
                auto reduced_merit = SupplyMerit(merit)
                    .OneJumpLessMerit(GetPathfinder()->LinearDistance(system_id, end_system));

                // Apply any system bonuses from the new system
                if (pod.system_bonuses) {
                    const auto& local_bonus = pod.system_bonuses->find(empire_id);
                    if (local_bonus != pod.system_bonuses->end())
                        reduced_merit.Localize(local_bonus->second);
                }

                // This corresponds to a range of -1.
                auto lowest_acceptable_merit = SupplyMerit().OneJumpLessMerit();
                if (reduced_merit <= lowest_acceptable_merit)
                    continue;

                // Insert the new data into the endpoint system's POD.
                if (!pod.merit_stealth_supply_empire)
                    pod.merit_stealth_supply_empire = std::set<SupplyPODTuple>();
                pod.merit_stealth_supply_empire->insert({reduced_merit, stealth, source_id, empire_id});

                // Add the new data into the list of changed supply
                if (!retval)
                    retval = std::map<SupplyMerit, std::tuple<int, float, int, int>>();
                DebugLogger() << "AttemptToPropagate succeeds source = " << source_id << " merit = "
                              << reduced_merit << " stealth = "<< stealth<< " end sys = "
                              << end_system << " empire = " << empire_id <<".";
                retval->insert({reduced_merit, {source_id, stealth, end_system, empire_id}});
                verifyType<Planet>(source_id, "planet2");
                verifyType<System>(end_system, "end sys");
            }
        }
        return retval;
    }

        // There is one empire here.
        // if (pod.empire_to_merit_stealth_supply->size() == 1) {
        //     if (!candidates.size() == 1 && )
        //         ErrorLogger() << "Supply ContestSystem passed " << candidates.size()
        //                       << " into empty system " << system.SystemID();
        //     auto supply_range_of_only_empire = pod.empire_to_merit_stealth_supply->begin()->second.begin()->second;
        //     return {supply_range_of_only_empire, empires_needing_traversal_adjustment};
        // }

        // Sort by range using tuples of (merit, stealth, empire id)
        // SupplyPODTuple> merit_stealth_empire_id;
        // for (auto it : *pod.empire_to_merit_stealth_supply) {
        //     auto empire_id = it.first;
        //     for (auto jt : it.second)
        //         merit_stealth_empire_id.insert({jt.second, jt.first, empire_id});
        // }

        // auto max_supply_range = std::get<ssMerit>(*merit_stealth_empire_id.begin());

        // auto& keepers = *pod.merit_stealth_supply_empire;
        // keepers.clear();

        // while (!merit_stealth_empire_id.empty()) {
        //     // Compare the candidate with the greatest range to determine if it should be kept and if
        //     // each other supply source should be kept or removed from further supply resolutions.
        //     auto candidate = *merit_stealth_empire_id.rbegin();
        //     SupplyMerit candidate_merit;
        //     float candidate_stealth;
        //     int candidate_empire_id;
        //     std::tie(candidate_merit, candidate_stealth, candidate_empire_id) = candidate;
        //     merit_stealth_empire_id.erase(std::next(merit_stealth_empire_id.rbegin()).base());

        //     auto candidate_detection_it = empire_to_detection.find(candidate_empire_id);
        //     if (candidate_detection_it == empire_to_detection.end()) {
        //         ErrorLogger() << "Candidate supply has an empire id " << candidate_empire_id
        //                       << " with no detection strength. Skipping...";
        //         continue;
        //     }
        //     auto candidate_detection = candidate_detection_it->second;

        //     // Conditionally add candidate to the keepers.  It might be removed if tied for range.
        //     bool is_keep_candidate = true;

        //     // A list of individual others to remove from the list.
        //     std::vector<SupplyPODTuple>::reverse_iterator> removed_others;

        //     for (auto other_it = merit_stealth_empire_id.rbegin();
        //          other_it != merit_stealth_empire_id.rend(); ++other_it)
        //     {
        //         SupplyMerit other_merit;
        //         float other_stealth;
        //         int other_empire_id;
        //         std::tie(other_merit, other_stealth, other_empire_id) = *other_it;

        //         // Note: All other_merit are <= candidate_merit

        //         // If the other is of the same empire and has less stealth then it can't improve
        //         // supply, so remove it.
        //         if (other_empire_id == candidate_empire_id) {
        //             if (other_stealth <= candidate_stealth)
        //                 removed_others.push_back(other_it);
        //             continue;
        //         }

        //         // If other can't be detected then skip it
        //         if (other_stealth > candidate_detection)
        //             continue;

        //         // If other is not hostile to candidate empire then skip it
        //         if (Empires().GetDiplomaticStatus(candidate_empire_id, other_empire_id) != DIPLO_WAR)
        //             continue;

        //         // If other has the same range then remove both the candidate and the other
        //         if (other_merit == candidate_merit) {
        //             is_keep_candidate = false;
        //             removed_others.push_back(other_it);
        //             continue;
        //         }

        //         // All remaining others are detectable, hostile and have less range so remove them.
        //         removed_others.push_back(other_it);
        //     }

        //     if (is_keep_candidate)
        //         keepers[candidate_empire_id].insert({candidate_stealth, candidate_merit});
        //     else
        //         empires_needing_traversal_adjustment.insert(candidate_empire_id);


        //     for (auto removed_other : removed_others) {
        //         int other_empire_id;
        //         std::tie(std::ignore, std::ignore, other_empire_id) = *removed_other;
        //         empires_needing_traversal_adjustment.insert(other_empire_id);

        //         // Remove the collected reverse iterator.  Should be O(1).
        //         merit_stealth_empire_id.erase(std::next(removed_other).base());
        //     }
        // }

        // return {max_supply_range, empires_needing_traversal_adjustment};

    /** Adjust the supply lane traversals, after a system is contested. */
    void AdjustStarlaneTraversals(
        std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>& supply_starlane_traversals,
        std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>& supply_starlane_obstructed_traversals,
        SupplySystemPOD& pod, const System& system,
        const std::unordered_map<int, float>& empire_to_detection,
        const std::unordered_set<int> & empires_needing_traversal_adjustment)
    {
        // Check each empire that needs adjustment and if they have no more traversals in the SystemPOD
        // remove them from the traversals maps.

        for(auto empire_id : empires_needing_traversal_adjustment) {

            // There no empires or empire id is not present
            if (!pod.merit_stealth_supply_empire || pod.merit_stealth_supply_empire->empty()) {
                RemoveSystemFromTraversals(system.SystemID(), supply_starlane_traversals[empire_id],
                                           supply_starlane_obstructed_traversals[empire_id]);
            }
        }
    }

    std::unordered_map<int, std::vector<int>> CalculateColonyDisruptedSupply(
        const std::unordered_map<int, std::unordered_map<int, float>>& empire_system_supply_ranges)
    {
        // Map from Empire to Systems where only other empires have supply sources and there is no
        // friendly fleet.  This is used to help the AI until it can full manage supply mechanics
        std::unordered_map<int, std::vector<int>> empire_to_colony_disrupted_systems;

        const std::vector<std::shared_ptr<Fleet>> fleets = GetUniverse().Objects().FindObjects<Fleet>();

        for (const std::map<int, Empire*>::value_type& entry : Empires()) {
            int empire_id = entry.first;

            auto& self_protected_supply = empire_to_colony_disrupted_systems[empire_id];

            // Find systems containing armed aggressive fleets from own empire.
            std::set<int> systems_containing_friendly_fleets;

            const std::set<int>& known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id);
            for (std::shared_ptr<const Fleet> fleet : fleets) {
                int system_id = fleet->SystemID();
                if (system_id == INVALID_OBJECT_ID) {
                    continue;   // not in a system, so can't affect system obstruction
                } else if (known_destroyed_objects.find(fleet->ID()) != known_destroyed_objects.end()) {
                    continue; //known to be destroyed so can't affect supply, important just in case being updated on client side
                }

                if ((fleet->HasArmedShips() || fleet->HasFighterShips()) && fleet->Aggressive()) {
                    if (fleet->OwnedBy(empire_id)) {
                        if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                            systems_containing_friendly_fleets.insert(system_id);
                        }
                    }
                }
            }

            std::set<int> systems_where_hostile_others_have_supply_sources_and_current_empire_doesnt;
            // add all systems where hostile_others have supply
            for (const auto& empire_supply : empire_system_supply_ranges) {
                if (empire_supply.first == empire_id || empire_supply.first == ALL_EMPIRES)
                    continue;

                if (Empires().GetDiplomaticStatus(empire_id, empire_supply.first) != DIPLO_WAR)
                    continue;

                for (const std::map<int, float>::value_type& supply_range : empire_supply.second) {
                    if (supply_range.second <= 0.0f)
                        continue;
                    systems_where_hostile_others_have_supply_sources_and_current_empire_doesnt.insert(supply_range.first);
                }
            }
            // remove systems were this empire has supply
            const auto& it = empire_system_supply_ranges.find(empire_id);
            if (it != empire_system_supply_ranges.end()) {
                for (const std::map<int, float>::value_type& supply_range : it->second) {
                    if (supply_range.second <= 0.0f)
                        continue;
                    systems_where_hostile_others_have_supply_sources_and_current_empire_doesnt.erase(supply_range.first);
                }
            }

            // for systems where others have supply sources and this empire doesn't
            // and where this empire has no fleets...
            // supply is obstructed
            for (int system_id : systems_where_hostile_others_have_supply_sources_and_current_empire_doesnt) {
                if (systems_containing_friendly_fleets.find(system_id) == systems_containing_friendly_fleets.end())
                    self_protected_supply.push_back(system_id);
            }
        }
        return empire_to_colony_disrupted_systems;
    }

    void ObstructSupplyIfUncontestedHostileSupplySource(
        const std::unordered_map<int, std::unordered_map<int, float>>& empire_system_supply_ranges,
        std::map<int, std::set<int>>& empire_supply_unobstructed_systems)
    {
        for (auto& empire_to_colony_disrupted_systems : CalculateColonyDisruptedSupply(empire_system_supply_ranges)) {
            for (auto system_id: empire_to_colony_disrupted_systems.second)
                empire_supply_unobstructed_systems.erase(system_id);
        }
    }

}

std::map<int, std::map<int, std::pair<float, float>>> SupplyManager::SupplyManagerImpl::PropagateSupplyAlongUnobstructedStarlanes(
    const std::unordered_map<int, std::unordered_map<int, float>>& empire_system_supply_ranges,
    std::map<int, std::set<int>>& empire_supply_unobstructed_systems
)
{
    // map from empire id to map from system id to sum of supply source ranges
    // owned by empire in that in system
    std::map<int, std::map<int, float>> empire_system_supply_range_sums;
    for (auto empire_id_pair : empire_system_supply_ranges) {
        for (auto sys_id_pair : empire_id_pair.second) {
            empire_system_supply_range_sums[empire_id_pair.first][sys_id_pair.first] =
                EmpireTotalSupplyRangeSumInSystem(empire_id_pair.first, sys_id_pair.first);
        }
    }

    // system connections each empire can see / use for supply propagation
    std::map<int, std::map<int, std::set<int>>> empire_visible_starlanes;
    for (std::map<int, Empire*>::value_type& entry : Empires()) {
        const Empire* empire = entry.second;
        empire_visible_starlanes[entry.first] = empire->KnownStarlanes();//  VisibleStarlanes();
    }

    std::set<int> systems_with_supply_in_them;

    // store (supply range in jumps, and distance to supply source) of all
    // unobstructed systems before propagation, and add to list of systems
    // to propagate from.
    std::map<int, std::map<int, std::pair<float, float>>> empire_propagating_supply_ranges;
    float max_range = 0.0f;

    for (const auto& empire_supply : empire_system_supply_ranges) {
        int empire_id = empire_supply.first;
        const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

        for (const std::map<int, float>::value_type& supply_range : empire_supply.second) {
            int system_id = supply_range.first;
            if (unobstructed_systems.find(system_id) != unobstructed_systems.end()) {
                // stored: first -> source supply range.  second -> distance to source (0 for the source itself)
                empire_propagating_supply_ranges[empire_id][system_id] = {supply_range.second, 0.0f};
                if (supply_range.second > max_range)
                    max_range = supply_range.second;
                systems_with_supply_in_them.insert(system_id);
            }
        }
    }


    // spread supply out from sources by "diffusion" like process, along unobstructed
    // starlanes, until the range is exhausted.
    for (float range_to_spread = max_range; range_to_spread >= 0;
         range_to_spread -= 1.0f)
    {
        //DebugLogger() << "!!!! Reduced spreading range to " << range_to_spread;

        // update systems that have supply in them
        for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
            for (const std::map<int, std::pair<float, float>>::value_type& supply_range : empire_supply.second)
            { systems_with_supply_in_them.insert(supply_range.first); }
        }


        // resolve supply fights between multiple empires in one system.
        // pass over all empire-supplied systems, removing supply for all
        // but the empire with the highest supply range in each system
        for (int sys_id : systems_with_supply_in_them) {
            // sort empires by range in this system
            std::map<float, std::set<int>> empire_ranges_here;
            for (std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
                int empire_id = empire_supply.first;
                std::map<int, std::pair<float, float>>::const_iterator empire_supply_it = empire_supply.second.find(sys_id);
                // does this empire have any range in this system? if so, store it
                if (empire_supply_it == empire_supply.second.end())
                    continue;

                // store ids of empires indexed by adjusted propgated range, in order to sort by range
                float propagated_range = empire_supply_it->second.first;
                empire_ranges_here[
                    propagated_range
                    + ComputeSupplyBonuses(empire_id, sys_id,
                                           empire_supply_it->second, empire_system_supply_range_sums)
                ].insert(empire_supply.first);
            }

            if (empire_ranges_here.empty())
                continue;   // no empire has supply here?
            if (empire_ranges_here.size() == 1 && empire_ranges_here.begin()->second.size() < 2)
                continue;   // only one empire has supply here

            // remove range entries and traversals for all empires hostile to any of the top empires
            const auto & top_empires = empire_ranges_here.rbegin()->second;

            for (std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
                int empire_id = empire_supply.first;

                // Dont remove if none of the top empires are hostile to this empire.
                if (std::none_of(top_empires.begin(), top_empires.end(),
                                 [empire_id](int a_top_empire_id) {
                                     return (empire_id == a_top_empire_id) ? false :
                                         Empires().GetDiplomaticStatus(empire_id, a_top_empire_id) == DIPLO_WAR;
                                 }))
                { continue; }

                // remove from range entry...
                std::map<int, std::pair<float, float>>& empire_ranges = empire_supply.second;
                empire_ranges.erase(sys_id);

                // Remove from unobstructed systems
                empire_supply_unobstructed_systems[empire_id].erase(sys_id);

                RemoveSystemFromTraversals(sys_id, m_supply_starlane_traversals[empire_id],
                                           m_supply_starlane_obstructed_traversals[empire_id]);
                //DebugLogger() << "... removed empire " << empire_id << " system " << sys_id << " supply.";
            }

            //// DEBUG
            // DebugLogger() << "after culling empires ranges at system " << sys_id << ":";
            // for (std::map<int, std::map<int, float>>::value_type& empire_supply : empire_propagating_supply_ranges) {
            //    std::map<int, float>& system_ranges = empire_supply.second;
            //    std::map<int, float>::iterator range_it = system_ranges.find(sys_id);
            //    if (range_it != system_ranges.end())
            //        DebugLogger() << empire_supply.first << " : " << range_it->second;
            // }
            //// END DEBUG
        }

        if (range_to_spread <= 0)
            break;

        // initialize next iteration with current supply distribution
        std::map<int, std::map<int, std::pair<float, float>>> empire_propagating_supply_ranges_next = empire_propagating_supply_ranges;


        // for sources of supply of at least the minimum range for this
        // iteration that are in the current map, give adjacent systems one
        // less supply in the next iteration (unless as much or more is already
        // there for a hostile empire)
            for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
            int empire_id = empire_supply.first;
            //DebugLogger() << ">-< Doing supply propagation for empire " << empire_id << " >-<";
            const std::map<int, std::pair<float, float>>& prev_sys_ranges = empire_supply.second;
            const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

            for (const std::map<int, std::pair<float, float>>::value_type& supply_range : empire_supply.second) {
                // does the source system have enough supply range to propagate outwards?
                float range = supply_range.second.first;
                if (range != range_to_spread)
                    continue;
                float range_after_one_more_jump = range - 1.0f; // what to set adjacent systems' ranges to (at least)

                // how far is this system from a source of supply for this empire?
                float distance_to_supply_source = supply_range.second.second;

                // what starlanes can be used to propagate supply?
                int system_id = supply_range.first;

                //DebugLogger() << "propagating from system " << system_id << " which has range: " << range << " and distance: " << distance_to_supply_source;

                // attempt to propagate to all adjacent systems...
                for (int lane_end_sys_id : empire_visible_starlanes[empire_id][system_id]) {
                    // is propagation to the adjacent system obstructed?
                    if (unobstructed_systems.find(lane_end_sys_id) == unobstructed_systems.end()) {
                        // propagation obstructed!
                        //DebugLogger() << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id << " due to not being on unobstructed systems";
                        AddObstructedTraversal(system_id, lane_end_sys_id,
                                               empire_supply_unobstructed_systems[empire_id],
                                               m_supply_starlane_traversals[empire_id],
                                               m_supply_starlane_obstructed_traversals[empire_id]);
                        continue;
                    }
                    // propagation not obstructed.


                    // does another hostile empire already have as much or more supply here from a previous iteration?
                    float other_empire_biggest_range = -10000.0f;   // arbitrary big numbeer
                    for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& other_empire_supply : empire_propagating_supply_ranges) {
                        int other_empire_id = other_empire_supply.first;
                        if ((other_empire_id == empire_id)
                            || Empires().GetDiplomaticStatus(empire_id, other_empire_id) != DIPLO_WAR)
                        { continue; }

                        const std::map<int, std::pair<float, float>>& prev_other_empire_sys_ranges = other_empire_supply.second;
                        std::map<int, std::pair<float, float>>::const_iterator prev_other_empire_range_it = prev_other_empire_sys_ranges.find(lane_end_sys_id);
                        if (prev_other_empire_range_it == prev_other_empire_sys_ranges.end())
                            continue;
                        if (prev_other_empire_range_it->second.first > other_empire_biggest_range)
                            other_empire_biggest_range = prev_other_empire_range_it->second.first;
                    }

                    // if so, add a blocked traversal and continue
                    if (range_after_one_more_jump <= other_empire_biggest_range) {
                        AddObstructedTraversal(system_id, lane_end_sys_id,
                                               empire_supply_unobstructed_systems[empire_id],
                                               m_supply_starlane_traversals[empire_id],
                                               m_supply_starlane_obstructed_traversals[empire_id]);
                        //DebugLogger() << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id << " due to other empire biggest range being " << other_empire_biggest_range;
                        continue;
                    }

                    // otherwise, propagate into system...
                    float lane_length = DistanceBetweenObjects(system_id, lane_end_sys_id);
                    float distance_to_supply_source_after_next_lane = lane_length + distance_to_supply_source;

                    //DebugLogger() << "Attempting to propagate into system: " << lane_end_sys_id << " the new range: " << range_after_one_more_jump << " and distance: " << distance_to_supply_source_after_next_lane;

                    // if propagating supply would increase the range of the adjacent system,
                    // or decrease the distance to the adjacent system from a supply source...
                    std::map<int, std::pair<float, float>>::const_iterator prev_range_it = prev_sys_ranges.find(lane_end_sys_id);
                    if (prev_range_it == prev_sys_ranges.end()) {
                        empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id] =
                            {range_after_one_more_jump, distance_to_supply_source_after_next_lane};
                        //DebugLogger() << " ... default case: no previous entry.";

                    } else {
                        //DebugLogger() << " ... previous entry values: " << prev_range_it->second.first << " and " << prev_range_it->second.second;

                        if (range_after_one_more_jump > prev_range_it->second.first) {
                            empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id].first =
                                range_after_one_more_jump;
                            //DebugLogger() << " ... range increased!";
                        }
                        if (distance_to_supply_source_after_next_lane < prev_range_it->second.second) {
                            empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id].second =
                                distance_to_supply_source_after_next_lane;
                            //DebugLogger() << " ... distance decreased!";
                        }
                    }

                    AddTraversal(system_id, lane_end_sys_id,
                                 empire_supply_unobstructed_systems[empire_id],
                                 m_supply_starlane_traversals[empire_id],
                                 m_supply_starlane_obstructed_traversals[empire_id]);
                }
            }
        }

        // save propagated results for next iteration
        empire_propagating_supply_ranges = empire_propagating_supply_ranges_next;
    }

    return empire_propagating_supply_ranges;
}

void SupplyManager::SupplyManagerImpl::DetermineSupplyConnectedSystemGroups() {
    // determine supply-connected groups of systems for each empire.
    // need to merge interconnected supply groups into as few sets of mutually-
    // supply-exchanging systems as possible.  This requires finding the
    // connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.

    for (auto&& empire : Empires()) {
        int empire_id = empire.first;

        // assemble all direct connections between systems from remaining traversals
        std::unordered_map<int, std::unordered_set<int> > supply_groups_map;
        for (const auto& lane : m_supply_starlane_traversals[empire_id]) {
            supply_groups_map[lane.first.first].insert(lane.first.second);
            supply_groups_map[lane.first.second].insert(lane.first.first);
        }

        // also add connections from all fleet-supplyable systems to themselves, so that
        // any fleet supply able system with no connection to another system can still
        // have resource sharing within tiself
        for (int system_id : m_fleet_supplyable_system_ids[empire_id]) {
            supply_groups_map[system_id].insert(system_id);
        }

        if (supply_groups_map.empty())
            continue;

        // create graph
        boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;

        // boost expects vertex labels to range from 0 to num vertices - 1, so need
        // to map from system id to graph id and back when accessing vertices
        std::vector<int> graph_id_to_sys_id;
        graph_id_to_sys_id.reserve(supply_groups_map.size());

        std::unordered_map<int, int> sys_id_to_graph_id;
        int graph_id = 0;
        for (auto&& supply_group : supply_groups_map) {
            int sys_id = supply_group.first;
            boost::add_vertex(graph);   // should add with index = graph_id

            graph_id_to_sys_id.push_back(sys_id);
            sys_id_to_graph_id[sys_id] = graph_id;
            ++graph_id;
        }

        // add edges for all direct connections between systems
        // and add edges from fleet supplyable systems to themselves
        for (auto&& supply_group : supply_groups_map) {
            int start_graph_id = sys_id_to_graph_id[supply_group.first];
            for (int system_id : supply_group.second) {
                int end_graph_id = sys_id_to_graph_id[system_id];
                boost::add_edge(start_graph_id, end_graph_id, graph);
            }
        }

        // declare storage and fill with the component id (group id of connected systems)
        // for each graph vertex
        std::vector<int> components(boost::num_vertices(graph));
        boost::connected_components(graph, &components[0]);

        // convert results back from graph id to system id, and into desired output format
        // output: std::map<int, std::set<std::set<int>>>& m_resource_supply_groups

        // first, sort into a map from component id to set of system ids in component
        std::unordered_map<int, std::shared_ptr<std::unordered_set<int>>> component_sets_map;
        for (std::size_t comp_graph_id = 0; comp_graph_id != components.size(); ++comp_graph_id) {
            int label = components[comp_graph_id];
            int sys_id = graph_id_to_sys_id[comp_graph_id];
            if (component_sets_map.find(label) == component_sets_map.end())
                component_sets_map.insert({label, std::make_shared<std::unordered_set<int>>()});
            component_sets_map[label]->insert(sys_id);
        }

        // copy sets in map into set of sets
        for (auto & component_set : component_sets_map) {
            m_resource_supply_groups[empire_id].push_back(component_set.second);
        }
    }
}

namespace {
    /** Return a map from empire id to detection strength.*/
    std::unordered_map<int, float> EmpireToDetectionStrengths() {
        std::unordered_map<int, float> retval;
        for (const auto& entry : Empires()) {
            const auto  empire_id = entry.first;
            const auto& empire = entry.second;
            const auto  meter = empire->GetMeter("METER_DETECTION_STRENGTH");
            retval[empire_id] = meter ? meter->Current() : 0.0f;
        }
        return retval;
    }
}

void SupplyManager::SupplyManagerImpl::Update() {
    SectionedScopedTimer timer("SupplyManager::Update", boost::chrono::microseconds(100));
    timer.EnterSection("update starlanes");
    DebugLogger() << "Entering SupplyManager update.";
    //TODO early exit when no supply sources


    DebugLogger() << "SupplyManager::Update() starlanes.";
    // system connections each empire can see / use for supply propagation
    std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>> empire_to_system_to_starlanes;
    for (auto && entry : Empires()) {
        const Empire* empire = entry.second;
        auto& per_empire = empire_to_system_to_starlanes[entry.first];
        // Remake the std::maps into faster std::unordered_maps
        // TODO push this improvement back into Empires.
        for (auto && startpoint_and_endpoints: empire->KnownStarlanes()) {
            for (auto&& endpoint : startpoint_and_endpoints.second) {
                per_empire[startpoint_and_endpoints.first].insert(endpoint);
            }
        }
    }

    timer.EnterSection("initial supply sources ");
    DebugLogger() << "SupplyManager::Update() Find initial supply sources.";
    // Map from system id to all sources of supply.
    std::unordered_map<int, std::set<SupplyPODTuple>> initial_supply_sources = CalculateInitialSupplySources();

    timer.EnterSection("create PODs");
    DebugLogger() << "SupplyManager::Update() create PODs.";
    //Map from system id to SupplySystemPOD containing all data needed to evaluate supply in one system.
    std::unordered_map<int, SupplySystemPOD> system_to_supply_pod = CalculateInitialSystemConditions(initial_supply_sources);

    timer.EnterSection("create tranches");
    DebugLogger() << "SupplyManager::Update() create tranches.";
    // Tranches of SupplyMerit sorted by merit.  Tranches are the size of a one supply hop change
    // in merit starting from the highest merit.  Each tranche of merits can be processed and then
    // propagated to adjacent systems and then never handled again.
    auto tranches = SupplyTranches(system_to_supply_pod);

    timer.EnterSection("grind");
    DebugLogger() << "SupplyManager::Update() start grind.";
    // Process each tranche until the merit threshold is negative
    const auto lowest_merit = SupplyMerit().OneJumpLessMerit();
    auto merit_threshold = tranches.MaxMerit();
    while (merit_threshold > lowest_merit) {
        DebugLogger() << "SupplyManager::Update() do tranche with merit " << merit_threshold <<".";

        // In each system the supply sources from this tranche will compete and determine which
        // supply sources in that system win and are propagated further or lose and are removed
        // from consideration.
        auto& tranche = tranches[merit_threshold];
        for (auto&& system_and_source_to_merit_stealth_empire : tranche) {
            auto system_id = system_and_source_to_merit_stealth_empire.first;
            DebugLogger() << "SupplyManager::Update() contest system "<< system_id;

            // System data
            auto& pod = system_to_supply_pod[system_id];

            // Contest each system
            std::vector<SupplyPODTuple> winners;
            std::unordered_map<int, SupplyMerit> losers;
            std::tie( winners, losers) =
                ContestSystem(pod, system_id, system_and_source_to_merit_stealth_empire.second);

            DebugLogger() << "SupplyManager::Update() remove " << losers.size() << " losers in "<< system_id;
            // Remove the losers the tranches.
            for (auto& loser : losers) {
                tranches.Remove(system_id, loser.first, loser.second);
            }

            DebugLogger() << "SupplyManager::Update() attempt propagate " << winners.size() << " winners in "<< system_id;
            // Propagate the winners
            auto propagated_winners = AttemptToPropagate(
                system_to_supply_pod, tranches, system_id,
                winners, empire_to_system_to_starlanes);

            if (propagated_winners) {
                DebugLogger() << "SupplyManager::Update() add " << propagated_winners->size()
                              << " propagated winners from "<< system_id;
                tranches.Add(*propagated_winners);
            }
        }

        merit_threshold.OneJumpLessMerit();
    }

    // Extract the information for each output map from system_to_supply_pod.

    timer.EnterSection("extract");

    DebugLogger() << "SupplyManager::Update() extract basics.";
    /** ids of systems where fleets can be resupplied. indexed by empire id. */
    /*std::map<int, std::set<int>>                 */  m_fleet_supplyable_system_ids.clear();

    /** for whichever empire can propagate supply into this system, what is the
        additional range from this system that empire can propagate supply */
    /*std::map<int, float>                          */  m_propagated_supply_ranges.clear();

    /** for each empire, what systems it can propagate supply into, and how many
      * further supply jumps it could propagate past this system, if not blocked
      * from doing so by supply obstructions. */
    /*std::map<int, std::map<int, float>>           */  m_empire_propagated_supply_ranges.clear();

    /** for whichever empire can propagate supply into this system, how far
      * that system is from the closest source of supply for that empire, along
      * possible supply propgation connections (ie. not though a supply
      * obstructed system for that empire) */
    /*std::map<int, float>                          */  m_propagated_supply_distances.clear();

    /** for each empire, what systems it can propagate supply into, and how far
      * that system is from the closest source of supply for that empire, along
      * possible supply propgation connections (ie. not though a supply
      * obstructed system) */
    /*std::map<int, std::map<int, float>>           */  m_empire_propagated_supply_distances.clear();


    /*// For each empire the highest stealth its supply in system that it has supply inn.
    std::unordered_map<int, std::unordered_map<int, float>> */ m_empire_to_system_to_stealth.clear();

    m_system_to_empire_to_final_supply.clear();

    for (auto&& system_and_supply_pod : system_to_supply_pod) {
        auto system_id = system_and_supply_pod.first;
        auto& pod = system_and_supply_pod.second;
        verifyType<System>(system_id, "system");

        // Create a supply source for each positive merit in the system.
        if (pod.merit_stealth_supply_empire) {
            for (const auto& merit_stealth_supply_empire: *pod.merit_stealth_supply_empire) {
                const SupplyMerit& merit     = std::get<ssMerit>(merit_stealth_supply_empire);
                const auto         stealth   = std::get<ssStealth>(merit_stealth_supply_empire);
                const auto         source_id = std::get<ssSource>(merit_stealth_supply_empire);
                const auto         empire_id = std::get<ssEmpireID>(merit_stealth_supply_empire);

                if (merit.Range() < 0.0f)
                    continue;

                DebugLogger() << "Added "<<merit<< " in system " << system_id << " for empire " << empire_id;

                m_fleet_supplyable_system_ids[empire_id].insert(system_id);

                m_propagated_supply_ranges.insert({empire_id, merit.Range()});
                m_empire_propagated_supply_ranges[empire_id].insert({system_id, merit.Range()});

                m_propagated_supply_distances.insert({empire_id, merit.Distance()});
                m_empire_propagated_supply_distances[empire_id].insert({system_id, merit.Distance()});

                // Keep the best local stealth.
                auto& current_stealth = m_empire_to_system_to_stealth[empire_id][system_id];
                current_stealth = std::max(current_stealth, stealth);

                m_system_to_empire_to_final_supply[system_id][empire_id].push_back(
                    std::make_tuple(source_id, merit.Range(), merit.Bonus(), merit.Distance(), stealth));
            }
        }
    }

        // boost::optional<std::set<std::tuple<SupplyMerit, float, int, int>>> merit_stealth_supply_empire;
        // boost::optional<std::unordered_set<int>> contesting_empires;
        // boost::optional<std::unordered_set<int>> blockading_fleets_empire_ids;
        // boost::optional<int> blockading_colonies_empire_id;


        /** ordered pairs of system ids between which a starlane runs that can be
        used to convey resources between systems. indexed first by empire id. */
    /*std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>*/  m_supply_starlane_traversals.clear();

    /** ordered pairs of system ids between which a starlane could be used to
        convey resources between system, but is not because something is
        obstructing the resource flow.  That is, the resource flow isn't limited
        by range, but by something blocking its flow. */
    /*std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>*/  m_supply_starlane_obstructed_traversals.clear();

    timer.EnterSection("update traversals");
    DebugLogger() << "SupplyManager::Update() traversals.";
    // std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>>
    for (auto&& empire_and_system_to_starlanes : empire_to_system_to_starlanes) {
        int empire_id = empire_and_system_to_starlanes.first;

        // auto& fleet_supplyable_system_ids = m_fleet_supplyable_system_ids[empire_id];
        // auto& propagated_supply_ranges = m_propagated_supply_ranges[empire_id];
        auto& empire_propagated_supply_ranges = m_empire_propagated_supply_ranges[empire_id];
        // auto& propagated_supply_distances = m_propagated_supply_distances[empire_id];
        // auto& empire_propagated_supply_distances = m_empire_propagated_supply_distances[empire_id];

        auto& supply_starlane_traversals = m_supply_starlane_traversals[empire_id];
        auto& supply_starlane_obstructed_traversals = m_supply_starlane_obstructed_traversals[empire_id];

        for (auto&& start_id_and_endpoints : empire_and_system_to_starlanes.second) {
            auto start_id = start_id_and_endpoints.first;

            // There is no supply in the starting system.
            const auto& start_range_it = empire_propagated_supply_ranges.find(start_id);
            auto start_range = (start_range_it != empire_propagated_supply_ranges.end()) ? start_range_it->second : -1.0f;

            for (auto&& end_id : start_id_and_endpoints.second) {

                // There is no supply in the end system.
                const auto& end_range_it = empire_propagated_supply_ranges.find(end_id);
                auto end_range = (end_range_it != empire_propagated_supply_ranges.end()) ? end_range_it->second : -1.0f;

                verifyType<System>(start_id, "start id");
                verifyType<System>(end_id, "end id");

                // Use the smallest stealth of either endpoint.
                const auto& stealth_map = m_empire_to_system_to_stealth[empire_id];
                const auto stealth_start_it = stealth_map.find(start_id);
                const auto stealth_start = stealth_start_it != stealth_map.end() ? stealth_start_it->second : 0.0f;
                const auto stealth_end_it = stealth_map.find(end_id);
                const auto stealth_end = stealth_end_it != stealth_map.end() ? stealth_end_it->second : 0.0f;
                const auto stealth = std::min(stealth_start, stealth_end);

                // If there is supply at both ends then it is traversable
                if (start_range >= 0.0f  && end_range >= 0.0f) {
                    DebugLogger() << " Good traversal added ("<<start_id<<","<<end_id<<") for empire "<<empire_id<< " stealth = "<<stealth<<".";
                    supply_starlane_traversals.insert({{start_id, end_id}, stealth});
                }

                // If there is surplus range >1.0f at the source end and 0.0f at the output end
                // then it is obstructed.  This is asymmetrical, so check both direction separately.
                if (start_range >= 1.0f  && end_range < 0.0f) {
                    DebugLogger() << " Obstructed traversal added ("<<start_id<<","<<end_id<<") for empire "<<empire_id<< " stealth = "<<stealth_start<<".";
                    supply_starlane_obstructed_traversals.insert({{start_id, end_id}, stealth_start});
                }
                if (end_range >= 1.0f  && start_range < 0.0f) {
                    DebugLogger() << " Obstructed traversal added ("<<start_id<<","<<end_id<<") for empire "<<empire_id<< " stealth = "<<stealth_end<<".";
                    supply_starlane_obstructed_traversals.insert({{end_id, start_id}, stealth_end});
                }
            }
        }
    }

    timer.EnterSection("group");
    DebugLogger() << "SupplyManager::Update() supply groups.";

    /** sets of system ids that are connected by supply lines and are able to
        share resources between systems or between objects in systems. indexed
        by empire id. */
    /*std::map<int, std::set<std::set<int>>>      */  m_resource_supply_groups.clear();
    DetermineSupplyConnectedSystemGroups();

    DebugLogger() << "Exiting SupplyManager update.";

}

void SupplyManager::SupplyManagerImpl::UpdateOld() {
    m_supply_starlane_traversals.clear();
    m_supply_starlane_obstructed_traversals.clear();
    m_fleet_supplyable_system_ids.clear();
    m_resource_supply_groups.clear();
    m_propagated_supply_ranges.clear();

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


    // // Map from system id to a map from empire id to the stealth and supply ranges of empire
    // // objects in that system.
    // std::unordered_map<int, std::unordered_map<int, std::set<std::pair<float, SupplyMerit>>>> system_to_empire_to_stealth_supply
    //     = CalculateSystemToEmpireToStealthSupply();

    // Map from system to blockading fleets.
    // These fleets will differ from the empire's known blockading fleets, because an empire may
    // not be able to detect all fleets.
    std::unordered_map<int, std::unordered_set<int>> system_to_contesting_fleets_empire_ids;
    std::unordered_map<int, std::unordered_set<int>> system_to_blockading_fleets_empire_ids;
    std::tie(system_to_contesting_fleets_empire_ids, system_to_blockading_fleets_empire_ids)
        = CalculateSystemToBlockadingFleetsEmpireIDs();

    // // Map from the system to empire id of colonies that are blockading.
    // // Note: This was an addition to ease the AI requirements of the supply mechanic.
    // std::unordered_map<int, std::unordered_set<int>> system_blockading_colony_empire_ids
    //     = CalculateSystemToBlockadingColonyEmpireIDs(system_to_empire_to_stealth_supply,
    //                                                  system_to_contesting_fleets_empire_ids);


    // //Map from system id to SupplySystemPOD containing all data needed to evaluate supply in one system.
    // std::unordered_map<int, SupplySystemPOD> system_to_initial_supply_pod = CalculateInitialSupply();


    // Map from empire id to a map from system id to the stealth and supply ranges of empire
    // objects in that system.
    std::unordered_map<int, std::unordered_map<int, std::set<std::pair<float, float>>>> empire_to_system_to_stealth_supply;

    // map from empire id to map from system id to range (in starlane jumps)
    // that supply can be propagated out of that system by that empire.
    std::unordered_map<int, std::unordered_map<int, float> > empire_system_supply_ranges;
    // map from empire id to which systems are obstructed for it for supply
    // propagation
    std::map<int, std::set<int> > empire_supply_unobstructed_systems;

    /** Map from empire id to detection strength.*/
    auto empire_to_detection = EmpireToDetectionStrengths();

    for (const auto& entry : Empires()) {
        const auto empire_id = entry.first;
        const auto& empire = entry.second;
        empire_to_system_to_stealth_supply[empire_id] = empire->SystemToStealthAndSupplyRange();
        empire_system_supply_ranges[empire_id] = empire->SystemSupplyRanges();
        empire_supply_unobstructed_systems[empire_id] = empire->SupplyUnobstructedSystems();

        //std::stringstream ss;
        //for (int system_id : empire_supply_unobstructed_systems[entry.first])
        //{ ss << system_id << ", "; }
        //DebugLogger() << "Empire " << empire->EmpireID() << " unobstructed systems: " << ss.str();
    }

    /////
    // probably temporary: additional restriction here for supply propagation
    // but not for general system obstruction as determind within Empire::UpdateSupplyUnobstructedSystems
    /////
    ObstructSupplyIfUncontestedHostileSupplySource(
        empire_system_supply_ranges,
        empire_supply_unobstructed_systems);
    /////
    // end probably temporary...
    /////


    // Calculate for each empire the supply available at each system
    std::map<int, std::map<int, std::pair<float, float>>>&& empire_propagating_supply_ranges =
        PropagateSupplyAlongUnobstructedStarlanes(
            empire_system_supply_ranges,
            empire_supply_unobstructed_systems);


    //// DEBUG
    //DebugLogger() << "SuppolyManager::Update: after removing conflicts, empires can provide supply to the following system ids:";
    //for (std::map<int, std::map<int, float>>::value_type& empire_supply : empire_propagating_supply_ranges) {
    //    int empire_id = empire_supply.first;
    //    std::stringstream ss;
    //    for (std::map<int, float>::value_type& supply_range : empire_supply.second) {
    //        ss << supply_range.first << " (" << supply_range.second << "),  ";
    //    }
    //    DebugLogger() << "empire: " << empire_id << ":  " << ss.str();
    //}
    //// END DEBUG

    // record which systems are fleet supplyable by each empire (after resolving conflicts in each system)
    for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
        int empire_id = empire_supply.first;
        for (const std::map<int, std::pair<float, float>>::value_type& supply_range : empire_supply.second) {
            if (supply_range.second.first < 0.0f)
                continue;   // negative supply doesn't count... zero does (it just reaches)
            m_fleet_supplyable_system_ids[empire_id].insert(supply_range.first);

            // should be only one empire per system at this point, but use max just to be safe...
            m_propagated_supply_ranges[supply_range.first] =
                std::max(supply_range.second.first, m_propagated_supply_ranges[supply_range.first]);
            m_empire_propagated_supply_ranges[empire_id][supply_range.first] =
                m_propagated_supply_ranges[supply_range.first];

            // should be only one empire per system at this point, but use max just to be safe...
            m_propagated_supply_distances[supply_range.first] =
                std::max(supply_range.second.second, m_propagated_supply_distances[supply_range.first]);
            m_empire_propagated_supply_distances[empire_id][supply_range.first] =
                m_propagated_supply_distances[supply_range.first];
        }

        //DebugLogger() << "For empire: " << empire_id << " system supply distances: ";
        //for (auto entry : m_empire_propagated_supply_distances[empire_id]) {
        //    DebugLogger() << entry.first << " : " << entry.second;
        //}
    }

    DetermineSupplyConnectedSystemGroups();
}

namespace boost {
    namespace serialization {

    // TODO: Write a generic template std::tuple serialization routine.

    template <class Archive>
    void serialize(Archive& ar, SupplySystemBonusTuple& x, const unsigned int version) {
        auto& bonus = std::get<ssbBonus>(x);
        auto& visibility = std::get<ssbVisibilityBonus>(x);
        auto& ship = std::get<ssbShipBonus>(x);
        auto& colony = std::get<ssbColonyBonus>(x);

        ar  & BOOST_SERIALIZATION_NVP(bonus)
            & BOOST_SERIALIZATION_NVP(visibility)
            & BOOST_SERIALIZATION_NVP(ship)
            & BOOST_SERIALIZATION_NVP(colony);
    }

    template <class Archive>
    void serialize(Archive& ar, SupplySystemEmpireTuple& x, const unsigned int version) {
        auto& source = std::get<sseSource>(x);
        auto& range = std::get<sseRange>(x);
        auto& bonus = std::get<sseBonus>(x);
        auto& dist = std::get<sseDistance>(x);
        auto& stealth = std::get<sseStealth>(x);

        ar  & BOOST_SERIALIZATION_NVP(source)
            & BOOST_SERIALIZATION_NVP(range)
            & BOOST_SERIALIZATION_NVP(bonus)
            & BOOST_SERIALIZATION_NVP(dist)
            & BOOST_SERIALIZATION_NVP(stealth);
    }

    }
}

template <class Archive>
void SupplyManager::SupplyManagerImpl::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_supply_starlane_traversals)
        & BOOST_SERIALIZATION_NVP(m_supply_starlane_obstructed_traversals)
        & BOOST_SERIALIZATION_NVP(m_fleet_supplyable_system_ids)
        & BOOST_SERIALIZATION_NVP(m_resource_supply_groups)
        & BOOST_SERIALIZATION_NVP(m_propagated_supply_ranges)
        & BOOST_SERIALIZATION_NVP(m_empire_propagated_supply_ranges)
        & BOOST_SERIALIZATION_NVP(m_propagated_supply_distances)
        & BOOST_SERIALIZATION_NVP(m_empire_propagated_supply_distances)
        & BOOST_SERIALIZATION_NVP(m_empire_to_system_to_stealth)
        & BOOST_SERIALIZATION_NVP(m_system_to_empire_to_final_supply);
}

template void SupplyManager::SupplyManagerImpl::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SupplyManager::SupplyManagerImpl::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SupplyManager::SupplyManagerImpl::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SupplyManager::SupplyManagerImpl::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


////////////////////////////////////////
// Supply Manager
////////////////////////////////////////

SupplyManager::SupplyManager() :
    pimpl(new SupplyManagerImpl)
{}

// Required to call pimpl destructor.
SupplyManager::~SupplyManager() {}

SupplyManager& SupplyManager::operator=(const SupplyManager& rhs) {
    pimpl->operator=(*rhs.pimpl);
    return *this;
}

const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>&   SupplyManager::SupplyStarlaneTraversals() const
{ return pimpl->SupplyStarlaneTraversals(); }

const std::unordered_map<std::pair<int, int>, float>&                   SupplyManager::SupplyStarlaneTraversals(int empire_id) const
{ return pimpl->SupplyStarlaneTraversals(empire_id); }

const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>&   SupplyManager::SupplyObstructedStarlaneTraversals() const
{ return pimpl->SupplyObstructedStarlaneTraversals(); }

const std::unordered_map<std::pair<int, int>, float>&                   SupplyManager::SupplyObstructedStarlaneTraversals(int empire_id) const
{ return pimpl->SupplyObstructedStarlaneTraversals(empire_id); }

const std::unordered_map<int, std::unordered_set<int> >&          SupplyManager::FleetSupplyableSystemIDs() const
{ return pimpl->FleetSupplyableSystemIDs(); }

const std::unordered_set<int>&                                    SupplyManager::FleetSupplyableSystemIDs(int empire_id) const
{ return pimpl->FleetSupplyableSystemIDs(empire_id); }

std::unordered_set<int>                                           SupplyManager::FleetSupplyableSystemIDs(int empire_id, bool include_allies) const
{ return pimpl->FleetSupplyableSystemIDs(empire_id, include_allies); }

const std::unordered_map<int, std::vector<std::shared_ptr<const std::unordered_set<int>>>>&         SupplyManager::ResourceSupplyGroups() const
{ return pimpl->ResourceSupplyGroups(); }

const std::vector<std::shared_ptr<const std::unordered_set<int>>>&                         SupplyManager::ResourceSupplyGroups(int empire_id) const
{ return pimpl->ResourceSupplyGroups(empire_id); }

const std::unordered_map<int, float>&                             SupplyManager::PropagatedSupplyRanges(int empire_id) const
{ return pimpl->PropagatedSupplyRanges(empire_id); }

const std::unordered_map<int, float>&                             SupplyManager::PropagatedSupplyDistances(int empire_id) const
{ return pimpl->PropagatedSupplyDistances(empire_id); }

const std::unordered_map<int, float>&                             SupplyManager::SupplyStealth(int empire_id) const
{ return pimpl->SupplyStealth(empire_id); }

bool                                                    SupplyManager::SystemHasFleetSupply(int system_id, int empire_id) const
{ return pimpl->SystemHasFleetSupply(system_id, empire_id); }

bool                                                    SupplyManager::SystemHasFleetSupply(int system_id, int empire_id, bool include_allies) const
{ return pimpl->SystemHasFleetSupply(system_id, empire_id, include_allies); }

boost::optional<const SupplyManager::system_iterator> SupplyManager::SystemSupply(const int system_id) const {
    return pimpl->SystemSupply(system_id);
}

std::string                                             SupplyManager::Dump(int empire_id /*= ALL_EMPIRES*/) const
{ return pimpl->Dump(empire_id); }

void                                                    SupplyManager::Update()
{ return pimpl->Update(); }

template <class Archive>
void SupplyManager::serialize(Archive& ar, const unsigned int version)
{ pimpl->serialize(ar, version); }

template void SupplyManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);
