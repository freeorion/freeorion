// -*- C++ -*-
#ifndef _ProdCenter_h_
#define _ProdCenter_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#ifndef _Ship_h_
#include "Ship.h"
#endif


/** a production center decoration for a UniverseObject.*/
class Empire;
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
                    
   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   enum BuildType {BUILD_UNKNOWN, NOT_BUILDING, INDUSTRY_BUILD, RESEARCH_BUILD, SCOUT, COLONY_SHIP, MARKI, MARKII, MARKIII, MARKIV, DEF_BASE};
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   /** \name Structors */ //@{
   ProdCenter(); ///< default ctor
   ProdCenter(const GG::XMLElement& elem); ///< ctor that constructs a ProdCenter object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ProdCenter object
   virtual ~ProdCenter(); ///< dtor
   //@}

   /** \name Accessors */ //@{
   FocusType      PrimaryFocus() const     {return m_primary;}
   FocusType      SecondaryFocus() const   {return m_secondary;}
   double         ProdPoints() const;
   double         BuildProgress() const    {return m_build_progress;}
   double         Rollover() const         {return m_rollover;}
   double         Workforce() const {return m_workforce;}

   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   BuildType     CurrentlyBuilding() const {return m_currently_building;}
   double        IndustryFactor() const    {return m_industry_factor;}
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   virtual GG::XMLElement XMLEncode(UniverseObject::Visibility vis) const; ///< constructs an XMLElement from a ProdCenter object with the given visibility
   //@}

   /** \name Mutators */ //@{
   void SetPrimaryFocus(FocusType focus);
   void SetSecondaryFocus(FocusType focus);
   void SetWorkforce(double workforce);
   void SetMaxWorkforce(double max_workforce);

   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   void SetProduction(ProdCenter::BuildType type);
   bool AdjustIndustry(double industry);  /// returns true if max is hit
   // V0.1 ONLY!!!!
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

   FocusType      m_primary;
   FocusType      m_secondary;
   
   double         m_workforce; ///< pop points present in this center

   double         m_max_workforce; ///< max pop points avail at this center

   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   double         m_industry_factor; ///< a number in [0.0 1.0] representing the percentage of industrialization
   BuildType      m_currently_building;
   double         m_build_progress; ///< build progress in [0.0 1.0] towards the current build target (may be 0.0 if not applicable, such as when no target exists)
   double         m_rollover;  ///< for build types that can span multiple turns this specifies how many production points are rolled over towards the next turn.
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
};

#endif // _ProdCenter_h_

