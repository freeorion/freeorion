#ifndef _Field_h_
#define _Field_h_


#include "UniverseObject.h"
#include "../util/Export.h"
#include "../util/Pending.h"


namespace Effect {
    class EffectsGroup;
}

/** a class representing a region of space */
class FO_COMMON_API Field final : public UniverseObject {
public:
    [[nodiscard]] TagVecs               Tags(const ScriptingContext&) const override;
    [[nodiscard]] bool                  HasTag(std::string_view name, const ScriptingContext&) const override;

    [[nodiscard]] std::string           Dump(uint8_t ntabs = 0) const override;

    [[nodiscard]] int                   ContainerObjectID() const noexcept override { return this->SystemID(); }
    [[nodiscard]] bool                  ContainedBy(int object_id) const override;

    [[nodiscard]] const std::string&    PublicName(int empire_id, const Universe&) const override;
    [[nodiscard]] const std::string&    FieldTypeName() const noexcept { return m_type_name; }

    /* Field is (presently) the only distributed UniverseObject that isn't just
     * location at a single point in space. These functions check if locations
     * or objecs are within this field's area. */
    [[nodiscard]] bool                  InField(std::shared_ptr<const UniverseObject> obj) const;
    [[nodiscard]] bool                  InField(double x, double y) const;

    std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const override;

    void Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id = ALL_EMPIRES) override;
    void Copy(const Field& copied_field, const Universe& universe, int empire_id = ALL_EMPIRES);

    void ResetTargetMaxUnpairedMeters() override;
    void ClampMeters() override;

    Field(std::string field_type, double x, double y, double radius, int creation_turn);
    Field() : UniverseObject(UniverseObjectType::OBJ_FIELD) {}

private:
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    /** Returns new copy of this Field. */
    [[nodiscard]] std::shared_ptr<UniverseObject> Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const override;

    std::string m_type_name;

    template <typename Archive>
    friend void serialize(Archive&, Field&, unsigned int const);
};


#endif
