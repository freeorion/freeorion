#ifndef _Planet_h_
#define _Planet_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#ifndef _PopCenter_h_
#include "PopCenter.h"
#endif

#ifndef _ProdCenter_h_
#include "ProdCenter.h"
#endif

/** a class representing a FreeOrion planet.  A Planet is a type of UniverseObject, and has some UniverseObject decorators
   attached to it as well.  See PopCenter for notes on decorators.*/
class Planet : virtual public UniverseObject, public PopCenter, public ProdCenter
{
public:
   /** the types of planets in FreeOrion*/
   enum PlanetType {TOXIC,
                    RADIATED,
                    BARREN,
                    DESERT,
                    TUNDRA,
                    OCEAN,
                    TERRAN,
                    GAIA
                   }; // others TBD (these are from the drek spreadsheet)

   /** the sizes of planets in FreeOrion*/
   enum PlanetSize {TINY,
                    SMALL,
                    MEDIUM,
                    LARGE,
                    HUGE
                   }; // others TBD (these are from the drek spreadsheet)

   /** \name Structors */ //@{
   Planet(); ///< default ctor
   Planet(PlanetType type, PlanetSize size); ///< general ctor taking just the planet's type and size
   Planet(const GG::XMLElement& elem); ///< ctor that constructs a Planet object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Planet object
   //@}

   /** \name Accessors */ //@{
   PlanetType  Type() const {return m_type;}
   PlanetSize  Size() const {return m_size;}
   
   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   int DefBases() const {return m_def_bases;}
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
  	virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a Planet object
   //@}
  	
   /** \name Mutators */ //@{
   virtual void MovementPhase(std::vector<SitRepEntry>& sit_reps);
   virtual void PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps);

   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   void AdjustDefBases(int bases) {m_def_bases += bases; if (m_def_bases < 0) m_def_bases = 0;}
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////

  	virtual void XMLMerge(const GG::XMLElement& elem); ///< updates the Planet object from an XMLElement object that represents the updates
   //@}

private:
   PlanetType     m_type;
   PlanetSize     m_size;
   
   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   int            m_def_bases;
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
};

#endif // _Planet_h_

