#ifndef _ProdCenter_h_
#define _ProdCenter_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

/** a production center decoration for a UniverseObject.  See PopCenter for notes on decorators and how to use them.*/
class ProdCenter : virtual public UniverseObject
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
   ~ProdCenter(); ///< dtor
   //@}

   /** \name Accessors */ //@{
   FocusType   PrimaryFocus() const    {return m_primary;}
   FocusType   SecondaryFocus() const  {return m_secondary;}
   double      ProdPoints() const;

   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   BuildType CurrentlyBuilding() const {return m_currently_building;}
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
  	virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a ProdCenter object
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
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
   
   virtual void MovementPhase(std::vector<SitRepEntry>& sit_reps);
   virtual void PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps);

  	virtual void XMLMerge(const GG::XMLElement& elem); ///< updates the ProdCenter object from an XMLElement object that represents the updates
   //@}
   
private:
   FocusType      m_primary;
   FocusType      m_secondary;
   
   double         m_workforce; ///< pop points present in this center
   
   /////////////////////////////////////////////////////////////////////////////
   // V0.1 ONLY!!!!
   double         m_industry_factor; ///< a number in [0.0 1.0] representing the percentage of industrialization
   BuildType      m_currently_building;
   // V0.1 ONLY!!!!
   /////////////////////////////////////////////////////////////////////////////
};

#endif // _ProdCenter_h_

