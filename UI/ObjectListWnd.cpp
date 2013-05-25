#include "ObjectListWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUISpin.h"
#include "FleetButton.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/AppInterface.h"
#include "../util/ModeratorAction.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/System.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/Field.h"
#include "../universe/Species.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>

std::vector<std::string> SpecialNames();

namespace {
    enum VIS_DISPLAY { SHOW_VISIBLE, SHOW_PREVIOUSLY_VISIBLE, SHOW_DESTROYED };

    const std::string EMPTY_STRING;
    const std::string ALL_CONDITION(UserStringNop("CONDITION_ALL"));
    const std::string EMPIREAFFILIATION_CONDITION(UserStringNop("CONDITION_EMPIREAFFILIATION"));
    const std::string HOMEWORLD_CONDITION(UserStringNop("CONDITION_HOMEWORLD"));
    const std::string CAPITAL_CONDITION(UserStringNop("CONDITION_CAPITAL"));
    const std::string MONSTER_CONDITION(UserStringNop("CONDITION_MONSTER"));
    const std::string ARMED_CONDITION(UserStringNop("CONDITION_ARMED"));
    const std::string STATIONARY_CONDITION(UserStringNop("CONDITION_STATIONARY"));
    const std::string CANPRODUCESHIPS_CONDITION(UserStringNop("CONDITION_CANPRODUCESHIPS"));
    const std::string CANCOLONIZE_CONDITION(UserStringNop("CONDITION_CANCOLONIZE"));
    const std::string BUILDING_CONDITION(UserStringNop("CONDITION_BUILDING"));
    const std::string HASSPECIAL_CONDITION(UserStringNop("CONDITION_HASSPECIAL"));
    const std::string HASTAG_CONDITION(UserStringNop("CONDITION_HASTAG"));
    const std::string SPECIES_CONDITION(UserStringNop("CONDITION_SPECIES"));
    const std::string PRODUCEDBYEMPIRE_CONDITION(UserStringNop("CONDITION_PRODUCEDBYEMPIRE"));
    const std::string EXPLOREDBYEMPIRE_CONDITION(UserStringNop("CONDITION_EXPLOREDBYEMPIRE"));
    const std::string CONTAINEDBY_CONDITION(UserStringNop("CONDITION_CONTAINEDBY"));
    const std::string INSYSTEM_CONDITION(UserStringNop("CONDITION_INSYSTEM"));
    const std::string OBJECTID_CONDITION(UserStringNop("CONDITION_OBJECTID"));
    const std::string CREATEDONTURN_CONDITION(UserStringNop("CONDITION_CREATEDONTURN"));
    const std::string PLANETSIZE_CONDITION(UserStringNop("CONDITION_PLANETSIZE"));
    const std::string PLANETTYPE_CONDITION(UserStringNop("CONDITION_PLANETTYPE"));
    const std::string FOCUSTYPE_CONDITION(UserStringNop("CONDITION_FOCUSTYPE"));
    const std::string STARTYPE_CONDITION(UserStringNop("CONDITION_STARTYPE"));
    const std::string METERVALUE_CONDITION(UserStringNop("CONDITION_METERVALUE"));

    ValueRef::ValueRefBase<std::string>*    CopyStringValueRef(const ValueRef::ValueRefBase<std::string>* const value_ref) {
        if (const ValueRef::Constant<std::string>* constant =
            dynamic_cast<const ValueRef::Constant<std::string>*>(value_ref))
        { return new ValueRef::Constant<std::string>(constant->Value()); }
        return new ValueRef::Constant<std::string>("");
    }
    ValueRef::ValueRefBase<int>*            CopyIntValueRef(const ValueRef::ValueRefBase<int>* const value_ref) {
        if (const ValueRef::Constant<int>* constant =
            dynamic_cast<const ValueRef::Constant<int>*>(value_ref))
        { return new ValueRef::Constant<int>(constant->Value()); }
        return new ValueRef::Constant<int>(0);
    }
    ValueRef::ValueRefBase<double>*         CopyDoubleValueRef(const ValueRef::ValueRefBase<double>* const value_ref) {
        if (const ValueRef::Constant<double>* constant =
            dynamic_cast<const ValueRef::Constant<double>*>(value_ref))
        { return new ValueRef::Constant<double>(constant->Value()); }
        return new ValueRef::Constant<double>(0.0);
    }

    template <class enumT>
    ValueRef::ValueRefBase<enumT>*          CopyEnumValueRef(const ValueRef::ValueRefBase<enumT>* const value_ref) {
        if (const ValueRef::Constant<enumT>* constant =
            dynamic_cast<const ValueRef::Constant<enumT>*>(value_ref))
        { return new ValueRef::Constant<enumT>(constant->Value()); }
        return new ValueRef::Constant<enumT>(enumT(-1));
    }

    Condition::ConditionBase*               CopyCondition(const Condition::ConditionBase* const condition) {
        if (dynamic_cast<const Condition::Source* const>(condition)) {
            return new Condition::Source();

        } else if (dynamic_cast<const Condition::Homeworld* const>(condition)) {
            return new Condition::Homeworld();

        } else if (dynamic_cast<const Condition::Building* const>(condition)) {

        }

        return new Condition::All();
    }

    const std::string&                      ConditionClassName(const Condition::ConditionBase* const condition) {
        if (dynamic_cast<const Condition::All* const>(condition))
            return ALL_CONDITION;
        else if (dynamic_cast<const Condition::Homeworld* const>(condition))
            return HOMEWORLD_CONDITION;
        else if (dynamic_cast<const Condition::Capital* const>(condition))
            return CAPITAL_CONDITION;
        else if (dynamic_cast<const Condition::Monster* const>(condition))
            return MONSTER_CONDITION;
        else if (dynamic_cast<const Condition::Armed* const>(condition))
            return ARMED_CONDITION;
        else if (dynamic_cast<const Condition::Stationary* const>(condition))
            return STATIONARY_CONDITION;
        else if (dynamic_cast<const Condition::CanProduceShips* const>(condition))
            return CANPRODUCESHIPS_CONDITION;
        else if (dynamic_cast<const Condition::CanColonize* const>(condition))
            return CANCOLONIZE_CONDITION;
        else if (dynamic_cast<const Condition::Building* const>(condition))
            return BUILDING_CONDITION;
        else if (dynamic_cast<const Condition::HasSpecial* const>(condition))
            return HASSPECIAL_CONDITION;
        else if (dynamic_cast<const Condition::HasTag* const>(condition))
            return HASTAG_CONDITION;
        else if (dynamic_cast<const Condition::Species* const>(condition))
            return SPECIES_CONDITION;
        else if (dynamic_cast<const Condition::ProducedByEmpire* const>(condition))
            return PRODUCEDBYEMPIRE_CONDITION;
        else if (dynamic_cast<const Condition::ExploredByEmpire* const>(condition))
            return EXPLOREDBYEMPIRE_CONDITION;
        else if (dynamic_cast<const Condition::ContainedBy* const>(condition))
            return CONTAINEDBY_CONDITION;
        else if (dynamic_cast<const Condition::InSystem* const>(condition))
            return INSYSTEM_CONDITION;
        else if (dynamic_cast<const Condition::ObjectID* const>(condition))
            return OBJECTID_CONDITION;
        else if (dynamic_cast<const Condition::CreatedOnTurn* const>(condition))
            return CREATEDONTURN_CONDITION;
        else if (dynamic_cast<const Condition::PlanetSize* const>(condition))
            return PLANETSIZE_CONDITION;
        else if (dynamic_cast<const Condition::PlanetType* const>(condition))
            return PLANETTYPE_CONDITION;
        else if (dynamic_cast<const Condition::FocusType* const>(condition))
            return FOCUSTYPE_CONDITION;
        else if (dynamic_cast<const Condition::StarType* const>(condition))
            return STARTYPE_CONDITION;
        else if (dynamic_cast<const Condition::MeterValue* const>(condition))
            return METERVALUE_CONDITION;
        else return EMPTY_STRING;
    }
}


////////////////////////////////////////////////
// ConditionWidget
////////////////////////////////////////////////
namespace {
    template <typename enumT>
    std::vector<std::string> StringsFromEnums(const std::vector<enumT>& enum_vals) {
        std::vector<std::string> retval;
        for (typename std::vector<enumT>::const_iterator it = enum_vals.begin(); it != enum_vals.end(); ++it)
            retval.push_back(boost::lexical_cast<std::string>(*it));
        return retval;
    }
}

class ConditionWidget : public GG::Control {
public:
    ConditionWidget(GG::X x, GG::Y y, const Condition::ConditionBase* initial_condition = 0) :
        GG::Control(x, y, GG::X(380), GG::Y1, GG::INTERACTIVE),
        m_class_drop(0),
        m_string_drop(0),
        m_param_spin1(0),
        m_param_spin2(0)
    {
        Condition::ConditionBase* init_condition = 0;
        if (!initial_condition) {
            init_condition = new Condition::All();
        } else {
            init_condition = CopyCondition(initial_condition);
        }
        Init(init_condition);
        delete init_condition;
    }

    ~ConditionWidget() {
        delete m_class_drop;
        delete m_string_drop;
        delete m_param_spin1;
        delete m_param_spin2;
    }

    Condition::ConditionBase*       GetCondition() {
        GG::ListBox::iterator row_it = m_class_drop->CurrentItem();
        if (row_it == m_class_drop->end())
            return new Condition::All();
        ConditionRow* condition_row = dynamic_cast<ConditionRow*>(*row_it);
        if (!condition_row)
            return new Condition::All();
        const std::string& condition_key = condition_row->GetKey();

        if (condition_key == ALL_CONDITION) {
            return new Condition::All();

        } else if (condition_key == EMPIREAFFILIATION_CONDITION) {
            EmpireAffiliationType affil = AFFIL_SELF;

            const std::string& empire_name = GetString();
            if (empire_name.empty()) {
                return new Condition::EmpireAffiliation(affil);
            }

            // get id of empire matching name
            int empire_id = ALL_EMPIRES;
            const EmpireManager& empires = Empires();
            for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it) {
                if (it->second->Name() == empire_name) {
                    empire_id = it->first;
                    break;
                }
            }
            return new Condition::EmpireAffiliation(new ValueRef::Constant<int>(empire_id), affil);

        } else if (condition_key == HOMEWORLD_CONDITION) {
            const std::string& species_name = GetString();
            if (species_name.empty())
                return new Condition::Homeworld();
            std::vector<const ValueRef::ValueRefBase<std::string>*> names;
            names.push_back(new ValueRef::Constant<std::string>(species_name));
            return new Condition::Homeworld(names);

        } else if (condition_key == HASSPECIAL_CONDITION) {
            return new Condition::HasSpecial(GetString());

        } else if (condition_key == HASTAG_CONDITION) {
            return new Condition::HasTag(GetString());

        } else if (condition_key == MONSTER_CONDITION) {
            return new Condition::Monster();

        } else if (condition_key == CAPITAL_CONDITION) {
            return new Condition::Capital();

        } else if (condition_key == ARMED_CONDITION) {
            return new Condition::Armed();

        } else if (condition_key == STATIONARY_CONDITION) {
            return new Condition::Stationary();

        } else if (condition_key == SPECIES_CONDITION) {
            return new Condition::Species(GetStringValueRefVec());

        } else if (condition_key == PLANETSIZE_CONDITION) {
            return new Condition::PlanetSize(GetEnumValueRefVec< ::PlanetSize>());

        } else if (condition_key == PLANETTYPE_CONDITION) {
            return new Condition::PlanetType(GetEnumValueRefVec< ::PlanetType>());

        } else if (condition_key == FOCUSTYPE_CONDITION) {
            return new Condition::FocusType(GetStringValueRefVec());

        } else if (condition_key == STARTYPE_CONDITION) {
            return new Condition::StarType(GetEnumValueRefVec< ::StarType>());

        } else if (condition_key == METERVALUE_CONDITION) {
            return new Condition::MeterValue(GetEnum< ::MeterType>(), GetDouble1ValueRef(), GetDouble2ValueRef());
        }

        return new Condition::All();
    }

    virtual void Render()
    { GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

private:
    class ConditionRow : public GG::ListBox::Row {
    public:
        ConditionRow(const std::string& key, GG::Y row_height) :
            GG::ListBox::Row(GG::X1, row_height, "ConditionRow"),
            m_condition_key(key)
        {
            SetChildClippingMode(ClipToClient);
            push_back(new GG::TextControl(GG::X0, GG::Y0, UserString(m_condition_key), ClientUI::GetFont(),
                                          ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER));
        }
        const std::string&  GetKey() const { return m_condition_key; }
    private:
        std::string m_condition_key;
    };

    class StringRow : public GG::ListBox::Row  {
    public:
        StringRow(const std::string& text, GG::Y row_height, bool stringtable_lookup = true) :
            GG::ListBox::Row(GG::X1, row_height, "StringRow"),
            m_string(text)
        {
            SetChildClippingMode(ClipToClient);
            const std::string& label = (text.empty() ? EMPTY_STRING :
                (stringtable_lookup ? UserString(text) : text));
            push_back(new GG::TextControl(GG::X0, GG::Y0, label, ClientUI::GetFont(),
                                          ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER));
        }
        const std::string&  Text() const { return m_string; }
    private:
        std::string m_string;
    };

    const std::string&              GetString() {
        if (!m_string_drop)
            return EMPTY_STRING;
        GG::ListBox::iterator row_it = m_string_drop->CurrentItem();
        if (row_it == m_string_drop->end())
            return EMPTY_STRING;
        StringRow* string_row = dynamic_cast<StringRow*>(*row_it);
        if (!string_row)
            return EMPTY_STRING;
        return string_row->Text();
    }

    ValueRef::ValueRefBase<std::string>*                    GetStringValueRef()
    { return new ValueRef::Constant<std::string>(GetString()); }

    std::vector<const ValueRef::ValueRefBase<std::string>*> GetStringValueRefVec() {
        std::vector<const ValueRef::ValueRefBase<std::string>*> retval;
        retval.push_back(GetStringValueRef());
        return retval;
    }

    int                                                     GetInt1() {
        if (m_param_spin1)
            return m_param_spin1->Value();
        else
            return 0;
    }

    ValueRef::ValueRefBase<int>*                            GetInt1ValueRef()
    { return new ValueRef::Constant<int>(GetInt1()); }

    int                                                     GetInt2() {
        if (m_param_spin2)
            return m_param_spin2->Value();
        else
            return 0;
    }

    ValueRef::ValueRefBase<int>*                            GetInt2ValueRef()
    { return new ValueRef::Constant<int>(GetInt2()); }

    double                                                  GetDouble1() {
        if (m_param_spin1)
            return m_param_spin1->Value();
        else
            return 0;
    }

    ValueRef::ValueRefBase<double>*                         GetDouble1ValueRef()
    { return new ValueRef::Constant<double>(GetDouble1()); }

    double                                                  GetDouble2() {
        if (m_param_spin2)
            return m_param_spin2->Value();
        else
            return 0;
    }

    ValueRef::ValueRefBase<double>*                         GetDouble2ValueRef()
    { return new ValueRef::Constant<double>(GetDouble2()); }

    template <typename T>
    T                                                       GetEnum() {
        const std::string& text = GetString();
        T enum_val = T(-1);
        try {
            enum_val = boost::lexical_cast<T>(text);
        } catch (...) {
            Logger().errorStream() << "ConditionWidget::GetEnum unable to convert text to enum type: " << text;
        }
        return enum_val;
    }

    template <typename T>
    ValueRef::ValueRefBase<T>*                              GetEnumValueRef()
    { return new ValueRef::Constant<T>(GetEnum<T>()); }

    template <typename T>
    std::vector<const ValueRef::ValueRefBase<T>*>           GetEnumValueRefVec() {
        std::vector<const ValueRef::ValueRefBase<T>*> retval;
        retval.push_back(GetEnumValueRef<T>());
        return retval;
    }

    GG::X   DropListWidth() const
    { return GG::X(ClientUI::Pts()*16); }

    GG::Y   DropListHeight() const
    { return GG::Y(ClientUI::Pts() + 4); }

    GG::Y   DropListDropHeight() const
    { return DropListHeight() * 10; }

    void    Init(const Condition::ConditionBase* init_condition) {
        // fill droplist with basic types of conditions and select appropriate row
        m_class_drop = new CUIDropDownList(GG::X0, GG::Y0, DropListWidth(),
                                           DropListHeight(), DropListDropHeight());
        m_class_drop->SetStyle(GG::LIST_NOSORT);
        AttachChild(m_class_drop);

        std::vector<std::string> row_keys;
        row_keys.push_back(ALL_CONDITION);              row_keys.push_back(EMPIREAFFILIATION_CONDITION);
        row_keys.push_back(PLANETSIZE_CONDITION);       row_keys.push_back(PLANETTYPE_CONDITION);
        row_keys.push_back(HOMEWORLD_CONDITION);        row_keys.push_back(CAPITAL_CONDITION);
        row_keys.push_back(MONSTER_CONDITION);          row_keys.push_back(ARMED_CONDITION);
        row_keys.push_back(STATIONARY_CONDITION);       row_keys.push_back(CANPRODUCESHIPS_CONDITION);
        row_keys.push_back(CANCOLONIZE_CONDITION);      row_keys.push_back(HASSPECIAL_CONDITION);
        row_keys.push_back(HASTAG_CONDITION);           row_keys.push_back(SPECIES_CONDITION);
        row_keys.push_back(FOCUSTYPE_CONDITION);        row_keys.push_back(STARTYPE_CONDITION);
        row_keys.push_back(METERVALUE_CONDITION);

        SetMinSize(m_class_drop->Size());
        GG::ListBox::iterator select_row_it = m_class_drop->end();
        const std::string& init_condition_key = ConditionClassName(init_condition);

        // fill droplist with rows for the available condition classes to be selected
        for (std::vector<std::string>::const_iterator key_it = row_keys.begin();
             key_it != row_keys.end(); ++key_it)
        {
            const std::string& key = *key_it;
            GG::ListBox::iterator row_it = m_class_drop->Insert(new ConditionRow(key,  GG::Y(ClientUI::Pts())));
            if (init_condition_key == key)
                select_row_it = row_it;
        }

        GG::Connect(m_class_drop->SelChangedSignal, &ConditionWidget::ConditionClassSelected, this);

        if (select_row_it != m_class_drop->end())
            m_class_drop->Select(select_row_it);
        else if (!m_class_drop->Empty())
            m_class_drop->Select(0);

        UpdateParameterControls();

        // TODO: set newly created parameter controls' values based on init condition
    }

    void    ConditionClassSelected(GG::ListBox::iterator iterator)
    { UpdateParameterControls(); }

    void    UpdateParameterControls() {
        if (!m_class_drop)
            return;
        // remove old parameter controls
        delete m_string_drop;   m_string_drop = 0;
        delete m_param_spin1;   m_param_spin1 = 0;
        delete m_param_spin2;   m_param_spin2 = 0;

        // determine which condition is selected
        GG::ListBox::iterator row_it = m_class_drop->CurrentItem();
        if (row_it == m_class_drop->end())
            return;
        ConditionRow* condition_row = dynamic_cast<ConditionRow*>(*row_it);
        if (!condition_row)
            return;
        const std::string& condition_key = condition_row->GetKey();

        GG::X PAD(3);
        GG::X param_widget_left = DropListWidth() + PAD;
        GG::Y param_widget_top = GG::Y0;

        // create controls for selected condition
        if (condition_key == ALL_CONDITION ||
            condition_key == CAPITAL_CONDITION ||
            condition_key == ARMED_CONDITION ||
            condition_key == STATIONARY_CONDITION ||
            condition_key == CANPRODUCESHIPS_CONDITION ||
            condition_key == CANCOLONIZE_CONDITION ||
            condition_key == MONSTER_CONDITION)
        {
            // no params
            param_widget_top += m_class_drop->Height();

        } else if (condition_key == HOMEWORLD_CONDITION ||
                   condition_key == SPECIES_CONDITION)
        {
            // droplist of valid species
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // add empty row, allowing for matching any species
            GG::ListBox::iterator row_it = m_string_drop->Insert(new StringRow("", GG::Y(ClientUI::Pts())));
            m_string_drop->Select(row_it);

            const SpeciesManager& sm = GetSpeciesManager();
            for (SpeciesManager::iterator sp_it = sm.begin(); sp_it != sm.end(); ++sp_it) {
                const std::string species_name = sp_it->first;
                row_it = m_string_drop->Insert(new StringRow(species_name, GG::Y(ClientUI::Pts())));
            }

        } else if (condition_key == HASSPECIAL_CONDITION) {
            // droplist of valid specials
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // add empty row, allowing for matching any special
            GG::ListBox::iterator row_it = m_string_drop->Insert(new StringRow("", GG::Y(ClientUI::Pts())));
            m_string_drop->Select(row_it);

            std::vector<std::string> special_names = SpecialNames();
            for (std::vector<std::string>::iterator sp_it = special_names.begin(); sp_it != special_names.end(); ++sp_it) {
                const std::string special_name = *sp_it;
                row_it = m_string_drop->Insert(new StringRow(special_name, GG::Y(ClientUI::Pts())));
            }

        } else if (condition_key == HASTAG_CONDITION) {
            // droplist of valid tags
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // collect all valid tags on any object in universe
            std::set<std::string> all_tags;
            const ObjectMap& known_objects = Objects();
            for (ObjectMap::const_iterator<> obj_it = known_objects.const_begin();
                 obj_it != known_objects.const_end(); ++obj_it)
            {
                std::vector<std::string> obj_tags = obj_it->Tags();
                std::copy(obj_tags.begin(), obj_tags.end(), std::inserter(all_tags, all_tags.end()));
            }

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (std::set<std::string>::iterator tag_it = all_tags.begin();
                 tag_it != all_tags.end(); ++tag_it)
            {
                const std::string& tag = *tag_it;
                row_it = m_string_drop->Insert(new StringRow(tag, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == PLANETSIZE_CONDITION) {
            // droplist of valid sizes
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            std::vector< ::PlanetSize> planet_sizes;
            for (::PlanetSize size = SZ_TINY; size != NUM_PLANET_SIZES; size = ::PlanetSize(size + 1))
                planet_sizes.push_back(size);
            std::vector<std::string> size_strings = StringsFromEnums(planet_sizes);

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (std::vector<std::string>::iterator string_it = size_strings.begin();
                 string_it != size_strings.end(); ++string_it)
            {
                const std::string& text = *string_it;
                row_it = m_string_drop->Insert(new StringRow(text, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == PLANETTYPE_CONDITION) {
            // droplist of valid types
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);

            std::vector< ::PlanetType> planet_types;
            for (::PlanetType type = PT_SWAMP; type != NUM_PLANET_TYPES; type = ::PlanetType(type + 1))
                planet_types.push_back(type);
            std::vector<std::string> type_strings = StringsFromEnums(planet_types);

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (std::vector<std::string>::iterator string_it = type_strings.begin();
                 string_it != type_strings.end(); ++string_it)
            {
                const std::string& text = *string_it;
                row_it = m_string_drop->Insert(new StringRow(text, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == STARTYPE_CONDITION) {
            // droplist of valid types
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            std::vector< ::StarType> star_types;
            for (::StarType type = STAR_BLUE; type != NUM_STAR_TYPES; type = ::StarType(type + 1))
                star_types.push_back(type);
            std::vector<std::string> type_strings = StringsFromEnums(star_types);

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (std::vector<std::string>::iterator string_it = type_strings.begin();
                 string_it != type_strings.end(); ++string_it)
            {
                const std::string& text = *string_it;
                row_it = m_string_drop->Insert(new StringRow(text, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == FOCUSTYPE_CONDITION) {
            // droplist of valid foci
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // collect all valid foci on any object in universe
            std::set<std::string> all_foci;
            for (ObjectMap::const_iterator<Planet> planet_it = Objects().const_begin<Planet>();
                 planet_it != Objects().const_end<Planet>(); ++planet_it)
            {
                std::vector<std::string> obj_foci = planet_it->AvailableFoci();
                std::copy(obj_foci.begin(), obj_foci.end(), std::inserter(all_foci, all_foci.end()));
            }

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (std::set<std::string>::iterator focus_it = all_foci.begin();
                 focus_it != all_foci.end(); ++focus_it)
            {
                const std::string& focus = *focus_it;
                row_it = m_string_drop->Insert(new StringRow(focus, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == METERVALUE_CONDITION) {
            // droplist of meter types
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);
            param_widget_left = GG::X0;
            param_widget_top = m_string_drop->Height() + GG::Y(Value(PAD));

            std::vector< ::MeterType> meter_types;
            for (::MeterType type = METER_TARGET_POPULATION; type != NUM_METER_TYPES; type = ::MeterType(type + 1))
                meter_types.push_back(type);
            std::vector<std::string> type_strings = StringsFromEnums(meter_types);

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (std::vector<std::string>::iterator string_it = type_strings.begin();
                 string_it != type_strings.end(); ++string_it)
            {
                const std::string& text = *string_it;
                row_it = m_string_drop->Insert(new StringRow(text, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

            m_param_spin1 = new CUISpin<int>(param_widget_left, param_widget_top, DropListWidth(),
                                             0, 1, 0, 1000, true);
            AttachChild(m_param_spin1);
            param_widget_left = DropListWidth() + PAD;

            m_param_spin2 = new CUISpin<int>(param_widget_left, param_widget_top, DropListWidth(),
                                             0, 1, 0, 1000, true);
            AttachChild(m_param_spin2);

            param_widget_top += m_param_spin1->Height();
        } else if (condition_key == EMPIREAFFILIATION_CONDITION) {
            // droplist of empires
            m_string_drop = new CUIDropDownList(param_widget_left, param_widget_top, DropListWidth(),
                                                DropListHeight(), DropListDropHeight());
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // add rows for empire names
            GG::ListBox::iterator row_it = m_string_drop->end();
            const EmpireManager& empires = Empires();
            for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it) {
                const std::string& empire_name = it->second->Name();
                row_it = m_string_drop->Insert(new StringRow(empire_name, GG::Y(ClientUI::Pts()), false));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);
        }

        Resize(GG::Pt(Width(), param_widget_top));
    }

    CUIDropDownList*    m_class_drop;
    CUIDropDownList*    m_string_drop;
    CUISpin<int>*       m_param_spin1;
    CUISpin<int>*       m_param_spin2;
};

////////////////////////////////////////////////
// FilterDialog                               //
////////////////////////////////////////////////
class FilterDialog : public CUIWnd {
public:
    FilterDialog(GG::X x, GG::Y y,
                 const std::map<UniverseObjectType, std::set<VIS_DISPLAY> >& vis_filters,
                 const Condition::ConditionBase* const condition_filter) :
        CUIWnd(UserString("FILTERS"), x, y, GG::X(400), GG::Y(250), GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL),
        m_vis_filters(vis_filters),
        m_accept_changes(false),
        m_filters_layout(0),
        m_cancel_button(0),
        m_apply_button(0)
    { Init(condition_filter); }

    bool    ChangesAccepted()
    { return m_accept_changes; }

    std::map<UniverseObjectType, std::set<VIS_DISPLAY> >    GetVisibilityFilters() const
    { return m_vis_filters; }

    // caller takes ownership of returned ConditionBase*
    Condition::ConditionBase*                               GetConditionFilter()
    { return m_condition_widget->GetCondition(); }

private:
    void    Init(const Condition::ConditionBase* const condition_filter) {
        if (m_filters_layout)
            delete m_filters_layout;
        m_filters_layout = new GG::Layout(GG::X0, GG::Y0, GG::X(390), GG::Y(90), 4, 7);
        AttachChild(m_filters_layout);

        m_filters_layout->SetMinimumColumnWidth(0, GG::X(ClientUI::Pts()*8));
        m_filters_layout->SetColumnStretch(0, 0.0);

        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        GG::Clr color = ClientUI::TextColor();

        m_filters_layout->Add(new GG::TextControl(GG::X0, GG::Y0, UserString("VISIBLE"),              font, color),
                              1, 0, GG::ALIGN_CENTER);
        m_filters_layout->Add(new GG::TextControl(GG::X0, GG::Y0, UserString("PREVIOUSLY_VISIBLE"),   font, color),
                              2, 0, GG::ALIGN_CENTER);
        m_filters_layout->Add(new GG::TextControl(GG::X0, GG::Y0, UserString("DESTROYED"),            font, color),
                              3, 0, GG::ALIGN_CENTER);

        int col = 1;
        for (std::map<UniverseObjectType, std::set<VIS_DISPLAY> >::const_iterator uot_it = m_vis_filters.begin();
             uot_it != m_vis_filters.end(); ++uot_it, ++col)
        {
            const UniverseObjectType& uot = uot_it->first;
            const std::string& uot_label = UserString(GG::GetEnumMap<UniverseObjectType>().FromEnum(uot));
            const std::set<VIS_DISPLAY>& vis_display = uot_it->second;

            m_filters_layout->SetColumnStretch(col, 1.0);

            m_filters_layout->Add(new GG::TextControl(GG::X0, GG::Y0, uot_label, font, color, GG::FORMAT_CENTER),
                                  0, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);

            CUIStateButton* button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, " ", GG::FORMAT_CENTER);
            button->SetCheck(vis_display.find(SHOW_VISIBLE) != vis_display.end());
            m_filters_layout->Add(button, 1, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisfiltersFromStateButtons,    this);

            button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, " ", GG::FORMAT_CENTER);
            button->SetCheck(vis_display.find(SHOW_PREVIOUSLY_VISIBLE) != vis_display.end());
            m_filters_layout->Add(button, 2, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisfiltersFromStateButtons,    this);

            button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, " ", GG::FORMAT_CENTER);
            button->SetCheck(vis_display.find(SHOW_DESTROYED) != vis_display.end());
            m_filters_layout->Add(button, 3, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisfiltersFromStateButtons,    this);
        }


        // TODO: Add multiple condition widgets initialized for input condition
        m_condition_widget = new ConditionWidget(GG::X(3), m_filters_layout->Height() + GG::Y(3));
        AttachChild(m_condition_widget);

        m_cancel_button = new CUIButton(GG::X0, GG::Y0, GG::X(ClientUI::Pts()*8), UserString("CANCEL"), font);
        AttachChild(m_cancel_button);
        GG::Connect(m_cancel_button->LeftClickedSignal, &FilterDialog::CancelClicked,   this);

        m_apply_button = new CUIButton(GG::X0, GG::Y0, GG::X(ClientUI::Pts()*8), UserString("APPLY"), font);
        AttachChild(m_apply_button);
        GG::Connect(m_apply_button->LeftClickedSignal, &FilterDialog::AcceptClicked,   this);

        GG::Pt button_lr = this->ClientSize();
        m_cancel_button->MoveTo(GG::Pt(button_lr.x - m_cancel_button->Width(),
                                       button_lr.y - m_cancel_button->Height()));
        button_lr = button_lr - GG::Pt(m_apply_button->Width() + GG::X(3), GG::Y0);
        m_apply_button->MoveTo(GG::Pt(button_lr.x - m_apply_button->Width(),
                                      button_lr.y - m_apply_button->Height()));
    }

    void    AcceptClicked() {
        m_accept_changes = true;
        m_done = true;
    }

    void    CancelClicked() {
        m_accept_changes = false;
        m_done = true;
    }

    void    UpdateVisfiltersFromStateButtons(bool button_checked) {
        std::vector<std::vector<const GG::Wnd*> > layout_cells = m_filters_layout->Cells();

        int col = 1;
        for (std::map<UniverseObjectType, std::set<VIS_DISPLAY> >::iterator uot_it = m_vis_filters.begin();
             uot_it != m_vis_filters.end(); ++uot_it, ++col)
        {
            const UniverseObjectType& uot = uot_it->first;
            std::set<VIS_DISPLAY>& vis_display = uot_it->second;
            vis_display.clear();

            const GG::Wnd* wnd = layout_cells[1][col];
            const CUIStateButton* button = dynamic_cast<const CUIStateButton*>(wnd);
            if (button && button->Checked())
                m_vis_filters[uot].insert(SHOW_VISIBLE);

            wnd = layout_cells[2][col];
            button = dynamic_cast<const CUIStateButton*>(wnd);
            if (button && button->Checked())
                m_vis_filters[uot].insert(SHOW_PREVIOUSLY_VISIBLE);

            wnd = layout_cells[3][col];
            button = dynamic_cast<const CUIStateButton*>(wnd);
            if (button && button->Checked())
                m_vis_filters[uot].insert(SHOW_DESTROYED);
        }
    }

    std::map<UniverseObjectType, std::set<VIS_DISPLAY> >    m_vis_filters;
    bool                                                    m_accept_changes;

    ConditionWidget*    m_condition_widget;
    GG::Layout*         m_filters_layout;
    GG::Button*         m_cancel_button;
    GG::Button*         m_apply_button;
};

namespace {
    std::vector<boost::shared_ptr<GG::Texture> > ObjectTextures(const UniverseObject* obj) {
        std::vector<boost::shared_ptr<GG::Texture> > retval;

        if (obj->ObjectType() == OBJ_SHIP) {
            const Ship* ship = universe_object_cast<const Ship*>(obj);
            if (ship) {
                if (const ShipDesign* design = ship->Design())
                    retval.push_back(ClientUI::ShipDesignIcon(design->ID()));
            }
            if (retval.empty()) {
                retval.push_back(ClientUI::ShipDesignIcon(INVALID_OBJECT_ID));  // default icon
            }
        } else if (obj->ObjectType() == OBJ_FLEET) {
            const Fleet* fleet = universe_object_cast<const Fleet*>(obj);
            if (fleet) {
                boost::shared_ptr<GG::Texture> head_icon = FleetHeadIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
                if (head_icon)
                    retval.push_back(head_icon);
                boost::shared_ptr<GG::Texture> size_icon = FleetSizeIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
                if (size_icon)
                    retval.push_back(size_icon);
            }
        } else if (obj->ObjectType() == OBJ_SYSTEM) {
            const System* system = universe_object_cast<const System*>(obj);
            if (system) {
                StarType star_type = system->GetStarType();
                ClientUI* ui = ClientUI::GetClientUI();
                boost::shared_ptr<GG::Texture> disc_texture = ui->GetModuloTexture(
                    ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[star_type], system->ID());
                if (disc_texture)
                    retval.push_back(disc_texture);
                boost::shared_ptr<GG::Texture> halo_texture = ui->GetModuloTexture(
                    ClientUI::ArtDir() / "stars", ClientUI::HaloStarTypeFilePrefixes()[star_type], system->ID());
                if (halo_texture)
                    retval.push_back(halo_texture);
            }
        } else if (obj->ObjectType() == OBJ_PLANET) {
            if (const Planet* planet = universe_object_cast<const Planet*>(obj))
                retval.push_back(ClientUI::PlanetIcon(planet->Type()));

        } else if (obj->ObjectType() == OBJ_BUILDING) {
            const Building* building = universe_object_cast<const Building*>(obj);
            if (building)
                retval.push_back(ClientUI::BuildingIcon(building->BuildingTypeName()));
            else
                retval.push_back(ClientUI::BuildingIcon(""));   // default building icon
        }
        if (retval.empty())
            retval.push_back(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "generic_object.png", true));
        return retval;
    }

    const std::string& ObjectName(const UniverseObject* obj) {
        if (!obj)
            return EMPTY_STRING;
        if (obj->ObjectType() == OBJ_SYSTEM) {
            if (const System* system = universe_object_cast<const System*>(obj))
                return system->ApparentName(HumanClientApp::GetApp()->EmpireID());
        }
        return obj->PublicName(HumanClientApp::GetApp()->EmpireID());
    }

    std::pair<std::string, GG::Clr> ObjectEmpireNameAndColour(const UniverseObject* obj) {
        if (!obj)
            return std::make_pair("", ClientUI::TextColor());
        if (const Empire* empire = Empires().Lookup(obj->Owner()))
            return std::make_pair(empire->Name(), empire->Color());
        return std::make_pair("", ClientUI::TextColor());
    }
}

////////////////////////////////////////////////
// ObjectPanel
////////////////////////////////////////////////
class ObjectPanel : public GG::Control {
public:
    ObjectPanel(GG::X w, GG::Y h, const UniverseObject* obj, bool expanded, bool has_contents, int indent = 0) :
        Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
        m_initialized(false),
        m_object_id(obj ? obj->ID() : INVALID_OBJECT_ID),
        m_indent(indent),
        m_expanded(expanded),
        m_has_contents(has_contents),
        m_expand_button(0),
        m_dot(0),
        m_icon(0),
        m_name_label(0),
        m_empire_label(0)
    {
        SetChildClippingMode(ClipToClient);
    }

    int                 ObjectID() const { return m_object_id; }

    virtual void        Render() {
        if (!m_initialized)
            Init();
        GG::Clr background_clr = this->Disabled() ? ClientUI::WndColor() : ClientUI::CtrlColor();
        GG::FlatRectangle(UpperLeft(), LowerRight(), background_clr, ClientUI::WndOuterBorderColor(), 1u);
    }

    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        GG::Control::SizeMove(ul, lr);
        if (old_size != Size())
            DoLayout();
    }

    void                Refresh() {
        if (!m_initialized)
            return;
        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        GG::Clr clr = ClientUI::TextColor();
        //int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE;

        delete m_dot;           m_dot = 0;
        delete m_expand_button; m_expand_button = 0;
        delete m_icon;          m_icon = 0;
        delete m_name_label;    m_name_label = 0;

        if (m_has_contents) {
            m_expand_button = new GG::Button(GG::X0, GG::Y0, GG::X(16), GG::Y(16),
                                                "", font, GG::CLR_WHITE, GG::CLR_ZERO, GG::ONTOP | GG::INTERACTIVE);
            AttachChild(m_expand_button);
            GG::Connect(m_expand_button->LeftClickedSignal, &ObjectPanel::ExpandCollapseButtonPressed, this);

            if (m_expanded) {
                m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "minusnormal.png"     , true)));
                m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "minusclicked.png"    , true)));
                m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "minusmouseover.png"  , true)));
            } else {
                m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "plusnormal.png"   , true)));
                m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "plusclicked.png"  , true)));
                m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "plusmouseover.png", true)));
            }
        } else {
            m_dot = new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(16), GG::Y(16),
                                          ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "dot.png", true),
                                          style, GG::Flags<GG::WndFlag>());
            AttachChild(m_dot);
        }

        const UniverseObject* obj = GetUniverseObject(m_object_id);
        std::vector<boost::shared_ptr<GG::Texture> > textures = ObjectTextures(obj);

        m_icon = new MultiTextureStaticGraphic(GG::X0, GG::Y0, GG::X(Value(ClientHeight())), ClientHeight(),
                                                textures, std::vector<GG::Flags<GG::GraphicStyle> >(textures.size(), style));
        AttachChild(m_icon);

        m_name_label = new GG::TextControl(GG::X0, GG::Y0, GG::X(Value(ClientHeight())), ClientHeight(), ObjectName(obj), font, clr, GG::FORMAT_LEFT);
        AttachChild(m_name_label);

        std::pair<std::string, GG::Clr> empire_and_colour = ObjectEmpireNameAndColour(obj);
        m_empire_label = new GG::TextControl(GG::X0, GG::Y0, GG::X(Value(ClientHeight())), ClientHeight(), empire_and_colour.first, font, empire_and_colour.second, GG::FORMAT_LEFT);
        AttachChild(m_empire_label);

        DoLayout();
    }

    void                SetHasContents(bool has_contents)
    { m_has_contents = has_contents; }

    mutable boost::signal<void ()>  ExpandCollapseSignal;
private:
    void                DoLayout() {
        if (!m_initialized)
            return;

        const GG::Y ICON_HEIGHT(ClientHeight());
        const GG::X ICON_WIDTH(Value(ClientHeight()));

        GG::X indent(ICON_WIDTH * m_indent);
        GG::X left = indent;
        GG::Y top(GG::Y0);
        GG::Y bottom(ClientHeight());
        GG::X PAD(3);

        GG::X ctrl_width = ICON_WIDTH;

        if (m_expand_button) {
            m_expand_button->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        } else if (m_dot) {
            m_dot->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        }
        left += ctrl_width + PAD;

        m_icon->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        left += ctrl_width + PAD;

        ctrl_width = GG::X(ClientUI::Pts()*14) - indent;    // so second column all line up
        m_name_label->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        left += ctrl_width + PAD;

        ctrl_width = GG::X(ClientUI::Pts()*8);
        m_empire_label->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        left += ctrl_width + PAD;
    }

    void                ExpandCollapseButtonPressed() {
        m_expanded = !m_expanded;
        ExpandCollapseSignal();
    }

    void                Init() {
        if (m_initialized)
            return;
        m_initialized = true;
        Refresh();
    }

    bool                        m_initialized;

    int                         m_object_id;
    int                         m_indent;
    bool                        m_expanded;
    bool                        m_has_contents;

    GG::Button*                 m_expand_button;
    GG::StaticGraphic*          m_dot;
    MultiTextureStaticGraphic*  m_icon;
    GG::TextControl*            m_name_label;
    GG::TextControl*            m_empire_label;
};

////////////////////////////////////////////////
// ObjectRow
////////////////////////////////////////////////
class ObjectRow : public GG::ListBox::Row {
public:
    ObjectRow(GG::X w, GG::Y h, const UniverseObject* obj, bool expanded,
              int container_object_panel,
              const std::set<int>& contained_object_panels, int indent) :
        GG::ListBox::Row(w, h, "ObjectRow", GG::ALIGN_CENTER, 1),
        m_panel(0),
        m_container_object_panel(container_object_panel),
        m_contained_object_panels(contained_object_panels)
    {
        SetName("ObjectRow");
        SetChildClippingMode(ClipToClient);
        SetDragDropDataType("ObjectRow");
        m_panel = new ObjectPanel(w, h, obj, expanded, !m_contained_object_panels.empty(), indent);
        push_back(m_panel);
        GG::Connect(m_panel->ExpandCollapseSignal,  &ObjectRow::ExpandCollapseClicked, this);
    }

    int                     ObjectID() const {
        if (m_panel)
            return m_panel->ObjectID();
        return INVALID_OBJECT_ID;
    }

    int                     ContainedByPanel() const
    { return m_container_object_panel; }

    const std::set<int>& ContainedPanels() const
    { return m_contained_object_panels; }

    void                    SetContainedPanels(const std::set<int>& contained_object_panels) {
        m_contained_object_panels = contained_object_panels;
        m_panel->SetHasContents(!m_contained_object_panels.empty());
        m_panel->Refresh();
    }

    void                    Update()
    { m_panel->Refresh(); }

    /** This function overridden because otherwise, rows don't expand
        * larger than their initial size when resizing the list. */
    void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        GG::ListBox::Row::SizeMove(ul, lr);
        //std::cout << "ObjectRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (!empty() && old_size != Size() && m_panel)
            m_panel->Resize(Size());
    }

    void                    ExpandCollapseClicked()
    { ExpandCollapseSignal(m_panel ? m_panel->ObjectID() : INVALID_OBJECT_ID); }

    mutable boost::signal<void (int)>   ExpandCollapseSignal;
private:
    ObjectPanel*        m_panel;
    int                 m_container_object_panel;
    std::set<int>       m_contained_object_panels;
};

////////////////////////////////////////////////
// ObjectListBox
////////////////////////////////////////////////
class ObjectListBox : public CUIListBox {
public:
    ObjectListBox() :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_object_change_connections(),
        m_collapsed_objects(),
        m_filter_condition(0),
        m_visibilities()
    {
        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        SetNumCols(1);
        SetColWidth(0, GG::X0);
        LockColWidths();

        m_filter_condition = new Condition::All();

        //m_visibilities[OBJ_BUILDING].insert(SHOW_VISIBLE);
        //m_visibilities[OBJ_BUILDING].insert(SHOW_PREVIOUSLY_VISIBLE);
        //m_visibilities[OBJ_SHIP].insert(SHOW_VISIBLE);
        //m_visibilities[OBJ_FLEET].insert(SHOW_VISIBLE);
        m_visibilities[OBJ_PLANET].insert(SHOW_VISIBLE);
        m_visibilities[OBJ_PLANET].insert(SHOW_PREVIOUSLY_VISIBLE);
        //m_visibilities[OBJ_SYSTEM].insert(SHOW_VISIBLE);
        //m_visibilities[OBJ_SYSTEM].insert(SHOW_PREVIOUSLY_VISIBLE);
        //m_visibilities[OBJ_FIELD].insert(SHOW_VISIBLE);

        GG::Connect(GetUniverse().UniverseObjectDeleteSignal,   &ObjectListBox::UniverseObjectDeleted,  this);
    }

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "ObjectListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "ObjectListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (GG::ListBox::iterator it = begin(); it != end(); ++it)
                (*it)->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const
    { return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight()); }

    static GG::Y    ListRowHeight()
    { return GG::Y(ClientUI::Pts() * 2); }

    const Condition::ConditionBase* const                       FilterCondition() const
    { return m_filter_condition; }

    const std::map<UniverseObjectType, std::set<VIS_DISPLAY> >  Visibilities() const
    { return m_visibilities; }

    void            CollapseObject(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID) {
            for (GG::ListBox::iterator row_it = this->begin(); row_it != this->end(); ++row_it)
                if (const ObjectRow* object_row = dynamic_cast<const ObjectRow*>(*row_it))
                    m_collapsed_objects.insert(object_row->ObjectID());
        } else {
            m_collapsed_objects.insert(object_id);
        }
        Refresh();
    }

    void            ExpandObject(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID) {
            m_collapsed_objects.clear();
        } else {
            m_collapsed_objects.erase(object_id);
        }
        Refresh();
    }

    bool            ObjectCollapsed(int object_id) const {
        if (object_id == INVALID_OBJECT_ID)
            return false;
        return m_collapsed_objects.find(object_id) != m_collapsed_objects.end();
    }

    bool            AnythingCollapsed() const
    { return !m_collapsed_objects.empty(); }

    void            SetFilterCondition(Condition::ConditionBase* condition) {
        m_filter_condition = condition;
        Refresh();
    }

    void            SetVisibilityFilters(const std::map<UniverseObjectType, std::set<VIS_DISPLAY> >& vis) {
        if (vis != m_visibilities) {
            m_visibilities = vis;
            Refresh();
        }
    }

    void            ClearContents() {
        Clear();
        for (std::map<int, boost::signals::connection>::iterator it = m_object_change_connections.begin();
             it != m_object_change_connections.end(); ++it)
        { it->second.disconnect(); }
        m_object_change_connections.clear();
    }

    bool            ObjectShown(int object_id, UniverseObjectType type, bool assume_visible_without_checking = false) {
        if (object_id == INVALID_OBJECT_ID)
            return false;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        const UniverseObject* obj = GetUniverseObject(object_id);
        if (!obj)
            return false;

        if (!m_filter_condition->Eval(obj))
            return false;

        if (GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id).find(object_id) != GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id).end())
            return m_visibilities[type].find(SHOW_DESTROYED) != m_visibilities[type].end();

        if (assume_visible_without_checking || GetUniverse().GetObjectVisibilityByEmpire(object_id, client_empire_id))
            return m_visibilities[type].find(SHOW_VISIBLE) != m_visibilities[type].end();

        return m_visibilities[type].find(SHOW_PREVIOUSLY_VISIBLE) != m_visibilities[type].end();
    }

    void            Refresh() {
        std::size_t first_visible_queue_row = std::distance(this->begin(), this->FirstRowShown());
        ClearContents();

        const ObjectMap& objects = GetUniverse().Objects();
        bool nested = true;

        if (!nested) {
        } else {
            // sort objects by containment associations
            std::set<int>                   systems;
            std::map<int, std::set<int> >   system_fleets;
            std::map<int, std::set<int> >   fleet_ships;
            std::map<int, std::set<int> >   system_planets;
            std::map<int, std::set<int> >   planet_buildings;
            std::set<int>                   fields;

            for (ObjectMap::const_iterator<> it = objects.const_begin(); it != objects.const_end(); ++it) {
                int object_id = it->ID();
                const UniverseObject* obj = *it;

                if (obj->ObjectType() == OBJ_SYSTEM) {
                    if (ObjectShown(object_id, OBJ_SYSTEM, true))
                        systems.insert(object_id);

                } else if (obj->ObjectType() == OBJ_FIELD) {
                    if (ObjectShown(object_id, OBJ_FIELD, true))
                        fields.insert(object_id);

                } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(obj)) {
                    if (ObjectShown(object_id, OBJ_FLEET, true))
                        system_fleets[fleet->SystemID()].insert(object_id);

                } else if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
                    if (ObjectShown(object_id, OBJ_SHIP, true))
                        fleet_ships[ship->FleetID()].insert(object_id);

                } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
                    if (ObjectShown(object_id, OBJ_PLANET, true))
                        system_planets[planet->SystemID()].insert(object_id);

                } else if (const Building* building = universe_object_cast<const Building*>(obj)) {
                    if (ObjectShown(object_id, OBJ_BUILDING, true))
                        planet_buildings[building->PlanetID()].insert(object_id);
                }
            }

            int indent = 0;

            // add system rows
            for (std::set<int>::const_iterator sys_it = systems.begin(); sys_it != systems.end(); ++sys_it) {
                int system_id = *sys_it;

                std::map<int, std::set<int> >::iterator sp_it = system_planets.find(system_id);
                std::map<int, std::set<int> >::iterator sf_it = system_fleets.find(system_id);
                std::set<int> system_contents;
                if (sp_it != system_planets.end())
                    system_contents = sp_it->second;
                if (sf_it != system_fleets.end())
                    system_contents.insert(sf_it->second.begin(), sf_it->second.end());

                AddObjectRow(system_id, INVALID_OBJECT_ID, system_contents, indent);
                ++indent;

                // add planet rows in this system
                if (sp_it != system_planets.end()) {
                    const std::set<int>& planets = sp_it->second;
                    for (std::set<int>::const_iterator planet_it = planets.begin(); planet_it != planets.end(); ++planet_it) {
                        int planet_id = *planet_it;

                        std::map<int, std::set<int> >::iterator pb_it = planet_buildings.find(planet_id);

                        if (!ObjectCollapsed(system_id)) {
                            AddObjectRow(planet_id, system_id,
                                         pb_it != planet_buildings.end() ? pb_it->second : std::set<int>(),
                                         indent);
                            ++indent;
                        }

                        // add building rows on this planet
                        if (pb_it != planet_buildings.end()) {
                            if (!ObjectCollapsed(planet_id) && !ObjectCollapsed(system_id)) {
                                const std::set<int>& buildings = pb_it->second;
                                for (std::set<int>::iterator building_it = buildings.begin(); building_it != buildings.end(); ++building_it) {
                                    AddObjectRow(*building_it, planet_id, std::set<int>(), indent);
                                }
                            }
                            planet_buildings.erase(pb_it);
                        }

                        if (!ObjectCollapsed(system_id))
                            indent--;
                    }
                    system_planets.erase(sp_it);
                }

                // add fleet rows in this system
                if (sf_it != system_fleets.end()) {
                    const std::set<int>& fleets = sf_it->second;
                    for (std::set<int>::const_iterator fleet_it = fleets.begin(); fleet_it != fleets.end(); ++fleet_it) {
                        int fleet_id = *fleet_it;

                        std::map<int, std::set<int> >::iterator fs_it = fleet_ships.find(fleet_id);

                        if (!ObjectCollapsed(system_id)) {
                            AddObjectRow(fleet_id, system_id, 
                                         fs_it != fleet_ships.end() ? fs_it->second : std::set<int>(),
                                         indent);
                            ++indent;
                        }

                        // add ship rows in this fleet
                        if (fs_it != fleet_ships.end()) {
                            if (!ObjectCollapsed(fleet_id) && !ObjectCollapsed(system_id)) {
                                const std::set<int>& ships = fs_it->second;
                                for (std::set<int>::const_iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
                                    AddObjectRow(*ship_it, fleet_id, std::set<int>(), indent);
                                }
                            }
                            fleet_ships.erase(fs_it);
                        }

                        if (!ObjectCollapsed(system_id))
                            indent--;
                    }
                    system_fleets.erase(sf_it);
                }

                indent--;
            }

            // add planets not in shown systems
            for (std::map<int, std::set<int> >::iterator sp_it = system_planets.begin();
                 sp_it != system_planets.end(); ++sp_it)
            {
                const std::set<int>& planets = sp_it->second;
                for (std::set<int>::iterator planet_it = planets.begin(); planet_it != planets.end(); ++planet_it) {
                    int planet_id = *planet_it;

                    std::map<int, std::set<int> >::iterator pb_it = planet_buildings.find(planet_id);

                    AddObjectRow(planet_id, INVALID_OBJECT_ID,
                                 pb_it != planet_buildings.end() ? pb_it->second : std::set<int>(),
                                 indent);
                    ++indent;

                    // add building rows on this planet
                    if (pb_it != planet_buildings.end()) {
                        if (!ObjectCollapsed(planet_id)) {
                            const std::set<int>& buildings = pb_it->second;
                            for (std::set<int>::iterator building_it = buildings.begin(); building_it != buildings.end(); ++building_it) {
                                AddObjectRow(*building_it, planet_id, std::set<int>(), indent);
                            }
                        }
                        planet_buildings.erase(pb_it);
                    }

                    --indent;
                }
            }
            system_planets.clear();

            // add buildings not in a shown planet
            for (std::map<int, std::set<int> >::iterator pb_it = planet_buildings.begin();
                 pb_it != planet_buildings.end(); ++pb_it)
            {
                const std::set<int>& buildings = pb_it->second;
                for (std::set<int>::const_iterator building_it = buildings.begin(); building_it != buildings.end(); ++building_it) {
                    AddObjectRow(*building_it, INVALID_OBJECT_ID, std::set<int>(), indent);
                }
            }
            planet_buildings.clear();

            // add fleets not in shown systems
            for (std::map<int, std::set<int> >::iterator sf_it = system_fleets.begin();
                 sf_it != system_fleets.end(); ++sf_it)
            {
                const std::set<int>& fleets = sf_it->second;
                for (std::set<int>::iterator fleet_it = fleets.begin(); fleet_it != fleets.end(); ++fleet_it) {
                    int fleet_id = *fleet_it;

                    std::map<int, std::set<int> >::iterator fs_it = fleet_ships.find(fleet_id);

                    AddObjectRow(fleet_id, INVALID_OBJECT_ID,
                                 fs_it != fleet_ships.end() ? fs_it->second : std::set<int>(),
                                 indent);
                    ++indent;

                    // add ship rows on this fleet
                    if (fs_it != fleet_ships.end()) {
                        if (!ObjectCollapsed(fleet_id)) {
                            const std::set<int>& ships = fs_it->second;
                            for (std::set<int>::iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
                                AddObjectRow(*ship_it, fleet_id, std::set<int>(), indent);
                            }
                        }
                        fleet_ships.erase(fs_it);
                    }
                    indent--;
                }
            }
            system_fleets.clear();

            // add any remaining ships not in shown fleets
            for (std::map<int, std::set<int> >::iterator fs_it = fleet_ships.begin();
                 fs_it != fleet_ships.end(); ++fs_it)
            {
                const std::set<int>& ships = fs_it->second;
                for (std::set<int>::iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
                    AddObjectRow(*ship_it, INVALID_OBJECT_ID, std::set<int>(), indent);
                }
            }
            fleet_ships.clear();

            for (std::set<int>::iterator fld_it = fields.begin(); fld_it != fields.end(); ++fld_it)
                AddObjectRow(*fld_it, INVALID_OBJECT_ID, std::set<int>(), indent);
        }


        if (!this->Empty())
            this->BringRowIntoView(--this->end());
        if (first_visible_queue_row < this->NumRows())
            this->BringRowIntoView(boost::next(this->begin(), first_visible_queue_row));
    }

    void            UpdateObjectPanel(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        for (GG::ListBox::iterator it = this->begin(); it != this->end(); ++it) {
            if (ObjectRow* row = dynamic_cast<ObjectRow*>(*it)) {
                row->Update();
                return;
            }
        }
    }

    mutable boost::signal<void ()> ExpandCollapseSignal;

private:
    void            AddObjectRow(int object_id, int container, const std::set<int>& contents, int indent)
    { AddObjectRow(object_id, container, contents, indent, this->end()); }

    void            AddObjectRow(int object_id, int container, const std::set<int>& contents,
                                 int indent, GG::ListBox::iterator it)
    {
        const UniverseObject* obj = GetUniverseObject(object_id);
        if (!obj)
            return;
        const GG::Pt row_size = ListRowSize();
        ObjectRow* object_row = new ObjectRow(row_size.x, row_size.y, obj, !ObjectCollapsed(object_id),
                                              container, contents, indent);
        this->Insert(object_row, it);
        object_row->Resize(row_size);
        GG::Connect(object_row->ExpandCollapseSignal,   &ObjectListBox::ObjectExpandCollapseClicked,    this, boost::signals::at_front);
        m_object_change_connections[obj->ID()].disconnect();
        m_object_change_connections[obj->ID()] = GG::Connect(obj->StateChangedSignal, boost::bind(&ObjectListBox::ObjectStateChanged, this, obj->ID()), boost::signals::at_front);
    }

    // Removes row of indicated object, and all contained rows, recursively.
    // Also updates contents tracking of containing row, if any.
    void            RemoveObjectRow(int object_id) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        int container_object_id = INVALID_OBJECT_ID;
        for (GG::ListBox::iterator it = this->begin(); it != this->end(); ++it) {
            if (ObjectRow* object_row = dynamic_cast<ObjectRow*>(*it)) {
                if (object_row->ObjectID() == object_id) {
                    container_object_id = object_row->ContainedByPanel();
                    RemoveObjectRow(it);
                    break;
                }
            }
        }

        if (container_object_id == INVALID_OBJECT_ID)
            return;

        // remove this row from parent row's contents
        for (GG::ListBox::iterator it = this->begin(); it != this->end(); ++it) {
            if (ObjectRow* object_row = dynamic_cast<ObjectRow*>(*it)) {
                if (object_row->ObjectID() == container_object_id) {
                    const std::set<int>& contents = object_row->ContainedPanels();
                    std::set<int> new_contents;
                    for (std::set<int>::const_iterator contents_it = contents.begin();
                         contents_it != contents.end(); ++contents_it)
                    {
                        if (*contents_it != object_id)
                            new_contents.insert(*contents_it);
                    }
                    object_row->SetContainedPanels(new_contents);
                    object_row->Update();
                    break;
                }
            }
        }
    }

    // Removes row at indicated iterator location and its contained rows.
    // Does not adjust containing row.
    void            RemoveObjectRow(GG::ListBox::iterator it) {
        if (it == this->end())
            return;
        ObjectRow* object_row = dynamic_cast<ObjectRow*>(*it);

        // recursively remove contained rows first
        const std::set<int>& contents = object_row->ContainedPanels();
        for (unsigned int i = 0; i < contents.size(); ++i) {
            GG::ListBox::iterator next_it = it; ++next_it;
            if (next_it == this->end())
                break;
            ObjectRow* contained_row = dynamic_cast<ObjectRow*>(*next_it);
            if (!contained_row)
                continue;
            // remove only rows that are contained by this row
            if (contained_row->ContainedByPanel() != object_row->ObjectID())
                break;
            RemoveObjectRow(next_it);
        }

        // erase this row and remove any signals related to it
        m_object_change_connections[object_row->ObjectID()].disconnect();
        m_object_change_connections.erase(object_row->ObjectID());
        this->Erase(it);
    }

    void            ObjectExpandCollapseClicked(int object_id) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        if (ObjectCollapsed(object_id))
            ExpandObject(object_id);
        else
            CollapseObject(object_id);
        ExpandCollapseSignal();
    }

    void            ObjectStateChanged(int object_id) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        const UniverseObject* obj = GetUniverseObject(object_id);
        Logger().debugStream() << "ObjectListBox::ObjectStateChanged: " << obj->Name();
        if (!obj)
            return;

        UniverseObjectType type = obj->ObjectType();
        if (type == OBJ_SHIP || type == OBJ_BUILDING)
            UpdateObjectPanel(object_id);
        else if (type == OBJ_FLEET || type == OBJ_PLANET || type == OBJ_SYSTEM)
            Refresh();
    }

    void            UniverseObjectDeleted(const UniverseObject* obj) {
        if (obj)
            RemoveObjectRow(obj->ID());
    }

    std::map<int, boost::signals::connection>           m_object_change_connections;
    std::set<int>                                       m_collapsed_objects;
    Condition::ConditionBase*                           m_filter_condition;
    std::map<UniverseObjectType, std::set<VIS_DISPLAY> >m_visibilities;
};

////////////////////////////////////////////////
// ObjectListWnd
////////////////////////////////////////////////
ObjectListWnd::ObjectListWnd(GG::X w, GG::Y h) :
    CUIWnd(UserString("MAP_BTN_OBJECTS"), GG::X1, GG::Y1, w - 1, h - 1, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE),
    m_list_box(0),
    m_filter_button(0),
    m_sort_button(0),
    m_columns_button(0),
    m_collapse_button(0)
{
    m_list_box = new ObjectListBox();
    m_list_box->SetHiliteColor(GG::CLR_ZERO);
    m_list_box->SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);
    GG::Connect(m_list_box->DoubleClickedSignal,        &ObjectListWnd::ObjectDoubleClicked,    this);
    GG::Connect(m_list_box->RightClickedSignal,         &ObjectListWnd::ObjectRightClicked,     this);
    GG::Connect(m_list_box->ExpandCollapseSignal,       &ObjectListWnd::DoLayout,               this);
    AttachChild(m_list_box);

    m_filter_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("FILTERS"));
    GG::Connect(m_filter_button->LeftClickedSignal,     &ObjectListWnd::FilterClicked,          this);
    AttachChild(m_filter_button);

    m_sort_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("SORT"));
    GG::Connect(m_sort_button->LeftClickedSignal,       &ObjectListWnd::SortClicked,            this);
    AttachChild(m_sort_button);
    m_sort_button->Disable();

    m_columns_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("COLUMNS"));
    GG::Connect(m_columns_button->LeftClickedSignal,    &ObjectListWnd::ColumnsClicked,         this);
    AttachChild(m_columns_button);
    m_columns_button->Disable();

    m_collapse_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("COLLAPSE_ALL"));
    GG::Connect(m_collapse_button->LeftClickedSignal,   &ObjectListWnd::CollapseExpandClicked,  this);
    AttachChild(m_collapse_button);

    DoLayout();
}

void ObjectListWnd::DoLayout() {
    GG::X BUTTON_WIDTH(ClientUI::Pts()*7);
    GG::Y BUTTON_HEIGHT = m_filter_button->Height();
    int PAD(3);

    GG::Pt button_ul(GG::X0, ClientHeight() - BUTTON_HEIGHT);

    m_filter_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_sort_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_columns_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_collapse_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);

    m_list_box->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(ClientWidth(), button_ul.y));

    SetMinSize(GG::Pt(5*BUTTON_WIDTH, 6*BUTTON_HEIGHT));

    if (m_list_box->AnythingCollapsed())
        m_collapse_button->SetText(UserString("EXPAND_ALL"));
    else
        m_collapse_button->SetText(UserString("COLLAPSE_ALL"));
}

void ObjectListWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void ObjectListWnd::Refresh()
{ m_list_box->Refresh(); }

void ObjectListWnd::ObjectDoubleClicked(GG::ListBox::iterator it) {
    int object_id = ObjectInRow(it);
    if (object_id != INVALID_OBJECT_ID)
        ObjectDoubleClickedSignal(object_id);
    ClientUI::GetClientUI()->ZoomToObject(object_id);
}

void ObjectListWnd::ObjectRightClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    int object_id = ObjectInRow(it);
    if (object_id == INVALID_OBJECT_ID)
        return;
    HumanClientApp* app = HumanClientApp::GetApp();
    ClientNetworking& net = app->Networking();
    bool moderator = false;
    if (app->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        moderator = true;


    // create popup menu with object commands in it
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("DUMP"), 1, false, false));

    // moderator actions...
    if (moderator) {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("MOD_DESTROY"),      10, false, false));
        menu_contents.next_level.push_back(GG::MenuItem(UserString("MOD_SET_OWNER"),    11, false, false));
    }

    // run popup and respond
    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor());
    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: {
            ObjectDumpSignal(object_id);
            break;
        }
        case 10: {
            net.SendMessage(ModeratorActionMessage(app->PlayerID(), Moderator::DestroyUniverseObject(object_id)));
            break;
        }
        case 11: {
            net.SendMessage(ModeratorActionMessage(app->PlayerID(), Moderator::SetOwner(object_id, ALL_EMPIRES)));
            break;
        }
        default:
            break;
        }
    }
}

int ObjectListWnd::ObjectInRow(GG::ListBox::iterator it) const {
    if (it == m_list_box->end())
        return INVALID_OBJECT_ID;

    if (ObjectRow* obj_row = dynamic_cast<ObjectRow*>(*it))
        return obj_row->ObjectID();

    return INVALID_OBJECT_ID;
}

void ObjectListWnd::FilterClicked() {
    FilterDialog dlg(GG::X(100), GG::Y(100),
                     m_list_box->Visibilities(), m_list_box->FilterCondition());
    dlg.Run();

    if (dlg.ChangesAccepted()) {
        m_list_box->SetVisibilityFilters(dlg.GetVisibilityFilters());
        m_list_box->SetFilterCondition(dlg.GetConditionFilter());
    }
}

void ObjectListWnd::SortClicked() {
}

void ObjectListWnd::ColumnsClicked() {
}

void ObjectListWnd::CollapseExpandClicked() {
    if (m_list_box->AnythingCollapsed())
        m_list_box->ExpandObject(INVALID_OBJECT_ID);
    else
        m_list_box->CollapseObject(INVALID_OBJECT_ID);
    DoLayout();
}

void ObjectListWnd::CloseClicked()
{ ClosingSignal(); }

