#ifndef _Fleet_h_
#define _Fleet_h_

#include "UniverseObject.h"


//! Contains info about a single notable point on the move path of a fleet or
//! other UniverseObject.
struct MovePathNode {
    //! Location in Universe of node
    double x, y;

    //! True if the fleet will end a turn at this point
    bool turn_end;

    //! Estimated turns to reach this node
    int eta;

    //! ID of object (most likely a system) located at this node, or
    //! INVALID_OBJECT_ID if there is no object here
    int object_id;

    //! ID of object (most likely a system) at the start of the starlane on
    //! which this MovePathNode is located, or INVALID_OBJECT_ID if not on
    //! a starlane
    int lane_start_id;

    //! ID of object (most likely a system) at the end of the starlane on which
    //! this MovePathNode is located, or INVALID_OBJECT_ID if not on a starlane
    int lane_end_id;

    //! Estimation of whether this node is past a blockade for the subject fleet
    bool post_blockade;
};


//! Encapsulates data for a FreeOrion fleet.  Fleets are basically a group of
//! ships that travel together.
class FO_COMMON_API Fleet : public UniverseObject {
public:
    //! Creates a new fleet with the given name, position and owner id.
    Fleet(const std::string& name, double x, double y, int owner);

    ~Fleet();

    auto HostileToEmpire(int empire_id) const -> bool override;

    auto ObjectType() const -> UniverseObjectType override;

    auto Dump(unsigned short ntabs = 0) const -> std::string override;

    auto ContainerObjectID() const -> int override
    { return SystemID(); }

    auto ContainedObjectIDs() const -> std::set<int> const& override
    { return m_ships; }

    auto Contains(int object_id) const -> bool override;

    auto ContainedBy(int object_id) const -> bool override;

    auto PublicName(int empire_id) const -> std::string const& override;

    auto Accept(UniverseObjectVisitor const& visitor) const -> std::shared_ptr<UniverseObject> override;

    auto ShipIDs() const -> std::set<int> const&
    { return m_ships; }

    //! Returns number of ships in fleet.
    auto NumShips() const -> int
    { return m_ships.size(); }

    //! Returns true if fleet contains no ships, false otherwise.
    auto Empty() const -> bool
    { return m_ships.empty(); }

    auto HasMonsters() const -> bool;

    //! Returns true if there is at least one armed ship in the fleet, meaning
    //! it has direct fire weapons or fighters that can be launched and that do
    //! damage.
    auto HasArmedShips() const -> bool;

    //! Returns true if there is at least one ship with fighters in the fleet.
    auto HasFighterShips() const -> bool;

    //! Returns true if there is at least one colony ship with nonzero capacity
    //! in the fleet.
    auto HasColonyShips() const -> bool;

    //! Returns true if there is at least one colony ship with zero capacity in
    //! the fleet
    auto HasOutpostShips() const -> bool;

    //! Returns true if there is at least one troop ship in the fleet.
    auto HasTroopShips() const -> bool;

    //! Returns true if there is at least one ship ordered scrapped in the
    //! fleet.
    auto HasShipsOrderedScrapped() const -> bool;

    //! Returns true if there is at least one ship without any scrap orders in
    //! the fleet.
    auto HasShipsWithoutScrapOrders() const -> bool;

    //! Adds the ships to the fleet
    void AddShips(const std::vector<int>& ship_ids);

    //! Removes the ships from the fleet.
    void RemoveShips(const std::vector<int>& ship_ids);

    //! Returns the age of the oldest ship in the fleet
    auto MaxShipAgeInTurns() const -> int;

    //! Returns the list of systems that this fleet will move through en route
    //! to its destination (may be empty).
    //!
    //! If this fleet is currently at a system, that system will be the first
    //! one in the list.
    auto TravelRoute() const -> std::list<int> const&
    { return m_travel_route; }

    //! Returns true iff this fleet is moving, but the route is unknown.i
    //!
    //! This is usually the case when a foreign player A's fleet is represented
    //! on another player B's client, and player B has never seen one or more
    //! of the systems in the fleet's route.
    auto UnknownRoute() const -> bool;

    //! Returns true iff this fleet can change its direction while in
    //! interstellar space.
    auto CanChangeDirectionEnRoute() const -> bool
    { return false; }

    //! Sets this fleet to move through the series of systems in the list, in
    //! order
    void SetRoute(std::list<int> const& route);

    //! Sets this fleet to move through the series of systems that makes the
    //! shortest path from its current location to target_system_id.
    void CalculateRouteTo(int target_system_id);

    //! Returns the ID of the empire this fleet has been ordered given to, or
    //! ALL_EMPIRES if this fleet hasn't been ordered given to an empire
    auto OrderedGivenToEmpire() const -> int
    { return m_ordered_given_to_empire_id; }

    //! Marks fleet to be given to empire
    void SetGiveToEmpire(int empire_id);

    //! Marks fleet not to be given to any empire
    void ClearGiveToEmpire()
    { SetGiveToEmpire(ALL_EMPIRES); }

    auto Aggressive() const -> bool
    { return m_aggressive; }

    //! Sets this fleet to be agressive (true) or passive (false)
    void SetAggressive(bool aggressive = true);

    //! Returns a list of locations at which notable events will occur along
    //! the fleet's path if it follows the specified route.
    //!
    //! It is assumed in the calculation that the fleet starts its move path at
    //! its actual current location, however the fleet's current location will
    //! not be on the list, even if it is currently in a system.
    auto MovePath(std::list<int> const& route, bool flag_blockades = false) const -> std::list<MovePathNode>;

    //! Returns MovePath for fleet's current TravelRoute.
    auto MovePath(bool flag_blockades = false) const -> std::list<MovePathNode>;

    //! Returned for ETA estimations, when the fleet can't reach destination
    //! due to lack of route or inability to move.
    static int const ETA_NEVER;

    //! Returned for ETA estimations, when the ETA can't be determined.
    static int const ETA_UNKNOWN;

    //! Returned for ETA estimations, when the fleet can't reacht the
    //! destination due to insufficient fuel capacity and lack of fleet
    //! resupply on route
    static int const ETA_OUT_OF_RANGE;

    //! Returns the number of turns to the final travel destination and the next
    //! system on the current travel path, respectively.
    auto ETA() const -> std::pair<int, int>
    { return ETA(MovePath()); }

    //! Returns the number of turns to the final travel destination and the next
    //! system on the given @p move_path, respectively.
    auto ETA(std::list<MovePathNode> const& move_path) const -> std::pair<int, int>;

    //! Returns the accumulated damage output of this fleet ships.
    auto Damage() const -> float;

    //! Returns the accumulated structural integrity of this fleet ships.
    auto Structure() const -> float;

    //! Returns the accumulated shield defense of this fleet ships.
    auto Shields() const -> float;

    //! Return the least fuel level of this fleet ships.
    auto Fuel() const -> float;

    //! Return the least fuel capacity of this fleet ships.
    auto MaxFuel() const -> float;

    //! Returns ID of system that this fleet is moving to or INVALID_OBJECT_ID
    //! if staying still.
    auto FinalDestinationID() const -> int;

    //! Returns ID of system that this fleet is moving away from as it moves to
    //! its destination.
    auto PreviousSystemID() const -> int
    { return m_prev_system; }

    //! Returns ID of system that this fleet is moving to next as it moves to
    //! its destination.
    auto NextSystemID() const -> int
    { return m_next_system; }

    //! Sets the previous and next systems for this fleet.  Useful after
    //! moving a moving fleet to a different location, so that it moves along
    //! its new local starlanes
    void SetNextAndPreviousSystems(int next, int prev);

    //! Returns true iff either:
    //!  * fleet is stationary and at least one system exit is blocked for this
    //!    fleet.
    //!  * fleet is attempting to depart a system along a blocked system exit
    auto Blockaded() const -> bool;

    //! Returns true iff this fleet's movement would be blockaded at system.
    auto BlockadedAtSystem(int start_system_id, int dest_system_id) const -> bool;

    //! Return the interstellar speed of the slowest ship in this fleet.
    auto Speed() const -> float;

    auto ResourceOutput(ResourceType type) const -> float;

    //! Returns true iff this fleet arrived at its current System this turn.
    auto ArrivedThisTurn() const -> bool
    { return m_arrived_this_turn; }

    //! Used to clear the m_arrived_this_turn flag, prior to any fleets moving,
    //! for accurate blockade tests
    void ClearArrivalFlag()
    { m_arrived_this_turn = false; }

    //! Has two uses: orientation in tactical combat, and determination of
    //! starlane blockade restrictions.
    //!
    //! Returns the ID of the starlane that this fleet arrived on, if it
    //! arrived into a blockade which is not yet broken.
    //!
    //! If in a system and not blockaded, the value is the current system ID.
    //! The blockade intent is that you can't break a blockade unless you beat
    //! the blockaders (via combat or they retreat).
    auto ArrivalStarlane() const -> int
    { return m_arrival_starlane; }

    //! Sets the arrival starlane, used to clear blockaded status after combat.
    void SetArrivalStarlane(int starlane)
    { m_arrival_starlane = starlane; }

    void Copy(std::shared_ptr<UniverseObject const> copied_object, int empire_id = ALL_EMPIRES) override;

    void MovementPhase() override;

    void ResetTargetMaxUnpairedMeters() override;

    /* returns a name for a fleet based on its ships*/
    std::string GenerateFleetName();

protected:
    friend class Universe;
    friend class ObjectMap;
    Fleet();

    template <typename T>
    friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    //! Returns new copy of this Fleet.
    auto Clone(int empire_id = ALL_EMPIRES) const -> Fleet* override;

private:
    std::set<int> m_ships;

    //! These two uniquely describe the starlane graph edge the fleet is
    //! traveling on, if it it's on one.
    //! @{
    int m_prev_system = INVALID_OBJECT_ID;
    int m_next_system = INVALID_OBJECT_ID;
    //! @}

    //! Should this fleet attack enemies in the same system?
    bool m_aggressive = true;

    int m_ordered_given_to_empire_id = ALL_EMPIRES;

    //! List of systems on travel route of fleet from current position to
    //! destination.
    //!
    //! If the fleet is currently in a system, that will be the first system on
    //! the list.  Otherwise, the first system on the list will be the next
    //! system the fleet will reach along its path.
    //!
    //! The list may also contain a single INVALID_OBJECT_ID, which indicates
    //! that the route is unknown.
    //!
    //! The list may also be empty, which indicates that the fleet is not
    //! planning to move.
    std::list<int> m_travel_route;

    bool m_arrived_this_turn = false;

    //! @see ArrivalStarlane()
    int m_arrival_starlane = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const version);
};


#endif // _Fleet_h_
