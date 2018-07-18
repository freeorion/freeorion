#ifndef _Order_h_
#define _Order_h_

#include "../universe/EnumsFwd.h"
#include "Export.h"
#include "../Empire/Empire.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/uuid/uuid.hpp>

#include <string>
#include <vector>

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
    /** \name Structors */ //@{
    Order()
    {}

    /** ctor taking the ID of the Empire issuing the order. */
    Order(int empire) :
        m_empire(empire)
    {}

    virtual ~Order()
    {}
    //@}

    /** \name Accessors */ //@{
    /** Returns the ID of the Empire issuing the order. */
    int EmpireID() const { return m_empire; }

    /** Returns true iff this order has been executed (a second execution
      * indicates server-side execution). */
    bool Executed() const;
    //@}

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
    void Execute() const;

    /** If this function returns true, it reverts the game state to what it was
     *  before this order was executed, otherwise it returns false and has no
     *  effect. If an order is undone on the client and then still sent to the server it will be
     *  executed on the server, which is probably not desired. */
    bool Undo() const;

protected:
    /** \name Mutators */ //@{
    /** Verifies that the empire ID in this order is valid and return the Empire pointer.
     *  Throws an std::runtime_error if not valid. */
    Empire* GetValidatedEmpire() const;
    //@}

private:
    virtual void ExecuteImpl() const = 0;
    virtual bool UndoImpl() const;

    int m_empire = ALL_EMPIRES;

    /** Indicates that Execute() has occured, and so an undo is legal. */
    mutable bool m_executed = false;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// RenameOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents the renaming of a UniverseObject. */
class FO_COMMON_API RenameOrder : public Order {
public:
    /** \name Structors */ //@{
    RenameOrder(int empire, int object, const std::string& name);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of fleet selected in this order. */
    int ObjectID() const
    { return m_object; }

    /** Returns the new name of the fleet. */
    const std::string& Name() const
    { return m_name; }
    //@}

    //! Returns true when the Order parameters are valid.
    static bool Check(int empire, int object, const std::string& new_name);

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
    void ExecuteImpl() const override;

    int m_object = INVALID_OBJECT_ID;
    std::string m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// NewFleetOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents forming a new fleet.
    Only one of system or position will be used to place the new fleet.*/
class FO_COMMON_API NewFleetOrder : public Order {
public:
    /** \name Structors */ //@{
    NewFleetOrder(int empire, const std::string& fleet_name,
                  const std::vector<int>& ship_ids,
                  bool aggressive);
    //@}

    /** \name Accessors */ //@{
    const std::string& FleetName() const
    { return m_fleet_name; }

    const int& FleetID() const
    { return m_fleet_id; }

    const std::vector<int>& ShipIDs() const
    { return m_ship_ids; }

    bool Aggressive() const
    { return m_aggressive; }
    //@}

    static bool Check(int empire, const std::string& fleet_name, const std::vector<int>& ship_ids, bool aggressive);
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
    void ExecuteImpl() const override;

    std::string m_fleet_name;
    /** m_fleet_id is mutable because ExecuteImpl generates the fleet id. */
    mutable int m_fleet_id;
    std::vector<int> m_ship_ids;
    bool m_aggressive;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// FleetMoveOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents fleet movement
    These orders change the current destination of a fleet */
class FO_COMMON_API FleetMoveOrder : public Order {
public:
    /** \name Structors */ //@{
    FleetMoveOrder(int empire_id, int fleet_id, int dest_system_id,
                   bool append = false);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of fleet selected in this order. */
    int FleetID() const
    { return m_fleet; }

    /* Returns ID of system set as destination for this order. */
    int DestinationSystemID() const
    { return m_dest_system; }

    /* Returns the IDs of the systems in the route specified by this Order. */
    const std::vector<int>& Route() const
    { return m_route; }
    //@}

    static bool Check(int empire_id, int fleet_id, int dest_fleet_id, bool append = false);
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
    void ExecuteImpl() const override;

    int m_fleet = INVALID_OBJECT_ID;
    int m_dest_system = INVALID_OBJECT_ID;
    std::vector<int> m_route;
    bool m_append = false;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// FleetTransferOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents transfer of ships between existing fleets
  * A FleetTransferOrder is used to transfer ships from one existing fleet to
  * another. */
class FO_COMMON_API FleetTransferOrder : public Order {
public:
    /** \name Structors */ //@{
    FleetTransferOrder(int empire, int dest_fleet, const std::vector<int>& ships);
    //@}

    /** \name Accessors */ //@{
    /* Returns ID of the fleet that the ships will go into. */
    int DestinationFleet() const
    { return m_dest_fleet; }

    /** Returns IDs of the ships selected for addition to the fleet. */
    const std::vector<int>& Ships() const
    { return m_add_ships; }
    //@}

    static bool Check(int empire_id, int dest_fleet_id, const std::vector<int>& ship_ids);

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
    void ExecuteImpl() const override;

    int m_dest_fleet = INVALID_OBJECT_ID;
    std::vector<int> m_add_ships;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ColonizeOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet colonization action*/
class FO_COMMON_API ColonizeOrder : public Order {
public:
    /** \name Structors */ //@{
    ColonizeOrder(int empire, int ship, int planet);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of the planet to be colonized. */
    int PlanetID() const
    { return m_planet; }

    /** Returns ID of the ship which is colonizing the planet. */
    int ShipID() const
    { return m_ship; }
    //@}

    static bool Check(int empire_id, int ship_id, int planet_id);

private:
    ColonizeOrder() = default;

    /**
     *  Preconditions:
     *     - m_planet must be the ID of an un-owned planet.
     *     - m_ship must be the the ID of a ship owned by the issuing empire
     *     - m_ship must be the ID of a ship that can colonize and that is in
     *       the same system as the planet.
     *
     *  Postconditions:
     *      - The ship with ID m_ship will be marked to colonize the planet with
     *        id m_planet during the next turn processing.
     */
    void ExecuteImpl() const override;

    bool UndoImpl() const override;

    int m_ship = INVALID_OBJECT_ID;
    int m_planet = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// InvadeOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet invasion action*/
class FO_COMMON_API InvadeOrder : public Order {
public:
    /** \name Structors */ //@{
    InvadeOrder(int empire, int ship, int planet);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of the planet to be invaded. */
    int PlanetID() const
    { return m_planet; }

    /** Returns ID of the ship which is invading the planet. */
    int ShipID() const
    { return m_ship; }
    //@}

    static bool Check(int empire_id, int ship_id, int planet_id);

private:
    InvadeOrder() = default;

    /**
     *  Preconditions:
     *     - m_planet must be the ID of a populated planet not owned by the issuing empire
     *     - m_ship must be the the ID of a ship owned by the issuing empire
     *     - m_ship must be the ID of a ship that can invade and that is in
     *       the same system as the planet.
     *
     *  Postconditions:
     *      - The ship with ID m_ship will be marked to invade the planet with
     *        id m_planet during the next turn processing.
     */
    void ExecuteImpl() const override;

    bool UndoImpl() const override;

    int m_ship = INVALID_OBJECT_ID;
    int m_planet = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// BombardOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet bombardment action*/
class FO_COMMON_API BombardOrder : public Order {
public:
    /** \name Structors */ //@{
    BombardOrder(int empire, int ship, int planet);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of the planet to be bombarded. */
    int PlanetID() const
    { return m_planet; }

    /** Returns ID of the ship which is bombarding the planet. */
    int ShipID() const
    { return m_ship; }
    //@}

    static bool Check(int empire_id, int ship_id, int planet_id);

private:
    BombardOrder() = default;

    /**
     *  Preconditions:
     *     - m_planet must be the ID of a planet
     *     - m_ship must be the the ID of a ship owned by the issuing empire
     *
     *  Postconditions:
     *      - The ship with ID m_ship will be marked to bombard the planet with
     *        id m_planet during the next turn processing.
     */
    void ExecuteImpl() const override;

    bool UndoImpl() const override;

    int m_ship = INVALID_OBJECT_ID;
    int m_planet = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ChangeFocusOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents changing a planet focus*/
class FO_COMMON_API ChangeFocusOrder : public Order {
public:
    /** \name Structors */ //@{
    ChangeFocusOrder(int empire, int planet, const std::string& focus);
    //@}

    /** \name Accessors */ //@{
    /* Returns ID of the fleet to be deleted. */
    int PlanetID() const
    { return m_planet; }
    //@}

    static bool Check(int empire_id, int planet_id, const std::string& focus);

private:
    ChangeFocusOrder() = default;

    /**
     * Preconditions of execute:
     *    - the designated planet must exist, be owned by the issuing empire
     *
     *  Postconditions:
     *    - the planet focus is changed
     */
    void ExecuteImpl() const override;

    int m_planet = INVALID_OBJECT_ID;
    std::string m_focus;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ResearchQueueOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents changing an empire's research queue.  The
  * 2-arg ctor removes the named tech from \a empire's queue, whereas the 3-arg
  * ctor places \a tech_name at position \a position in \a empire's research queue. */
class FO_COMMON_API ResearchQueueOrder : public Order {
public:
    /** \name Structors */ //@{
    ResearchQueueOrder(int empire, const std::string& tech_name);
    ResearchQueueOrder(int empire, const std::string& tech_name, int position);
    ResearchQueueOrder(int empire, const std::string& tech_name, bool pause, float dummy);
    //@}

private:
    ResearchQueueOrder() = default;

    void ExecuteImpl() const override;

    std::string m_tech_name;
    int m_position = INVALID_INDEX;
    bool m_remove = false;
    int m_pause = INVALID_PAUSE_RESUME;

    static const int INVALID_INDEX = -500;
    static const int PAUSE = 1;
    static const int RESUME = 2;
    static const int INVALID_PAUSE_RESUME = -1;

    friend class boost::serialization::access;
    template <class Archive>
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
class FO_COMMON_API ProductionQueueOrder : public Order {
public:
    /** \name Structors */ //@{
    ProductionQueueOrder(int empire, const ProductionQueue::ProductionItem& item, int number, int location, int pos = -1);
    ProductionQueueOrder(int empire, int index, int new_quantity, bool dummy);
    ProductionQueueOrder(int empire, int index, int rally_point_id, bool dummy1, bool dummy2);
    ProductionQueueOrder(int empire, int index, int new_quantity, int new_blocksize);
    ProductionQueueOrder(int empire, int index, int new_index);
    ProductionQueueOrder(int empire, int index);
    ProductionQueueOrder(int empire, int index, bool pause, float dummy);
    ProductionQueueOrder(int empire, int index, float dummy1);
    ProductionQueueOrder(int empire, int index, float dummy1, float dummy2);
    ProductionQueueOrder(int empire, int index, bool allow_use_imperial_pp, float dummy, float dummy2);
    //@}

private:
    ProductionQueueOrder() = default;

    void ExecuteImpl() const override;

    ProductionQueue::ProductionItem m_item;
    int m_number = 0;
    int m_location = INVALID_OBJECT_ID;
    int m_index = INVALID_INDEX;
    int m_new_quantity = INVALID_QUANTITY;
    int m_new_blocksize = INVALID_QUANTITY;
    int m_new_index = INVALID_INDEX;
    int m_rally_point_id = INVALID_OBJECT_ID;
    int m_pause = INVALID_PAUSE_RESUME;
    int m_split_incomplete = INVALID_SPLIT_INCOMPLETE;
    int m_dupe = INVALID_SPLIT_INCOMPLETE;
    int m_use_imperial_pp = INVALID_USE_IMPERIAL_PP;

    static const int INVALID_INDEX = -500;
    static const int INVALID_QUANTITY = -1000;
    static const int PAUSE = 1;
    static const int RESUME = 2;
    static const int INVALID_PAUSE_RESUME = -1;
    static const int INVALID_SPLIT_INCOMPLETE = -1;
    static const int USE_IMPERIAL_PP = 4;
    static const int DONT_USE_IMPERIAL_PP = 8;
    static const int INVALID_USE_IMPERIAL_PP = -4;

    friend class boost::serialization::access;
    template <class Archive>
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
class FO_COMMON_API ShipDesignOrder : public Order {
public:
    /** \name Structors */ //@{
    ShipDesignOrder(int empire, int existing_design_id_to_remember);
    ShipDesignOrder(int empire, int design_id_to_erase, bool dummy);
    ShipDesignOrder(int empire, const ShipDesign& ship_design);
    ShipDesignOrder(int empire, int existing_design_id, const std::string& new_name,
                    const std::string& new_description = "");
    //@}

    int DesignID() const
    { return m_design_id; }

private:
    ShipDesignOrder();

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
    void ExecuteImpl() const override;

    /// m_design_id is mutable to save the id for the server when the client calls ExecuteImpl.
    mutable int m_design_id = INVALID_DESIGN_ID;
    boost::uuids::uuid m_uuid;
    bool m_update_name_or_description = false;
    bool m_delete_design_from_empire = false;
    bool m_create_new_design = false;

    // details of design to create
    std::string m_name;
    std::string m_description;
    int m_designed_on_turn = 0;
    std::string m_hull;
    std::vector<std::string> m_parts;
    bool m_is_monster  = false;
    std::string m_icon;
    std::string m_3D_model;
    bool m_name_desc_in_stringtable = false;
    // end details of design to create

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ScrapOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents the scrapping / recycling / destroying
  * a building or ship owned by an empire. */
class FO_COMMON_API ScrapOrder : public Order {
public:
    /** \name Structors */ //@{
    ScrapOrder(int empire, int object_id);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of object selected in this order. */
    int ObjectID() const
    { return m_object_id; }
    //@}

    static bool Check(int empire_id, int object_id);
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
    void ExecuteImpl() const override;

    bool UndoImpl() const override;

    int m_object_id = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// AggressiveOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents setting the aggression state of objects
  * controlled by an empire. */
class FO_COMMON_API AggressiveOrder : public Order {
public:
    /** \name Structors */ //@{
    AggressiveOrder(int empire, int object_id, bool aggression = true);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of object selected in this order. */
    int ObjectID() const
    { return m_object_id; }

    /** Returns aggression state to set object to. */
    bool Aggression() const
    { return m_aggression; }
    //@}

    static bool Check(int empire_id, int object_id, bool aggression);

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
    void ExecuteImpl() const override;

    int m_object_id = INVALID_OBJECT_ID;
    bool m_aggression = false;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// GiveObjectToEmpireOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents giving control of a ship to
  * another empire */
class FO_COMMON_API GiveObjectToEmpireOrder : public Order {
public:
    /** \name Structors */ //@{
    GiveObjectToEmpireOrder(int empire, int object_id, int recipient);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of object selected in this order. */
    int ObjectID() const
    { return m_object_id; }

    /** Returns ID of empire to which object is given. */
    int RecipientEmpireID()
    { return m_recipient_empire_id; }
    //@}

    static bool Check(int empire_id, int object_id, int recipient_empire_id);
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
    void ExecuteImpl() const override;

    bool UndoImpl() const override;

    int m_object_id = INVALID_OBJECT_ID;
    int m_recipient_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/////////////////////////////////////////////////////
// ForgetOrder
/////////////////////////////////////////////////////
/** ForgetOrder removes the object from the empire's known objects. */
class FO_COMMON_API ForgetOrder : public Order {
public:
    /** \name Structors */ //@{
    ForgetOrder(int empire, int object_id);
    //@}

    /** \name Accessors */ //@{
    /** Returns ID of object selected in this order. */
    int ObjectID() const
    { return m_object_id; }
    //@}

private:
    ForgetOrder() = default;

    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object not owned by issuing empire
     *
     *  Postconditions:
     *     - the object is removed from the table of known objects.
     */
    void ExecuteImpl() const override;

    int m_object_id = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// Note: *::serialize() implemented in SerializeOrderSet.cpp.

#endif // _Order_h_
