#ifndef _Building_h_
#define _Building_h_


#include "ObjectMap.h"
#include "UniverseObject.h"
#include "../util/Export.h"


//! A Building UniverseObject type.
class FO_COMMON_API Building : public UniverseObject {
public:
    Building(int empire_id, std::string const& building_type, int produced_by_empire_id = ALL_EMPIRES);

    ~Building();

    auto HostileToEmpire(int empire_id) const -> bool override;

    auto Tags() const -> std::set<std::string> override;

    auto HasTag(std::string const& name) const -> bool override;

    auto ObjectType() const -> UniverseObjectType override;

    auto Dump(unsigned short ntabs = 0) const -> std::string override;

    auto ContainerObjectID() const -> int override
    { return m_planet_id; }

    auto ContainedBy(int object_id) const -> bool override;

    auto Accept(UniverseObjectVisitor const& visitor) const -> std::shared_ptr<UniverseObject> override;

    auto BuildingTypeName() const -> std::string const&
    { return m_building_type; };

    auto PlanetID() const -> int
    { return m_planet_id; }

    void SetPlanetID(int planet_id);

    auto ProducedByEmpireID() const -> int
    { return m_produced_by_empire_id; }

    auto OrderedScrapped() const -> bool
    { return m_ordered_scrapped; }

    void SetOrderedScrapped(bool b = true);

    void Copy(std::shared_ptr<UniverseObject const> copied_object, int empire_id = ALL_EMPIRES) override;

    //! Resets any building state, and removes owners
    void Reset();

    void ResetTargetMaxUnpairedMeters() override;

protected:
    friend class Universe;
    Building();

    template <typename T>
    friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    auto Clone(int empire_id = ALL_EMPIRES) const -> Building* override;

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
