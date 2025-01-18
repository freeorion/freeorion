#ifndef _Building_h_
#define _Building_h_


#include <boost/serialization/access.hpp>
#include "UniverseObject.h"
#include "../util/Export.h"


/** A Building UniverseObject type. */
class FO_COMMON_API Building final : public UniverseObject {
public:
    [[nodiscard]] TagVecs     Tags() const;
    [[nodiscard]] TagVecs     Tags(const ScriptingContext&) const override { return Tags(); }
    [[nodiscard]] bool        HasTag(std::string_view name) const;
    [[nodiscard]] bool        HasTag(std::string_view name, const ScriptingContext&) const override { return HasTag(name); }

    [[nodiscard]] bool        HostileToEmpire(int empire_id, const EmpireManager& empires) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] int         ContainerObjectID() const noexcept override { return m_planet_id; }
    [[nodiscard]] bool        ContainedBy(int object_id) const noexcept override;

    [[nodiscard]] const auto& BuildingTypeName() const noexcept   { return m_building_type; };
    [[nodiscard]] int         PlanetID() const noexcept           { return m_planet_id; }             ///< returns the ID number of the planet this building is on
    [[nodiscard]] int         ProducedByEmpireID() const noexcept { return m_produced_by_empire_id; } ///< returns the empire ID of the empire that produced this building
    [[nodiscard]] bool        OrderedScrapped() const noexcept    { return m_ordered_scrapped; }

    [[nodiscard]] std::size_t SizeInMemory() const override;

    void Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id = ALL_EMPIRES) override;
    void Copy(const Building& copied_building, const Universe& universe, int empire_id);

    void SetPlanetID(int planet_id);         ///< sets the planet on which the building is located
    void Reset();                            ///< resets any building state, and removes owners
    void SetOrderedScrapped(bool b = true);  ///< flags building for scrapping
    void ResetTargetMaxUnpairedMeters() override;

    Building(int empire_id, std::string building_type, int produced_by_empire_id, int creation_turn);
    Building() : UniverseObject(UniverseObjectType::OBJ_BUILDING) {}

private:
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    /** Returns new copy of this Building. */
    [[nodiscard]] std::shared_ptr<UniverseObject> Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const override;

    std::string m_building_type;
    int         m_planet_id = INVALID_OBJECT_ID;
    bool        m_ordered_scrapped = false;
    int         m_produced_by_empire_id = ALL_EMPIRES;

    template <typename Archive>
    friend void serialize(Archive&, Building&, unsigned int const);
};


#endif
