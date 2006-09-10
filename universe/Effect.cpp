#include "Effect.h"

#include "../util/AppInterface.h"
#include "ValueRef.h"
#include "Condition.h"
#include "Universe.h"
#include "UniverseObject.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "Planet.h"
#include "System.h"
#include "Tech.h"

#include <cctype>

using namespace Effect;
using namespace boost::io;
using boost::lexical_cast;
using boost::format;

extern int g_indent;

namespace {
    boost::tuple<bool, ValueRef::OpType, double>
    SimpleMeterModification(MeterType meter, bool max, const ValueRef::ValueRefBase<double>* ref)
    {
        boost::tuple<bool, ValueRef::OpType, double> retval(false, ValueRef::PLUS, 0.0);
        if (const ValueRef::Operation<double>* op = dynamic_cast<const ValueRef::Operation<double>*>(ref)) {
            if (!op->LHS() || !op->RHS())
                return retval;
            if (const ValueRef::Variable<double>* var = dynamic_cast<const ValueRef::Variable<double>*>(op->LHS())) {
                if (const ValueRef::Constant<double>* constant = dynamic_cast<const ValueRef::Constant<double>*>(op->RHS())) {
                    std::string meter_str = UserString(lexical_cast<std::string>(meter));
                    if (!meter_str.empty())
                        meter_str[0] = std::toupper(meter_str[0]);
                    retval.get<0>() = var->PropertyName().size() == 1 &&
                        ((max ? "Max" : "Current") + meter_str) == var->PropertyName()[0];
                    retval.get<1>() = op->GetOpType();
                    retval.get<2>() = constant->Value();
                    return retval;
                }
            } else if (const ValueRef::Variable<double>* var = dynamic_cast<const ValueRef::Variable<double>*>(op->RHS())) {
                if (const ValueRef::Constant<double>* constant = dynamic_cast<const ValueRef::Constant<double>*>(op->LHS())) {
                    std::string meter_str = UserString(lexical_cast<std::string>(meter));
                    if (!meter_str.empty())
                        meter_str[0] = std::toupper(meter_str[0]);
                    retval.get<0>() = var->PropertyName().size() == 1 &&
                        ((max ? "Max" : "Current") + meter_str) == var->PropertyName()[0];
                    retval.get<1>() = op->GetOpType();
                    retval.get<2>() = constant->Value();
                    return retval;
                }
            }
        }
        return retval;
    }

}

///////////////////////////////////////////////////////////
// EffectsGroup                                          //
///////////////////////////////////////////////////////////
EffectsGroup::EffectsGroup(const Condition::ConditionBase* scope, const Condition::ConditionBase* activation,
                           const std::vector<EffectBase*>& effects, const std::string& stacking_group/* = ""*/) :
    m_scope(scope),
    m_activation(activation),
    m_stacking_group(stacking_group),
    m_effects(effects)
{}

EffectsGroup::~EffectsGroup()
{
    delete m_scope;
    delete m_activation;
    for (unsigned int i = 0; i < m_effects.size(); ++i) {
        delete m_effects[i];
    }
}

void EffectsGroup::GetTargetSet(int source_id, TargetSet& targets) const
{
    Universe& universe = GetUniverse();
    UniverseObject* source = universe.Object(source_id);
    assert(source);

    targets.clear();

    // evaluate the activation condition only on the source object
    Condition::ObjectSet non_targets;
    non_targets.insert(source);
    m_activation->Eval(source, targets, non_targets);

    // if the activation condition did not evaluate to true for the source object, do nothing
    if (targets.empty())
        return;

    // evaluate the scope condition
    targets.clear();
    non_targets.clear();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        non_targets.insert(it->second);
    }
    m_scope->Eval(source, targets, non_targets);
}

void EffectsGroup::Execute(int source_id, const TargetSet& targets) const
{
    UniverseObject* source = GetUniverse().Object(source_id);
    assert(source);

    // execute effects on targets
    for (Condition::ObjectSet::const_iterator it = targets.begin(); it != targets.end(); ++it) {
        for (unsigned int i = 0; i < m_effects.size(); ++i) {
            m_effects[i]->Execute(source, *it);
        }
    }
}

const std::string& EffectsGroup::StackingGroup() const
{
    return m_stacking_group;
}

const std::vector<EffectBase*>& EffectsGroup::EffectsList() const
{
    return m_effects;
}

EffectsGroup::Description EffectsGroup::GetDescription() const
{
    Description retval;
    if (dynamic_cast<const Condition::Self*>(m_scope))
        retval.scope_description = UserString("DESC_EFFECTS_GROUP_SELF_SCOPE");
    else
        retval.scope_description = str(format(UserString("DESC_EFFECTS_GROUP_SCOPE")) % m_scope->Description());
    if (dynamic_cast<const Condition::Self*>(m_activation) || dynamic_cast<const Condition::All*>(m_activation))
        retval.activation_description = UserString("DESC_EFFECTS_GROUP_ALWAYS_ACTIVE");
    else
        retval.activation_description = str(format(UserString("DESC_EFFECTS_GROUP_ACTIVATION")) % m_activation->Description());
    for (unsigned int i = 0; i < m_effects.size(); ++i) {
        retval.effect_descriptions.push_back(m_effects[i]->Description());
    }
    return retval;
}

std::string EffectsGroup::DescriptionString() const
{
    if (!m_explicit_description.empty()) {
        return UserString(m_explicit_description);
    } else {
        std::stringstream retval;
        Description description = GetDescription();
        retval << str(format(UserString("DESC_EFFECTS_GROUP_SCOPE_DESC")) % description.scope_description);
        if (!dynamic_cast<const Condition::Self*>(m_activation) && !dynamic_cast<const Condition::All*>(m_activation))
            retval << str(format(UserString("DESC_EFFECTS_GROUP_ACTIVATION_DESC")) % description.activation_description);
        for (unsigned int i = 0; i < description.effect_descriptions.size(); ++i) {
            retval << str(format(UserString("DESC_EFFECTS_GROUP_EFFECT_DESC")) % description.effect_descriptions[i]);
        }
        return retval.str();
    }
}

std::string EffectsGroup::Dump() const
{
    std::string retval = DumpIndent() + "EffectsGroup\n";
    ++g_indent;
    retval += DumpIndent() + "scope =\n";
    ++g_indent;
    retval += m_scope->Dump();
    --g_indent;
    if (m_activation) {
        retval += DumpIndent() + "activation =\n";
        ++g_indent;
        retval += m_activation->Dump();
        --g_indent;
    }
    if (!m_stacking_group.empty())
        retval += DumpIndent() + "stackinggroup = \"" + m_stacking_group + "\"\n";
    if (m_effects.size() == 1) {
        retval += DumpIndent() + "effects =\n";
        ++g_indent;
        retval += m_effects[0]->Dump();
        --g_indent;
    } else {
        retval += DumpIndent() + "effects = [\n";
        ++g_indent;
        for (unsigned int i = 0; i < m_effects.size(); ++i) {
            retval += m_effects[i]->Dump();
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    --g_indent;
    return retval;
}


///////////////////////////////////////////////////////////
// EffectsDescription function                           //
///////////////////////////////////////////////////////////
std::string EffectsDescription(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups)
{
    std::stringstream retval;
    if (effects_groups.size() == 1) {
        retval << str(format(UserString("DESC_EFFECTS_GROUP_EFFECTS_GROUP_DESC")) % effects_groups[0]->DescriptionString());
    } else {
        for (unsigned int i = 0; i < effects_groups.size(); ++i) {
            retval << str(format(UserString("DESC_EFFECTS_GROUP_NUMBERED_EFFECTS_GROUP_DESC")) % (i + 1) % effects_groups[i]->DescriptionString());
        }
    }
    return retval.str();
}


///////////////////////////////////////////////////////////
// EffectBase                                            //
///////////////////////////////////////////////////////////
EffectBase::~EffectBase()
{}


///////////////////////////////////////////////////////////
// SetMeter                                              //
///////////////////////////////////////////////////////////
SetMeter::SetMeter(MeterType meter, const ValueRef::ValueRefBase<double>* value, bool max) :
    m_meter(meter),
    m_value(value),
    m_max(max)
{}

SetMeter::~SetMeter()
{
    delete m_value;
}

void SetMeter::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (Meter* m = target->GetMeter(m_meter)) {
        double val = m_value->Eval(source, target);
        m_max ? m->SetMax(val) : m->SetCurrent(val);
    }
}

std::string SetMeter::Description() const
{
    bool simple;
    ValueRef::OpType op;
    double const_operand;
    boost::tie(simple, op, const_operand) = SimpleMeterModification(m_meter, m_max, m_value);
    if (simple) {
        char op_char = '+';
        switch (op) {
        case ValueRef::PLUS:    op_char = '+'; break;
        case ValueRef::MINUS:   op_char = '-'; break;
        case ValueRef::TIMES:   op_char = '*'; break;
        case ValueRef::DIVIDES: op_char = '/'; break;
        default: op_char = '?';
        }
        return str(format(UserString(m_max ? "DESC_SIMPLE_SET_METER_MAX" : "DESC_SIMPLE_SET_METER_CURRENT"))
                   % UserString(lexical_cast<std::string>(m_meter))
                   % op_char
                   % lexical_cast<std::string>(const_operand));
    } else {
        return str(format(UserString(m_max ? "DESC_COMPLEX_SET_METER_MAX" : "DESC_COMPLEX_SET_METER_CURRENT"))
                   % UserString(lexical_cast<std::string>(m_meter))
                   % m_value->Description());
    }
}

std::string SetMeter::Dump() const
{
    std::string retval = DumpIndent() + (m_max ? "SetMax" : "SetCurrent");
    switch (m_meter) {
    case METER_POPULATION:   retval += "Population"; break;
    case METER_FARMING:      retval += "Farming"; break;
    case METER_INDUSTRY:     retval += "Industry"; break;
    case METER_RESEARCH:     retval += "Research"; break;
    case METER_TRADE:        retval += "Trade"; break;
    case METER_MINING:       retval += "Mining"; break;
    case METER_CONSTRUCTION: retval += "Construction"; break;
    case METER_HEALTH:       retval += "Health"; break;
    default: retval += "?"; break;
    }
    retval += " value = " + m_value->Dump() + "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// SetEmpireStockpile                                    //
///////////////////////////////////////////////////////////
SetEmpireStockpile::SetEmpireStockpile(ResourceType stockpile, const ValueRef::ValueRefBase<double>* value) :
    m_stockpile(stockpile),
    m_value(value)
{}

SetEmpireStockpile::~SetEmpireStockpile()
{
    delete m_value;
}

void SetEmpireStockpile::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (source->Owners().size() != 1)
        return;

    double value = m_value->Eval(source, target);
    Empire* empire = Empires().Lookup(*source->Owners().begin());
    if (m_stockpile == RE_FOOD) {
        empire->GetFoodResPool().SetStockpile(value);
    } else if (m_stockpile == RE_MINERALS) {
        empire->GetMineralResPool().SetStockpile(value);
    } else if (m_stockpile == RE_TRADE) {
        empire->GetTradeResPool().SetStockpile(value);
    }
}

std::string SetEmpireStockpile::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_value) ? lexical_cast<std::string>(m_value->Eval(0, 0)) : m_value->Description();
    return str(format(UserString("DESC_SET_EMPIRE_STOCKPILE")) % UserString(lexical_cast<std::string>(m_stockpile)) % value_str);
}

std::string SetEmpireStockpile::Dump() const
{
    std::string retval = DumpIndent();
    switch (m_stockpile) {
    case RE_FOOD:       retval += "SetOwnerFoodStockpile"; break;
    case RE_MINERALS:   retval += "SetOwnerMineralStockpile"; break;
    case RE_TRADE:      retval += "SetOwnerTradeStockpile"; break;
    default:            retval += "?"; break;
    }
    retval += " value = " + m_value->Dump() + "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// SetPlanetType                                         //
///////////////////////////////////////////////////////////
SetPlanetType::SetPlanetType(const ValueRef::ValueRefBase<PlanetType>* type) :
    m_type(type)
{}

SetPlanetType::~SetPlanetType()
{
    delete m_type;
}

void SetPlanetType::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (Planet* p = universe_object_cast<Planet*>(target)) {
        PlanetType type = m_type->Eval(source, target);
        p->SetType(type);
        if (type == PT_ASTEROIDS)
            p->SetSize(SZ_ASTEROIDS);
        else if (type == PT_GASGIANT)
            p->SetSize(SZ_GASGIANT);
        else if (p->Size() == SZ_ASTEROIDS)
            p->SetSize(SZ_TINY);
        else if (p->Size() == SZ_GASGIANT)
            p->SetSize(SZ_HUGE);
    }
}

std::string SetPlanetType::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_type) ? UserString(lexical_cast<std::string>(m_type->Eval(0, 0))) : m_type->Description();
    return str(format(UserString("DESC_SET_PLANET_TYPE")) % value_str);
}

std::string SetPlanetType::Dump() const
{
    return DumpIndent() + "SetPlanetType type = " + m_type->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// SetPlanetSize                                         //
///////////////////////////////////////////////////////////
SetPlanetSize::SetPlanetSize(const ValueRef::ValueRefBase<PlanetSize>* size) :
    m_size(size)
{}

SetPlanetSize::~SetPlanetSize()
{
    delete m_size;
}

void SetPlanetSize::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (Planet* p = universe_object_cast<Planet*>(target)) {
        PlanetSize size = m_size->Eval(source, target);
        p->SetSize(size);
        if (size == SZ_ASTEROIDS)
            p->SetType(PT_ASTEROIDS);
        else if (size == SZ_GASGIANT)
            p->SetType(PT_GASGIANT);
        else if (p->Type() == PT_ASTEROIDS || p->Type() == PT_GASGIANT)
            p->SetType(PT_BARREN);
    }
}

std::string SetPlanetSize::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_size) ? UserString(lexical_cast<std::string>(m_size->Eval(0, 0))) : m_size->Description();
    return str(format(UserString("DESC_SET_PLANET_SIZE")) % value_str);
}

std::string SetPlanetSize::Dump() const
{
    return DumpIndent() + "SetPlanetSize size = " + m_size->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// AddOwner                                              //
///////////////////////////////////////////////////////////
AddOwner::AddOwner(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

AddOwner::~AddOwner()
{
    delete m_empire_id;
}

void AddOwner::Execute(const UniverseObject* source, UniverseObject* target) const
{
    int empire_id = m_empire_id->Eval(source, target);
    assert(Empires().Lookup(empire_id));
    target->AddOwner(empire_id);
}

std::string AddOwner::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_empire_id) ? Empires().Lookup(m_empire_id->Eval(0, 0))->Name() : m_empire_id->Description();
    return str(format(UserString("DESC_ADD_OWNER")) % value_str);
}

std::string AddOwner::Dump() const
{
    return DumpIndent() + "AddOwner empire = " + m_empire_id->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// RemoveOwner                                           //
///////////////////////////////////////////////////////////
RemoveOwner::RemoveOwner(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

RemoveOwner::~RemoveOwner()
{
    delete m_empire_id;
}

void RemoveOwner::Execute(const UniverseObject* source, UniverseObject* target) const
{
    int empire_id = m_empire_id->Eval(source, target);
    assert(Empires().Lookup(empire_id));
    target->RemoveOwner(empire_id);
}

std::string RemoveOwner::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_empire_id) ? Empires().Lookup(m_empire_id->Eval(0, 0))->Name() : m_empire_id->Description();
    return str(format(UserString("DESC_REMOVE_OWNER")) % value_str);
}

std::string RemoveOwner::Dump() const
{
    return DumpIndent() + "RemoveOwner empire = " + m_empire_id->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// Create                                                //
///////////////////////////////////////////////////////////
// TODO: create multiple Create*s for different kinds of objects
/*class Effect::Create : public Effect::EffectBase
{
public:
    Create();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
};*/


///////////////////////////////////////////////////////////
// Destroy                                               //
///////////////////////////////////////////////////////////
Destroy::Destroy()
{}

void Destroy::Execute(const UniverseObject* source, UniverseObject* target) const
{
    GetUniverse().EffectDestroy(target->ID());
}

std::string Destroy::Description() const
{
    return UserString("DESC_DESTROY");
}

std::string Destroy::Dump() const
{
    return DumpIndent() + "Destroy\n";
}


///////////////////////////////////////////////////////////
// AddSpecial                                            //
///////////////////////////////////////////////////////////
AddSpecial::AddSpecial(const std::string& name) :
    m_name(name)
{}

void AddSpecial::Execute(const UniverseObject* source, UniverseObject* target) const
{
    target->AddSpecial(m_name);
}

std::string AddSpecial::Description() const
{
    return str(format(UserString("DESC_ADD_SPECIAL")) % UserString(m_name));
}

std::string AddSpecial::Dump() const
{
    return DumpIndent() + "AddSpecial name = \"" + m_name + "\"\n";
}


///////////////////////////////////////////////////////////
// RemoveSpecial                                         //
///////////////////////////////////////////////////////////
RemoveSpecial::RemoveSpecial(const std::string& name) :
    m_name(name)
{}

void RemoveSpecial::Execute(const UniverseObject* source, UniverseObject* target) const
{
    target->RemoveSpecial(m_name);
}

std::string RemoveSpecial::Description() const
{
    return str(format(UserString("DESC_REMOVE_SPECIAL")) % UserString(m_name));
}

std::string RemoveSpecial::Dump() const
{
    return DumpIndent() + "RemoveSpecial name = \"" + m_name + "\"\n";
}


///////////////////////////////////////////////////////////
// SetStarType                                           //
///////////////////////////////////////////////////////////
SetStarType::SetStarType(const ValueRef::ValueRefBase<StarType>* type) :
    m_type(type)
{}

SetStarType::~SetStarType()
{
    delete m_type;
}

void SetStarType::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (System* s = universe_object_cast<System*>(target)) {
        s->SetStarType(m_type->Eval(source, target));
    }
}

std::string SetStarType::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_type) ? UserString(lexical_cast<std::string>(m_type->Eval(0, 0))) : m_type->Description();
    return str(format(UserString("DESC_SET_STAR_TYPE")) % value_str);
}

std::string SetStarType::Dump() const
{
    return DumpIndent() + "SetStarType type = " + m_type->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// SetTechAvailability                                   //
///////////////////////////////////////////////////////////
SetTechAvailability::SetTechAvailability(const std::string& tech_name, const ValueRef::ValueRefBase<int>* empire_id, bool available, bool include_tech) :
    m_tech_name(tech_name),
    m_empire_id(empire_id),
    m_available(available),
    m_include_tech(include_tech)
{}

SetTechAvailability::~SetTechAvailability()
{
    delete m_empire_id;
}

void SetTechAvailability::Execute(const UniverseObject* source, UniverseObject* target) const
{
    Empire* empire = Empires().Lookup(m_empire_id->Eval(source, target));
    const Tech* tech = GetTech(m_tech_name);
    assert(empire && tech);

    const std::vector<ItemSpec>& items = tech->UnlockedItems();
    for (unsigned int i = 0; i < items.size(); ++i) {
        if (m_available)
            empire->UnlockItem(items[i]);
        else
            empire->LockItem(items[i]);
    }

    if (m_include_tech) {
        if (m_available)
            empire->AddTech(m_tech_name);
        else
            empire->RemoveTech(m_tech_name);
    }
}

std::string SetTechAvailability::Description() const
{
    std::string affected = str(format(UserString(m_include_tech ? "DESC_TECH_AND_ITEMS_AFFECTED" : "DESC_ITEMS_ONLY_AFFECTED")) % m_tech_name);
    std::string empire_str = ValueRef::ConstantExpr(m_empire_id) ? Empires().Lookup(m_empire_id->Eval(0, 0))->Name() : m_empire_id->Description();
    return str(format(UserString(m_available ? "DESC_SET_TECH_AVAIL" : "DESC_SET_TECH_UNAVAIL"))
               % affected
               % empire_str);
}

std::string SetTechAvailability::Dump() const
{
    std::string retval = DumpIndent();
    if (m_available && m_include_tech)
        retval += "GiveTechToOwner";
    if (!m_available && m_include_tech)
        retval += "RevokeTechFromOwner";
    if (m_available && !m_include_tech)
        retval += "UnlockTechItemsForOwner";
    if (!m_available && !m_include_tech)
        retval += "LockTechItemsForOwner";
    retval += " name = \"" + m_tech_name + "\"\n";
    return retval;
}


///////////////////////////////////////////////////////////
// SetEffectTarget                                       //
///////////////////////////////////////////////////////////
SetEffectTarget::SetEffectTarget(const ValueRef::ValueRefBase<int>* effect_target_id) :
    m_effect_target_id(effect_target_id)
{}

SetEffectTarget::~SetEffectTarget()
{
    delete m_effect_target_id;
}

void SetEffectTarget::Execute(const UniverseObject* source, UniverseObject* target) const
{
    // TODO: implement after Effect targets are implemented
}

std::string SetEffectTarget::Description() const
{
    // TODO: implement after Effect targets are implemented
    return "ERROR: SetEffectTarget is currently unimplemented.";
}

std::string SetEffectTarget::Dump() const
{
    return "";
}
