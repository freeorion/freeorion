// -*- C++ -*-
#ifndef _ProdCenter_h_
#define _ProdCenter_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#ifndef _Ship_h_
#include "Ship.h"
#endif

class Empire;

/** a production center decoration for a UniverseObject.*/
class ProdCenter
{
public:
   /** the types of production focus*/
   enum FocusType {FOCUS_UNKNOWN,
                   BALANCED,
                   FARMING,
                   INDUSTRY,
                   MINING,
                   SCIENCE
                  }; // others TBD
                      
   /** \name Signal Types */ //@{
   typedef boost::signal<void ()> ProdCenterChangedSignalType; ///< emitted when the UniverseObject is altered in any way
   //@}

   /////////////////////////////////////////////////////////////////////////////
   // V0.2 ONLY!!!!
   enum BuildType {BUILD_UNKNOWN, NOT_BUILDING, SCOUT, COLONY_SHIP, MARKI, MARKII, MARKIII, MARKIV, DEF_BASE};
   // V0.2 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   /** \name Structors */ //@{
   ProdCenter(); ///< default ctor
   ProdCenter(const GG::XMLElement& elem); ///< ctor that constructs a ProdCenter object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ProdCenter object
   virtual ~ProdCenter(); ///< dtor
   //@}

   /** \name Accessors */ //@{
   FocusType      PrimaryFocus() const     {return m_primary;}
   FocusType      SecondaryFocus() const   {return m_secondary;}
   double         FarmingPoints() const;
   double         IndustryPoints() const;
   double         MiningPoints() const;
   double         ResearchPoints() const;
   double         BuildProgress() const    {return m_build_progress;}
   double         Rollover() const         {return m_rollover;}
   double         Workforce() const        {return m_workforce;}
   PlanetType     GetPlanetType() const    {return m_planet_type;}

   /////////////////////////////////////////////////////////////////////////////
   // V0.2 ONLY!!!!
   BuildType     CurrentlyBuilding() const {return m_currently_building;}
   // V0.2 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   virtual GG::XMLElement XMLEncode(UniverseObject::Visibility vis) const; ///< constructs an XMLElement from a ProdCenter object with the given visibility
     
   ProdCenterChangedSignalType& ProdCenterChangedSignal() const {return m_prod_changed_sig;} ///< returns the state changed signal object for this UniverseObject
   //@}

   /** \name Mutators */ //@{
   void SetPrimaryFocus(FocusType focus);
   void SetSecondaryFocus(FocusType focus);
   void SetWorkforce(double workforce);
   void SetMaxWorkforce(double max_workforce);
   void SetPlanetType(PlanetType planet_type);

   /////////////////////////////////////////////////////////////////////////////
   // V0.2 ONLY!!!!
   void SetProduction(ProdCenter::BuildType type);
   // V0.2 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   virtual void MovementPhase( );
   void PopGrowthProductionResearchPhase( Empire *pEmpire, const int system_id, const int planet_id );
   //@}
   
private:

   ///< Updates build progress and determines if an item of given cost has been built. Handles
   ///< logic for rollovers and multiple items. Returns the number of items built, 0 if none
   int UpdateBuildProgress( int item_cost );

   ///< until shipyards, planets build ships as psrt of it's implementation of ProdCenters
   ///< takes a design ID and if any are build, adds the ships to a fleet.
   void UpdateShipBuildProgress( Empire *empire, const int system_id, const int planet_id, ShipDesign::V01DesignID design_id );

   FocusType  m_primary;
   FocusType  m_secondary;
   
   double     m_workforce;     ///< pop points present in this center
   double     m_max_workforce; ///< max pop points available at this center
   PlanetType m_planet_type;   ///< the environment at this center

   /////////////////////////////////////////////////////////////////////////////
   // V0.2 ONLY!!!!
   BuildType      m_currently_building;
   double         m_build_progress; ///< build progress towards the current build target (may be 0.0 if not applicable, such as when no target exists)
   double         m_rollover;  ///< for build types that can span multiple turns this specifies how many production points are rolled over towards the next turn.
   // V0.2 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////

   mutable ProdCenterChangedSignalType m_prod_changed_sig;
};

#endif // _ProdCenter_h_


