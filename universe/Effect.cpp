#include "Effect.h"

#include "../util/AppInterface.h"
#include "../util/Parse.h"

#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>

using namespace Effect;
using namespace boost::io;
using boost::lexical_cast;
using boost::format;

namespace {
    EffectBase* NewSetMeter(const GG::XMLElement& elem)             {return new SetMeter(elem);}
    EffectBase* NewSetEmpireStockpile(const GG::XMLElement& elem)   {return new SetEmpireStockpile(elem);}
    EffectBase* NewSetPlanetType(const GG::XMLElement& elem)        {return new SetPlanetType(elem);}
    EffectBase* NewSetPlanetSize(const GG::XMLElement& elem)        {return new SetPlanetSize(elem);}
    EffectBase* NewAddOwner(const GG::XMLElement& elem)             {return new AddOwner(elem);}
    EffectBase* NewRemoveOwner(const GG::XMLElement& elem)          {return new RemoveOwner(elem);}
    //EffectBase* NewCreate(const GG::XMLElement& elem)             {return new Create(elem);}
    EffectBase* NewDestroy(const GG::XMLElement& elem)              {return new Destroy(elem);}
    EffectBase* NewAddSpecial(const GG::XMLElement& elem)           {return new AddSpecial(elem);}
    EffectBase* NewRemoveSpecial(const GG::XMLElement& elem)        {return new RemoveSpecial(elem);}
    EffectBase* NewSetStarType(const GG::XMLElement& elem)          {return new SetStarType(elem);}
    EffectBase* NewSetTechAvailability(const GG::XMLElement& elem)  {return new SetTechAvailability(elem);}
    EffectBase* NewSetEffectTarget(const GG::XMLElement& elem)      {return new SetEffectTarget(elem);}

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

    bool temp_header_bool = RecordHeaderFile(EffectRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

GG::XMLObjectFactory<EffectBase> Effect::EffectFactory()
{
    static GG::XMLObjectFactory<EffectBase> factory;
    static bool init = false;
    if (!init) {
        factory.AddGenerator("Effect::SetMeter", &NewSetMeter);
        factory.AddGenerator("Effect::SetEmpireStockpile", &NewSetEmpireStockpile);
        factory.AddGenerator("Effect::SetPlanetType", &NewSetPlanetType);
        factory.AddGenerator("Effect::SetPlanetSize", &NewSetPlanetSize);
        factory.AddGenerator("Effect::AddOwner", &NewAddOwner);
        factory.AddGenerator("Effect::RemoveOwner", &NewRemoveOwner);
        //factory.AddGenerator("Effect::Create", &NewCreate);
        factory.AddGenerator("Effect::Destroy", &NewDestroy);
        factory.AddGenerator("Effect::AddSpecial", &NewAddSpecial);
        factory.AddGenerator("Effect::RemoveSpecial", &NewRemoveSpecial);
        factory.AddGenerator("Effect::SetStarType", &NewSetStarType);
        factory.AddGenerator("Effect::SetTechAvailability", &NewSetTechAvailability);
        factory.AddGenerator("Effect::SetEffectTarget", &NewSetEffectTarget);
        init = true;
    }
    return factory;
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
{
}

EffectsGroup::EffectsGroup(const GG::XMLElement& elem)
{
    if (elem.Tag() != "EffectsGroup")
        throw std::invalid_argument(("Attempted to construct a EffectsGroup from an XMLElement that had a tag other than \"EffectsGroup\"" + (" (\"" + elem.Tag() + "\")")).c_str());

    m_scope = Condition::ConditionFactory().GenerateObject(elem.Child("scope").Child(0));
    if (elem.ContainsChild("activation"))
        m_activation = Condition::ConditionFactory().GenerateObject(elem.Child("activation").Child(0));
    else
        m_activation = new Condition::Self();
    if (elem.ContainsChild("stacking_group"))
        m_stacking_group = elem.Child("stacking_group").Text();
    for (GG::XMLElement::const_child_iterator it = elem.Child("effects").child_begin(); it != elem.Child("effects").child_end(); ++it) {
        m_effects.push_back(EffectFactory().GenerateObject(*it));
    }
}

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
    Condition::ConditionBase::ObjectSet non_targets;
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
    for (Condition::ConditionBase::ObjectSet::const_iterator it = targets.begin(); it != targets.end(); ++it) {
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
{
}


///////////////////////////////////////////////////////////
// SetMeter                                              //
///////////////////////////////////////////////////////////
SetMeter::SetMeter(MeterType meter, const ValueRef::ValueRefBase<double>* value, bool max) :
    m_meter(meter),
    m_value(value),
    m_max(max)
{
}

SetMeter::SetMeter(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::SetMeter")
        throw std::invalid_argument("Attempted to construct a Effect::SetMeter from an XMLElement that had a tag other than \"Effect::SetMeter\"");

    m_meter = lexical_cast<MeterType>(elem.Child("meter").Text());
    m_value = ParseArithmeticExpression<double>(elem.Child("value").Text());
    m_max = lexical_cast<bool>(elem.Child("max").Text());
}

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


///////////////////////////////////////////////////////////
// SetEmpireStockpile                                    //
///////////////////////////////////////////////////////////
SetEmpireStockpile::SetEmpireStockpile(StockpileType stockpile, const ValueRef::ValueRefBase<double>* value) :
    m_stockpile(stockpile),
    m_value(value)
{
}

SetEmpireStockpile::SetEmpireStockpile(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::SetEmpireStockpile")
        throw std::invalid_argument("Attempted to construct a Effect::SetEmpireStockpile from an XMLElement that had a tag other than \"Effect::SetEmpireStockpile\"");

    m_stockpile = lexical_cast<StockpileType>(elem.Child("stockpile").Text());
    m_value = ParseArithmeticExpression<double>(elem.Child("value").Text());
}

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
    if (m_stockpile == ST_FOOD) {
        empire->FoodResPool().SetStockpile(value);
    } else if (m_stockpile == ST_MINERAL) {
        empire->MineralResPool().SetStockpile(value);
    } else if (m_stockpile == ST_TRADE) {
        empire->TradeResPool().SetStockpile(value);
    }
}

std::string SetEmpireStockpile::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_value) ? lexical_cast<std::string>(m_value->Eval(0, 0)) : m_value->Description();
    return str(format(UserString("DESC_SET_EMPIRE_STOCKPILE")) % UserString(lexical_cast<std::string>(m_stockpile)) % value_str);
}


///////////////////////////////////////////////////////////
// SetPlanetType                                         //
///////////////////////////////////////////////////////////
SetPlanetType::SetPlanetType(const ValueRef::ValueRefBase<PlanetType>* type) :
    m_type(type)
{
}

SetPlanetType::SetPlanetType(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::SetPlanetType")
        throw std::invalid_argument("Attempted to construct a Effect::SetPlanetType from an XMLElement that had a tag other than \"Effect::SetPlanetType\"");

    m_type = ParseArithmeticExpression<PlanetType>(elem.Text());
}

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


///////////////////////////////////////////////////////////
// SetPlanetSize                                         //
///////////////////////////////////////////////////////////
SetPlanetSize::SetPlanetSize(const ValueRef::ValueRefBase<PlanetSize>* size) :
    m_size(size)
{
}

SetPlanetSize::SetPlanetSize(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::SetPlanetSize")
        throw std::invalid_argument("Attempted to construct a Effect::SetPlanetSize from an XMLElement that had a tag other than \"Effect::SetPlanetSize\"");

    m_size = ParseArithmeticExpression<PlanetSize>(elem.Text());
}

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


///////////////////////////////////////////////////////////
// AddOwner                                              //
///////////////////////////////////////////////////////////
AddOwner::AddOwner(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{
}

AddOwner::AddOwner(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::AddOwner")
        throw std::invalid_argument("Attempted to construct a Effect::AddOwner from an XMLElement that had a tag other than \"Effect::AddOwner\"");

    m_empire_id = ParseArithmeticExpression<int>(elem.Text());
}

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


///////////////////////////////////////////////////////////
// RemoveOwner                                           //
///////////////////////////////////////////////////////////
RemoveOwner::RemoveOwner(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{
}

RemoveOwner::RemoveOwner(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::RemoveOwner")
        throw std::invalid_argument("Attempted to construct a Effect::RemoveOwner from an XMLElement that had a tag other than \"Effect::RemoveOwner\"");

    m_empire_id = ParseArithmeticExpression<int>(elem.Text());
}

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


///////////////////////////////////////////////////////////
// Create                                                //
///////////////////////////////////////////////////////////
// TODO: create multiple Create*s for different kinds of objects
/*class Effect::Create : public Effect::EffectBase
{
public:
    Create();
    Create(const GG::XMLElement& elem);

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
};*/


///////////////////////////////////////////////////////////
// Destroy                                               //
///////////////////////////////////////////////////////////
Destroy::Destroy()
{
}

Destroy::Destroy(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::Destroy")
        throw std::invalid_argument("Attempted to construct a Effect::Destroy from an XMLElement that had a tag other than \"Effect::Destroy\"");
}

void Destroy::Execute(const UniverseObject* source, UniverseObject* target) const
{
    GetUniverse().EffectDestroy(target->ID());
}

std::string Destroy::Description() const
{
    return UserString("DESC_DESTROY");
}


///////////////////////////////////////////////////////////
// AddSpecial                                            //
///////////////////////////////////////////////////////////
AddSpecial::AddSpecial(const std::string& name) :
    m_name(name)
{
}

AddSpecial::AddSpecial(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::AddSpecial")
        throw std::invalid_argument("Attempted to construct a Effect::AddSpecial from an XMLElement that had a tag other than \"Effect::AddSpecial\"");

    m_name = elem.Text();
}

void AddSpecial::Execute(const UniverseObject* source, UniverseObject* target) const
{
    target->AddSpecial(m_name);
}

std::string AddSpecial::Description() const
{
    return str(format(UserString("DESC_ADD_SPECIAL")) % UserString(m_name));
}


///////////////////////////////////////////////////////////
// RemoveSpecial                                         //
///////////////////////////////////////////////////////////
RemoveSpecial::RemoveSpecial(const std::string& name) :
    m_name(name)
{
}

RemoveSpecial::RemoveSpecial(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::RemoveSpecial")
        throw std::invalid_argument("Attempted to construct a Effect::RemoveSpecial from an XMLElement that had a tag other than \"Effect::RemoveSpecial\"");

    m_name = elem.Text();
}

void RemoveSpecial::Execute(const UniverseObject* source, UniverseObject* target) const
{
    target->RemoveSpecial(m_name);
}

std::string RemoveSpecial::Description() const
{
    return str(format(UserString("DESC_REMOVE_SPECIAL")) % UserString(m_name));
}


///////////////////////////////////////////////////////////
// SetStarType                                           //
///////////////////////////////////////////////////////////
SetStarType::SetStarType(const ValueRef::ValueRefBase<StarType>* type) :
    m_type(type)
{
}

SetStarType::SetStarType(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::SetStarType")
        throw std::invalid_argument("Attempted to construct a Effect::SetStarType from an XMLElement that had a tag other than \"Effect::SetStarType\"");

    m_type = ParseArithmeticExpression<StarType>(elem.Text());
}

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


///////////////////////////////////////////////////////////
// SetTechAvailability                                   //
///////////////////////////////////////////////////////////
SetTechAvailability::SetTechAvailability(const std::string& tech_name, const ValueRef::ValueRefBase<int>* empire_id, bool available, bool include_tech) :
    m_tech_name(tech_name),
    m_empire_id(empire_id),
    m_available(available),
    m_include_tech(include_tech)
{
}

SetTechAvailability::SetTechAvailability(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::SetTechAvailability")
        throw std::invalid_argument("Attempted to construct a Effect::SetTechAvailability from an XMLElement that had a tag other than \"Effect::SetTechAvailability\"");

    m_tech_name = elem.Child("tech_name").Text();
    m_empire_id = ParseArithmeticExpression<int>(elem.Child("empire_id").Text());
    m_available = lexical_cast<bool>(elem.Child("available").Text());
    m_include_tech = lexical_cast<bool>(elem.Child("include_tech").Text());
}

SetTechAvailability::~SetTechAvailability()
{
    delete m_empire_id;
}

void SetTechAvailability::Execute(const UniverseObject* source, UniverseObject* target) const
{
    Empire* empire = Empires().Lookup(m_empire_id->Eval(source, target));
    const Tech* tech = GetTech(m_tech_name);
    assert(empire && tech);

    const std::vector<Tech::ItemSpec>& items = tech->UnlockedItems();
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


///////////////////////////////////////////////////////////
// RefineBuildingType                                    //
///////////////////////////////////////////////////////////
RefineBuildingType::RefineBuildingType(const std::string& building_type_name,
                                       const ValueRef::ValueRefBase<int>* empire_id,
                                       const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects) :
    m_building_type_name(building_type_name),
    m_empire_id(empire_id),
    m_effects(effects)
{
}

RefineBuildingType::RefineBuildingType(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::RefineBuildingType")
        throw std::invalid_argument("Attempted to construct a Effect::RefineBuildingType from an XMLElement that had a tag other than \"Effect::RefineBuildingType\"");

    m_building_type_name = elem.Child("building_type").Text();
    m_empire_id = ParseArithmeticExpression<int>(elem.Child("empire_id").Text());
    for (GG::XMLElement::const_child_iterator it = elem.Child("effects").child_begin(); it != elem.Child("effects").child_end(); ++it) {
        m_effects.push_back(boost::shared_ptr<const Effect::EffectsGroup>(new Effect::EffectsGroup(*it)));
    }
}

RefineBuildingType::~RefineBuildingType()
{
    delete m_empire_id;
}

void RefineBuildingType::Execute(const UniverseObject* source, UniverseObject* target) const
{
    int empire_id = m_empire_id->Eval(source, target);
    Empire* empire = Empires().Lookup(empire_id);
    empire->RefineBuildingType(m_building_type_name, m_effects);
}

std::string RefineBuildingType::Description() const
{
    std::string retval;
    std::string empire_str = ValueRef::ConstantExpr(m_empire_id) ? Empires().Lookup(m_empire_id->Eval(0, 0))->Name() : m_empire_id->Description();
#if 0
    retval = str(format(UserString("DESC_REFINE_BUILDING_TYPE"))
                 % m_building_type_name
                 % empire_str);
    for (unsigned int i = 0; i < m_effects.size(); ++i) {
        EffectsGroup::Description description = m_effects.GetDescription();
        retval += str(format(UserString("DESC_EFFECTS_GROUP"))
                      % description.scope_description
                      % description.activation_description);
        for (unsigned int j = 0; j < description.effect_descriptions.size(); ++j) {
            retval += str(format(UserString("DESC_EFFECT"))
                          % lexical_cast<std::string>(j)
                          % description.effect_descriptions[i]);
        }
    }
#endif
    return retval;
}


///////////////////////////////////////////////////////////
// SetEffectTarget                                       //
///////////////////////////////////////////////////////////
SetEffectTarget::SetEffectTarget(const ValueRef::ValueRefBase<int>* effect_target_id) :
    m_effect_target_id(effect_target_id)
{
}

SetEffectTarget::SetEffectTarget(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::SetEffectTarget")
        throw std::invalid_argument("Attempted to construct a Effect::SetEffectTarget from an XMLElement that had a tag other than \"Effect::SetEffectTarget\"");

    m_effect_target_id = ParseArithmeticExpression<int>(elem.Child("effect_target_id").Text());
}

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
