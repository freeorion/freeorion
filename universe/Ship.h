#ifndef _Ship_h_
#define _Ship_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

/** a class representing a FreeOrion designed ship class*/
struct ShipDesign
{
   /** \name Structors */ //@{
   ShipDesign(); ///< default ctor
   ShipDesign(const GG::XMLElement& elem); ///< ctor that constructs a ShipDesign object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ShipDesign object
   //@}
   
   int         id;      ///< unique ID to identify this design
   int         race;    ///< the race that designed this ship
   std::string name;    ///< the name of the design
   
   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   int         attack;  ///< the attack value of the design
   int         defense; ///< the defense value of the design
   int         cost;    ///< the cost of the design, in PP
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////

   /** \name Accessors */ //@{
  	GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a ShipDesign object
   //@}
};

/** a class representing a single FreeOrion ship*/
class Ship : virtual public UniverseObject
{
public:
   /** \name Structors */ //@{
   Ship(); ///< default ctor
   Ship(int race, int design_id); ///< general ctor taking just the ship's race and design id; from this the design can be looked up and used to create the ship
   Ship(const GG::XMLElement& elem); ///< ctor that constructs a Ship object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Ship object
   //@}

   /** \name Accessors */ //@{
   const ShipDesign& Design() const {return m_design;} ///< returns the design of the ship, containing engine type, weapons, etc.
   
  	virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a Ship object
   //@}
  	
   /** \name Mutators */ //@{   
   virtual void MovementPhase(std::vector<SitRepEntry>& sit_reps);
   virtual void PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps);

  	virtual void XMLMerge(const GG::XMLElement& elem); ///< updates the Ship object from an XMLElement object that represents the updates
   //@}

private:
   ShipDesign  m_design;
};

#endif // _Ship_h_

