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
// ORDER
/////////////////////////////////////////////////////


/** the abstract base class for serializable player actions (Orders)*/
class Order
{
public:
   /** \name Structors */ //@{
   Order() : m_empire(-1) {}  ///< default ctor
   Order(const GG::XMLElement& elem);  ///< serialization constructor
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
   virtual void           Execute() const = 0; ///< executes the order on the Universe and Empires
   virtual GG::XMLElement XMLEncode() const;   ///< constructs an XMLElement for the order
   
   //@}

    /// initializes an XML object factory for constructing orders
    static void InitOrderFactory(GG::XMLObjectFactory<Order>& fact);

protected:
    /** \name Mutators */ //@{
    /**
    * This is here so that I do not have to type the same 'if' 5 times.  -- jbarcz1
    */
    void ValidateEmpireID() const; ///< verifies that the empire ID in this order is that of an existing empire.  Throws an std::runtime_error if not
    //@}
private:
   int m_empire;
};


/////////////////////////////////////////////////////
// RENAME ORDER
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
   
   /**
   *  Preconditions for fleet rename order are:
   *    - m_object must be the id of an object in the universe, wholly owned by the empire issuing the order
   *
   *  Postconditions:
   *    - the object's name attribute will be changed to the one desired.
   *
   */
   virtual void           Execute() const;
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
   //@}
   
private:
   int           m_object;
   std::string   m_name;
};


/////////////////////////////////////////////////////
// PLANET BUILD ORDER
/////////////////////////////////////////////////////

/** the Order subclass that represents planetary production.
    PlanetBuildOrder's when issued, cause planets to change their
    production project. */
class PlanetBuildOrder : public Order
{
public:
   /** \name Structors */ //@{
   PlanetBuildOrder();
   PlanetBuildOrder(const GG::XMLElement& elem);
   PlanetBuildOrder(int empire, int planet, ProdCenter::BuildType build);
   //@}
   
   /** \name Accessors */ //@{
   int                   PlanetID() const {return m_planet;}       ///< returns ID of planet selected in this order
   ProdCenter::BuildType Type() const     {return m_build_type;}   ///< returns the build selection set in this order
   
   /**
   * Preconditions for executing planet build order:
   *  - m_planet must be the ID of a planet, owned by the empire
   *     issuing the order
   *
   *  Postconditions:
   *    - the specified planet will have its build orders set 
   *       to the specified build type and ship type (if building ships)
   */
   virtual void           Execute() const;
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
   //@}
   
private:
   int                     m_planet;
   ProdCenter::BuildType   m_build_type;
};


/////////////////////////////////////////////////////
// NEW FLEET ORDER
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
   
   /**
   * Preconditions of execute: 
   *    None.
   *
   *  Postconditions:
   *    - a new fleet will exist either in system m_system_id or at position m_position,
   *          and will belong to the creating empire.
   *
   */
   virtual void           Execute() const;
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
   //@}
   
private:
   std::string               m_fleet_name;
   int                       m_system_id;
   int                       m_new_id;
   std::pair<double, double> m_position;
};


/////////////////////////////////////////////////////
// FLEET MOVE ORDER
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
   
   /**
   * Preconditions for fleet move order are:
   *  - m_fleet must be the id of a fleet, owned by the empire issuing the order
   *  - m_dest_system must be the ID of a star system
   *  - the destination star system must be within fleet's range (this precondition is not checked yet)
   *
   *  postconditions:
   *     - the specified fleet will have its move orders set to the specified destination
   */
   virtual void           Execute() const;
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
   //@}
   
private:
   int              m_fleet;
   int              m_start_system;
   int              m_dest_system;
   std::vector<int> m_route;
   double           m_route_length;
};

/////////////////////////////////////////////////////
// FLEET TRANSFER ORDER
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
   virtual void           Execute() const;
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
      
   //@}
   
private:
   int               m_fleet_from;
   int               m_fleet_to;
   std::vector<int>  m_add_ships;
};


/////////////////////////////////////////////////////
// FLEET COLONIZE ORDER
/////////////////////////////////////////////////////

/** the Order subclass that represents a planet colonization action*/
class FleetColonizeOrder : public Order
{
public:
   /** \name Structors */ //@{
   FleetColonizeOrder();
   FleetColonizeOrder(const GG::XMLElement& elem);
   FleetColonizeOrder(int empire, Ship *ship, int planet);

   virtual ~FleetColonizeOrder();
   //@}
   
   /** \name Accessors */ //@{
   int   EmpireID() const  {return m_empire;} ///< returns ID of the empire id of the empire which tries to colonize the planet
   int   PlanetID() const  {return m_planet;} ///< returns ID of the planet to be colonized
   
   
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
   virtual void           Execute() const;
   //< either ExecuteServerApply or ExecuteServerRevoke is called!!!
   virtual void           ExecuteServerApply() const; //< called if the server seconds the colonization effort 
   virtual void           ExecuteServerRevoke() const;//< called if the server doesn't seconds the colonization effort
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
   //@}
   
private:
   int   m_empire;
   Ship  *m_ship;
   int   m_planet;
};


/////////////////////////////////////////////////////
// DELETE FLEET ORDER
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
   
   /**
   * Preconditions of execute: 
   *    - the designated fleet must exist, be owned by the issuing empire, and be empty
   *
   *  Postconditions:
   *    - the fleet will no longer exist
   *
   */
   virtual void           Execute() const;
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
   //@}
   
private:
   int                       m_fleet;
};

/////////////////////////////////////////////////////
// CHANGE PLANET FOCUS ORDER
/////////////////////////////////////////////////////

/** the Order subclass that represents changing a planet focus*/
class ChangeFocusOrder : public Order
{
public:
   /** \name Structors */ //@{
   ChangeFocusOrder();
   ChangeFocusOrder(const GG::XMLElement& elem);
   ChangeFocusOrder(int empire, int planet,FocusType focus,int which);
   //@}
   
   /** \name Accessors */ //@{
   int   PlanetID() const   {return m_planet;}  ///< returns ID of the fleet to be deleted
   
   /**
   * Preconditions of execute: 
   *    - the designated planet must exist, be owned by the issuing empire
   *
   *  Postconditions:
   *    - the planet focus is changed which=0(primary),1(secondary)
   *
   */
   virtual void           Execute() const;
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement for the order
   //@}
   
private:
   int       m_planet;
   FocusType m_focus;
   int       m_which;
};
#endif // _Order_h_

