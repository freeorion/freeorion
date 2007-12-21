// -*- C++ -*-
#ifndef _ShipDesign_h_
#define _ShipDesign_h_

#include "Effect.h"

/** A type of ship part */
class PartType {
public:
    /** \name Structors */ //@{
    PartType();
    PartType(std::string name, std::string description, ShipPartClass part_class, std::string upgrade, 
             double mass, double power, double range, /* TODO: add effects group parameter */ 
             std::string graphic);
    //@}

    /** \name Accessors */ //@{
    std::string         Name() const;           ///< returns name of part
    std::string         Description() const;    ///< returns stringtable entry name of description

    std::string         Upgrade() const;        ///< returns name of part that this part can be upgraded to.  may return an empty string if there is no such upgrade

    ShipPartClass       Class() const;          ///< returns that class of part that this is.

    double              Mass() const;           ///< returns mass of part
    double              Power() const;          ///< returns how good the part is at its function.  might be weapon or shield strength, or cargo hold capacity
    double              Range() const;          ///< returns the range of a part.  may not have meaning for all part types.

    std::string         Graphic() const;        ///< returns graphic that represents part in UI
    std::string         Animation() const;      ///< returns name of animation that is played when part activates in battle

    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&
                        Effects() const;        ///< returns the EffectsGroups that encapsulate the effects this part has
    //@}

private:
    std::string         m_name;
    std::string         m_description;
    std::string         m_upgrade;

    ShipPartClass       m_class;

    double              m_mass;
    double              m_power;
    double              m_range;

    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                        m_effects;
    
    std::string         m_graphic;
    std::string         m_battle_animation;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Holds FreeOrion ship part types */
class PartTypeManager {
public:
    typedef std::map<std::string, PartType*>::const_iterator iterator;

    /** \name Accessors */ //@{
    /** returns the part type with the name \a name; you should use the free function GetPartType() instead */
    const PartType* GetPartType(const std::string& name) const;

    /** iterator to the first part type */
    iterator begin() const;

    /** iterator to the last + 1th part type */
    iterator end() const;

    /** returns the instance of this singleton class; you should use the free function GetPartTypeManager() instead */
    static const PartTypeManager& GetPartTypeManager();
    //@}

private:
    PartTypeManager();
    std::string PartTypeManager::FindFirstDependencyCycle();
    std::map<std::string, PartType*> m_parts;
    static PartTypeManager* s_instance;
};


/** returns the singleton part type manager */
const PartTypeManager& GetPartTypeManager();

/** Returns the ship PartType specification object with name \a name.  If no such PartType exists,
    0 is returned instead. */
const PartType* GetPartType(const std::string& name);


/** Specification for the hull, or base, on which ship designs are created by adding parts.  The hull 
    determines some final design characteristics directly, and also determine how many parts can be
    added to the design. */
class HullType {
public:
    /** \name Structors */ //@{
    HullType();
    HullType(std::string name, std::string description, double mass, double speed, int num_slots,
             std::string graphic);
    //@}

    /** \name Accessors */ //@{
    std::string Name() const;           ///< returns name of hull
    std::string Description() const;    ///< returns stringtable entry name of description

    double      Mass() const;           ///< returns mass of hull
    double      Speed() const;          ///< returns speed (?) of hull

    int         NumberSlots();          ///< returns number of part slots in hull

    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&
                Effects() const;        ///< returns the EffectsGroups that encapsulate the effects this part hull has
    //@}

private:
    std::string         m_name;
    std::string         m_description;
    double              m_mass;
    double              m_speed;

    int                 m_number_slots;

    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                        m_effects;
    std::string         m_graphic;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Holds FreeOrion hull types */
class HullTypeManager {
public:
    typedef std::map<std::string, HullType*>::const_iterator iterator;

    /** \name Accessors */ //@{
    /** returns the hull type with the name \a name; you should use the free function GetHullType() instead */
    const HullType* GetHullType(const std::string& name) const;

    /** iterator to the first hull type */
    iterator begin() const;

    /** iterator to the last + 1th hull type */
    iterator end() const;

    /** returns the instance of this singleton class; you should use the free function GetHullTypeManager() instead */
    static const HullTypeManager& GetHullTypeManager();
    //@}

private:
    HullTypeManager();
    std::map<std::string, HullType*> m_hulls;
    static HullTypeManager* s_instance;
};

/** returns the singleton hull type manager */
const HullTypeManager& GetHullTypeManager();

/** Returns the ship HullType specification object with name \a name.  If no such HullType exists,
    0 is returned instead. */
const HullType* GetHullType(const std::string& name);


class ShipDesign {
public:
    /** \name Structors */ //@{
    ShipDesign(); ///< default ctor
    ShipDesign(int id, std::string name, int designed_by_empire_id, int designed_on_turn, 
               std::string hull, std::vector<std::string> parts, std::string graphic,
               std::string model);
    //@}

    /** \name Accessors */ //@{
    int                             ID() const;                 ///< returns id number of design
    std::string                     Name() const;               ///< returns name of design
    std::string                     Description() const;        ///< returns description of design
    int                             DesignedByEmpire() const;   ///< returns id of empire that created design
    int                             DesginedOnTurn() const;     ///< returns turn on which design was created

    double                          StarlaneSpeed() const;      ///< returns design speed along starlanes
    double                          BattleSpeed() const;        ///< returns design speed on the battle map
    double                          Mass() const;               ///< returns design mass

    /////// TEMPORARY ///////
    double      Defense() const;
    double      Speed() const;
    double      Attack() const;
    bool        Colonize() const;
    double      Cost() const;
    /////// TEMPORARY ///////

    std::string                     Hull() const;               ///< returns name of hull on which design is based
    const std::vector<std::string>  Parts() const;              ///< returns vector of names of parts in design

    std::string                     Graphic() const;            ///< returns filename of graphic for design
    std::string                     Model() const;              ///< returns filename of 3D model that represents ships of design
    //@}

    /** \name Mutators */ //@{
    void SetID(int id);                   ///< sets the ID number of the design to \a id
    void Rename(const std::string& name); ///< renames this design to \a name
    //@}

private:
    int                         m_id;

    std::string                 m_name;

    int                         m_designed_by_empire_id;
    int                         m_designed_on_turn;

    std::string                 m_hull;
    std::vector<std::string>    m_parts;

    std::string                 m_graphic;
    std::string                 m_3D_model;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns the ShipDesign specification object with id \a ship_design_id.  If no such ShipDesign
    is present in the Universe (because it doesn't exist, or isn't know to this client), 0 is
    returned instead. */
const ShipDesign* GetShipDesign(int ship_design_id);


// template implementations
template <class Archive>
void PartType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name);
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_upgrade)
        & BOOST_SERIALIZATION_NVP(m_class)
        & BOOST_SERIALIZATION_NVP(m_mass)
        & BOOST_SERIALIZATION_NVP(m_power)
        & BOOST_SERIALIZATION_NVP(m_range)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_graphic)
        & BOOST_SERIALIZATION_NVP(m_battle_animation)
}

template <class Archive>
void HullType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_mass)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_number_slots)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_graphic)
}

template <class Archive>
void ShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_designed_by_empire_id)
        & BOOST_SERIALIZATION_NVP(m_designed_on_turn)
        & BOOST_SERIALIZATION_NVP(m_hull)
        & BOOST_SERIALIZATION_NVP(m_parts)
        & BOOST_SERIALIZATION_NVP(m_graphic)
        & BOOST_SERIALIZATION_NVP(m_3D_model);
}

#endif // _ShipDesign_h_