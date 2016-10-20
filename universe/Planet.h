#ifndef _Planet_h_
#define _Planet_h_

#include "UniverseObject.h"
#include "PopCenter.h"
#include "ResourceCenter.h"
#include "Meter.h"

#include "../util/Export.h"

/** a class representing a FreeOrion planet. */
class FO_COMMON_API Planet :
    public UniverseObject,
    public PopCenter,
    public ResourceCenter
{
public:
    /** \name Accessors */ //@{
    virtual std::set<std::string>
                                Tags() const;                                       ///< returns all tags this object has
    virtual bool                HasTag(const std::string& name) const;              ///< returns true iff this object has the tag with the indicated \a name

    virtual UniverseObjectType  ObjectType() const;
    virtual std::string         Dump() const;

    PlanetType                  Type() const                        { return m_type; }
    PlanetType                  OriginalType() const                { return m_original_type; }
    int                         DistanceFromOriginalType() const    { return TypeDifference(m_type, m_original_type); }
    PlanetSize                  Size() const                        { return m_size; }
    int                         SizeAsInt() const;

    PlanetEnvironment           EnvironmentForSpecies(const std::string& species_name = "") const;
    PlanetType                  NextBetterPlanetTypeForSpecies(const std::string& species_name = "") const;
    PlanetType                  NextCloserToOriginalPlanetType() const;
    PlanetType                  ClockwiseNextPlanetType() const;
    PlanetType                  CounterClockwiseNextPlanetType() const;
    PlanetSize                  NextLargerPlanetSize() const;
    PlanetSize                  NextSmallerPlanetSize() const;

    /**
     * An orbital period is equal to a planets "year". A "year" is arbitrarily
     * defined to be 4 turns. */
    float                       OrbitalPeriod() const;
    /** @returns an angle in radians. */
    float                       InitialOrbitalPosition() const;
    /** @returns an angle in radians. */
    float                       OrbitalPositionOnTurn(int turn) const;
    /**
     * The rotational period represents a planets "day".  A "day" is
     * arbitrarily defined to be 1/360 of a "year", and 1/90 of a turn. */
    float                       RotationalPeriod() const;
    /** @returns an angle in degree. */
    float                       AxialTilt() const;

    const std::set<int>&        BuildingIDs() const {return m_buildings;}

    virtual int                 ContainerObjectID() const;          ///< returns id of the object that directly contains this object, if any, or INVALID_OBJECT_ID if this object is not contained by any other
    virtual const std::set<int>&ContainedObjectIDs() const;         ///< returns ids of objects contained within this object
    virtual bool                Contains(int object_id) const;      ///< returns true if there is an object with id \a object_id is contained within this UniverseObject
    virtual bool                ContainedBy(int object_id) const;   ///< returns true if there is an object with id \a object_id that contains this UniverseObject

    virtual std::vector<std::string>    AvailableFoci() const;
    virtual const std::string&          FocusIcon(const std::string& focus_name) const;

    bool                        IsAboutToBeColonized() const    { return m_is_about_to_be_colonized; }
    bool                        IsAboutToBeInvaded() const      { return m_is_about_to_be_invaded; }
    bool                        IsAboutToBeBombarded() const    { return m_is_about_to_be_bombarded; }
    int                         OrderedGivenToEmpire() const    { return m_ordered_given_to_empire_id; }
    int                         LastTurnAttackedByShip() const  { return m_last_turn_attacked_by_ship; }

    virtual TemporaryPtr<UniverseObject>
                                Accept(const UniverseObjectVisitor& visitor) const;

    virtual float               InitialMeterValue(MeterType type) const;
    virtual float               CurrentMeterValue(MeterType type) const;
    virtual float               NextTurnCurrentMeterValue(MeterType type) const;

    const std::string&          SurfaceTexture() const          { return m_surface_texture; }
    std::string                 CardinalSuffix() const;     ///< returns a roman number representing this planets orbit in relation to other planets
    //@}

    /** \name Mutators */ //@{
    virtual void    Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES);

    virtual Meter*  GetMeter(MeterType type);

    void            SetType(PlanetType type);           ///< sets the type of this Planet to \a type
    void            SetOriginalType(PlanetType type);   ///< sets the original type of this Planet to \a type
    void            SetSize(PlanetSize size);           ///< sets the size of this Planet to \a size

    void            SetRotationalPeriod(float days);    ///< sets the rotational period of this planet
    void            SetHighAxialTilt();                 ///< randomly generates a new, high axial tilt

    void            AddBuilding(int building_id);       ///< adds the building to the planet
    bool            RemoveBuilding(int building_id);    ///< removes the building from the planet; returns false if no such building was found

    virtual void    Reset();                            ///< Resets the meters, specials, etc., of a planet to an unowned state.
    virtual void    Depopulate();

    void            Conquer(int conquerer);             ///< Called during combat when a planet changes hands
    bool            Colonize(int empire_id, const std::string& species_name, double population); ///< Called during colonization handling to do the actual colonizing
    void            SetIsAboutToBeColonized(bool b);    ///< Called during colonization when a planet is about to be colonized
    void            ResetIsAboutToBeColonized();        ///< Called after colonization, to reset the number of prospective colonizers to 0
    void            SetIsAboutToBeInvaded(bool b);      ///< Marks planet as being invaded or not, depending on whether \a b is true or false
    void            ResetIsAboutToBeInvaded();          ///< Marks planet as not being invaded
    void            SetIsAboutToBeBombarded(bool b);    ///< Marks planet as being bombarded or not, depending on whether \a b is true or false
    void            ResetIsAboutToBeBombarded();        ///< Marks planet as not being bombarded
    void            SetGiveToEmpire(int empire_id);     ///< Marks planet to be given to empire
    void            ClearGiveToEmpire();                ///< Marks planet not to be given to any empire

    void            SetLastTurnAttackedByShip(int turn);///< Sets the last turn this planet was attacked by a ship

    void            SetSurfaceTexture(const std::string& texture);

    virtual void    ResetTargetMaxUnpairedMeters();
    //@}

    static int      TypeDifference(PlanetType type1, PlanetType type2);

protected:
    friend class Universe;
    friend class ObjectMap;

    /** \name Structors */ //@{
    Planet();                                   ///< default ctor
    Planet(PlanetType type, PlanetSize size);   ///< general ctor taking just the planet's type and size

    template <class T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);
    template <class T> friend void boost::checked_delete(T* x);

#if BOOST_VERSION >= 106100
public:
#endif
    ~Planet() {}
#if BOOST_VERSION >= 106100
protected:
#endif

    virtual Planet*         Clone(int empire_id = ALL_EMPIRES) const;  ///< returns new copy of this Planet
    //@}

private:
    void Init();

    virtual const Meter*    GetMeter(MeterType type) const;

    virtual void            PopGrowthProductionResearchPhase();
    virtual void            ClampMeters();

    virtual Visibility      GetVisibility(int empire_id) const  { return UniverseObject::GetVisibility(empire_id); }
    virtual void            AddMeter(MeterType meter_type)      { UniverseObject::AddMeter(meter_type); }

    PlanetType      m_type;
    PlanetType      m_original_type;
    PlanetSize      m_size;
    float           m_orbital_period;
    float           m_initial_orbital_position;
    float           m_rotational_period;
    float           m_axial_tilt;

    std::set<int>   m_buildings;

    bool            m_just_conquered;
    bool            m_is_about_to_be_colonized;
    bool            m_is_about_to_be_invaded;
    bool            m_is_about_to_be_bombarded;
    int             m_ordered_given_to_empire_id;
    int             m_last_turn_attacked_by_ship;

    std::string     m_surface_texture;  // intentionally not serialized; set by local effects

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif // _Planet_h_
