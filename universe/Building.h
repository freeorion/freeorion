#ifndef _Building_h_
#define _Building_h_


#include <boost/serialization/access.hpp>
#include "UniverseObject.h"
#include "../util/Export.h"


/** A Building UniverseObject type. */
class FO_COMMON_API Building : public UniverseObject {
public:
    [[nodiscard]] bool                  HostileToEmpire(int empire_id, const EmpireManager& empires) const override;
    [[nodiscard]] std::set<std::string> Tags() const override;
    [[nodiscard]] bool                  HasTag(const std::string& name) const override;
    [[nodiscard]] UniverseObjectType    ObjectType() const override;
    [[nodiscard]] std::string           Dump(unsigned short ntabs = 0) const override;
    [[nodiscard]] int                   ContainerObjectID() const override { return m_planet_id; }
    [[nodiscard]] bool                  ContainedBy(int object_id) const override;

    std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const override;

    /** Returns the name of the BuildingType object for this building. */
    [[nodiscard]] const std::string&    BuildingTypeName() const    { return m_building_type; };
    [[nodiscard]] int                   PlanetID() const            { return m_planet_id; }             ///< returns the ID number of the planet this building is on
    [[nodiscard]] int                   ProducedByEmpireID() const  { return m_produced_by_empire_id; } ///< returns the empire ID of the empire that produced this building
    [[nodiscard]] bool                  OrderedScrapped() const     { return m_ordered_scrapped; }

    void Copy(std::shared_ptr<const UniverseObject> copied_object, Universe& universe,
              int empire_id = ALL_EMPIRES) override;
    void SetPlanetID(int planet_id);         ///< sets the planet on which the building is located
    void Reset();                            ///< resets any building state, and removes owners
    void SetOrderedScrapped(bool b = true);  ///< flags building for scrapping
    void ResetTargetMaxUnpairedMeters() override;

    Building(int empire_id, const std::string& building_type, int produced_by_empire_id = ALL_EMPIRES);
    Building() = default;

private:
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    /** Returns new copy of this Building. */
    [[nodiscard]] Building* Clone(Universe& universe, int empire_id = ALL_EMPIRES) const override;

    std::string m_building_type;
    int         m_planet_id = INVALID_OBJECT_ID;
    bool        m_ordered_scrapped = false;
    int         m_produced_by_empire_id = ALL_EMPIRES;

    template <typename Archive>
    friend void serialize(Archive&, Building&, unsigned int const);
};


#endif
