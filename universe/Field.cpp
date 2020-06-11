#include "Field.h"

#include "Enums.h"
#include "FieldType.h"
#include "UniverseObjectVisitor.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"


Field::Field() = default;

Field::~Field() = default;

Field::Field(std::string const& field_type, double x, double y, double radius) :
    UniverseObject("", x, y),
    m_type_name(field_type)
{
    if (const FieldType* type = GetFieldType(m_type_name))
        Rename(UserString(type->Name()));
    else
        Rename(UserString("ENC_FIELD"));

    UniverseObject::Init();

    AddMeter(METER_SPEED);
    AddMeter(METER_SIZE);

    UniverseObject::GetMeter(METER_SIZE)->Set(radius, radius);
}

auto Field::Clone(int empire_id) const -> Field*
{
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return nullptr;

    Field* retval = new Field();
    retval->Copy(shared_from_this(), empire_id);
    return retval;
}

void Field::Copy(std::shared_ptr<UniverseObject const> copied_object, int empire_id)
{
    if (copied_object.get() == this)
        return;
    std::shared_ptr<const Field> copied_field = std::dynamic_pointer_cast<const Field>(copied_object);
    if (!copied_field) {
        ErrorLogger() << "Field::Copy passed an object that wasn't a Field";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    std::set<std::string> visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_name =                      copied_field->m_name;
        this->m_type_name =                 copied_field->m_type_name;
    }
}

auto Field::Tags() const -> std::set<std::string>
{
    const FieldType* type = GetFieldType(m_type_name);
    if (!type)
        return {};
    return type->Tags();
}

auto Field::HasTag(std::string const& name) const -> bool
{
    const FieldType* type = GetFieldType(m_type_name);

    return type && type->Tags().count(name);
}

auto Field::ObjectType() const -> UniverseObjectType
{ return OBJ_FIELD; }

auto Field::Dump(unsigned short ntabs) const -> std::string
{
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " field type: " << m_type_name;
    return os.str();
}

auto Field::PublicName(int empire_id) const -> std::string const&
{
    // always just return name since fields (as of this writing) don't have owners
    return UserString(m_type_name);
}

auto Field::Accept(UniverseObjectVisitor const& visitor) const -> std::shared_ptr<UniverseObject>
{ return visitor.Visit(std::const_pointer_cast<Field>(std::static_pointer_cast<const Field>(shared_from_this()))); }

auto Field::ContainerObjectID() const -> int
{ return this->SystemID(); }

auto Field::ContainedBy(int object_id) const -> bool
{
    return object_id != INVALID_OBJECT_ID
        && object_id == this->SystemID();
}

auto Field::InField(std::shared_ptr<UniverseObject const> obj) const -> bool
{ return obj && InField(obj->X(), obj->Y()); }

auto Field::InField(double x, double y) const -> bool
{
    const Meter* size_meter = GetMeter(METER_SIZE);
    double radius = 1.0;
    if (size_meter)
        radius = size_meter->Current();

    double dist2 = (x - this->X())*(x - this->X()) + (y - this->Y())*(y - this->Y());
    return dist2 < radius*radius;
}

void Field::ResetTargetMaxUnpairedMeters()
{
    UniverseObject::ResetTargetMaxUnpairedMeters();

    GetMeter(METER_SPEED)->ResetCurrent();
    // intentionally not resetting size, so that it is presistant
}

void Field::ClampMeters()
{
    UniverseObject::ClampMeters();

    // intentionally not clamping METER_SPEED, to allow negative speeds
    UniverseObject::GetMeter(METER_SIZE)->ClampCurrentToRange();
}
