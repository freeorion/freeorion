#include "Field.h"

#include "Enums.h"
#include "FieldType.h"
#include "Meter.h"
#include "UniverseObjectVisitor.h"
#include "Universe.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"


/////////////////////////////////////////////////
// Field                                       //
/////////////////////////////////////////////////
Field::~Field()
{}

Field::Field(const std::string& field_type, double x, double y, double radius) :
    UniverseObject("", x, y),
    m_type_name(field_type)
{
    if (const FieldType* type = GetFieldType(m_type_name))
        Rename(UserString(type->Name()));
    else
        Rename(UserString("ENC_FIELD"));

    UniverseObject::Init();

    AddMeter(MeterType::METER_SPEED);
    AddMeter(MeterType::METER_SIZE);

    UniverseObject::GetMeter(MeterType::METER_SIZE)->Set(radius, radius);
}

Field* Field::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    Field* retval = new Field(m_type_name, X(), Y(), GetMeter(MeterType::METER_SIZE)->Current());
    retval->Copy(shared_from_this(), empire_id);
    return retval;
}

void Field::Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object.get() == this)
        return;
    std::shared_ptr<const Field> copied_field = std::dynamic_pointer_cast<const Field>(copied_object);
    if (!copied_field) {
        ErrorLogger() << "Field::Copy passed an object that wasn't a Field";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(std::move(copied_object), vis, visible_specials);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_name =      copied_field->m_name;
        this->m_type_name = copied_field->m_type_name;
    }
}

std::set<std::string> Field::Tags() const {
    const FieldType* type = GetFieldType(m_type_name);
    if (!type)
        return {};
    return type->Tags();
}

bool Field::HasTag(const std::string& name) const {
    const FieldType* type = GetFieldType(m_type_name);

    return type && type->Tags().count(name);
}

UniverseObjectType Field::ObjectType() const
{ return UniverseObjectType::OBJ_FIELD; }

std::string Field::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " field type: " << m_type_name;
    return os.str();
}

const std::string& Field::PublicName(int empire_id, const ObjectMap&) const {
    // always just return name since fields (as of this writing) don't have owners
    return UserString(m_type_name);
}

std::shared_ptr<UniverseObject> Field::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Field>(std::static_pointer_cast<const Field>(shared_from_this()))); }

int Field::ContainerObjectID() const
{ return this->SystemID(); }

bool Field::ContainedBy(int object_id) const {
    return object_id != INVALID_OBJECT_ID
        && object_id == this->SystemID();
}

bool Field::InField(std::shared_ptr<const UniverseObject> obj) const
{ return obj && InField(obj->X(), obj->Y()); }

bool Field::InField(double x, double y) const {
    const Meter* size_meter = GetMeter(MeterType::METER_SIZE);
    double radius = 1.0;
    if (size_meter)
        radius = size_meter->Current();

    double dist2 = (x - this->X())*(x - this->X()) + (y - this->Y())*(y - this->Y());
    return dist2 < radius*radius;
}

void Field::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    GetMeter(MeterType::METER_SPEED)->ResetCurrent();
    // intentionally not resetting size, so that it is presistant
}

void Field::ClampMeters() {
    UniverseObject::ClampMeters();

    // intentionally not clamping MeterType::METER_SPEED, to allow negative speeds
    UniverseObject::GetMeter(MeterType::METER_SIZE)->ClampCurrentToRange();
}
