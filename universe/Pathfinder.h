#ifndef _Pathfinder_h_
#define _Pathfinder_h_


#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include "UniverseObject.h"

class ObjectMap;
class EmpireManager;
namespace Condition {
    using ObjectSet = std::vector<const UniverseObject*>;
}

/** The Pathfinder  class contains the locations of systems, the
  * starlanes and functions to determine pathing and trip duration
  * around the Universe. */
class FO_COMMON_API Pathfinder final {
public:
    using SystemExclusionPredicateType = std::function<bool(const UniverseObject*)>;

    Pathfinder();
    ~Pathfinder();
    Pathfinder& operator=(Pathfinder&&) noexcept;

    /** Returns the straight-line distance between the objects with the given
      * IDs. \throw std::out_of_range This function will throw if either object
      * ID is out of range. */
    double LinearDistance(int object1_id, int object2_id, const ObjectMap& objects) const;

    /** Returns the number of starlane jumps between the systems with the given
      * IDs. If there is no path between the systems, -1 is returned.
      * \throw std::out_of_range This function will throw if either system
      * ID is not a valid system id. */
    int16_t JumpDistanceBetweenSystems(int system1_id, int system2_id) const;

    /** Returns the number of starlane jumps between any two objects, accounting
      * for cases where one or the other are fleets / ships on starlanes between
      * systems. Returns INT_MAX when no path exists, or either object does not
      * exist. */
    int JumpDistanceBetweenObjects(int object1_id, int object2_id, const ObjectMap& objects) const;

    /** Returns the sequence of systems, including \a system1_id and
      * \a system2_id, that defines the shortest path from \a system1 to
      * \a system2, and the distance travelled to get there.  If no such path
      * exists, the list will be empty.  Note that the path returned may be via
      * one or more starlane, or may be "offroad".  The path is calculated
      * using the visibility for empire \a empire_id, or without regard to
      * visibility if \a empire_id == ALL_EMPIRES.
      * \throw std::out_of_range This function will throw if either system ID
      * is out of range, or if the empire ID is not known. */
    std::pair<std::vector<int>, double> ShortestPath(
        int system1_id, int system2_id, int empire_id, const ObjectMap& objects) const;
    auto ShortestPath(int system1_id, int system2_id, const ObjectMap& objects) const
    { return ShortestPath(system1_id, system2_id, ALL_EMPIRES, objects); }

    /** Shortest path known to an empire between two systems, excluding routes
     *  for systems containing objects for @p system_predicate.
     * @param system1_id source System id
     * @param system2_id destination System id
     * @param empire_id ID of viewing Empire
     * @param system_predicate A System is excluded as a potential node in any route
     *                         if it is or contains a matched object
     * 
     * @returns list of System ids, distance between systems */
    std::pair<std::vector<int>, double> ShortestPath(int system1_id, int system2_id,
                                                     const SystemExclusionPredicateType& system_predicate,
                                                     const ObjectMap& objects) const;

    /** Returns the shortest starlane path distance between any two objects, accounting
      * for cases where one or the other are fleets / ships on starlanes between
      * systems. Returns -1 when no path exists, or either object does not
      * exist. */
    double ShortestPathDistance(int object1_id, int object2_id, const ObjectMap& objects) const;

    /** Returns the sequence of systems, including \a system1 and \a system2,
      * that defines the path with the fewest jumps from \a system1 to
      * \a system2, and the number of jumps to get there.  If no such path
      * exists, the list will be empty.  The path is calculated using the
      * visibility for empire \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function
      * will throw if either system ID is out of range or if the empire ID is
      * not known. */
    std::pair<std::vector<int>, int> LeastJumpsPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES,
                                                    int max_jumps = INT_MAX) const;

    /** Returns whether there is a path known to empire \a empire_id between
      * system \a system1 and system \a system2.  The path is calculated using
      * the visibility for empire \a empire_id, or without regard to visibility
      * if \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function
      * will throw if either system ID is out of range or if the empire ID is
      * not known. */
    bool SystemsConnected(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** Returns true iff \a system is reachable from another system (i.e. it
      * has at least one known starlane to it).   This does not guarantee that
      * the system is reachable from any specific other system, as two separate
      * groups of locally but not globally interconnected systems may exist.
      * The starlanes considered depend on their visibility for empire
      * \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES. */
    bool SystemHasVisibleStarlanes(int system_id, const ObjectMap& objects) const;

    /** Returns the systems that are one starlane hop away from system
      * \a system.  The returned systems are indexed by distance from
      * \a system.  The neighborhood is calculated using the visibility
      * for empire \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES.
      * \throw std::out_of_range This function will throw if the  system
      * ID is out of range. */
    //TODO empire_id is never set to anything other than self, which in
    //the AI's is the same as ALL_EMPIRES
    std::vector<std::pair<double, int>> ImmediateNeighbors(int system_id, int empire_id = ALL_EMPIRES) const;

    /** Returns the system ids of systems that are within \p jumps of the \p
        candidates system ids.*/
    std::vector<int> WithinJumps(std::size_t jumps, std::vector<int> candidates) const;
    std::vector<int> WithinJumps(std::size_t jumps, int candidate) const;

    /** Returns the partition (near, far) of the \p candidate objects into two sets,
        those that are within \p jumps of the \p stationary objects and that are not.*/
    std::pair<Condition::ObjectSet, Condition::ObjectSet>
        WithinJumpsOfOthers(
            int jumps, const ObjectMap& objects,
            const Condition::ObjectSet& candidates,
            const Condition::ObjectSet& stationary) const;

    /** Returns the id of the System object that is closest to the specified
      * (\a x, \a y) location on the map, by direct-line distance. */
    int NearestSystemTo(double x, double y, const ObjectMap& objects) const;

    /** Fills pathfinding data structure and determines least jumps distances
      * between systems. */
    void InitializeSystemGraph(const ObjectMap& objects, const EmpireManager& empires);

    /** Regenerates per-empire system view graphs by filtering the complete
      * system graph based on empire visibilities. Does not regenerate the base
      * graph to account for actual system-starlane connectivity changes. */
    void UpdateCommonFilteredSystemGraphs(const EmpireManager& empires, const ObjectMap& objects);
    void UpdateEmpireVisibilityFilteredSystemGraphs(const EmpireManager& empires,
                                                    const std::map<int, ObjectMap>& empire_object_maps);

    class PathfinderImpl;
private:
    std::unique_ptr<PathfinderImpl> pimpl;
};


#endif
