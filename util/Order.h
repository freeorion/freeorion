#ifndef _Order_h_
#define _Order_h_

#include <vector>


class ServerApp;

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
   
   virtual void Execute() const = 0; ///< executes the order on the server's Universe and Empires; does nothing in the client apps
   //@}
   
private:
   int m_empire;
};


/** the Order subclass that represents planetary production*/
class PlanetBuildOrder : public Order
{
public:
   enum BuildType
   {
      INVALID = -1,
      INDUSTRY_BUILD,
      RESEARCH_BUILD,
      SHIP_BUILD
   };
   
   /** \name Structors */ //@{
   PlanetBuildOrder();
   PlanetBuildOrder(int empire, int planet, BuildType build, int ship_type = -1);
   //@}
   
   /** \name Accessors */ //@{
   int   PlanetID() const     {return m_planet;}       ///< returns ID of planet selected in this order
   int   BuildOrder() const   {return m_build_type;}   ///< returns the build selection set in this order
   int   ShipType() const     {return m_ship_type;}    ///< returns the ship type selected for build.  Returns -1 if build type is not SHIP_BUILD.
   
   virtual void Execute() const;
   //@}
   
private:
   int         m_planet;
   BuildType   m_build_type;
   int         m_ship_type;
};


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
   
   virtual void Execute() const;
   //@}
   
private:
   int   m_fleet;
   int   m_dest_system;
};


/** the Order subclass that represents addition of ships to a fleet*/
class FleetMergeOrder : public Order
{
public:
   /** \name Structors */ //@{
   FleetMergeOrder();
   FleetMergeOrder(int empire, int fleet, const std::vector<int>& ships);
   //@}
   
   /** \name Accessors */ //@{
   int                     FleetID() const   {return m_fleet;}     ///< returns ID of the fleet selected in this order
   const std::vector<int>& Ships() const     {return m_add_ships;} ///< returns IDs of the ships selected for addition to the fleet
   
   virtual void Execute() const;
   //@}
   
private:
   int               m_fleet;
   std::vector<int>  m_add_ships;
};


/** the Order subclass that represents removal of ships from a fleet*/
class FleetSplitOrder : public Order
{
public:
   /** \name Structors */ //@{
   FleetSplitOrder();
   FleetSplitOrder(int empire, int fleet, const std::vector<int>& ships);
   //@}
   
   /** \name Accessors */ //@{
   int                     FleetID() const   {return m_fleet;}        ///< returns ID of the fleet selected in this order
   const std::vector<int>& Ships() const     {return m_remove_ships;} ///< returns IDs of the ships selected for removal from the fleet
   
   virtual void Execute() const;
   //@}
   
private:
   int               m_fleet;
   std::vector<int>  m_remove_ships;
};


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
   
   virtual void Execute() const;
   //@}
   
private:
   int   m_fleet;
   int   m_planet;
};

#endif // _Order_h_
