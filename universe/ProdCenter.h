// -*- C++ -*-
#ifndef _ProdCenter_h_
#define _ProdCenter_h_

#ifndef _Meter_h_
#include <Meter.h>
#endif

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
   const Meter&   FarmingMeter() const     {return m_farming;}      ///< returns the farming Meter for this center
   const Meter&   IndustryMeter() const    {return m_industry;}     ///< returns the industry Meter for this center
   const Meter&   MiningMeter() const      {return m_mining;}       ///< returns the mining Meter for this center
   const Meter&   ResearchMeter() const    {return m_research;}     ///< returns the research Meter for this center
   const Meter&   TradeMeter() const       {return m_trade;}        ///< returns the trade Meter for this center
   const Meter&   ConstructionMeter() const{return m_construction;} ///< returns the construction Meter for this center
   double         FarmingPoints() const;
   double         IndustryPoints() const;
   double         MiningPoints() const;
   double         ResearchPoints() const;
   double         PercentComplete() const;
   double         Rollover() const         {return m_rollover;}
   double         Workforce() const        {return m_workforce;}
   PlanetType     GetPlanetType() const    {return m_planet_type;}
   
   ///< returns costs to build one item of the currently produced type
   double ItemBuildCost() const;

   double AvailableMinerals() const {return m_available_minerals;}
   double ProductionPoints() const;
   double ProductionPointsMax() const;

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
   Meter&   FarmingMeter()                  {return m_farming;}      ///< returns the farming Meter for this center
   Meter&   IndustryMeter()                 {return m_industry;}     ///< returns the industry Meter for this center
   Meter&   MiningMeter()                   {return m_mining;}       ///< returns the mining Meter for this center
   Meter&   ResearchMeter()                 {return m_research;}     ///< returns the research Meter for this center
   Meter&   TradeMeter()                    {return m_trade;}        ///< returns the trade Meter for this center
   Meter&   ConstructionMeter()             {return m_construction;} ///< returns the construction Meter for this center
   void SetWorkforce(double workforce);
   void SetMaxWorkforce(double max_workforce);
   void SetPlanetType(PlanetType planet_type);
   void SetAvailableMinerals(double available_minerals) {m_available_minerals = available_minerals;}

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
   void UpdateShipBuildProgress( Empire *empire, const int system_id, const int planet_id, ShipDesign::V02DesignID design_id );

   FocusType  m_primary;
   FocusType  m_secondary;

   Meter      m_farming;
   Meter      m_industry;
   Meter      m_mining;
   Meter      m_research;
   Meter      m_trade;
   Meter      m_construction;

   double     m_workforce;     ///< pop points present in this center
   double     m_max_workforce; ///< max pop points available at this center
   PlanetType m_planet_type;   ///< the environment at this center

   double     m_available_minerals;

   /////////////////////////////////////////////////////////////////////////////
   // V0.2 ONLY!!!!
   BuildType      m_currently_building;
   double         m_rollover;  ///< for build types that can span multiple turns this specifies how many production points are rolled over towards the next turn.
   // V0.2 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////

   mutable ProdCenterChangedSignalType m_prod_changed_sig;
};

#endif // _ProdCenter_h_


