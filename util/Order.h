// -*- C++ -*-
#ifndef _Order_h_
#define _Order_h_

#include <vector>

#ifndef _XMLDoc_h_
#include "XMLDoc.h"
#endif

#ifndef _XMLObjectFactory_h_
#include "XMLObjectFactory.h"
#endif

#ifndef _ProdCenter_h_
#include "../universe/ProdCenter.h"
#endif


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
    Order(const GG::XMLElement& elem);  ///< XML constructor
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
    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
    //@}

    /// initializes an XML object factory for constructing orders
    static void InitOrderFactory(GG::XMLObjectFactory<Order>& fact);

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
    RenameOrder(const GG::XMLElement& elem);
    RenameOrder(int empire, int object, const std::string& name);
    //@}
   
    /** \name Accessors */ //@{
    int                  ObjectID() const {return m_object;} ///< returns ID of fleet selected in this order
    const std::string&   Name() const     {return m_name;}  ///< returns the new name of the fleet
   
    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
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
};


/////////////////////////////////////////////////////
// BuildOrder
/////////////////////////////////////////////////////
/** the Order subclass that represents planetary production.  BuildOrder's when issued, cause ProdCenters
    to change their production project. */
class PlanetBuildOrder : public Order
{
public:
    /** \name Structors */ //@{
    PlanetBuildOrder();
    PlanetBuildOrder(const GG::XMLElement& elem);
    PlanetBuildOrder(int empire, int planet_id, BuildType build, const std::string& name);
    //@}

    /** \name Accessors */ //@{
    int                PlanetID() const {return m_planet;}     ///< returns ID of planet selected in this order
    BuildType          Type() const     {return m_build_type;} ///< returns the build type set in this order (eg BUILDING)
    const std::string& Name() const     {return m_name;}       ///< returns the name of the exact type of item within the build type that should be produced (eg "SuperFarm")

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
    //@}

private:
    /**
     * Preconditions for executing planet build order:
     *  - m_prodcenter must be the ID of a planet, owned by the empire
     *     issuing the order
     *
     *  Postconditions:
     *    - the specified planet will have its build orders set 
     *       to the specified build type and ship type (if building ships)
     */
    virtual void ExecuteImpl() const;

    int         m_planet;
    BuildType   m_build_type;
    std::string m_name;
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
    NewFleetOrder(const GG::XMLElement& elem);
    NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, int system_id);
    NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, double x, double y);
    //@}

    /** \name Accessors */ //@{
    const std::string&        FleetName() const    {return m_fleet_name;} ///< returns the name of the new fleet
    int                       SystemID() const     {return m_system_id;}  ///< returns the system the new fleet will be placed into (may be INVALID_OBJECT_ID if a position is specified)
    std::pair<double, double> Position() const     {return m_position;}   ///< returns the position of the new fleet (may be (INVALID_POSITION, INVALID_POSITION) if in a system)
    int                       NewID() const     {return m_new_id;}  ///< returns the ID for this fleet 

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
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
    FleetMoveOrder(const GG::XMLElement& elem);
    FleetMoveOrder(int empire, int fleet, int start_system, int dest_system);
    //@}

    /** \name Accessors */ //@{
    int                      FleetID() const             {return m_fleet;}        ///< returns ID of fleet selected in this order
    int                      StartSystemID() const       {return m_start_system;} ///< returns ID of system set as the start system for this order (the system the route starts from)
    int                      DestinationSystemID() const {return m_dest_system;}  ///< returns ID of system set as destination for this order
    const std::vector<int>&  Route() const               {return m_route;}        ///< returns the IDs of the systems in the route specified by this Order
    double                   RouteLength() const         {return m_route_length;} ///< returns the length of the route specified by this Order

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
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
    FleetTransferOrder(const GG::XMLElement& elem);
    FleetTransferOrder(int empire, int fleet_from, int fleet_to, const std::vector<int>& ships);
    //@}

    /** \name Accessors */ //@{
    int                     SourceFleet() const      {return m_fleet_from;}  ///< returns ID of the fleet the ships will come from
    int                     DestinationFleet() const {return m_fleet_to;}    ///< returns ID of the fleet that the ships will go into             
    const std::vector<int>& Ships() const            {return m_add_ships;}   ///< returns IDs of the ships selected for addition to the fleet

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order

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
    FleetColonizeOrder(const GG::XMLElement& elem);
    FleetColonizeOrder(int empire, int ship, int planet);
    //@}

    /** \name Accessors */ //@{
    int   PlanetID() const  {return m_planet;} ///< returns ID of the planet to be colonized

    virtual void           ServerExecute() const; //< called if the server allows the colonization effort 

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
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
    DeleteFleetOrder(const GG::XMLElement& elem);
    DeleteFleetOrder(int empire, int fleet);
    //@}

    /** \name Accessors */ //@{
    int   FleetID() const   {return m_fleet;}  ///< returns ID of the fleet to be deleted

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
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
    ChangeFocusOrder(const GG::XMLElement& elem);
    ChangeFocusOrder(int empire, int planet, FocusType focus, bool primary);
    //@}

    /** \name Accessors */ //@{
    int   PlanetID() const   {return m_planet;}  ///< returns ID of the fleet to be deleted

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
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
};


/////////////////////////////////////////////////////
// ResearchQueueOrder
/////////////////////////////////////////////////////
/** The Order subclass that represents changing an empire's research queue.  The 2-arg ctor removes the names
    tech from \a empire's queue, whereas the 3-arg ctor places \a tech_name at position \a position in
    \a empire's research queue. */
class ResearchQueueOrder : public Order
{
public:
    /** \name Structors */ //@{
    ResearchQueueOrder();
    ResearchQueueOrder(const GG::XMLElement& elem);
    ResearchQueueOrder(int empire, const std::string& tech_name);
    ResearchQueueOrder(int empire, const std::string& tech_name, int position);
    //@}

    /** \name Accessors */ //@{
    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
    //@}

private:
    virtual void ExecuteImpl() const;

    std::string m_tech_name;
    int         m_position;
    bool        m_remove;
};

inline std::pair<std::string, std::string> OrderRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _Order_h_

