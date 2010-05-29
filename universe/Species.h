// -*- C++ -*-
#ifndef _Species_h_
#define _Species_h_

#include <boost/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <vector>
#include <map>

namespace Condition {
    struct ConditionBase;
}
namespace Effect {
    class EffectsGroup;
    class EffectBase;
}

/** A setting that a ResourceCenter can be assigned to influence what it
  * produces.  Doesn't directly affect the ResourceCenter, but effectsgroups
  * can use activation or scope conditions that check whether a potential
  * target has a particular focus.  By this method, techs or buildings or
  * species can act on planets or other ResourceCenters depending what their
  * focus setting is. */
class FocusType
{
public:
    /** \name Structors */ //@{
    /** default ctor */
    FocusType();
    /** basic ctor */
    FocusType(const std::string& name, const std::string& description, const Condition::ConditionBase* location, const std::string& graphic);
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const;       ///< returns the name for this focus type
    const std::string&              Description() const;///< returns a text description of this focus type
    const Condition::ConditionBase* Location() const;   ///< returns the condition that determines whether an UniverseObject can use this FocusType
    const std::string&              Graphic() const;    ///< returns the name of the grapic file for this focus type
    //@}

private:
    std::string                                         m_name;
    std::string                                         m_description;
    boost::shared_ptr<const Condition::ConditionBase>   m_location;
    std::string                                         m_graphic;
};

/** A predefined type of population that can exist on a PopulationCenter.
  * Species have associated sets of EffectsGroups, and various other 
  * properties that affect how the object on which they reside functions.
  * Each kind of Species must have a \a unique name string, by which it can be
  * looked up using GetSpecies(). */
class Species
{
public:
    /** \name Structors */ //@{
    /** basic ctor */
    Species(const std::string& name, const std::string& description,
            const std::vector<FocusType>& foci,
            const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
            const std::string& graphic);
    //@}

    /** \name Accessors */ //@{
    const std::string&  Name() const;       ///< returns the unique name for this type of species
    const std::string&  Description() const;///< returns a text description of this type of species
    std::string         Dump() const;       ///< returns a data file format representation of this object
    const std::map<std::string, FocusType>&
                        Foci() const;       ///< returns the focus types this species can use, indexed by name
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&
                        Effects() const;    ///< returns the EffectsGroups that encapsulate the effects that species of this type have
    const std::string&  Graphic() const;    ///< returns the name of the grapic file for this species
    //@}

private:
    std::string                         m_name;
    std::string                         m_description;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                        m_effects;
    std::map<std::string, FocusType>    m_foci;
    std::string                         m_graphic;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** Holds all FreeOrion species.  Types may be looked up by name. */
class SpeciesManager
{
public:
    typedef std::map<std::string, Species*>::const_iterator iterator;

    /** \name Accessors */ //@{
    /** returns the building type with the name \a name; you should use the
      * free function GetSpecies() instead, mainly to save some typing. */
    const Species*          GetSpecies(const std::string& name) const;

    /** iterator to the first building type */
    iterator                begin() const;

    /** iterator to the last + 1th building type */
    iterator                end() const;

    /** returns the instance of this singleton class; you should use the free
      * function GetSpeciesManager() instead */
    static SpeciesManager&  GetSpeciesManager();
    //@}

private:
    SpeciesManager();
    ~SpeciesManager();

    std::map<std::string, Species*> m_species;

    static SpeciesManager* s_instance;
};

/** returns the singleton species manager */
SpeciesManager& GetSpeciesManager();

/** Returns the Species object used to represent species of type \a name.
  * If no such Species exists, 0 is returned instead. */
const Species* GetSpecies(const std::string& name);

// template implementations
template <class Archive>
void Species::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_graphic);
}

#endif // _Species_h_
