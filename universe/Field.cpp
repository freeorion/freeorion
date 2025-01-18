#include "Field.h"

#include "Enums.h"
#include "FieldType.h"
#include "Meter.h"
#include "Universe.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"


/////////////////////////////////////////////////
// Field                                       //
/////////////////////////////////////////////////
Field::Field(std::string field_type, double x, double y, double radius, int creation_turn) :
    UniverseObject{UniverseObjectType::OBJ_FIELD, "", x, y, ALL_EMPIRES, creation_turn},
    m_type_name(std::move(field_type))
{
    if (const FieldType* type = GetFieldType(m_type_name))
        Rename(UserString(type->Name()));
    else
        Rename(UserString("ENC_FIELD"));

    AddMeters(std::array{MeterType::METER_SIZE, MeterType::METER_SPEED});

    UniverseObject::GetMeter(MeterType::METER_SIZE)->Set(radius, radius);
}

std::shared_ptr<UniverseObject> Field::Clone(const Universe& universe, int empire_id) const {
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    auto retval = std::make_shared<Field>();
    retval->Copy(*this, universe, empire_id);
    return retval;
}

void Field::Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id) {
    if (&copied_object == this)
        return;
    if (copied_object.ObjectType() != UniverseObjectType::OBJ_FIELD) {
        ErrorLogger() << "Field::Copy passed an object that wasn't a Field";
        return;
    }

    Copy(static_cast<const Field&>(copied_object), universe, empire_id);
}

void Field::Copy(const Field& copied_field, const Universe& universe, int empire_id) {
    if (&copied_field == this)
        return;

    int copied_object_id = copied_field.ID();
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = universe.GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_field, vis, visible_specials, universe);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_name =      copied_field.m_name;
        this->m_type_name = copied_field.m_type_name;
    }
}

UniverseObject::TagVecs Field::Tags() const {
    const FieldType* type = GetFieldType(m_type_name);
    return type ? TagVecs{type->Tags()} : TagVecs{};
}

bool Field::HasTag(std::string_view name) const {
    const FieldType* type = GetFieldType(m_type_name);
    return type && type->HasTag(name);
}

std::string Field::Dump(uint8_t ntabs) const {
    auto retval = UniverseObject::Dump(ntabs);
    retval.append(" field type: ").append(m_type_name);
    return retval;
}

const std::string& Field::PublicName(int empire_id, const Universe&) const {
    // always just return name since fields (as of this writing) don't have owners
    return UserString(m_type_name);
}

bool Field::ContainedBy(int object_id) const noexcept
{ return object_id != INVALID_OBJECT_ID && object_id == this->SystemID(); }

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

std::size_t Field::SizeInMemory() const {
    std::size_t retval = UniverseObject::SizeInMemory();
    retval += sizeof(Field) - sizeof(UniverseObject);
    retval += sizeof(decltype(m_type_name)::value_type)*m_type_name.capacity();
    return retval;
}

void Field::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    static_assert(noexcept(GetMeter(MeterType::METER_SPEED)->ResetCurrent()));
    GetMeter(MeterType::METER_SPEED)->ResetCurrent();
    // intentionally not resetting size, so that it is presistant
}

void Field::ClampMeters() {
    UniverseObject::ClampMeters();

    // intentionally not clamping MeterType::METER_SPEED, to allow negative speeds
    UniverseObject::GetMeter(MeterType::METER_SIZE)->ClampCurrentToRange();
}
