#ifndef _Field_h_
#define _Field_h_


#include "UniverseObject.h"
#include "../util/Export.h"
#include "../util/Pending.h"


//! A class representing a region of space
class FO_COMMON_API Field : public UniverseObject {
public:
    Field(std::string const& field_type, double x, double y, double radius);

    ~Field();

    auto Tags() const -> std::set<std::string> override;

    auto HasTag(std::string const& name) const -> bool override;

    auto ObjectType() const -> UniverseObjectType override;

    auto Dump(unsigned short ntabs = 0) const -> std::string override;

    auto ContainerObjectID() const -> int override;

    auto ContainedBy(int object_id) const -> bool override;

    auto PublicName(int empire_id) const -> std::string const& override;

    auto FieldTypeName() const -> std::string const&
    { return m_type_name; }

    //! Field is (presently) the only distributed UniverseObject that isn't
    //! just location at a single point in space. These functions check if
    //! locations or objecs are within this field's area.
    auto InField(std::shared_ptr<UniverseObject const> obj) const -> bool;

    auto InField(double x, double y) const -> bool;

    auto Accept(UniverseObjectVisitor const& visitor) const -> std::shared_ptr<UniverseObject> override;

    void Copy(std::shared_ptr<UniverseObject const> copied_object, int empire_id = ALL_EMPIRES) override;

    void ResetTargetMaxUnpairedMeters() override;

    void ClampMeters() override;

protected:
    friend class Universe;
    Field();

    template <typename T>
    friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    auto Clone(int empire_id = ALL_EMPIRES) const -> Field* override;

private:
    std::string m_type_name;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const version);
};


#endif // _Field_h_
