// -*- C++ -*-
#ifndef _PopCenter_h_
#define _PopCenter_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#ifndef _Meter_h_
#include "Meter.h"
#endif

/** a population center decoration for a UniverseObject.  This is a decorator class.  That is, it is designed to 
    augment another class with extra functionality.  Planet is the most obvious class to inherit PopCenter, but 
    other classes could be made from it as well (e.g., a ship that is large enough to support population and still 
    travel between systems).*/
class PopCenter
{
public:
    /** the types of population density*/
    enum DensityType {
        OUTPOST,
        SPARSE,
        AVERAGE,
        DENSE,
        SUPERDENSE
    }; // others TBD (these are from the Public Review: Population & Econ Model thread on the forums)

    /** \name Structors */ //@{
    PopCenter(UniverseObject* object); ///< basic ctor that only specifies a max pop (DEFAULT_POP_SCALE_FACTOR is used for the scale factor)
    PopCenter(int race, UniverseObject* object); ///< basic ctor that specifies a max pop and race
    PopCenter(const GG::XMLElement& elem, UniverseObject* object); ///< ctor that constructs a PopCenter object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a PopCenter object
    virtual ~PopCenter(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    const Meter& PopulationMeter() const      {return m_pop;}        ///< returns the population Meter for this center
    const Meter& HealthMeter() const          {return m_health;}     ///< returns the population Meter for this center
    double       PopPoints() const            {return m_pop.Current();}///< returns the number of pop points in this center
    double       MaxPop() const               {return m_pop.Max();}  ///< returns the max number of pop points possible in this center
    double       PopGrowth() const            {return m_growth;}     ///< returns the change in pop points since the last turn (may be negative)
    int          Race() const                 {return m_race;}       ///< returns the race that the population is composed of
    double       Inhabitants() const; ///< returns the number of inhabitants in the center (not the pop points); depends on race
    DensityType  PopDensity() const;  ///< returns the density of this center, based on its population
    double       AvailableFood() const {return m_available_food;}   ///< returns the amount of food which is currently available

    double       FuturePopGrowth() const;    ///< predicts by which amount the population will grow next turn, AvailableFood might limit growth rate
    double       FuturePopGrowthMax() const; ///< predicts by which amount the population will grow at maximum next turn (assuming there is enough food)

    virtual GG::XMLElement XMLEncode(UniverseObject::Visibility vis) const; ///< constructs an XMLElement from a PopCenter object with the given visibility
    //@}

    /** \name Mutators */ //@{
    Meter& PopulationMeter()      {return m_pop;}    ///< returns the population Meter for this center
    Meter& HealthMeter()          {return m_health;} ///< returns the population Meter for this center

    /** adjusts the population by \a pop, down to a minimum of 0.0, or up to a maximum of MaxPop().  This function 
        returns the (positive) pop surplus, or the (negative) pop deficit that would result from 
        adjusting the population by \a pop points, or 0 if the adjustment falls within [0.0, MaxPop()]*/
    double AdjustPop(double pop);

    void SetRace(int race)                       {m_race = race;}   ///< sets the race of the population to \a race
    void SetAvailableFood(double available_food) {m_available_food = available_food;}   ///< sets the amount of food which is currently available

    virtual void AdjustMaxMeters();
    virtual void PopGrowthProductionResearchPhase();
    //@}
   
private:
    Meter    m_pop;
    Meter    m_health;
    double   m_growth;
    int      m_race; ///< the id of the race that occupies this planet
    double   m_available_food;

    UniverseObject* const m_object; ///< the UniverseObject of which this center is a part
};

#endif // _PopCenter_h_


