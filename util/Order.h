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
    issued. For example, FleetColonizeOrder needs to be undoable; otherwise, once the user clicks the
    colonize button, she is locked in to this decision. */
class Order
{
public:
    /** \name Structors */ //@{
    Order(); ///< default ctor
    Order(int empire) : m_empire(empire) {} ///< ctor taking the ID of the Empire issuing the order
    virtual ~Order() {}
    //@}
   
    /** \name Accessors */ //@{
    int          EmpireID() const {return m_empire;} ///< returns the ID of the Empire issuing the order

    /**
     *  Preconditions of Execute():
     *  For all order subclasses, the empire ID for the order
     *  must be that of an existing empire.  
     * 
     *  Subclasses add additional preconditions.  An std::runtime_error
     *   should be thrown if any precondition fails.
     */
    void                   Execute() const;   ///< executes the order on the Universe and Empires
    bool                   Undo() const;      ///< if this function returns true, it reverts the game state to what it was before this order was executed, otherwise it returns false and has no effect
    //@}

protected:
    /** \name Mutators */ //@{
    /**
     * This is here so that I do not have to type the same 'if' 5 times.  -- jbarcz1
     */
    void ValidateEmpireID() const; ///< verifies that the empire ID in this order is that of an existing empire.  Throws an std::runtime_error if not
    bool Executed() const;         ///< returns true iff this order has been executed (a second execution indicates server-side execution)
    //@}

private:
    virtual void ExecuteImpl() const = 0;
    virtual bool UndoImpl() const;

    int m_empire;

    mutable bool m_executed; // indicates that Execute() has occured, and so an undo is legal

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
    explicit NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, double x, double y, const std::vector<int>& ship_ids);
    //@}

    /** \name Accessors */ //@{
    const std::string&        FleetName() const    {return m_fleet_name;} ///< returns the name of the new fleet
    int                       SystemID() const     {return m_system_id;}  ///< returns the system the new fleet will be placed into (may be INVALID_OBJECT_ID if a position is specified)
    std::pair<double, double> Position() const     {return m_position;}   ///< returns the position of the new fleet (may be (INVALID_POSITION, INVALID_POSITION) if in a system)
    int                       NewID() const        {return m_new_id;}     ///< returns the ID for this fleet 
    const std::vector<int>&   ShipIDs() const      {return m_ship_ids;}   ///< returns the IDa for the ships used to start this fleet
    //@}

private:
    /**
     * Preconditions of execute: 
     *    None.
     *
     *  Postconditions:
     *    - a new fleet will exist either in system m_system_id or at position m_position,
     *          and will belong to the creating empire.
     *
     */
    virtual void ExecuteImpl() const;

    std::string               m_fleet_name;
    int                       m_system_id;
    int                       m_new_id;
    std::pair<double, double> m_position;
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
    FleetMoveOrder(int empire, int fleet, int start_system, int dest_system);
    //@}

    /** \name Accessors */ //@{
    int                      FleetID() const             {return m_fleet;}        ///< returns ID of fleet selected in this order
    int                      StartSystemID() const       {return m_start_system;} ///< returns ID of system set as the start system for this order (the system the route starts from)
    int                      DestinationSystemID() const {return m_dest_system;}  ///< returns ID of system set as destination for this order
    const std::vector<int>&  Route() const               {return m_route;}        ///< returns the IDs of the systems in the route specified by this Order
    double                   RouteLength() const         {return m_route_length;} ///< returns the length of the route specified by this Order
    //@}

private:
    /**
     * Preconditions of execute: 
     *    None.
     *
     *  Postconditions:
     *    - a new fleet will exist either in system m_system_id or at position m_position,
     *          and will belong to the creating empire.
     *
     */
    virtual void ExecuteImpl() const;

    int              m_fleet;
    int              m_start_system;
    int              m_dest_system;
    std::vector<int> m_route;
    double           m_route_length;

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
// FleetColonizeOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents a planet colonization action*/
class FleetColonizeOrder : public Order
{
public:
    /** \name Structors */ //@{
    FleetColonizeOrder();
    FleetColonizeOrder(int empire, int ship, int planet);
    //@}

    /** \name Accessors */ //@{
    int   PlanetID() const  {return m_planet;} ///< returns ID of the planet to be colonized
    int   ShipID  () const  {return m_ship  ;} ///< returns ID of the ship which is colonizing the planet

    virtual void           ServerExecute() const; //< called if the server allows the colonization effort 
    //@}

private:
    /**
     *  Preconditions:
     *     - m_fleet must be the ID of a fleet owned by issuing empire
     *     - m_planet must be the ID of an un-owned planet.
     *     - the fleet and the planet must have the same x,y coordinates
     *     - the fleet must contain a colony ship
     *
     *  Postconditions:
     *      - a colony ship will be removed from the fleet and deallocated
     *        if the fleet becomes empty it will be deallocated.
     *      - the empire issuing the order will be added to the list of owners
     *            for the planet
     *      - the planet's population will be increased
     *      - the planet will be added to the empire's list of owned planets
     *     
     */
    //< either ExecuteServerApply or ExecuteServerRevoke is called!!!
    virtual void ExecuteImpl() const;
    virtual bool UndoImpl() const;

    int   m_ship;
    int   m_planet;

    // these are for undoing this order only
    mutable int         m_colony_fleet_id;   // the fleet from which the colony ship was taken
    mutable std::string m_colony_fleet_name; // the name of fleet from which the colony ship was taken

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/////////////////////////////////////////////////////
// DeleteFleetOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents forming a new fleet. 
    Only one of system or position will be used to place the new fleet.*/
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
     *     - m_planet must be the ID of an un-owned planet.
     *     - the fleet and the planet must have the same x,y coordinates
     *     - the fleet must contain a colony ship
     *
     *  Postconditions:
     *      - a colony ship will be removed from the fleet and deallocated
     *        if the fleet becomes empty it will be deallocated.
     *      - the empire issuing the order will be added to the list of owners
     *            for the planet
     *      - the planet's population will be increased
     *      - the planet will be added to the empire's list of owned planets
     *     
     */
    //< either ExecuteServerApply or ExecuteServerRevoke is called!!!
    virtual void ExecuteImpl() const;

    int                       m_fleet;

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
    ChangeFocusOrder(int empire, int planet, FocusType focus, bool primary);
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
     *    - the planet focus is changed which=0(primary),1(secondary)
     *
     */
    virtual void ExecuteImpl() const;

    int       m_planet;
    FocusType m_focus;
    bool      m_primary;

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
     *    - For creating a new design, the passed design is a valid reference to a design created by the
     *      empire issuing the order
     *    - For remembering an existing ship design, there exists a ship design with the passed id, and
     *      the empire is aware of this ship design
     *    - For removing a shipdesign from the empire's set of designs, there empire has a design with the
     *      passed id in its set of designs
     *
     *  Postconditions:
     *    - For creating a new ship design, the universe will contain a new ship design, and the creating
     *      empire will have the new design as of of its designs
     *    - For remembering a ship design, the empire will have the design's id in its set of design ids
     *    - For removing a design, the empire will no longer have the design's id in its set of design ids
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
// FleetMoveOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents fleet movement
    These orders change the current destination of a fleet */
// Template Implementations
template <class Archive>
void Order::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_empire)
        & BOOST_SERIALIZATION_NVP(m_executed);
}

template <class Archive>
void RenameOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void NewFleetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet_name)
        & BOOST_SERIALIZATION_NVP(m_system_id)
        & BOOST_SERIALIZATION_NVP(m_new_id)
        & BOOST_SERIALIZATION_NVP(m_position)
        & BOOST_SERIALIZATION_NVP(m_ship_ids);
}

template <class Archive>
void FleetMoveOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet)
        & BOOST_SERIALIZATION_NVP(m_start_system)
        & BOOST_SERIALIZATION_NVP(m_dest_system)
        & BOOST_SERIALIZATION_NVP(m_route)
        & BOOST_SERIALIZATION_NVP(m_route_length);
}

template <class Archive>
void FleetTransferOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet_from)
        & BOOST_SERIALIZATION_NVP(m_fleet_to)
        & BOOST_SERIALIZATION_NVP(m_add_ships);
}

template <class Archive>
void FleetColonizeOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship)
        & BOOST_SERIALIZATION_NVP(m_planet)
        & BOOST_SERIALIZATION_NVP(m_colony_fleet_id)
        & BOOST_SERIALIZATION_NVP(m_colony_fleet_name);
}

template <class Archive>
void DeleteFleetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet);
}

template <class Archive>
void ChangeFocusOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_planet)
        & BOOST_SERIALIZATION_NVP(m_focus)
        & BOOST_SERIALIZATION_NVP(m_primary);
}

template <class Archive>
void ResearchQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_position)
        & BOOST_SERIALIZATION_NVP(m_remove);
}

template <class Archive>
void ProductionQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_build_type)
        & BOOST_SERIALIZATION_NVP(m_item_name)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_number)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_index)
        & BOOST_SERIALIZATION_NVP(m_new_quantity)
        & BOOST_SERIALIZATION_NVP(m_new_index);
}

template <class Archive>
void ShipDesignOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship_design)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_delete_design_from_empire)
        & BOOST_SERIALIZATION_NVP(m_create_new_design);
}

#endif // _Order_h_
