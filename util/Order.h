#ifndef _Order_h_
#define _Order_h_

#include "../universe/Enums.h"
#include "Export.h"
#include "../Empire/Empire.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

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
    Order(); ///< default ctor
    Order(int empire) : m_empire(empire), m_executed(false) {}     ///< ctor taking the ID of the Empire issuing the order
    virtual ~Order() {}
    //@}

    /** \name Accessors */ //@{
    int     EmpireID() const {return m_empire;} ///< returns the ID of the Empire issuing the order
    //@}

    /**
     *  Preconditions of Execute():
     *  For all order subclasses, the empire ID for the order
     *  must be that of an existing empire.
     *
     *  Subclasses add additional preconditions.  An std::runtime_error
     *   should be thrown if any precondition fails.
     */
    void    Execute() const;            ///< executes the order on the Universe and Empires

    bool    Undo() const;               ///< if this function returns true, it reverts the game state to what it was before this order was executed, otherwise it returns false and has no effect

protected:
    /** \name Mutators */ //@{
    void    ValidateEmpireID() const;   ///< verifies that the empire ID in this order is that of an existing empire.  Throws an std::runtime_error if not
    bool    Executed() const;           ///< returns true iff this order has been executed (a second execution indicates server-side execution)
    //@}

private:
    virtual void ExecuteImpl() const = 0;
    virtual bool UndoImpl() const;

    int             m_empire;

    mutable bool    m_executed; // indicates that Execute() has occured, and so an undo is legal

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
    RenameOrder();
    RenameOrder(int empire, int object, const std::string& name);
    //@}

    /** \name Accessors */ //@{
    int                  ObjectID() const {return m_object;} ///< returns ID of fleet selected in this order
    const std::string&   Name() const     {return m_name;}  ///< returns the new name of the fleet
    //@}

private:
    /**
     * Preconditions of execute:
     *    - the designated planet must exist, be owned by the issuing empire
     *
     *  Postconditions:
     *    - the planet focus is changed which=0(primary),1(secondary)
     *
     */
    virtual void ExecuteImpl() const;

    int           m_object;
    std::string   m_name;

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
    NewFleetOrder();
    NewFleetOrder(int empire, const std::string& fleet_name, int fleet_id,
                  int system_id, const std::vector<int>& ship_ids,
                  bool aggressive = false);
    NewFleetOrder(int empire, const std::vector<std::string>& fleet_names, const std::vector<int>& fleet_ids,
                  int system_id, const std::vector<std::vector<int> >& ship_id_groups,
                  const std::vector<bool>& aggressives);
    //@}

    /** \name Accessors */ //@{
    int                                     SystemID() const    { return m_system_id; }
    const std::vector<std::string>&         FleetNames() const  { return m_fleet_names; }
    const std::vector<int>&                 FleetIDs() const    { return m_fleet_ids; }
    const std::vector<std::vector<int> >&   ShipIDGroups() const{ return m_ship_id_groups; }
    const std::vector<bool>&                Aggressive() const  { return m_aggressives; }
    //@}

private:
    /**
     * Preconditions of execute:
     *    None.
     *
     *  Postconditions:
     *    - new fleets will exist in system with id m_system_id,
     *      and will belong to the creating empire.
     */
    virtual void ExecuteImpl() const;

    std::vector<std::string>        m_fleet_names;
    int                             m_system_id;
    std::vector<int>                m_fleet_ids;
    std::vector<std::vector<int> >  m_ship_id_groups;
    std::vector<bool>               m_aggressives;

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
    FleetMoveOrder();
    FleetMoveOrder(int empire, int fleet_id, int start_system_id, int dest_system_id, bool append = false);
    //@}

    /** \name Accessors */ //@{
    int                      FleetID() const             {return m_fleet;}        ///< returns ID of fleet selected in this order
    int                      StartSystemID() const       {return m_start_system;} ///< returns ID of system set as the start system for this order (the system the route starts from)
    int                      DestinationSystemID() const {return m_dest_system;}  ///< returns ID of system set as destination for this order
    const std::vector<int>&  Route() const               {return m_route;}        ///< returns the IDs of the systems in the route specified by this Order
    //@}

private:
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
    virtual void ExecuteImpl() const;

    int              m_fleet;
    int              m_start_system;
    int              m_dest_system;
    std::vector<int> m_route;
    bool             m_append;

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
    FleetTransferOrder();
    FleetTransferOrder(int empire, int dest_fleet, const std::vector<int>& ships);
    //@}

    /** \name Accessors */ //@{
    int                     DestinationFleet() const {return m_dest_fleet;} ///< returns ID of the fleet that the ships will go into
    const std::vector<int>& Ships() const            {return m_add_ships;}  ///< returns IDs of the ships selected for addition to the fleet
    //@}

private:
    /**
     *  FleetTransferOrder's preconditions are:
     *    - m_into_fleet must be the ID of a fleet owned by the issuing empire
     *    - each element of m_add_ships must be the ID of a ship owned by the issuing empire
     *
     *  Postconditions:
     *     - all ships in m_add_ships will be moved from their initial fleet to the destination fleet
     */
    virtual void ExecuteImpl() const;

    int               m_dest_fleet;
    std::vector<int>  m_add_ships;

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
    ColonizeOrder();
    ColonizeOrder(int empire, int ship, int planet);
    //@}

    /** \name Accessors */ //@{
    int             PlanetID() const  {return m_planet;}    ///< returns ID of the planet to be colonized
    int             ShipID  () const  {return m_ship  ;}    ///< returns ID of the ship which is colonizing the planet
    //@}

private:
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
    virtual void ExecuteImpl() const;
    virtual bool UndoImpl() const;

    int                 m_ship;
    int                 m_planet;

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
    InvadeOrder();
    InvadeOrder(int empire, int ship, int planet);
    //@}

    /** \name Accessors */ //@{
    int             PlanetID() const  {return m_planet;}    ///< returns ID of the planet to be invaded
    int             ShipID  () const  {return m_ship  ;}    ///< returns ID of the ship which is invading the planet
    //@}

private:
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
    virtual void ExecuteImpl() const;
    virtual bool UndoImpl() const;

    int                 m_ship;
    int                 m_planet;

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
    BombardOrder();
    BombardOrder(int empire, int ship, int planet);
    //@}

    /** \name Accessors */ //@{
    int             PlanetID() const  {return m_planet;}    ///< returns ID of the planet to be bombarded
    int             ShipID  () const  {return m_ship  ;}    ///< returns ID of the ship which is bombarding the planet
    //@}

private:
    /**
     *  Preconditions:
     *     - m_planet must be the ID of a planet
     *     - m_ship must be the the ID of a ship owned by the issuing empire
     *
     *  Postconditions:
     *      - The ship with ID m_ship will be marked to bombard the planet with
     *        id m_planet during the next turn processing.
     */
    virtual void ExecuteImpl() const;
    virtual bool UndoImpl() const;

    int                 m_ship;
    int                 m_planet;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// DeleteFleetOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents removing an existing fleet that contains
  * no ships. This is mainly a utility order that is issued automatically by the
  * game when the user removes all ships from a fleet. */
class FO_COMMON_API DeleteFleetOrder : public Order {
public:
    /** \name Structors */ //@{
    DeleteFleetOrder();
    DeleteFleetOrder(int empire, int fleet);
    //@}

    /** \name Accessors */ //@{
    int   FleetID() const   {return m_fleet;}  ///< returns ID of the fleet to be deleted
    //@}

private:
    /**
     *  Preconditions:
     *     - m_fleet must be the ID of a fleet owned by issuing empire
     *     - the fleet must contain no ships
     *
     *  Postconditions:
     *     - the fleet is deleted
     */
    //< either ExecuteServerApply or ExecuteServerRevoke is called!!!
    virtual void ExecuteImpl() const;

    int m_fleet;

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
    ChangeFocusOrder();
    ChangeFocusOrder(int empire, int planet, const std::string& focus);
    //@}

    /** \name Accessors */ //@{
    int   PlanetID() const   {return m_planet;}  ///< returns ID of the fleet to be deleted
    //@}

private:
    /**
     * Preconditions of execute:
     *    - the designated planet must exist, be owned by the issuing empire
     *
     *  Postconditions:
     *    - the planet focus is changed
     */
    virtual void ExecuteImpl() const;

    int         m_planet;
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
    ResearchQueueOrder();
    ResearchQueueOrder(int empire, const std::string& tech_name);
    ResearchQueueOrder(int empire, const std::string& tech_name, int position);
    ResearchQueueOrder(int empire, const std::string& tech_name, bool pause, float dummy);
    //@}

private:
    virtual void ExecuteImpl() const;

    std::string m_tech_name;
    int         m_position;
    bool        m_remove;
    int         m_pause;

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
    ProductionQueueOrder();
    ProductionQueueOrder(int empire, const ProductionQueue::ProductionItem& item, int number, int location, int pos = -1);
    ProductionQueueOrder(int empire, int index, int new_quantity, bool dummy);
    ProductionQueueOrder(int empire, int index, int rally_point_id, bool dummy1, bool dummy2);
    ProductionQueueOrder(int empire, int index, int new_quantity, int new_blocksize);
    ProductionQueueOrder(int empire, int index, int new_index);
    ProductionQueueOrder(int empire, int index);
    ProductionQueueOrder(int empire, int index, bool pause, float dummy);
    //@}

private:
    virtual void ExecuteImpl() const;

    ProductionQueue::ProductionItem m_item;
    int         m_number;
    int         m_location;
    int         m_index;
    int         m_new_quantity;
    int         m_new_blocksize;
    int         m_new_index;
    int         m_rally_point_id;
    int         m_pause;

    static const int INVALID_INDEX = -500;
    static const int INVALID_QUANTITY = -1000;
    static const int PAUSE = 1;
    static const int RESUME = 2;
    static const int INVALID_PAUSE_RESUME = -1;

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
  * The 3-arg ctor (int,int,int) moves a design_id_to_move to before design_id_after
  */
class FO_COMMON_API ShipDesignOrder : public Order {
public:
    /** \name Structors */ //@{
    ShipDesignOrder();
    ShipDesignOrder(int empire, int existing_design_id_to_remember);
    ShipDesignOrder(int empire, int design_id_to_erase, bool dummy);
    ShipDesignOrder(int empire, int new_design_id, const ShipDesign& ship_design);
    ShipDesignOrder(int empire, int existing_design_id, const std::string& new_name = "", const std::string& new_description = "");
    ShipDesignOrder(int empire, int design_id_to_move, int design_id_after);
    //@}

private:
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
    virtual void ExecuteImpl() const;

    int                         m_design_id;
    bool                        m_update_name_or_description;
    bool                        m_delete_design_from_empire;
    bool                        m_create_new_design;
    bool                        m_move_design;

    // details of design to create
    std::string                 m_name;
    std::string                 m_description;
    int                         m_designed_on_turn;
    std::string                 m_hull;
    std::vector<std::string>    m_parts;
    bool                        m_is_monster;
    std::string                 m_icon;
    std::string                 m_3D_model;
    bool                        m_name_desc_in_stringtable;
    int                         m_design_id_after;             //<location after the inserted design
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
    ScrapOrder();
    ScrapOrder(int empire, int object_id);
    //@}

    /** \name Accessors */ //@{
    int             ObjectID() const { return m_object_id; }///< returns ID of object selected in this order
    //@}

private:
    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object owned by issuing empire
     *     - the object must be scrappable: ships or buildings
     *
     *  Postconditions:
     *     - the object is marked to be scrapped during the next turn processing.
     */
    virtual void    ExecuteImpl() const;
    virtual bool    UndoImpl() const;

    int m_object_id;

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
    AggressiveOrder();
    AggressiveOrder(int empire, int object_id, bool aggression = true);
    //@}

    /** \name Accessors */ //@{
    int             ObjectID() const    { return m_object_id; } ///< returns ID of object selected in this order
    bool            Aggression() const  { return m_aggression; }///< returns aggression state to set object to
    //@}

private:
    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object owned by issuing empire
     *     - the object must have an aggression status: fleets
     *
     *  Postconditions:
     *     - the object is set to the new aggression state
     */
    virtual void    ExecuteImpl() const;

    int     m_object_id;
    bool    m_aggression;

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
    GiveObjectToEmpireOrder();
    GiveObjectToEmpireOrder(int empire, int object_id, int recipient);
    //@}

    /** \name Accessors */ //@{
    int             ObjectID() const    { return m_object_id; }             ///< returns ID of object selected in this order
    int             RecipientEmpireID() { return m_recipient_empire_id; }   ///< returns ID of empire to which object is given
    //@}

private:
    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object owned by issuing empire
     *     - m_recipient_empire_id must be the ID of another empire
     *
     *  Postconditions:
     *     - the object's ownership is set to the other empire
     */
    virtual void    ExecuteImpl() const;
    virtual bool    UndoImpl() const;

    int     m_object_id;
    int     m_recipient_empire_id;

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
    ForgetOrder();
    ForgetOrder(int empire, int object_id);
    //@}

    /** \name Accessors */ //@{
    int             ObjectID() const { return m_object_id; }///< returns ID of object selected in this order
    //@}

private:
    /**
     *  Preconditions:
     *     - m_object_id must be the ID of an object not owned by issuing empire
     *
     *  Postconditions:
     *     - the object is removed from the table of known objects.
     */
    virtual void    ExecuteImpl() const;

    int m_object_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// Note: *::serialize() implemented in SerializeOrderSet.cpp.

#endif // _Order_h_
