#ifndef _PopCenter_h_
#define _PopCenter_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

/** a population center decoration for a UniverseObject.  This is a decorator class.  That is, it is designed to 
   augment another class with extra functionality.  Any class that wishes to inherit from this one should be 
   sure to inherit for UniversObject virtually as well, to make sure that there is only one copy of UniverseObject
   in each inheriting class.  The serialization functions for decorators should not do anything with their base class,
   in this case UniverseObject.  This is because the decorated class should be inherited from UniverseObject directly
   as well, and should construct the UniverseObject portion of itself first.  The first constructed instance of a 
   virtual base class is always the one that matters, so the decorators should not bother with constructing 
   UniverseObject at all.  This means that PopCenter and the other UniverseObject decorators should always default
   construct their UniverseObject bases in their constructors, even in the GG::XMLElement constructors.  Planet 
   is the most obvious class to inherit PopCenter, but other classes could be made from it as well (e.g., a ship 
   that is large enough to support population and still travel between systems).*/
class PopCenter : virtual public UniverseObject
{
public:
   /** the types of population density*/
   enum DensityType {OUTPOST,
                     SPARSE,
                     AVERAGE,
                     DENSE,
                     SUPERDENSE
                    }; // others TBD (these are from the Public Review: Population & Econ Model thread on the forums)

   /** \name Structors */ //@{
   PopCenter(); ///< default ctor
   PopCenter(double max_pop); ///< basic ctor that only specifies a max pop (DEFAULT_POP_SCALE_FACTOR is used for the scale factor)
   PopCenter(double max_pop, int race); ///< basic ctor that specifies a max pop and race
   PopCenter(const GG::XMLElement& elem); ///< ctor that constructs a PopCenter object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a PopCenter object
   ~PopCenter(); ///< dtor
   //@}

   /** \name Accessors */ //@{
   double               PopPoints() const    {return m_pop;}                  ///< returns the number of pop points in this center
   double               MaxPop() const       {return m_max_pop;}              ///< returns the max number of pop points possible in this center
   double               PopGrowth() const    {return m_growth;}               ///< returns the change in pop points since the last turn (may be negative)
   int                  Race() const         {return m_race;}                 ///< returns the race that the population is composed of
   double               Inhabitants() const; ///< returns the number of inhabitants in the center (not the pop points); depends on race
   DensityType          PopDensity() const;  ///< returns the density of this center, based on its population

  	virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a PopCenter object
   //@}

   /** \name Mutators */ //@{
   /** adjusts the population by \a pop, down to a minimum of 0.0, or up to a maximum of MaxPop().  This function 
      returns the (positive) pop surplus, or the (negative) pop deficit that would result from 
      adjusting the population by \a pop points, or 0 if the adjustment falls within [0.0, MaxPop()]*/
   double AdjustPop(double pop);
   void SetMaxPop(double max_pop) {m_max_pop = max_pop;}
   
   void SetRace(int race)  {m_race = race;}   ///< sets the race of the population to \a race
   
   virtual void MovementPhase(std::vector<SitRepEntry>& sit_reps);
   virtual void PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps);

  	virtual void XMLMerge(const GG::XMLElement& elem); ///< updates the PopCenter object from an XMLElement object that represents the updates
   //@}
   
private:
   double   m_pop;          // these are all in points, not inhabitants
   double   m_max_pop;
   double   m_growth;
   int      m_race; ///< the id of the race that occupies this planet
};

#endif // _PopCenter_h_

