#ifndef _Order_h_
#define _Order_h_

#include <vector>


class ServerApp;

/////////////////////////////////////////////////////
// ORDER
/////////////////////////////////////////////////////

/** the abstract base class for serializable player actions (Orders)*/
class Order
{
public:
   /** \name Structors */ //@{
   Order() : m_empire(-1) {}  ///< default ctor
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
   *   will be thrown if any precondition fails.
   */
   virtual void Execute() const = 0; ///< executes the order on the server's Universe and Empires; does nothing in the client apps
   //@}

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
// PLANET BUILD ORDER
/////////////////////////////////////////////////////

/** the Order subclass that represents planetary production*/
class PlanetBuildOrder : public Order
{
public:
   enum BuildType
   {
      INVALID = -1,
      INDUSTRY_BUILD,
      RESEARCH_BUILD,
      SHIP_BUILD,
      DEF_BASE
   };
   
   /** \name Structors */ //@{
   PlanetBuildOrder();
   PlanetBuildOrder(int empire, int planet, BuildType build, int ship_type = -1);
   //@}
   
   /** \name Accessors */ //@{
   int   PlanetID() const     {return m_planet;}       ///< returns ID of planet selected in this order
   int   BuildOrder() const   {return m_build_type;}   ///< returns the build selection set in this order
   int   ShipType() const     {return m_ship_type;}    ///< returns the ship type selected for build.  Returns -1 if build type is not SHIP_BUILD.
   
   /**
   * Preconditions for executing planet build order:
   *  - m_planet must be the ID of a planet, owned by the empire
   *     issuing the order
   *
   *  Postconditions:
   *    - the specified planet will have its build orders set 
   *       to the specified build type and ship type (if building ships)
   */
   virtual void Execute() const;
   //@}
   
private:
   int         m_planet;
   BuildType   m_build_type;
   int         m_ship_type;
};


/////////////////////////////////////////////////////
// FLEET MOVE ORDER
/////////////////////////////////////////////////////

/** the Order subclass that represents fleet movement*/
class FleetMoveOrder : public Order
{
public:
   /** \name Structors */ //@{
   FleetMoveOrder();
   FleetMoveOrder(int empire, int fleet, int dest_system);
   //@}
   
   /** \name Accessors */ //@{
   int  FleetID() const             {return m_fleet;}        ///< returns ID of fleet selected in this order
   int  DestinationSystemID() const {return m_dest_system;}  ///< returns ID of system set as destination for this order
   
   /**
   * Preconditions for fleet move order are:
   *  - m_fleet must be the id of a fleet, owned by the empire issuing the order
   *  - m_dest_system must be the ID of a star system
   *  - star system must be within fleet's range (this precondition is not checked yet)
   *
   *  postconditions:
   *     - the specified fleet will have its move orders set to the specified destination
   */
   virtual void Execute() const;
   //@}
   
private:
   int   m_fleet;
   int   m_dest_system;
};

/////////////////////////////////////////////////////
// FLEET MERGE ORDER
/////////////////////////////////////////////////////

/** the Order subclass that represents transfer of ships between existing fleets */
class FleetMergeOrder : public Order
{
public:
   /** \name Structors */ //@{
   FleetMergeOrder();
   FleetMergeOrder(int empire, int fleet_from, int fleet_to, const std::vector<int>& ships);
   //@}
   
   /** \name Accessors */ //@{
   int SourceFleet() const   {return m_fleet_from;}     ///< returns ID of the fleet the ships will come from
   int DestinationFleet() const { return m_fleet_to;}    ///< returns ID of the fleet that the ships will go into             
   const std::vector<int>& Ships() const     {return m_add_ships;} ///< returns IDs of the ships selected for addition to the fleet
   
   /**
   *  FleetMergeOrder's preconditions are:
   *    - m_fleet_from must be the ID of a fleet owned by the issuing empire
   *    - m_fleet_to must be the ID of a fleet owned by the issuing empire
   *    - each element of m_add_ships must be the ID of a ship whose
   *         fleet ID equals m_fleet_from
   *
   *  Postconditions:
   *     - all ships in m_add_ships will be moved from the Source fleet to the destination fleet
   */
   virtual void Execute() const;
   //@}
   
private:
   int               m_fleet_from;
   int               m_fleet_to;
   std::vector<int>  m_add_ships;
};

/////////////////////////////////////////////////////
// FLEET SPLIT ORDER
/////////////////////////////////////////////////////

/** the Order subclass that represents forming a new fleet from a group
    of existing ships
*/
class FleetSplitOrder : public Order
{
public:
   /** \name Structors */ //@{
   FleetSplitOrder();
   FleetSplitOrder(int empire, const std::vector<int>& ships);
   //@}
   
   /** \name Accessors */ //@{
   const std::vector<int>& Ships() const     {return m_remove_ships;} ///< returns IDs of the ships selected for removal from the fleet
   
   /**
   * Preconditions of execute:
   *    - all ships in m_remove_ships must be part of a fleet owned by the issuing empire
   *    - all of the fleets whose ships are used must have same x,y coordinates
   *         and they must not have move orders
   *
   *  Postconditions:
   *     - all ships in m_remove_ships will be removed from their respective fleets
   *     - a new fleet will be created using the issuing empire's next fleet ID, which
   *         will contain all of the ships in m_remove_ships
   *
   */
   virtual void Execute() const;
   //@}
   
private:
   std::vector<int>  m_remove_ships;
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
   FleetColonizeOrder(int empire, int fleet, int planet);
   //@}
   
   /** \name Accessors */ //@{
   int   FleetID() const   {return m_fleet;}  ///< returns ID of the colonization fleet selected in this order
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
   *      - the empire issuing the order will be added to the list of owners
   *            for the planet
   *      - the planet's population will be increased
   *      - the planet will be added to the empire's list of owned planets
   */
   virtual void Execute() const;
   //@}
   
private:
   int   m_fleet;
   int   m_planet;
};

#endif // _Order_h_
