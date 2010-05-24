// -*- C++ -*-
#ifndef _Planet_h_
#define _Planet_h_

#include "UniverseObject.h"
#include "PopCenter.h"
#include "ResourceCenter.h"
#include "Meter.h"

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
    Planet();                                                                               ///< default ctor
    Planet(PlanetType type, PlanetSize size);                                               ///< general ctor taking just the planet's type and size

    virtual Planet*             Clone(int empire_id = ALL_EMPIRES) const;  ///< returns new copy of this Planet
    //@}

    /** \name Accessors */ //@{
    virtual const std::string&  TypeName() const;                               ///< returns user-readable string indicating the type of UniverseObject this is

    PlanetType                  Type() const {return m_type;}
    PlanetSize                  Size() const {return m_size;}
    PlanetEnvironment           Environment() const;

    Year                        OrbitalPeriod() const;
    Radian                      InitialOrbitalPosition() const;
    Radian                      OrbitalPositionOnTurn(int turn) const;
    Day                         RotationalPeriod() const;
    Degree                      AxialTilt() const;

    const std::set<int>&        Buildings() const {return m_buildings;}

    double                      AvailableTrade() const;                         ///< returns the trade available at this planet for use in building maintenance
    double                      BuildingCosts() const;                          ///< returns the cost in trade for the upkeep of all currently-enabled buildings

    virtual bool                        Contains(int object_id) const;                  ///< returns true iff this Planet contains a building with ID \a id.
    virtual std::vector<UniverseObject*>FindObjects() const;                            ///< returns objects contained within this object
    virtual std::vector<int>            FindObjectIDs() const;                          ///< returns ids of objects contained within this object

    bool                        IsAboutToBeColonized() const {return m_is_about_to_be_colonized;}

    virtual UniverseObject*     Accept(const UniverseObjectVisitor& visitor) const;

    virtual double              CurrentMeterValue(MeterType type) const;
    virtual double              NextTurnCurrentMeterValue(MeterType type) const;

    virtual const std::vector<std::string>&
                                AvailableFoci() const;
    //@}

    /** \name Mutators */ //@{
    virtual void    Copy(const UniverseObject* copied_object, int empire_id = ALL_EMPIRES);

    virtual Meter*  GetMeter(MeterType type);

    virtual void    SetSystem(int sys);
    virtual void    MoveTo(double x, double y);

    void            SetType(PlanetType type);           ///< sets the type of this Planet to \a type
    void            SetSize(PlanetSize size);           ///< sets the size of this Planet to \a size

    /** Sets the orbital period based on the orbit this planet is in. If
        tidally locked, the rotational period is also adjusted. */
    void            SetOrbitalPeriod(unsigned int orbit, bool tidal_lock);

    void            SetRotationalPeriod(Day days);      ///< sets the rotational period of this planet
    void            SetHighAxialTilt();                 ///< randomly generates a new, high axial tilt

    void            AddBuilding(int building_id);       ///< adds the building to the planet
    bool            RemoveBuilding(int building_id);    ///< removes the building from the planet; returns false if no such building was found

    void            SetAvailableTrade(double trade);    ///< sets the trade available at this planet for use in building maintenance

    virtual void    AddOwner(int id);                   ///< adds the Empire with ID \a id to the list of owners of this planet, update system owners and empire planets
    virtual void    RemoveOwner(int id);                ///< removes the Empire with ID \a id to the list of owners of this planet, update system owners and empire planets

    void            Reset();                            ///< Resets the meters, specials, etc., of a planet to an unowned state.  This should be called when a planet is wiped out due to starvation, etc.

    void            Conquer(int conquerer);             ///< Called during combat when a planet changes hands
    void            SetIsAboutToBeColonized(bool b);    ///< Called during colonization when a planet is about to be colonized
    void            ResetIsAboutToBeColonized();        ///< Called after colonization, to reset the number of prospective colonizers to 0
    //@}

    static PlanetEnvironment    Environment(PlanetType type);           ///< returns the environment that corresponds to each planet type

private:
    void Init();

    virtual const Meter*    GetMeter(MeterType type) const;

    virtual void            PopGrowthProductionResearchPhase();
    virtual void            ResetTargetMaxUnpairedMeters(MeterType meter_type = INVALID_METER_TYPE);
    virtual void            ClampMeters();

    virtual Visibility      GetVisibility(int empire_id) const  {return UniverseObject::GetVisibility(empire_id);}
    virtual void            AddMeter(MeterType meter_type)      {UniverseObject::AddMeter(meter_type);}

    std::set<int>           VisibleContainedObjects(int empire_id) const;   ///< returns the subset of m_buildings that is visible to empire with id \a empire_id

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


// Tactical combat planet geometry free functions:

/** Returns the radius, in tactical combat units, of a planet.  Note that 0.0
    is returned for PlanetSize enumerators that have no size, whereas
    PlanetRadius(SZ_MEDIUM) is returned for unknown values. */
double PlanetRadius(PlanetSize size);

/** Returns the radius, in tactical combat units, of the tube in which an
    asteroid belt lies. */
double AsteroidBeltRadius();

#endif // _Planet_h_


