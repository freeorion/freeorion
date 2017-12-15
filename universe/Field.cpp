#include "Field.h"

#include "Condition.h"
#include "Effect.h"
#include "Enums.h"
#include "Meter.h"
#include "Predicates.h"
#include "Universe.h"
#include "ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/fstream.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

namespace {
    std::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, double increase) {
        typedef std::vector<std::unique_ptr<Effect::EffectBase>> Effects;
        auto scope = boost::make_unique<Condition::Source>();
        std::unique_ptr<Condition::Source> activation = nullptr;
        auto vr =
            boost::make_unique<ValueRef::Operation<double>>(
                ValueRef::PLUS,
                boost::make_unique<ValueRef::Variable<double>>(
                    ValueRef::EFFECT_TARGET_VALUE_REFERENCE, std::vector<std::string>()),
                boost::make_unique<ValueRef::Constant<double>>(increase)
            );
        auto effects = Effects();
        effects.push_back(boost::make_unique<Effect::SetMeter>(meter_type, std::move(vr)));
        return std::make_shared<Effect::EffectsGroup>(std::move(scope), std::move(activation), std::move(effects));
    }
}

/////////////////////////////////////////////////
// Field                                       //
/////////////////////////////////////////////////
Field::Field()
{}

Field::~Field()
{}

Field::Field(const std::string& field_type, double x, double y, double radius) :
    UniverseObject("", x, y),
    m_type_name(field_type)
{
    const FieldType* type = GetFieldType(m_type_name);
    if (type)
        Rename(UserString(type->Name()));
    else
        Rename(UserString("ENC_FIELD"));

    UniverseObject::Init();

    AddMeter(METER_SPEED);
    AddMeter(METER_SIZE);

    UniverseObject::GetMeter(METER_SIZE)->Set(radius, radius);
}

Field* Field::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return nullptr;

    Field* retval = new Field();
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
    std::set<std::string> visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_name =                      copied_field->m_name;
        this->m_type_name =                 copied_field->m_type_name;
    }
}

std::set<std::string> Field::Tags() const {
    const FieldType* type = GetFieldType(m_type_name);
    if (!type)
        return std::set<std::string>();
    return type->Tags();
}

bool Field::HasTag(const std::string& name) const {
    const FieldType* type = GetFieldType(m_type_name);

    return type && type->Tags().count(name);
}

UniverseObjectType Field::ObjectType() const
{ return OBJ_FIELD; }

std::string Field::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " field type: " << m_type_name;
    return os.str();
}

const std::string& Field::PublicName(int empire_id) const {
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
    const Meter* size_meter = GetMeter(METER_SIZE);
    double radius = 1.0;
    if (size_meter)
        radius = size_meter->Current();

    double dist2 = (x - this->X())*(x - this->X()) + (y - this->Y())*(y - this->Y());
    return dist2 < radius*radius;
}

void Field::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    GetMeter(METER_SPEED)->ResetCurrent();
    // intentionally not resetting size, so that it is presistant
}

void Field::ClampMeters() {
    UniverseObject::ClampMeters();

    // intentionally not clamping METER_SPEED, to allow negative speeds
    UniverseObject::GetMeter(METER_SIZE)->ClampCurrentToRange();
}

/////////////////////////////////////////////////
// FieldType                                   //
/////////////////////////////////////////////////
FieldType::FieldType(const std::string& name, const std::string& description,
                     float stealth, const std::set<std::string>& tags,
                     std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                     const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_stealth(stealth),
    m_tags(),
    m_effects(),
    m_graphic(graphic)
{
    for (const std::string& tag : tags)
        m_tags.insert(boost::to_upper_copy<std::string>(tag));

    for (auto&& effect : effects)
        m_effects.emplace_back(std::move(effect));

    if (m_stealth != 0.0f)
        m_effects.push_back(IncreaseMeter(METER_STEALTH,    m_stealth));

    for (auto& effect : m_effects) {
        effect->SetTopLevelContent(m_name);
    }
}

FieldType::~FieldType()
{}

std::string FieldType::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "FieldType\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "location = \n";
    //retval += m_location->Dump(ntabs+2);
    if (m_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
        retval += m_effects[0]->Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects) {
            retval += effect->Dump(ntabs+2);
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

unsigned int FieldType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_stealth);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_graphic);

    DebugLogger() << "FieldTypeManager checksum: " << retval;
    return retval;
}

/////////////////////////////////////////////////
// FieldTypeManager                         //
/////////////////////////////////////////////////
// static(s)
FieldTypeManager* FieldTypeManager::s_instance = nullptr;

FieldTypeManager::FieldTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one FieldTypeManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

const FieldType* FieldTypeManager::GetFieldType(const std::string& name) const {
    CheckPendingFieldTypes();
    auto it = m_field_types.find(name);
    return it != m_field_types.end() ? it->second.get() : nullptr;
}

FieldTypeManager::iterator FieldTypeManager::begin() const {
    CheckPendingFieldTypes();
    return m_field_types.begin();
}

FieldTypeManager::iterator FieldTypeManager::end() const {
    CheckPendingFieldTypes();
    return m_field_types.end();
}

FieldTypeManager& FieldTypeManager::GetFieldTypeManager() {
    static FieldTypeManager manager;
    return manager;
}

unsigned int FieldTypeManager::GetCheckSum() const {
    CheckPendingFieldTypes();
    unsigned int retval{0};
    for (auto const& name_type_pair : m_field_types)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, m_field_types.size());

    return retval;
}

void FieldTypeManager::SetFieldTypes(Pending::Pending<FieldTypeMap>&& future)
{ m_pending_types = std::move(future); }

void FieldTypeManager::CheckPendingFieldTypes() const
{ Pending::SwapPending(m_pending_types, m_field_types); }


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
FieldTypeManager& GetFieldTypeManager()
{ return FieldTypeManager::GetFieldTypeManager(); }

const FieldType* GetFieldType(const std::string& name)
{ return FieldTypeManager::GetFieldTypeManager().GetFieldType(name); }
