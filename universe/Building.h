#ifndef _Building_h_
#define _Building_h_


#include "ObjectMap.h"
#include "UniverseObject.h"
#include "../util/Export.h"


/** A Building UniverseObject type. */
class FO_COMMON_API Building : public UniverseObject {
public:
    bool                    HostileToEmpire(int empire_id) const override;
    std::set<std::string>   Tags() const override;
    bool                    HasTag(const std::string& name) const override;
    UniverseObjectType      ObjectType() const override;
    std::string             Dump(unsigned short ntabs = 0) const override;
    int                     ContainerObjectID() const override { return m_planet_id; }
    bool                    ContainedBy(int object_id) const override;

    std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const override;

    /** Returns the name of the BuildingType object for this building. */
    const std::string&      BuildingTypeName() const    { return m_building_type; };
    int                     PlanetID() const            { return m_planet_id; }             ///< returns the ID number of the planet this building is on
    int                     ProducedByEmpireID() const  { return m_produced_by_empire_id; } ///< returns the empire ID of the empire that produced this building
    bool                    OrderedScrapped() const     { return m_ordered_scrapped; }

    void Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES) override;
    void SetPlanetID(int planet_id);         ///< sets the planet on which the building is located
    void Reset();                            ///< resets any building state, and removes owners
    void SetOrderedScrapped(bool b = true);  ///< flags building for scrapping
    void ResetTargetMaxUnpairedMeters() override;

protected:
    friend class Universe;
    Building() {}

public:
    Building(int empire_id, const std::string& building_type,
             int produced_by_empire_id = ALL_EMPIRES);

protected:
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

public:
    ~Building() {}

protected:
    /** Returns new copy of this Building. */
    Building* Clone(int empire_id = ALL_EMPIRES) const override;

private:
    std::string m_building_type;
    int         m_planet_id = INVALID_OBJECT_ID;
    bool        m_ordered_scrapped = false;
    int         m_produced_by_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif // _Building_h_
