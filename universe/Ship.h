// -*- C++ -*-
#ifndef _Ship_h_
#define _Ship_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

class Fleet;

/** a class representing a FreeOrion designed ship class*/
struct ShipDesign
{
   /** \name Structors */ //@{
   ShipDesign(); ///< default ctor
   ShipDesign(const GG::XMLElement& elem); ///< ctor that constructs a ShipDesign object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ShipDesign object
   //@}
   
   int         id;      ///< unique ID to identify this design
   int         empire;  ///< the empire that designed this ship
   std::string name;    ///< the name of the design
   
   int WarpSpeed() const; ///< returns the maximum speed for ships of this design.  Returns 1 for 0.1
   
   
   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   enum V01DesignID
   {
     SCOUT = 1,  // Ship designs IDs start at 1
     COLONY,
     MARK1,
     MARK2,
     MARK3,
     MARK4
   };

   int         attack;  ///< the attack value of the design
   int         defense; ///< the defense value of the design
   int         cost;    ///< the cost of the design, in PP
   bool        colonize; ///< whether or not the ship is capable of creating a new colony
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////


   /** \name Accessors */ //@{
  	GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a ShipDesign object
   //@}
};

/** a class representing a single FreeOrion ship*/
class Ship : public UniverseObject
{
public:
   /** \name Structors */ //@{
   Ship(); ///< default ctor
   Ship(int empire_id, int design_id); ///< general ctor taking just the ship's empire and design id; from this the design can be looked up and used to create the ship
   Ship(const GG::XMLElement& elem); ///< ctor that constructs a Ship object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Ship object
   //@}

   /** \name Accessors */ //@{
   const  ShipDesign& Design() const {return m_design;}   ///< returns the design of the ship, containing engine type, weapons, etc.
   int    FleetID() const            {return m_fleet_id;} ///< returns the ID of the fleet the ship is residing in
   Fleet* GetFleet() const;                               ///< returns the ID of the fleet the ship is residing in
   
   virtual UniverseObject::Visibility Visible(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
   virtual GG::XMLElement XMLEncode(int empire_id = ENCODE_FOR_ALL_EMPIRES) const; ///< constructs an XMLElement from a Ship object with visibility limited relative to the input empire
   
   bool IsArmed() const; 
   
   //@}
  	
   /** \name Mutators */ //@{   
   void         SetFleetID(int fleet_id) {m_fleet_id = fleet_id; StateChangedSignal()();} ///< sets the ID of the fleet the ship resides in
   virtual void MovementPhase( );
   virtual void PopGrowthProductionResearchPhase( );
   //@}

private:
   ShipDesign  m_design;
   int         m_fleet_id;
};

#endif // _Ship_h_

