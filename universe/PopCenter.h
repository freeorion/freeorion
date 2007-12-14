// -*- C++ -*-
#ifndef _PopCenter_h_
#define _PopCenter_h_

#include "UniverseObject.h"
#include "Meter.h"

/** The PopCenter class is an abstract base class for anything in the FreeOrion gamestate that has population on 
 *  or in it.  Most likely, such an object will also be a subclass of UniverseObject.
 *  
 *  Planet is the most obvious class to inherit PopCenter, but other classes could be made from it as well (e.g.,
 *  a ship that is large enough to support population and still travel between systems).
 */
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
    PopCenter(int race); ///< basic ctor
    virtual ~PopCenter(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    int                     Race() const {return m_race;}                       ///< returns the race that the population is composed of
    double                  Inhabitants() const;                                ///< returns the number of inhabitants in the center (not the pop points); depends on race
    DensityType             PopDensity() const;                                 ///< returns the density of this center, based on its population
    double                  AvailableFood() const {return m_available_food;}    ///< returns the amount of food which is currently available

    virtual double          ProjectedCurrentMeter(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn
    virtual double          MeterPoints(MeterType type) const;              ///< returns "true amount" associated with a meter.  In some cases (METER_POPULATION) this is just the meter value.  In other cases (METER_FARMING) this is some other value (a function of population and meter value).
    virtual double          ProjectedMeterPoints(MeterType type) const;     ///< returns expected "true amount" associated with a meter on the next turn

    double  FuturePopGrowth() const;    ///< predicts by which amount the population will grow next turn, AvailableFood might limit growth rate
    double  FuturePopGrowthMax() const; ///< predicts by which amount the population will grow at maximum next turn (assuming there is enough food)
    double  FutureHealthGrowth() const; ///< predicts by which amount the health meter will grow next turn
    //@}

    /** \name Mutators */ //@{

    /** adjusts the population by \a pop, down to a minimum of 0.0, or up to a maximum of MaxPop().  This function 
        returns the (positive) pop surplus, or the (negative) pop deficit that would result from 
        adjusting the population by \a pop points, or 0 if the adjustment falls within [0.0, MaxPop()]*/
    double          AdjustPop(double pop);

    void            SetRace(int race)                       {m_race = race;}   ///< sets the race of the population to \a race
    void            SetAvailableFood(double available_food) {m_available_food = available_food;}   ///< sets the amount of food which is currently available

    virtual void    ApplyUniverseTableMaxMeterAdjustments();
    virtual void    PopGrowthProductionResearchPhase();

    void            Reset(double max_pop_mod, double max_health_mod);   ///< Resets the meters, etc.  This should be called when a PopCenter is wiped out due to starvation, etc.
    //@}

protected:
    mutable GetObjectSignalType GetObjectSignal; ///< the UniverseObject-retreiving signal object for this PopCenter

    void Init(double max_pop_mod, double max_health_mod);    ///< initialization that needs to be called by derived class after derived class is constructed

private:
    PopCenter(); ///< default ctor

    virtual const Meter*    GetMeter(MeterType type) const = 0; ///< implimentation should return the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual Meter*          GetMeter(MeterType type) = 0;       ///< implimentation should return the requested Meter, or 0 if no such Meter of that type is found in this object

    virtual void InsertMeter(MeterType meter_type, Meter meter) = 0; ///< implimentation should add \a meter to the object so that it can be accessed with the GetMeter() functions

    int     m_race; ///< the id of the race that occupies this planet
    double  m_available_food;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void PopCenter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_race)
        & BOOST_SERIALIZATION_NVP(m_available_food);
}

#endif // _PopCenter_h_


