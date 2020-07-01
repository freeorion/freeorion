#ifndef _Field_h_
#define _Field_h_


#include "UniverseObject.h"
#include "../util/Export.h"
#include "../util/Pending.h"


namespace Effect {
    class EffectsGroup;
}

/** a class representing a region of space */
class FO_COMMON_API Field : public UniverseObject {
public:
    /** \name Accessors */ //@{
    std::set<std::string>   Tags() const override;
    bool                    HasTag(const std::string& name) const override;

    UniverseObjectType  ObjectType() const override;

    std::string         Dump(unsigned short ntabs = 0) const override;

    int                 ContainerObjectID() const override;
    bool                ContainedBy(int object_id) const override;

    const std::string&  PublicName(int empire_id) const override;
    const std::string&  FieldTypeName() const { return m_type_name; }

    /* Field is (presently) the only distributed UniverseObject that isn't just
     * location at a single point in space. These functions check if locations
     * or objecs are within this field's area. */
    bool                InField(std::shared_ptr<const UniverseObject> obj) const;
    bool                InField(double x, double y) const;

    std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const override;
    //@}

    /** \name Mutators */ //@{
    void Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES) override;

    void ResetTargetMaxUnpairedMeters() override;
    void ClampMeters() override;
    //@}

protected:
    friend class Universe;
    Field();

public:
    Field(const std::string& field_type, double x, double y, double radius);
    ~Field();

protected:
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    /** Returns new copy of this Field. */
    Field* Clone(int empire_id = ALL_EMPIRES) const override;

private:
    std::string m_type_name;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif // _Field_h_
