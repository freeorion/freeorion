// -*- C++ -*-
#ifndef _ProdCenter_h_
#define _ProdCenter_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

/** a production center decoration for a UniverseObject.*/
class ProdCenter
{
public:
   /** the types of production focus*/
   enum FocusType {BALANCED,
                   FARMING,
                   INDUSTRY,
                   MINING,
                   SCIENCE
                  }; // others TBD
                    
   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   enum BuildType {NOT_BUILDING, INDUSTRY_BUILD, RESEARCH_BUILD, SHIP_BUILD, DEF_BASE};
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   /** \name Structors */ //@{
   ProdCenter(); ///< default ctor
   ProdCenter(double workforce); ///< general ctor taking an initial workforce value, in pop points
   ProdCenter(const GG::XMLElement& elem); ///< ctor that constructs a ProdCenter object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ProdCenter object
   virtual ~ProdCenter(); ///< dtor
   //@}

   /** \name Accessors */ //@{
   FocusType   PrimaryFocus() const    {return m_primary;}
   FocusType   SecondaryFocus() const  {return m_secondary;}
   double      ProdPoints() const;
   double      Rollover() const        {return m_rollover;}

   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   BuildType CurrentlyBuilding() const {return m_currently_building;}
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   virtual UniverseObject::Visibility Visible(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a ProdCenter object
   virtual GG::XMLElement XMLEncode(int empire_id) const; ///< constructs an XMLElement from a ProdCenter object with visibility limited relative to the input empire
   //@}

   /** \name Mutators */ //@{
   void SetPrimaryFocus(FocusType focus);
   void SetSecondaryFocus(FocusType focus);
   void AdjustWorkforce(double workforce);
   
   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   void BuildDefBase();
   void BuildShip(int id);
   void BuildIndustry();
   void DoResearch();
   void AdjustIndustry(double industry);
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   virtual void MovementPhase(std::vector<SitRepEntry>& sit_reps);
   virtual void PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps);

   //@}
   
private:
   FocusType      m_primary;
   FocusType      m_secondary;
   
   double         m_workforce; ///< pop points present in this center

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

