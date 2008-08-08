// -*- C++ -*-
#ifndef _Planet_h_
#define _Planet_h_

#include "Universe.h"
#include "UniverseObject.h"
#include "PopCenter.h"
#include "ResourceCenter.h"


/** A type that is implicitly convertible to and from double, but which is not
    implicitly convertible among other numeric types. */
class TypesafeDouble
{
public:
    TypesafeDouble() : m_value(0.0) {}
    TypesafeDouble(double d) : m_value(d) {}
    operator double () const { return m_value; }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        { ar & BOOST_SERIALIZATION_NVP(m_value); }

private:
    double m_value;
};

class Day;

/** A value type representing a "year".  A "year" is arbitrarily defined to be 4
    turns. */
class Year : public TypesafeDouble
{
public:
    Year() : TypesafeDouble() {}
    Year(double d) : TypesafeDouble(d) {}
    explicit Year(Day d);

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        { ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TypesafeDouble); }
};

/** A value type representing a "day".  A "day" is arbitrarily defined to be
    1/360 of a "year", and 1/90 of a turn. */
class Day : public TypesafeDouble
{
public:
    Day() : TypesafeDouble() {}
    Day(double d) : TypesafeDouble(d) {}
    explicit Day(Year y) : TypesafeDouble(y * 360.0) {}

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        { ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TypesafeDouble); }
};

/** A value type used to represent an angle in radians. */
class Radian : public TypesafeDouble
{
public:
    Radian() : TypesafeDouble() {}
    Radian(double d) : TypesafeDouble(d) {}

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        { ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TypesafeDouble); }
};

/** A value type used to represent an angle in degrees. */
class Degree : public TypesafeDouble
{
public:
    Degree() : TypesafeDouble() {}
    Degree(double d) : TypesafeDouble(d) {}

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        { ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TypesafeDouble); }
};

/** a class representing a FreeOrion planet. */
class Planet :
    public UniverseObject,
    public PopCenter,
    public ResourceCenter
{
public:
    /** \name Structors */ //@{
    Planet(); ///< default ctor
    Planet(PlanetType type, PlanetSize size); ///< general ctor taking just the planet's type and size
    //@}

    /** \name Accessors */ //@{
    PlanetType                          Type() const {return m_type;}
    PlanetSize                          Size() const {return m_size;}
    PlanetEnvironment                   Environment() const;

    Year                                OrbitalPeriod() const;
    Radian                              InitialOrbitalPosition() const;
    Day                                 RotationalPeriod() const;
    Degree                              AxialTilt() const;

    const std::set<int>&                Buildings() const {return m_buildings;}

    double                              AvailableTrade() const;                         ///< returns the trade available at this planet for use in building maintenance
    double                              BuildingCosts() const;                          ///< returns the cost in trade for the upkeep of all currently-enabled buildings

    virtual bool                        Contains(int object_id) const;                  ///< returns true iff this Planet contains a building with ID \a id.
    virtual std::vector<UniverseObject*>FindObjects() const;                            ///< returns objects contained within this object
    virtual std::vector<int>            FindObjectIDs() const;                          ///< returns ids of objects contained within this object

    bool                                IsAboutToBeColonized() const {return m_is_about_to_be_colonized;}

    virtual Visibility                  GetVisibility(int empire_id) const;             ///< returns the visibility status of this universe object relative to the input empire.

    virtual UniverseObject*             Accept(const UniverseObjectVisitor& visitor) const;

    virtual double                      ProjectedCurrentMeter(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn
    virtual double                      MeterPoints(MeterType type) const;              ///< returns "true amount" associated with a meter.  In some cases (METER_POPULATION) this is just the meter value.  In other cases (METER_FARMING) this is some other value (a function of population and meter value).
    virtual double                      ProjectedMeterPoints(MeterType type) const;     ///< returns expected "true amount" associated with a meter on the next turn

    virtual const Meter*                GetMeter(MeterType type) const  {return UniverseObject::GetMeter(type);}
    //@}

    /** \name Mutators */ //@{
    virtual void                        SetSystem(int sys);
    virtual void                        MoveTo(double x, double y);

    virtual void                        MovementPhase();
    virtual void                        ApplyUniverseTableMaxMeterAdjustments(MeterType meter_type = INVALID_METER_TYPE);
    virtual void                        PopGrowthProductionResearchPhase();

    virtual Meter*                      GetMeter(MeterType type)    {return UniverseObject::GetMeter(type);}


    void                                SetType(PlanetType type);           ///< sets the type of this Planet to \a type
    void                                SetSize(PlanetSize size);           ///< sets the size of this Planet to \a size

    /** randomly generates an orbital period based on the orbit this planet is
        in, and whether it is tidally locked. */
    void                                SetOrbitalPeriod(int orbit, bool tidal_lock);

    void                                SetRotationalPeriod(Day days);      ///< sets the rotational period of this planet
    void                                SetHighAxialTilt();                  ///< randomly generates a new, high axial tilt

    void                                AddBuilding(int building_id);       ///< adds the building to the planet
    bool                                RemoveBuilding(int building_id);    ///< removes the building from the planet; returns false if no such building was found

    void                                SetAvailableTrade(double trade);    ///< sets the trade available at this planet for use in building maintenance

    virtual void                        AddOwner(int id);                   ///< adds the Empire with ID \a id to the list of owners of this planet, update system owners and empire planets
    virtual void                        RemoveOwner(int id);                ///< removes the Empire with ID \a id to the list of owners of this planet, update system owners and empire planets

    void                                Reset();                            ///< Resets the meters, specials, etc., of a planet to an unowned state.  This should be called when a planet is wiped out due to starvation, etc.

    void                                Conquer(int conquerer);             ///< Called during combat when a planet changes hands
    void                                SetIsAboutToBeColonized(bool b);    ///< Called during colonization when a planet is about to be colonized
    void                                ResetIsAboutToBeColonized();        ///< Called after colonization, to reset the number of prospective colonizers to 0
    //@}

    static PlanetEnvironment            Environment(PlanetType type);       ///< returns the environment that corresponds to each planet type

protected:
    void Init();

private:
    UniverseObject*                     This() {return this;}

    virtual void                        InsertMeter(MeterType meter_type, Meter meter) {UniverseObject::InsertMeter(meter_type, meter);}

    virtual const Meter*                GetPopMeter() const {return GetMeter(METER_POPULATION);}

    PlanetType      m_type;
    PlanetSize      m_size;
    Year            m_orbital_period;
    Radian          m_initial_orbital_position;
    Day             m_rotational_period;
    Degree          m_axial_tilt;

    std::set<int>   m_buildings;
    double          m_available_trade;

    bool            m_just_conquered;

    bool            m_is_about_to_be_colonized;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Planet::serialize(Archive& ar, const unsigned int version)
{
    Visibility vis;
    std::set<int> buildings;
    if (Archive::is_saving::value) {
        vis = GetVisibility(Universe::s_encoding_empire);

        const Universe& universe = GetUniverse();
        for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
            int obj_id = *it;
            const UniverseObject* obj = universe.Object(obj_id);
            if (obj->GetVisibility(Universe::s_encoding_empire) != VIS_NO_VISIBITY)
                buildings.insert(obj_id);
        }
    }

    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PopCenter)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ResourceCenter)
        & BOOST_SERIALIZATION_NVP(vis)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_orbital_period)
        & BOOST_SERIALIZATION_NVP(m_initial_orbital_position)
        & BOOST_SERIALIZATION_NVP(m_rotational_period)
        & BOOST_SERIALIZATION_NVP(m_axial_tilt)
        & BOOST_SERIALIZATION_NVP(m_just_conquered)
        & BOOST_SERIALIZATION_NVP(buildings);
    if (Universe::ALL_OBJECTS_VISIBLE || vis == VIS_FULL_VISIBILITY) {
        ar  & BOOST_SERIALIZATION_NVP(m_available_trade)
            & BOOST_SERIALIZATION_NVP(m_is_about_to_be_colonized);
    }
    if (Archive::is_loading::value)
        m_buildings = buildings;
}

#endif // _Planet_h_


