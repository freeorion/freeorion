// -*- C++ -*-
#ifndef _PopCenter_h_
#define _PopCenter_h_

#include "Enums.h"

#include <boost/serialization/nvp.hpp>

class Meter;
class UniverseObject;

/** The PopCenter class is an abstract base class for anything in the FreeOrion
  * gamestate that has population on or in it.  Most likely, such an object
  * will also be a subclass of UniverseObject.
  * Planet is the most obvious class to inherit PopCenter, but other classes
  * could be made from it as well (e.g., a ship that is large enough to support
  * population and still travel between systems). */
class PopCenter {
public:
    /** \name Structors */ //@{
    PopCenter();                                        ///< default ctor
    explicit PopCenter(const std::string& species_name);///< basic ctor
    virtual ~PopCenter();                               ///< dtor
    //@}

    /** \name Accessors */ //@{
    const std::string&  SpeciesName() const {return m_species_name;}        ///< returns the name of the species that populates this planet

    std::string         Dump() const;

    double              NextTurnPopGrowth() const;                          ///< predicted pop growth next turn

    virtual double      InitialMeterValue(MeterType type) const = 0;        ///< implementation should return the initial value of the specified meter \a type
    virtual double      CurrentMeterValue(MeterType type) const = 0;        ///< implementation should current value of the specified meter \a type
    virtual double      NextTurnCurrentMeterValue(MeterType type) const = 0;///< implementation should return an estimate of the next turn's current value of the specified meter \a type
    //@}

    /** \name Mutators */ //@{
    void    Copy(const PopCenter* copied_object, Visibility vis = VIS_FULL_VISIBILITY);
    void    SetSpecies(const std::string& species_name);///< sets the species of the population to \a species_name
    void    Reset();                                    ///< Sets all meters to 0, clears race name
    //@}

protected:
    void    Init();                                     ///< initialization that needs to be called by derived class after derived class is constructed

    double  PopCenterNextTurnMeterValue(MeterType meter_type) const;///< returns estimate of the next turn's current values of meters relevant to this PopCenter
    void    PopCenterResetTargetMaxUnpairedMeters();
    void    PopCenterClampMeters();

    void    PopCenterPopGrowthProductionResearchPhase();

private:
    virtual Meter*          GetMeter(MeterType type) = 0;       ///< implementation should return the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual const Meter*    GetMeter(MeterType type) const = 0; ///< implementation should return the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual void            AddMeter(MeterType meter_type) = 0; ///< implementation should add a meter to the object so that it can be accessed with the GetMeter() functions

    std::string m_species_name;                                 ///< the name of the species that occupies this planet

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void PopCenter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_species_name);
}

#endif // _PopCenter_h_


