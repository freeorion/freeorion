// -*- C++ -*-
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

#ifndef _Ship_h_
#include "Ship.h"
#endif


/** a class representing a FreeOrion planet.*/
class Planet : public UniverseObject, public PopCenter, public ProdCenter
{
public:
   /** the sizes of planets in FreeOrion*/
   enum PlanetSize {SZ_NOWORLD,   // used to designate an empty planet slot
                    SZ_TINY,
                    SZ_SMALL,
                    SZ_MEDIUM,
                    SZ_LARGE,
                    SZ_HUGE,
                    SZ_ASTEROIDS,
                    SZ_GASGIANT,
                    MAX_PLANET_SIZE   //keep this last
                   };

   /** the environmental conditions of planets in FreeOrion*/
   enum PlanetEnvironment {PE_UNINHABITABLE,   //for gas giants and asteroids
                    PE_TERRIBLE,
                    PE_ADEQUATE,
                    PE_OPTIMAL,
                    PE_SUPERB,
                    MAX_PLANET_ENVIRONMENT   //keep this last
                   };

   /** \name Structors */ //@{
   Planet(); ///< default ctor
   Planet(PlanetType type, PlanetSize size); ///< general ctor taking just the planet's type and size
   Planet(const GG::XMLElement& elem); ///< ctor that constructs a Planet object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Planet object
   //@}

   /** \name Accessors */ //@{
   PlanetType     Type() const {return m_type;}
   PlanetSize     Size() const {return m_size;}
   
   bool IsAboutToBeColonized() const {return m_is_about_to_be_colonized;}

   /////////////////////////////////////////////////////////////////////////////
   // V0.2 ONLY!!!!
   int DefBases() const {return m_def_bases;}
   // V0.2 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   virtual UniverseObject::Visibility GetVisibility(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
   virtual GG::XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a planet object with visibility limited relative to the input empire
   //@}
  	
   /** \name Mutators */ //@{
   virtual void MovementPhase( );
   virtual void PopGrowthProductionResearchPhase( );

   /////////////////////////////////////////////////////////////////////////////
   // V0.2 ONLY!!!!
   void AdjustDefBases(int bases) {m_def_bases += bases; if (m_def_bases < 0) m_def_bases = 0; StateChangedSignal()();}
   // V0.2 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////

   //@}

    /** \name Mutators */ //@{

    virtual void AddOwner   (int id);  ///< adds the Empire with ID \a id to the list of owners of this planet, update system owners and empire planets
    virtual void RemoveOwner(int id);   ///< removes the Empire with ID \a id to the list of owners of this planet, update system owners and empire planets

   /// Called during combat when a planet changes hands
    void Conquer( int conquerer ) ;
    
    /// Called during colonization when a planet is about to be colonized
    void IsAboutToBeColonized(bool bB);

    //@}
   PlanetEnvironment   Environment();

private:
   PlanetType     m_type;
   PlanetSize     m_size;
   
   int m_just_conquered;

   ///< flag to indicate that the planet is colonized the very next turn
   ///< isn't stored by XMLEncode
   bool m_is_about_to_be_colonized; 
   
   /////////////////////////////////////////////////////////////////////////////
   // V0.2 ONLY!!!!
   int            m_def_bases;
   // V0.2 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
};

#endif // _Planet_h_


