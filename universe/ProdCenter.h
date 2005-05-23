// -*- C++ -*-
#ifndef _ProdCenter_h_
#define _ProdCenter_h_

#ifndef _Meter_h_
#include "Meter.h"
#endif

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#ifndef _Ship_h_
#include "Ship.h"
#endif

#ifndef _GGEnum_h_
#include "GGEnum.h"
#endif

class Empire;


/** a production center decoration for a UniverseObject. */
class ProdCenter
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ProdCenterChangedSignalType; ///< emitted when the ProdCenter is altered in any way
    //@}

    /** \name Structors */ //@{
    ProdCenter(const Meter& pop, UniverseObject* object); ///< default ctor
    ProdCenter(const GG::XMLElement& elem, const Meter& pop, UniverseObject* object); ///< ctor that constructs a ProdCenter object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ProdCenter object
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
    double         TradePoints() const;
    double         PercentComplete() const;
    double         Rollover() const         {return m_rollover;}

    ///< returns costs to build one item of the currently produced type
    double ItemBuildCost() const;

    double AvailableMinerals() const {return m_available_minerals;}
    double ProductionPoints() const;
    double ProductionPointsMax() const;

    std::pair<BuildType, std::string> CurrentlyBuilding() const {return m_currently_building;}

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
    void SetAvailableMinerals(double available_minerals) {m_available_minerals = available_minerals;}

    void SetProduction(BuildType type, const std::string& name);

    virtual void AdjustMaxMeters();
    virtual void PopGrowthProductionResearchPhase();

    /// Resets the meters, etc.  This should be called when a ProdCenter is wiped out due to starvation, etc.
    void Reset();
    //@}

private:
    ///< Updates build progress and determines if an item of given cost has been built. Handles
    ///< logic for rollovers and multiple items. Returns the number of items built, 0 if none
    int UpdateBuildProgress(double item_cost);

    ///< until shipyards, planets build ships as part of it's implementation of ProdCenters
    ///< takes a design name and if any are build, adds the ships to a new fleet.
    void UpdateShipBuildProgress(Empire *empire, const std::string& design_name);

    void UpdateBuildingBuildProgress(Empire *empire, const std::string& building_type_name);

    FocusType  m_primary;
    FocusType  m_secondary;

    Meter      m_farming;
    Meter      m_industry;
    Meter      m_mining;
    Meter      m_research;
    Meter      m_trade;
    Meter      m_construction;

    const Meter&          m_pop;    ///< current / max pop present in this center (may be the one from m_object, e.g. if m_object is a Planet)
    UniverseObject* const m_object; ///< the UniverseObject of which this center is a part

    double     m_available_minerals; ///< the minerals available at this center for use in local industry

    std::pair<BuildType, std::string> m_currently_building;
    double         m_rollover;  ///< for build types that can span multiple turns this specifies how many production points are rolled over towards the next turn.

    mutable ProdCenterChangedSignalType m_prod_changed_sig;
};

inline std::pair<std::string, std::string> ProdCenterRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _ProdCenter_h_


