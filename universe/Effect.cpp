#include "Effect.h"

#include "../util/Parse.h"

using namespace Effect;

namespace {
    Effect::EffectBase* NewSetMeter(const GG::XMLElement& elem)             {return new Effect::SetMeter(elem);}
    //Effect::EffectBase* NewSetEmpireStockpile(const GG::XMLElement& elem) {return new Effect::SetEmpireStockpile(elem);}
    Effect::EffectBase* NewSetPlanetType(const GG::XMLElement& elem)        {return new Effect::SetPlanetType(elem);}
    Effect::EffectBase* NewSetPlanetSize(const GG::XMLElement& elem)        {return new Effect::SetPlanetSize(elem);}
    Effect::EffectBase* NewAddOwner(const GG::XMLElement& elem)             {return new Effect::AddOwner(elem);}
    Effect::EffectBase* NewRemoveOwner(const GG::XMLElement& elem)          {return new Effect::RemoveOwner(elem);}
    //Effect::EffectBase* NewCreate(const GG::XMLElement& elem)             {return new Effect::Create(elem);}
    Effect::EffectBase* NewDestroy(const GG::XMLElement& elem)              {return new Effect::Destroy(elem);}
    Effect::EffectBase* NewAddSpecial(const GG::XMLElement& elem)           {return new Effect::AddSpecial(elem);}
    Effect::EffectBase* NewRemoveSpecial(const GG::XMLElement& elem)        {return new Effect::RemoveSpecial(elem);}
    Effect::EffectBase* NewSetStarType(const GG::XMLElement& elem)          {return new Effect::SetStarType(elem);}
    Effect::EffectBase* NewSetAvailability(const GG::XMLElement& elem)      {return new Effect::SetAvailability(elem);}
    Effect::EffectBase* NewSetEffectTarget(const GG::XMLElement& elem)      {return new Effect::SetEffectTarget(elem);}

    bool temp_header_bool = RecordHeaderFile(EffectRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

GG::XMLObjectFactory<Effect::EffectBase> Effect::EffectFactory()
{
    static GG::XMLObjectFactory<Effect::EffectBase> factory;
    static bool init = false;
    if (!init) {
        factory.AddGenerator("Effect::SetMeter", &NewSetMeter);
        //factory.AddGenerator("Effect::SetEmpireStockpile", &NewSetEmpireStockpile);
        factory.AddGenerator("Effect::SetPlanetType", &NewSetPlanetType);
        factory.AddGenerator("Effect::SetPlanetSize", &NewSetPlanetSize);
        factory.AddGenerator("Effect::AddOwner", &NewAddOwner);
        factory.AddGenerator("Effect::RemoveOwner", &NewRemoveOwner);
        //factory.AddGenerator("Effect::Create", &NewCreate);
        factory.AddGenerator("Effect::Destroy", &NewDestroy);
        factory.AddGenerator("Effect::AddSpecial", &NewAddSpecial);
        factory.AddGenerator("Effect::RemoveSpecial", &NewRemoveSpecial);
        factory.AddGenerator("Effect::SetAvailability", &NewSetAvailability);
        factory.AddGenerator("Effect::SetEffectTarget", &NewSetEffectTarget);
        init = true;
    }
    return factory;
}

///////////////////////////////////////////////////////////
// EffectsGroup                                          //
///////////////////////////////////////////////////////////
EffectsGroup::EffectsGroup(const Condition::ConditionBase* scope, const Condition::ConditionBase* activation, const std::vector<EffectBase*>& effects) :
    m_scope(scope),
    m_activation(activation),
    m_effects(effects)
{
}

EffectsGroup::EffectsGroup(const GG::XMLElement& elem)
{
    if (elem.Tag() != "EffectsGroup")
        throw std::invalid_argument(("Attempted to construct a EffectsGroup from an XMLElement that had a tag other than \"EffectsGroup\"" + (" (\"" + elem.Tag() + ")")).c_str());

    m_scope = Condition::ConditionFactory().GenerateObject(elem.Child("scope").Child(0));
    m_activation = Condition::ConditionFactory().GenerateObject(elem.Child("activation").Child(0));
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

void EffectsGroup::Execute(int source_id) const
{
    Universe& universe = GetUniverse();
    UniverseObject* source = universe.Object(source_id);

    // evaluate the activation condition only on the source object
    Condition::ConditionBase::ObjectSet condition_targets;
    Condition::ConditionBase::ObjectSet condition_non_targets;
    condition_non_targets.insert(source);
    m_activation->Eval(source, condition_targets, condition_non_targets);

    // if the activation condition did not evaluate to true for the source object, do nothing
    if (condition_targets.empty())
        return;

    // evaluate the scope condition
    condition_targets.clear();
    condition_non_targets.clear();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        condition_non_targets.insert(it->second);
    }
    m_scope->Eval(source, condition_targets, condition_non_targets);

    // execute effects on targets in scope
    for (Condition::ConditionBase::ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it) {
        for (unsigned int i = 0; i < m_effects.size(); ++i) {
            m_effects[i]->Execute(source, *it);
        }
    }
}

const std::vector<EffectBase*>& EffectsGroup::EffectsList() const
{
    return m_effects;
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

    m_meter = boost::lexical_cast<MeterType>(elem.Child("meter").Text());
    m_value = ParseArithmeticExpression<double>(elem.Child("value").Text());
    m_max = boost::lexical_cast<bool>(elem.Child("max").Text());
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
        p->SetType(m_type->Eval(source, target));
    }
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
        p->SetSize(m_size->Eval(source, target));
    }
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
    // TODO : verify that such an empire exists
    target->AddOwner(m_empire_id->Eval(source, target));
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
    // TODO : verify that such an empire exists
    target->RemoveOwner(m_empire_id->Eval(source, target));
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
    GetUniverse().Delete(target->ID());
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


///////////////////////////////////////////////////////////
// SetAvailability                                           //
///////////////////////////////////////////////////////////
SetAvailability::SetAvailability(const std::string& tech_name, const ValueRef::ValueRefBase<int>* empire_id, bool available, bool grant_tech) :
    m_tech_name(tech_name),
    m_empire_id(empire_id),
    m_available(available),
    m_grant_tech(grant_tech)
{
}

SetAvailability::SetAvailability(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Effect::SetAvailability")
        throw std::invalid_argument("Attempted to construct a Effect::SetAvailability from an XMLElement that had a tag other than \"Effect::SetAvailability\"");

    m_tech_name = elem.Child("tech_name").Text();
    m_empire_id = ParseArithmeticExpression<int>(elem.Child("empire_id").Text());
    m_available = boost::lexical_cast<bool>(elem.Child("available").Text());
    m_grant_tech = boost::lexical_cast<bool>(elem.Child("grant_tech").Text());
}

SetAvailability::~SetAvailability()
{
    delete m_empire_id;
}

void SetAvailability::Execute(const UniverseObject* source, UniverseObject* target) const
{
    // TODO: implement after availability is implemented
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
