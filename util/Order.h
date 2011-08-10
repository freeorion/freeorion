// -*- C++ -*-
#ifndef _Order_h_
#define _Order_h_

#ifndef _Enums_h_
#include "../universe/Enums.h"
#endif

#include "../universe/ShipDesign.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <vector>


/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
/** the abstract base class for serializable player actions.  Orders are generally executed on the
    client side as soon as they are issued.  Those that define UndoImpl() may also be undone on the client
    side.  Subclass-defined UndoImpl() \a must return true, indicating that the call had some effect; the
    default implementation does nothing and returns false. Note that only some Order subclasses define
    UndoImpl(), specifically those that need to be undone before another order of a similar type can be
    issued. For example, ColonizeOrder needs to be undoable; otherwise, once the user clicks the
    colonize button, she is locked in to this decision. */
class Order
{
public:
    /** \name Structors */ //@{
    Order(); ///< default ctor
    Order(int empire) : m_empire(empire) {}     ///< ctor taking the ID of the Empire issuing the order
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
class RenameOrder : public Order
{
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
class NewFleetOrder : public Order
{
public:
    /** \name Structors */ //@{
    NewFleetOrder();
    explicit NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, int system_id, const std::vector<int>& ship_ids);
    //@}

    /** \name Accessors */ //@{
    const std::string&        FleetName() const    {return m_fleet_name;} ///< returns the name of the new fleet
    int                       SystemID() const     {return m_system_id;}  ///< returns the system the new fleet will be placed into (may be INVALID_OBJECT_ID if a position is specified)
    int                       NewID() const        {return m_new_id;}     ///< returns the ID for this fleet 
    const std::vector<int>&   ShipIDs() const      {return m_ship_ids;}   ///< returns the IDa for the ships used to start this fleet
    //@}

private:
    /**
     * Preconditions of execute:
     *    None.
     *
     *  Postconditions:
     *    - a new fleet will exist either in system m_system_id,
     *      and will belong to the creating empire.
     */
    virtual void ExecuteImpl() const;

    std::string               m_fleet_name;
    int                       m_system_id;
    int                       m_new_id;
    std::vector<int>          m_ship_ids;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// FleetMoveOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents fleet movement
    These orders change the current destination of a fleet */
class FleetMoveOrder : public Order
{
public:
    /** \name Structors */ //@{
    FleetMoveOrder();
    FleetMoveOrder(int empire, int fleet_id, int start_system_id, int dest_system_id);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// FleetTransferOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents transfer of ships between existing fleets
    A FleetTransferOrder is used to transfer ships from one existing fleet to another
 */
class FleetTransferOrder : public Order
{
public:
    /** \name Structors */ //@{
    FleetTransferOrder();
    FleetTransferOrder(int empire, int fleet_from, int fleet_to, const std::vector<int>& ships);
    //@}

    /** \name Accessors */ //@{
    int                     SourceFleet() const      {return m_fleet_from;}  ///< returns ID of the fleet the ships will come from
    int                     DestinationFleet() const {return m_fleet_to;}    ///< returns ID of the fleet that the ships will go into             
    const std::vector<int>& Ships() const            {return m_add_ships;}   ///< returns IDs of the ships selected for addition to the fleet
    //@}

private:
    /**
     *  FleetTransferOrder's preconditions are:
     *    - m_fleet_from must be the ID of a fleet owned by the issuing empire
     *    - m_fleet_to must be the ID of a fleet owned by the issuing empire
     *    - each element of m_add_ships must be the ID of a ship whose
     *         fleet ID equals m_fleet_from
     *
     *  Postconditions:
     *     - all ships in m_add_ships will be moved from the Source fleet to the destination fleet
     */
    virtual void ExecuteImpl() const;

    int               m_fleet_from;
    int               m_fleet_to;
    std::vector<int>  m_add_ships;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ColonizeOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet colonization action*/
class ColonizeOrder : public Order
{
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
class InvadeOrder : public Order
{
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
// DeleteFleetOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents removing an existing fleet that contains
  * no ships */
class DeleteFleetOrder : public Order
{
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
class ChangeFocusOrder : public Order
{
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
/** The Order subclass that represents changing an empire's research queue.  The 2-arg ctor removes the named
    tech from \a empire's queue, whereas the 3-arg ctor places \a tech_name at position \a position in
    \a empire's research queue. */
class ResearchQueueOrder : public Order
{
public:
    /** \name Structors */ //@{
    ResearchQueueOrder();
    ResearchQueueOrder(int empire, const std::string& tech_name);
    ResearchQueueOrder(int empire, const std::string& tech_name, int position);
    //@}

private:
    virtual void ExecuteImpl() const;

    std::string m_tech_name;
    int         m_position;
    bool        m_remove;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ProductionQueueOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents changing an empire's production queue.  The 5-arg ctor adds the build to the end
    of \a empire's queue, the 3-arg ctor moves an existing build from its current location at \a index to a new one at
    \a new_index, and the 2-arg ctor removes the build at \a index from \a empire's queue. */
class ProductionQueueOrder : public Order
{
public:
    /** \name Structors */ //@{
    ProductionQueueOrder();
    ProductionQueueOrder(int empire, BuildType build_type, const std::string& item, int number, int location);
    ProductionQueueOrder(int empire, BuildType build_type, int design_id, int number, int location);
    ProductionQueueOrder(int empire, int index, int new_quantity, bool dummy);
    ProductionQueueOrder(int empire, int index, int new_index);
    ProductionQueueOrder(int empire, int index);
    //@}

private:
    virtual void ExecuteImpl() const;

    BuildType   m_build_type;
    std::string m_item_name;
    int         m_design_id;
    int         m_number;
    int         m_location;
    int         m_index;
    int         m_new_quantity;
    int         m_new_index;

    static const int INVALID_INDEX = -500;
    static const int INVALID_QUANTITY = -1000;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ShipDesignOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents manipulating an empire's ship designs.
    The 2-arg ctor adds the existing ship design to the \a empire's set of designs - remembering, or "keeping" the
    design and enabling the \a empire to produce ships of that design (if all design prerequisites are met)
    The 3-arg ctor taking a bool removes the indicated design from the empire's set of remembered designs
    The 3-arg ctor taking a ShipDesign argument creates a new shipdesign in the universe's catalog of shipdesigns
    with the passed new design id, and adds this design to the \a empire's set of remembered designs.  The new design must
    be marked as designed by this \a empire.*/
class ShipDesignOrder : public Order
{
public:
    /** \name Structors */ //@{
    ShipDesignOrder();
    ShipDesignOrder(int empire, int existing_design_id_to_remember);
    ShipDesignOrder(int empire, int design_id_to_erase, bool dummy);
    ShipDesignOrder(int empire, int new_design_id, const ShipDesign& ship_design);
    //@}

    /** \name Accessors */ //@{
    const ShipDesign&         GetShipDesign() const {return m_ship_design;} ///< returns the ship design to be created and/or added or removed to/from the empire's designs
    int                       DesignID() const      {return m_design_id;}   ///< returns the ship design ID
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

    ShipDesign                  m_ship_design;
    int                         m_design_id;
    bool                        m_delete_design_from_empire;
    bool                        m_create_new_design;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// ScrapOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents the scrapping / recycling / destroying
  * a building or ship owned by an empire. */
class ScrapOrder : public Order
{
public:
    /** \name Structors */ //@{
    ScrapOrder();
    ScrapOrder(int empire, int object_id);
    //@}

    /** \name Accessors */ //@{
    int             ObjectID() const {return m_object_id;}  ///< returns ID of object selected in this order
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

// Note: *::serialize() implemented in SerializeOrderSet.cpp.

#endif // _Order_h_
