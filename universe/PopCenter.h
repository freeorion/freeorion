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

    /** \name Signal Types */ //@{
    typedef boost::signal<UniverseObject* (), Default0Combiner> GetObjectSignalType; ///< emitted as a request for the UniverseObject to which this PopCenter is attached
    //@}

    /** \name Structors */ //@{
    PopCenter(double max_pop_mod, double max_health_mod); ///< basic ctor
    PopCenter(int race, double max_pop_mod, double max_health_mod); ///< basic ctor
    virtual ~PopCenter(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    const Meter& PopulationMeter() const      {return m_pop;}               ///< returns the population Meter for this center
    const Meter& HealthMeter() const          {return m_health;}            ///< returns the population Meter for this center
    double       PopPoints() const            {return m_pop.Current();}     ///< returns the number of pop points in this center
    double       MaxPop() const               {return m_pop.Max();}         ///< returns the max number of pop points possible in this center
    double       PopGrowth() const            {return m_growth;}            ///< returns the change in pop points since the last turn (may be negative)
    double       Health() const               {return m_health.Current();}  ///< returns current health of this center
    double       MaxHealth() const            {return m_health.Max();}      ///< returns max health of this center
    int          Race() const                 {return m_race;}              ///< returns the race that the population is composed of
    double       Inhabitants() const; ///< returns the number of inhabitants in the center (not the pop points); depends on race
    DensityType  PopDensity() const;  ///< returns the density of this center, based on its population
    double       AvailableFood() const {return m_available_food;}   ///< returns the amount of food which is currently available

    double       FuturePopGrowth() const;    ///< predicts by which amount the population will grow next turn, AvailableFood might limit growth rate
    double       FuturePopGrowthMax() const; ///< predicts by which amount the population will grow at maximum next turn (assuming there is enough food)

    double       FutureHealthGrowth() const; ///< predicts by which amount the health meter will grow next turn

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

    /// Resets the meters, etc.  This should be called when a PopCenter is wiped out due to starvation, etc.
    void Reset(double max_pop_mod, double max_health_mod);
    //@}

protected:
    mutable GetObjectSignalType GetObjectSignal; ///< the UniverseObject-retreiving signal object for this PopCenter

private:
    PopCenter(); ///< default ctor

    Meter    m_pop;
    Meter    m_health;
    double   m_growth;
    int      m_race; ///< the id of the race that occupies this planet
    double   m_available_food;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void PopCenter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_pop)
        & BOOST_SERIALIZATION_NVP(m_health)
        & BOOST_SERIALIZATION_NVP(m_growth)
        & BOOST_SERIALIZATION_NVP(m_race)
        & BOOST_SERIALIZATION_NVP(m_available_food);
}

#endif // _PopCenter_h_


