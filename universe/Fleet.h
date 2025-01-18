#ifndef _Fleet_h_
#define _Fleet_h_

#include "UniverseObject.h"
#include "ScriptingContext.h"
#include "../util/AppInterface.h"
#include "../util/Enum.h"
#include "../util/Export.h"


////////////////////////////////////////////////
// MovePathNode
////////////////////////////////////////////////
/** Contains info about a single notable point on the move path of a fleet or
  * other UniverseObject. */
struct MovePathNode {
    [[nodiscard]] constexpr MovePathNode(double x_, double y_, bool turn_end_, uint8_t eta_,
                                         int id_, int lane_start_id_, int lane_end_id_,
                                         bool blockade_here_, bool post_blockade_) noexcept :
        x(x_),
        y(y_),
        object_id(id_),
        lane_start_id(lane_start_id_),
        lane_end_id(lane_end_id_),
        eta(eta_),
        turn_end(turn_end_),
        blockaded_here(blockade_here_),
        post_blockade(post_blockade_)
    {}
    double  x, y;           ///< location in Universe of node
    int     object_id;      ///< id of object (most likely a system) located at this node, or INVALID_OBJECT_ID if there is no object here
    int     lane_start_id;  ///< id of object (most likely a system) at the start of the starlane on which this MovePathNode is located, or INVALID_OBJECT_ID if not on a starlane
    int     lane_end_id;    ///< id of object (most likely a system) at the end of the starlane on which this MovePathNode is located, or INVALID_OBJECT_ID if not on a starlane
    uint8_t eta;            ///< estimated turns to reach this node
    bool    turn_end;       ///< is this node a location where the fleet will end a turn?
    bool    blockaded_here; ///< is there a blockade at this node?
    bool    post_blockade;  ///< is this node past a blockade for the subject fleet?
};

//! How to Fleets control or not their system?
FO_ENUM(
    (FleetAggression),
    ((INVALID_FLEET_AGGRESSION, -1))
    ((FLEET_PASSIVE))
    ((FLEET_DEFENSIVE))
    ((FLEET_OBSTRUCTIVE))
    ((FLEET_AGGRESSIVE))
    ((NUM_FLEET_AGGRESSIONS))
)

namespace FleetDefaults {
    inline constexpr auto FLEET_DEFAULT_ARMED = FleetAggression::FLEET_AGGRESSIVE;
    inline constexpr auto FLEET_DEFAULT_UNARMED = FleetAggression::FLEET_DEFENSIVE;
}

inline constexpr double FLEET_MOVEMENT_EPSILON = 0.1;  // how close a fleet needs to be to a system to have arrived in the system

/** Encapsulates data for a FreeOrion fleet.  Fleets are basically a group of
  * ships that travel together. */
class FO_COMMON_API Fleet final : public UniverseObject {
public:
    [[nodiscard]] bool         HostileToEmpire(int empire_id, const EmpireManager& empires) const override;

    [[nodiscard]] std::string  Dump(uint8_t ntabs = 0) const override;

    using UniverseObject::IDSet;
    [[nodiscard]] int          ContainerObjectID() const noexcept override { return this->SystemID(); }
    [[nodiscard]] const IDSet& ContainedObjectIDs() const noexcept override { return m_ships; }
    [[nodiscard]] bool         Contains(int object_id) const override;
    [[nodiscard]] bool         ContainedBy(int object_id) const noexcept override;

    [[nodiscard]] const std::string& PublicName(int empire_id, const Universe& universe) const override;

    [[nodiscard]] const auto&        ShipIDs() const noexcept { return m_ships; } ///< returns set of IDs of ships in fleet.
    [[nodiscard]] int                MaxShipAgeInTurns(const ObjectMap& objects, int current_turn) const; ///< Returns the age of the oldest ship in the fleet

    /** Returns the list of systems that this fleet will move through en route
      * to its destination (may be empty).  If this fleet is currently at a
      * system, that system will be the first one in the list. */
    [[nodiscard]] const auto&        TravelRoute() const noexcept { return m_travel_route; };
    [[nodiscard]] int                OrderedGivenToEmpire() const noexcept { return m_ordered_given_to_empire_id; }   ///< returns the ID of the empire this fleet has been ordered given to, or ALL_EMPIRES if this fleet hasn't been ordered given to an empire
    [[nodiscard]] int                LastTurnMoveOrdered() const noexcept { return m_last_turn_move_ordered; }
    [[nodiscard]] bool               Aggressive() const noexcept { return m_aggression >= FleetAggression::FLEET_AGGRESSIVE; }
    [[nodiscard]] bool               Obstructive() const noexcept { return m_aggression >= FleetAggression::FLEET_OBSTRUCTIVE; }
    [[nodiscard]] bool               Passive() const noexcept { return m_aggression <= FleetAggression::FLEET_PASSIVE; }
    [[nodiscard]] FleetAggression    Aggression() const noexcept { return m_aggression; }

    /** Returns a list of locations at which notable events will occur along the fleet's
      * path if it follows the specified route.  It is assumed in the calculation that the
      * fleet starts its move path at its actual current location, however the fleet's
      * current location will not be on the list, even if it is currently in a system. */
    [[nodiscard]] std::vector<MovePathNode> MovePath(const std::vector<int>& route, bool flag_blockades,
                                                     const ScriptingContext& context) const;
    [[nodiscard]] std::vector<MovePathNode> MovePath(bool flag_blockades, const ScriptingContext& context) const;   ///< Returns MovePath for fleet's current TravelRoute

    [[nodiscard]] std::pair<uint8_t, uint8_t> ETA(const ScriptingContext& context) const;            ///< turns which must elapse before the fleet arrives at its current final destination and the turns to the next system, respectively.
    [[nodiscard]] std::pair<uint8_t, uint8_t> ETA(const std::vector<MovePathNode>& move_path) const; ///< turns which must elapse before the fleet arrives at the final destination and next system in the spepcified \a move_path

    [[nodiscard]] float   Damage(const Universe& universe) const;                     ///< total amount of damage this fleet has, which is the sum of the ships' damage
    [[nodiscard]] float   Structure(const ObjectMap& objects) const;                  ///< total amount of structure this fleet has, which is the sum of the ships' structure
    [[nodiscard]] float   Shields(const ObjectMap& objects) const;                    ///< total amount of shields this fleet has, which is the sum of the ships' shields
    [[nodiscard]] float   Fuel(const ObjectMap& objects) const;                       ///< effective amount of fuel this fleet has, which is the least of the amounts of fuel that the ships have
    [[nodiscard]] float   MaxFuel(const ObjectMap& objects) const;                    ///< effective maximum amount of fuel this fleet has, which is the least of the max amounts of fuel that the ships can have
    [[nodiscard]] int     FinalDestinationID() const;                                 ///< ID of system that this fleet is moving to or INVALID_OBJECT_ID if staying still.
    [[nodiscard]] int     PreviousToFinalDestinationID() const;                       ///< ID of system previous to the destination system that this fleet is moving to or INVALID_OBJECT_ID not moving at least two jumps.
    [[nodiscard]] int     PreviousSystemID() const noexcept { return m_prev_system; } ///< ID of system that this fleet is moving away from as it moves to its destination.
    [[nodiscard]] int     NextSystemID() const noexcept     { return m_next_system; } ///< ID of system that this fleet is moving to next as it moves to its destination.

    [[nodiscard]] bool             Blockaded(const ScriptingContext& context) const;  ///< true iff either (i) fleet is stationary and at least one system exit is blocked for this fleet or (ii) fleet is attempting to depart a system along a blocked system exit
    [[nodiscard]] bool             BlockadedAtSystem(int start_system_id, int dest_system_id, const ScriptingContext& context) const; ///< returns true iff this fleet's movement would be blockaded at system.
    [[nodiscard]] std::vector<int> BlockadingFleetsAtSystem(int start_system_id, int dest_system_id, const ScriptingContext& context) const; ///< returns ids of fleets that would blockade this fleet's movement at system.

    [[nodiscard]] float   Speed(const ObjectMap& objects) const;                      ///< speed of fleet. (Should be equal to speed of slowest ship in fleet, unless in future the calculation of fleet speed changes.)
    [[nodiscard]] bool    CanChangeDirectionEnRoute() const noexcept { return false; }///< true iff this fleet can change its direction while in interstellar space.
    [[nodiscard]] bool    CanDamageShips(const ScriptingContext& context,
                                         float target_shields = 0.0f) const;          ///< true if there is at least one ship in the fleet which is currently able to damage a ship using direct weapons or fighters
    [[nodiscard]] bool    CanDestroyFighters(const ScriptingContext& context) const;  ///< true if there is at least one ship in the fleet which is currently able to destroy fighters using direct weapons or fighters
    [[nodiscard]] bool    HasMonsters(const Universe& universe) const;                ///< true iff this fleet contains monster ships.
    [[nodiscard]] bool    HasArmedShips(const ScriptingContext& context) const;       ///< true if there is at least one armed ship in the fleet, meaning it has direct fire weapons or fighters that can be launched and that do damage
    [[nodiscard]] bool    HasFighterShips(const Universe& universe) const;            ///< true if there is at least one ship with fighters in the fleet.
    [[nodiscard]] bool    HasColonyShips(const Universe& universe) const;             ///< true if there is at least one colony ship with nonzero capacity in the fleet.
    [[nodiscard]] bool    HasOutpostShips(const Universe& universe) const;            ///< true if there is at least one colony ship with zero capacity in the fleet
    [[nodiscard]] bool    HasTroopShips(const Universe& universe) const;              ///< true if there is at least one troop ship in the fleet.
    [[nodiscard]] bool    HasShipsOrderedScrapped(const Universe& universe) const;    ///< true if there is at least one ship ordered scrapped in the fleet.
    [[nodiscard]] bool    HasShipsWithoutScrapOrders(const Universe& universe) const; ///< true if there is at least one ship without any scrap orders in the fleet.
    [[nodiscard]] auto    NumShips() const noexcept { return m_ships.size(); }        ///< number of ships in fleet.
    [[nodiscard]] bool    Empty() const noexcept    { return m_ships.empty(); }       ///< true if fleet contains no ships, false otherwise.
    [[nodiscard]] float   ResourceOutput(ResourceType type, const ObjectMap& objects) const;

    /** Returns true iff this fleet is moving, but the route is unknown.  This
      * is usually the case when a foreign player A's fleet is represented on
      * another player B's client, and player B has never seen one or more of
      * the systems in the fleet's route. */
    [[nodiscard]] bool UnknownRoute() const;

    /** Returns true iff this fleet arrived at its current System this turn. */
    [[nodiscard]] bool ArrivedThisTurn() const noexcept { return m_arrived_this_turn; }

    /** Returns the ID of the starlane that this fleet arrived on, if it arrived
      * into a blockade which is not yet broken. If in a system and not
      * blockaded, the value is the current system ID. The blockade intent is
      * that you can't break a blockade unless you beat the blockaders
      * (via combat or they retreat). **/
    [[nodiscard]] int ArrivalStarlane() const noexcept { return m_arrival_starlane; }

    [[nodiscard]] std::size_t SizeInMemory() const override;

    void Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id = ALL_EMPIRES) override;
    void Copy(const Fleet& copied_fleet, const Universe& universe, int empire_id = ALL_EMPIRES);

    /** Moves fleet and its ships, consumes fuel or resupplies ships,
      * and sets systems as explored for empires. */
    void MoveAlongPath(ScriptingContext& context, const std::vector<MovePathNode>& move_path);

    void ResetTargetMaxUnpairedMeters() override;

    /** Sets this fleet to move through the series of systems in the list, in order */
    void SetRoute(std::vector<int> route, const ObjectMap& objects);
    void ClearRoute(const ObjectMap& objects) { SetRoute({}, objects); }
    /** Removes ids in this fleet's route (list of system ids) after \a system_id.
      * If \a system_id is not in the route, the route is cleared. */
    static std::vector<int> TruncateRouteToEndAtFirstOf(std::vector<int> route, int system_id);
    void TruncateRouteToEndAtFirstOf(int system_id)
    { m_travel_route = TruncateRouteToEndAtFirstOf(std::move(m_travel_route), system_id); }
    static std::vector<int> TruncateRouteToEndAtLastOf(std::vector<int> route, int system_id);
    void TruncateRouteToEndAtLAstOf(int system_id)
    { m_travel_route = TruncateRouteToEndAtLastOf(std::move(m_travel_route), system_id); }

    void ResetPrevNextSystems() noexcept { m_next_system = m_prev_system = INVALID_OBJECT_ID; }

    /** Sets this fleet to move through the series of systems that makes the
      * shortest path from its current location to target_system_id */
    void CalculateRouteTo(int target_system_id, const Universe& universe);

    void SetAggression(FleetAggression aggression);         ///< sets this fleet's aggression level towards other fleets

    void AddShips(const std::vector<int>& ship_ids);        ///< adds the ships to the fleet
    void RemoveShips(const std::vector<int>& ship_ids);     ///< removes the ships from the fleet.

    void SetNextAndPreviousSystems(int next, int prev);     ///< sets the previous and next systems for this fleet.  Useful after moving a moving fleet to a different location, so that it moves along its new local starlanes
    void SetArrivalStarlane(int starlane) noexcept  { m_arrival_starlane = starlane; }  ///< sets the arrival starlane, used to clear blockaded status after combat
    void ClearArrivalFlag() noexcept { m_arrived_this_turn = false; }                   ///< used to clear the m_arrived_this_turn flag, prior to any fleets moving, for accurate blockade tests

    void SetGiveToEmpire(int empire_id);                    ///< marks fleet to be given to empire
    void ClearGiveToEmpire();                               ///< marks fleet not to be given to any empire

    void SetMoveOrderedTurn(int turn);                      ///< marks fleet to as being ordered to move on indicated turn

    /* returns a name for a fleet based on its ships*/
    [[nodiscard]] std::string GenerateFleetName(const ScriptingContext& context) const;

    static constexpr uint8_t ETA_NEVER = 255;       ///< returned by ETA when fleet can't reach destination due to lack of route or inability to move
    static constexpr uint8_t ETA_UNKNOWN = 254;     ///< returned when ETA can't be determined
    static constexpr uint8_t ETA_OUT_OF_RANGE = 253;///< returned by ETA when fleet can't reach destination due to insufficient fuel capacity and lack of fleet resupply on route

    explicit Fleet(std::string name = "", double x = INVALID_POSITION, double y = INVALID_POSITION,
                   int owner_id = ALL_EMPIRES, int creation_turn = INVALID_GAME_TURN) :
        UniverseObject(UniverseObjectType::OBJ_FLEET, std::move(name), x, y, owner_id, creation_turn)
    {}

private:
    friend class ObjectMap;
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    /** Returns new copy of this Fleet. */
    [[nodiscard]] std::shared_ptr<UniverseObject> Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const override;

    IDSet m_ships;

    // these two uniquely describe the starlane graph edge the fleet is on, if it it's on one
    int             m_prev_system = INVALID_OBJECT_ID;  ///< the previous system in the route, if any
    int             m_next_system = INVALID_OBJECT_ID;  ///< the next system in the route, if any

    FleetAggression m_aggression = FleetAggression::FLEET_OBSTRUCTIVE;  ///< should this fleet attack enemies in the same system, block their passage, or ignore them

    int             m_ordered_given_to_empire_id = ALL_EMPIRES;
    int             m_last_turn_move_ordered = BEFORE_FIRST_TURN;
    /** list of systems on travel route of fleet from current position to
      * destination.  If the fleet is currently in a system, that will be the
      * first system on the list.  Otherwise, the first system on the list will
      * be the next system the fleet will reach along its path.  The list may
      * also contain a single null pointer, which indicates that the route is
      * unknown.  The list may also be empty, which indicates that the fleet
      * is not planning to move. */
    std::vector<int>m_travel_route;

    int             m_arrival_starlane = INVALID_OBJECT_ID; // see comment for ArrivalStarlane()
    bool            m_arrived_this_turn = false;

    template <typename Archive>
    friend void serialize(Archive&, Fleet&, unsigned int const);
};

#endif
