#ifndef _Order_h_
#define _Order_h_

#include <string>
#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include "Export.h"
#include "../Empire/Empire.h"
#include "../universe/EnumsFwd.h"


class ShipDesign;

/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
/** The abstract base class for serializable player actions.  Orders are
  * generally executed on the client side as soon as they are issued.  Those
  * that define UndoImpl() may also be undone on the client side.  Subclass-
  * defined UndoImpl() \a must return true, indicating that the call had some
  * effect; the default implementation does nothing and returns false. Note that
  * only some Order subclasses define UndoImpl(), specifically those that need
  * to be undone before another order of a similar type can be issued. For
  * example, ColonizeOrder needs to be undoable; otherwise, once the user clicks
  * the colonize button, she is locked in to this decision. */
class FO_COMMON_API Order {
public:
    constexpr Order() noexcept = default;

    /** ctor taking the ID of the Empire issuing the order. */
    constexpr Order(int empire) noexcept :
        m_empire(empire)
    {}

    constexpr virtual ~Order() noexcept = default;

    [[nodiscard]] virtual std::string Dump() const { return ""; }

    /** Returns the ID of the Empire issuing the order. */
    [[nodiscard]] int EmpireID() const noexcept { return m_empire; }

    /** Returns true iff this order has been executed (a second execution
      * indicates server-side execution). */
    [[nodiscard]] bool Executed() const noexcept { return m_executed; }

    /** Executes the order on the Universe and Empires.
     *
     *  Preconditions of Execute():
     *  For all order subclasses, the empire ID for the order
     *  must be that of an existing empire.
     *
     *  The order has not already been executed.
     *
     *  Subclasses add additional preconditions.  An std::runtime_error
     *   should be thrown if any precondition fails.
     */
    void Execute(ScriptingContext& context) const;

    /** If this function returns true, it reverts the game state to what it was
     *  before this order was executed, otherwise it returns false and has no
     *  effect. If an order is undone on the client and then still sent to the server it will be
     *  executed on the server, which is probably not desired. */
    bool Undo(ScriptingContext& context) const;

protected:
    /** Verifies that the empire ID in this order is valid and return the Empire pointer.
     *  Throws an std::runtime_error if not valid. */
    std::shared_ptr<Empire> GetValidatedEmpire(ScriptingContext& context) const;

private:
    virtual void ExecuteImpl(ScriptingContext& context) const = 0;
    virtual bool UndoImpl(ScriptingContext& context) const { return false; }

    int m_empire = ALL_EMPIRES;

    /** Indicates that Execute() has occured, and so an undo is legal. */
    mutable bool m_executed = false;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// RenameOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents the renaming of a UniverseObject. */
class FO_COMMON_API RenameOrder final : public Order {
public:
    RenameOrder(int empire, int object, std::string name, const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of fleet selected in this order. */
    [[nodiscard]] int ObjectID() const noexcept { return m_object; }

    /** Returns the new name of the fleet. */
    [[nodiscard]] const std::string& Name() const noexcept { return m_name; }

    //! Returns true when the Order parameters are valid.
    [[nodiscard]] static bool Check(int empire, int object, std::string new_name,
                                    const ScriptingContext& context);

private:
    RenameOrder() = default;

    /**
     * Preconditions of execute:
     *    - the designated planet must exist, be owned by the issuing empire
     *
     *  Postconditions:
     *    - the planet focus is changed which=0(primary),1(secondary)
     *
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    int m_object = INVALID_OBJECT_ID;
    std::string m_name;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// NewFleetOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents forming a new fleet.
    Only one of system or position will be used to place the new fleet.*/
class FO_COMMON_API NewFleetOrder final : public Order {
public:
    NewFleetOrder(int empire, std::string fleet_name,
                  std::vector<int> ship_ids, const ScriptingContext& context,
                  bool aggressive, bool passive = false, bool defensive = false);
    NewFleetOrder(int empire, std::string fleet_name,
                  std::vector<int> ship_ids, FleetAggression aggression,
                  const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    [[nodiscard]] const std::string& FleetName() const noexcept { return m_fleet_name; }

    [[nodiscard]] const int& FleetID() const noexcept { return m_fleet_id; }

    [[nodiscard]] const std::vector<int>& ShipIDs() const noexcept { return m_ship_ids; }

    [[nodiscard]] bool Aggressive() const noexcept;

    [[nodiscard]] FleetAggression Aggression() const noexcept { return m_aggression; }

    [[nodiscard]] static bool Check(int empire, const std::string& fleet_name,
                                    const std::vector<int>& ship_ids, FleetAggression aggression,
                                    const ScriptingContext& context);
private:
    NewFleetOrder() = default;

    /**
     * Preconditions of execute:
     *    None.
     *
     *  Postconditions:
     *    - new fleets will exist in system with id m_system_id,
     *      and will belong to the creating empire.
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    std::string m_fleet_name;
    /** m_fleet_id is mutable because ExecuteImpl generates the fleet id. */
    mutable int m_fleet_id = INVALID_OBJECT_ID;
    std::vector<int> m_ship_ids;
    FleetAggression m_aggression{0};

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// FleetMoveOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents fleet movement
    These orders change the current destination of a fleet */
class FO_COMMON_API FleetMoveOrder final : public Order {
public:
    FleetMoveOrder(int empire_id, int fleet_id, int dest_system_id, bool append,
                   const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of fleet selected in this order. */
    [[nodiscard]] int FleetID() const noexcept { return m_fleet; }

    /* Returns ID of system set as destination for this order. */
    [[nodiscard]] int DestinationSystemID() const noexcept { return m_dest_system; }

    /* Returns the IDs of the systems in the route specified by this Order. */
    [[nodiscard]] const std::vector<int>& Route() const noexcept { return m_route; }

    static bool Check(int empire_id, int fleet_id, int dest_fleet_id, bool append,
                      const ScriptingContext& context);
private:
    FleetMoveOrder() = default;

    /**
     * Preconditions of execute:
     *    - m_fleet is a valid id of a fleet owned by the order-giving empire
     *    - if the fleet is located in a system, m_start_system is the id of that system
     *    - if the fleet is not located in a system, m_start_system is the id of the system the fleet is moving to
     *    -
     *
     *  Postconditions:
     *    - TODO: WRITE THIS
     *
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    int              m_fleet = INVALID_OBJECT_ID;
    int              m_dest_system = INVALID_OBJECT_ID;
    std::vector<int> m_route;
    bool             m_append = false;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// FleetTransferOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents transfer of ships between existing fleets
  * A FleetTransferOrder is used to transfer ships from one existing fleet to
  * another. */
class FO_COMMON_API FleetTransferOrder final : public Order {
public:
    FleetTransferOrder(int empire, int dest_fleet, std::vector<int> ships,
                       const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /* Returns ID of the fleet that the ships will go into. */
    [[nodiscard]] int DestinationFleet() const noexcept { return m_dest_fleet; }

    /** Returns IDs of the ships selected for addition to the fleet. */
    [[nodiscard]] const std::vector<int>& Ships() const noexcept { return m_add_ships; }

    [[nodiscard]] static bool Check(int empire_id, int dest_fleet_id, const std::vector<int>& ship_ids,
                                    const ScriptingContext& context);

private:
    FleetTransferOrder() = default;

    /**
     *  FleetTransferOrder's preconditions are:
     *    - m_into_fleet must be the ID of a fleet owned by the issuing empire
     *    - each element of m_add_ships must be the ID of a ship owned by the issuing empire
     *
     *  Postconditions:
     *     - all ships in m_add_ships will be moved from their initial fleet to the destination fleet
     *     - any resulting empty fleets will be deleted
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    int m_dest_fleet = INVALID_OBJECT_ID;
    std::vector<int> m_add_ships;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// AnnexOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet colonization action*/
class FO_COMMON_API AnnexOrder final : public Order {
public:
    AnnexOrder(int empire, int planet, const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of the planet to be colonized. */
    [[nodiscard]] int PlanetID() const noexcept { return m_planet; }

    static bool Check(int empire_id, int planet_id, const ScriptingContext& context);

private:
    AnnexOrder() = default;

    /**
    *  Preconditions:
    *     - m_planet must be the ID of an un-owned planet.
    *
    *  Postconditions:
    *      - The planet with ID will be marked to be annexed by the empire during the next turn processing.
    */
    void ExecuteImpl(ScriptingContext& context) const override;

    bool UndoImpl(ScriptingContext& context) const override;

    int m_planet = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ColonizeOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet colonization action*/
class FO_COMMON_API ColonizeOrder final : public Order {
public:
    ColonizeOrder(int empire, int ship, int planet, const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of the planet to be colonized. */
    [[nodiscard]] int PlanetID() const noexcept { return m_planet; }

    /** Returns ID of the ship which is colonizing the planet. */
    [[nodiscard]] int ShipID() const noexcept { return m_ship; }

    static bool Check(int empire_id, int ship_id, int planet_id, const ScriptingContext& context);

private:
    ColonizeOrder() = default;

    /**
     *  Preconditions:
     *     - m_planet must be the ID of an un-owned planet.
     *     - m_ship must be the ID of a ship owned by the issuing empire
     *     - m_ship must be the ID of a ship that can colonize and that is in
     *       the same system as the planet.
     *
     *  Postconditions:
     *      - The ship with ID m_ship will be marked to colonize the planet with
     *        id m_planet during the next turn processing.
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    bool UndoImpl(ScriptingContext& context) const override;

    int m_ship = INVALID_OBJECT_ID;
    int m_planet = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// InvadeOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet invasion action*/
class FO_COMMON_API InvadeOrder final : public Order {
public:
    InvadeOrder(int empire, int ship, int planet, const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of the planet to be invaded. */
    [[nodiscard]] int PlanetID() const noexcept { return m_planet; }

    /** Returns ID of the ship which is invading the planet. */
    [[nodiscard]] int ShipID() const noexcept { return m_ship; }

    static bool Check(int empire_id, int ship_id, int planet_id, const ScriptingContext& context);

private:
    InvadeOrder() = default;

    /**
     *  Preconditions:
     *     - m_planet must be the ID of a populated planet not owned by the issuing empire
     *     - m_ship must be the ID of a ship owned by the issuing empire
     *     - m_ship must be the ID of a ship that can invade and that is in
     *       the same system as the planet.
     *
     *  Postconditions:
     *      - The ship with ID m_ship will be marked to invade the planet with
     *        id m_planet during the next turn processing.
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    bool UndoImpl(ScriptingContext& context) const override;

    int m_ship = INVALID_OBJECT_ID;
    int m_planet = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// BombardOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet bombardment action*/
class FO_COMMON_API BombardOrder final : public Order {
public:
    BombardOrder(int empire, int ship, int planet, const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of the planet to be bombarded. */
    [[nodiscard]] int PlanetID() const noexcept { return m_planet; }

    /** Returns ID of the ship which is bombarding the planet. */
    [[nodiscard]] int ShipID() const noexcept { return m_ship; }

    static bool Check(int empire_id, int ship_id, int planet_id, const ScriptingContext& context);

private:
    BombardOrder() = default;

    /**
     *  Preconditions:
     *     - m_planet must be the ID of a planet
     *     - m_ship must be the ID of a ship owned by the issuing empire
     *
     *  Postconditions:
     *      - The ship with ID m_ship will be marked to bombard the planet with
     *        id m_planet during the next turn processing.
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    bool UndoImpl(ScriptingContext& context) const override;

    int m_ship = INVALID_OBJECT_ID;
    int m_planet = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ChangeFocusOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents changing a planet focus*/
class FO_COMMON_API ChangeFocusOrder final : public Order {
public:
    ChangeFocusOrder(int empire, int planet, std::string focus,
                     const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /* Returns ID of the fleet to be deleted. */
    [[nodiscard]] int PlanetID() const noexcept { return m_planet; }

    static bool Check(int empire_id, int planet_id, const std::string& focus, const ScriptingContext& context);

private:
    ChangeFocusOrder() = default;

    /**
     * Preconditions of execute:
     *    - the designated planet must exist, be owned by the issuing empire
     *
     *  Postconditions:
     *    - the planet focus is changed
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    int m_planet = INVALID_OBJECT_ID;
    std::string m_focus;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/////////////////////////////////////////////////////
// PolicyOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents the adoptiong imperial polices. */
class FO_COMMON_API PolicyOrder final : public Order {
public:
    PolicyOrder(int empire, std::string name, std::string category, int slot = -1) : // adopt
        Order(empire),
        m_policy_name(std::move(name)),
        m_category(std::move(category)),
        m_slot(slot),
        m_adopt(true)
    {}

    PolicyOrder(int empire, std::string name) : // de-adopt
        Order(empire),
        m_policy_name(std::move(name))
    {}

    PolicyOrder(int empire) : // revert all changes
        Order(empire),
        m_revert{true}
    {}

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of fleet selected in this order. */
    [[nodiscard]] const auto& PolicyName() const noexcept   { return m_policy_name; }
    [[nodiscard]] const auto& CategoryName() const noexcept { return m_category; }
    [[nodiscard]] bool        Adopt() const noexcept        { return m_adopt; }
    [[nodiscard]] int         Slot() const noexcept         { return m_slot; }

private:
    PolicyOrder() = default;

    void ExecuteImpl(ScriptingContext& context) const override;

    std::string m_policy_name;
    std::string m_category;
    int         m_slot = -1;
    bool        m_adopt = false;
    bool        m_revert = false;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ResearchQueueOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents changing an empire's research queue.  The
  * 2-arg ctor removes the named tech from \a empire's queue, whereas the 3-arg
  * ctor places \a tech_name at position \a position in \a empire's research queue. */
class FO_COMMON_API ResearchQueueOrder final : public Order {
public:
    ResearchQueueOrder(int empire, std::string tech_name);
    ResearchQueueOrder(int empire, std::string tech_name, int position);
    ResearchQueueOrder(int empire, std::string tech_name, bool pause, float dummy);

    [[nodiscard]] std::string Dump() const override;

private:
    ResearchQueueOrder() = default;

    void ExecuteImpl(ScriptingContext& context) const override;

    std::string m_tech_name;
    int m_position = INVALID_INDEX;
    bool m_remove = false;
    int m_pause = INVALID_PAUSE_RESUME;

    static constexpr int INVALID_INDEX = -500;
    static constexpr int PAUSE = 1;
    static constexpr int RESUME = 2;
    static constexpr int INVALID_PAUSE_RESUME = -1;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ProductionQueueOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents changing an empire's production queue.
  * The ProductionItem ctor adds the build to the beginning or end of \a empire's queue, the 3-arg
  * ctor moves an existing build from its current location at \a index to a new
  * one at \a new_index, and the 2-arg ctor removes the build at \a index from
  * \a empire's queue. */
class FO_COMMON_API ProductionQueueOrder final : public Order {
public:
    enum class ProdQueueOrderAction : int8_t {
        INVALID_PROD_QUEUE_ACTION = -1,
        PLACE_IN_QUEUE,
        REMOVE_FROM_QUEUE,
        SPLIT_INCOMPLETE,
        DUPLICATE_ITEM,
        SET_QUANTITY_AND_BLOCK_SIZE,
        SET_QUANTITY,
        MOVE_ITEM_TO_INDEX,
        SET_RALLY_POINT,
        PAUSE_PRODUCTION,
        RESUME_PRODUCTION,
        ALLOW_STOCKPILE_USE,
        DISALLOW_STOCKPILE_USE,
        UNREMOVE_FROM_QUEUE,
        NUM_PROD_QUEUE_ACTIONS
    };

    ProductionQueueOrder(ProdQueueOrderAction action, int empire, ProductionQueue::ProductionItem item,
                         int number, int location, int pos = -1);
    // num1 and num2 may be quantity and blocksize, or just quantity, or rally point id, or new index in queue
    ProductionQueueOrder(ProdQueueOrderAction action, int empire, boost::uuids::uuid uuid,
                         int num1 = -1, int num2 = -1);

    [[nodiscard]] std::string Dump() const override;

private:
    ProductionQueueOrder() = default;

    void ExecuteImpl(ScriptingContext& context) const override;

    ProductionQueue::ProductionItem m_item;
    int                             m_location = INVALID_OBJECT_ID;
    int                             m_new_quantity = INVALID_QUANTITY;
    int                             m_new_blocksize = INVALID_QUANTITY;
    int                             m_new_index = INVALID_INDEX;
    int                             m_rally_point_id = INVALID_OBJECT_ID;
    boost::uuids::uuid              m_uuid = boost::uuids::nil_uuid();
    boost::uuids::uuid              m_uuid2 = boost::uuids::nil_uuid();
    ProdQueueOrderAction            m_action = ProdQueueOrderAction::INVALID_PROD_QUEUE_ACTION;

    static constexpr int INVALID_INDEX = -500;
    static constexpr int INVALID_QUANTITY = -1000;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ShipDesignOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents manipulating an empire's ship designs.
  * The 2-arg ctor adds the existing ship design to the \a empire's set of
  * designs - remembering, or "keeping" the design and enabling the \a empire to
  * produce ships of that design (if all design prerequisites are met)
  * The 3-arg ctor taking a bool removes the indicated design from the empire's
  * set of remembered designs.
  * The 3-arg ctor taking a ShipDesign argument creates a new shipdesign in the
  * universe's catalog of shipdesigns with the passed new design id, and adds
  * this design to the \a empire's set of remembered designs.  The new design
  * must be marked as designed by this \a empire.
  */
class FO_COMMON_API ShipDesignOrder final : public Order {
public:
    ShipDesignOrder(int empire_id, int existing_design_id_to_remember,
                    const ScriptingContext& context);
    ShipDesignOrder(int empire_id, int design_id_to_erase, bool dummy,
                    const ScriptingContext& context);
    ShipDesignOrder(int empire_id, const ShipDesign& ship_design,
                    const ScriptingContext& context);
    ShipDesignOrder(int empire_id, int existing_design_id, std::string new_name,
                    std::string new_description,
                    const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    [[nodiscard]] int DesignID() const noexcept { return m_design_id; }

    static bool CheckRemember(int empire_id, int existing_design_id_to_remember, const ScriptingContext& context);

    static bool CheckErase(int empire_id, int design_id_to_erase, bool dummy, const ScriptingContext& context);

    static bool CheckNew(int empire_id, const std::string& name, const std::string& desc,
                         const std::string& hull, const std::vector<std::string>& parts,
                         const ScriptingContext& context);

    static bool CheckRename(int empire_id, int existing_design_id, const std::string& new_name,
                            const std::string& new_description, const ScriptingContext& context);

private:
    ShipDesignOrder() = default;

    /**
     * Preconditions of execute:
     *    - For creating a new design, the passed design is a valid reference
     *      to a design created by the empire issuing the order
     *    - For remembering an existing ship design, there exists a ship design
     *      with the passed id, and the empire is aware of this ship design
     *    - For removing a shipdesign from the empire's set of designs, there
     *      empire has a design with the passed id in its set of designs
     *
     *  Postconditions:
     *    - For creating a new ship design, the universe will contain a new ship
     *      design, and the creating empire will have the new design as of of
     *      its designs
     *    - For remembering a ship design, the empire will have the design's id
     *      in its set of design ids
     *    - For removing a design, the empire will no longer have the design's
     *      id in its set of design ids
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    boost::uuids::uuid       m_uuid = boost::uuids::nil_generator()();

    // details of design to create
    std::string              m_name;
    std::string              m_description;
    std::string              m_hull;
    std::vector<std::string> m_parts;
    std::string              m_icon;
    std::string              m_3D_model;

    mutable int              m_design_id = INVALID_DESIGN_ID; /// m_design_id is mutable to save the id for the server when the client calls ExecuteImpl.

    int                      m_designed_on_turn = 0;
    bool                     m_update_name_or_description = false;
    bool                     m_delete_design_from_empire = false;
    bool                     m_create_new_design = false;
    bool                     m_is_monster  = false;
    bool                     m_name_desc_in_stringtable = false;
    // end details of design to create

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ScrapOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents the scrapping / recycling / destroying
  * a building or ship owned by an empire. */
class FO_COMMON_API ScrapOrder final : public Order {
public:
    ScrapOrder(int empire, int object_id, const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of object selected in this order. */
    [[nodiscard]] int ObjectID() const noexcept { return m_object_id; }

    static bool Check(int empire_id, int object_id, const ScriptingContext& context);
private:
    ScrapOrder() = default;

    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object owned by issuing empire
     *     - the object must be scrappable: ships or buildings
     *
     *  Postconditions:
     *     - the object is marked to be scrapped during the next turn processing.
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    bool UndoImpl(ScriptingContext& context) const override;

    int m_object_id = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// AggressiveOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents setting the aggression state of objects
  * controlled by an empire. */
class FO_COMMON_API AggressiveOrder final : public Order {
public:
    AggressiveOrder(int empire, int object_id, FleetAggression aggression,
                    const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of object selected in this order. */
    [[nodiscard]] int ObjectID() const noexcept { return m_object_id; }

    /** Returns aggression state to set object to. */
    [[nodiscard]] FleetAggression Aggression() const noexcept { return m_aggression; }

    static bool Check(int empire_id, int object_id, FleetAggression aggression,
                      const ScriptingContext& context);

private:
    AggressiveOrder() = default;

    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object owned by issuing empire
     *     - the object must have an aggression status: fleets
     *
     *  Postconditions:
     *     - the object is set to the new aggression state
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    int m_object_id = INVALID_OBJECT_ID;
    FleetAggression m_aggression{};

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// GiveObjectToEmpireOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents giving control of a ship to
  * another empire */
class FO_COMMON_API GiveObjectToEmpireOrder final : public Order {
public:
    GiveObjectToEmpireOrder(int empire, int object_id, int recipient,
                            const ScriptingContext& context);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of object selected in this order. */
    [[nodiscard]] int ObjectID() const noexcept { return m_object_id; }

    /** Returns ID of empire to which object is given. */
    [[nodiscard]] int RecipientEmpireID() const noexcept { return m_recipient_empire_id; }

    static bool Check(int empire_id, int object_id, int recipient_empire_id,
                      const ScriptingContext& context);
private:
    GiveObjectToEmpireOrder() = default;

    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object owned by issuing empire
     *     - m_recipient_empire_id must be the ID of another empire
     *
     *  Postconditions:
     *     - the object's ownership is set to the other empire
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    bool UndoImpl(ScriptingContext& context) const override;

    int m_object_id = INVALID_OBJECT_ID;
    int m_recipient_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/////////////////////////////////////////////////////
// ForgetOrder
/////////////////////////////////////////////////////
/** ForgetOrder removes the object from the empire's known objects. */
class FO_COMMON_API ForgetOrder final : public Order {
public:
    ForgetOrder(int empire, int object_id);

    [[nodiscard]] std::string Dump() const override;

    /** Returns ID of object selected in this order. */
    [[nodiscard]] int ObjectID() const noexcept { return m_object_id; }

private:
    ForgetOrder() = default;

    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object not owned by issuing empire
     *
     *  Postconditions:
     *     - the object is removed from the table of known objects.
     */
    void ExecuteImpl(ScriptingContext& context) const override;

    int m_object_id = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif
