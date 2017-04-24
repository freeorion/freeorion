#include "ObjectListWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUISpin.h"
#include "FleetButton.h"
#include "../client/human/HumanClientApp.h"
#include "../network/ClientNetworking.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/ModeratorAction.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/System.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/Field.h"
#include "../universe/Species.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>

#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>

#include <iterator>
#include <sstream>

std::vector<std::string> SpecialNames();

namespace {
    const unsigned int NUM_COLUMNS(12u);

    void AddOptions(OptionsDB& db) {
        std::vector<std::pair<std::string, int>> default_columns_widths = {
            {"NAME",             12*12},    {"ID",               4*12},
            {"OBJECT_TYPE",      5*12},     {"OWNER",            10*12},
            {"SPECIES",          8*12},     {"PLANET_TYPE",      8*12},
            {"SIZE_AS_DOUBLE",   8*12}};

        for (unsigned int i = default_columns_widths.size(); i < NUM_COLUMNS; ++i)
            default_columns_widths.push_back({"", 8*12});   // arbitrary default width

        for (unsigned int i = 0; i < default_columns_widths.size(); ++i) {
            db.Add<std::string>("UI.objects-list-info-col-" + std::to_string(i),
                                UserStringNop("OPTIONS_DB_OBJECTS_LIST_COLUMN_INFO"),
                                default_columns_widths[i].first);
            db.Add<int>("UI.objects-list-width-col-" + std::to_string(i),
                        UserStringNop("OPTIONS_DB_OBJECTS_LIST_COLUMN_WIDTH"),
                        default_columns_widths[i].second,
                        RangedValidator<int>(1, 200));
        }
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    ValueRef::Variable<std::string>* StringValueRef(const std::string& token) {
        return new ValueRef::Variable<std::string>(
            ValueRef::SOURCE_REFERENCE, token);
    }

    ValueRef::Variable<std::string>* UserStringValueRef(const std::string& token) {
        return new ValueRef::UserStringLookup<std::string>(
            new ValueRef::Variable<std::string>(
                ValueRef::SOURCE_REFERENCE, token));
    }

    ValueRef::Variable<std::string>* UserStringVecValueRef(const std::string& token) {
        return new ValueRef::UserStringLookup<std::vector<std::string>>(
            new ValueRef::Variable<std::vector<std::string>>(
                ValueRef::SOURCE_REFERENCE, token));
    }

    template <typename T>
    ValueRef::Variable<std::string>* StringCastedValueRef(const std::string& token) {
        return new ValueRef::StringCast<T>(
            new ValueRef::Variable<T>(
                ValueRef::SOURCE_REFERENCE, token));
    }

    template <typename T>
    ValueRef::Variable<std::string>* StringCastedComplexValueRef(const std::string& token,
                                                                 ValueRef::ValueRefBase<int>* int_ref1 = nullptr,
                                                                 ValueRef::ValueRefBase<int>* int_ref2 = nullptr,
                                                                 ValueRef::ValueRefBase<int>* int_ref3 = nullptr,
                                                                 ValueRef::ValueRefBase<std::string>* string_ref1 = nullptr,
                                                                 ValueRef::ValueRefBase<std::string>* string_ref2 = nullptr)
    {
        return new ValueRef::StringCast<T>(
            new ValueRef::ComplexVariable<T>(token, int_ref1, int_ref2, int_ref3, string_ref1, string_ref2));
    }

    ValueRef::Variable<std::string>* SystemSupplyRangeValueRef(bool propagated = false) {
        return StringCastedComplexValueRef<double>(
            propagated ? "PropagatedSystemSupplyRange" :"SystemSupplyRange",
            nullptr,
            new ValueRef::Variable<int>(ValueRef::SOURCE_REFERENCE, "SystemID"));
    }

    ValueRef::Variable<std::string>* SystemSupplyDistanceValueRef() {
        return StringCastedComplexValueRef<double>("PropagatedSystemSupplyDistance",
            nullptr,
            new ValueRef::Variable<int>(ValueRef::SOURCE_REFERENCE, "SystemID"));
    }

    template <typename T>
    ValueRef::Variable<std::string>* UserStringCastedValueRef(const std::string& token) {
        return new ValueRef::UserStringLookup<std::string>(
            new ValueRef::StringCast<T>(
                new ValueRef::Variable<T>(
                    ValueRef::SOURCE_REFERENCE, token)));
    }

    ValueRef::Variable<std::string>* ObjectNameValueRef(const std::string& token) {
        return new ValueRef::NameLookup(
            new ValueRef::Variable<int>(
                ValueRef::SOURCE_REFERENCE, token),
            ValueRef::NameLookup::OBJECT_NAME);
    }

    ValueRef::Variable<std::string>* EmpireNameValueRef(const std::string& token) {
        return new ValueRef::NameLookup(
            new ValueRef::Variable<int>(
                ValueRef::SOURCE_REFERENCE, token),
            ValueRef::NameLookup::EMPIRE_NAME);
    }

    ValueRef::Variable<std::string>* DesignNameValueRef(const std::string& token) {
        return new ValueRef::NameLookup(
            new ValueRef::Variable<int>(
                ValueRef::SOURCE_REFERENCE, token),
            ValueRef::NameLookup::SHIP_DESIGN_NAME);
    }

    const std::map<std::pair<std::string, std::string>, ValueRef::ValueRefBase<std::string>*>& AvailableColumnTypes() {
        static std::map<std::pair<std::string, std::string>, ValueRef::ValueRefBase<std::string>*> col_types;
        if (col_types.empty()) {
            // General
            col_types[{UserStringNop("NAME"),                   ""}] =  StringValueRef("Name");
            col_types[{UserStringNop("OBJECT_TYPE"),            ""}] =  UserStringValueRef("TypeName");
            col_types[{UserStringNop("ID"),                     ""}] =  StringCastedValueRef<int>("ID");
            col_types[{UserStringNop("CREATION_TURN"),          ""}] =  StringCastedValueRef<int>("CreationTurn");
            col_types[{UserStringNop("AGE"),                    ""}] =  StringCastedValueRef<int>("Age");
            col_types[{UserStringNop("SYSTEM"),                 ""}] =  ObjectNameValueRef("SystemID");
            col_types[{UserStringNop("STAR_TYPE"),              ""}] =  UserStringCastedValueRef<StarType>("StarType");
            col_types[{UserStringNop("BUILDING_TYPE"),          ""}] =  UserStringValueRef("BuildingType");
            col_types[{UserStringNop("LAST_TURN_BATTLE_HERE"),  ""}] =  StringCastedValueRef<int>("LastTurnBattleHere");
            col_types[{UserStringNop("NUM_SPECIALS"),           ""}] =  StringCastedValueRef<int>("NumSpecials");
            col_types[{UserStringNop("SPECIALS"),               ""}] =  UserStringVecValueRef("Specials");
            col_types[{UserStringNop("TAGS"),                   ""}] =  UserStringVecValueRef("Tags");
            col_types[{UserStringNop("X"),                      ""}] =  StringCastedValueRef<double>("X");
            col_types[{UserStringNop("Y"),                      ""}] =  StringCastedValueRef<double>("Y");
            // empire
            col_types[{UserStringNop("SUPPLYING_EMPIRE"),       ""}] =  EmpireNameValueRef("SupplyingEmpire");
            col_types[{UserStringNop("SYSTEM_SUPPLY_RANGE"),    ""}] =  SystemSupplyRangeValueRef(false);
            col_types[{UserStringNop("PROPAGATED_SUPPLY_RANGE"),""}] =  SystemSupplyRangeValueRef(true);
            col_types[{UserStringNop("PROPAGATED_SUPPLY_DISTANCE"),""}]=SystemSupplyDistanceValueRef();
            col_types[{UserStringNop("OWNER"),                  ""}] =  EmpireNameValueRef("Owner");
            col_types[{UserStringNop("PRODUCED_BY"),            ""}] =  EmpireNameValueRef("ProducedByEmpireID");

            // planet
            col_types[{UserStringNop("SPECIES"),                    UserStringNop("PLANETS_SUBMENU")}]= UserStringValueRef("Species");
            col_types[{UserStringNop("FOCUS"),                      UserStringNop("PLANETS_SUBMENU")}]= UserStringValueRef("Focus");
            col_types[{UserStringNop("PREFERRED_FOCUS"),            UserStringNop("PLANETS_SUBMENU")}]= UserStringValueRef("PreferredFocus");
            col_types[{UserStringNop("TURNS_SINCE_FOCUS_CHANGE"),   UserStringNop("PLANETS_SUBMENU")}]= StringCastedValueRef<int>("TurnsSinceFocusChange");
            col_types[{UserStringNop("SIZE_AS_DOUBLE"),             UserStringNop("PLANETS_SUBMENU")}]= StringCastedValueRef<double>("SizeAsDouble");
            col_types[{UserStringNop("NEXT_TURN_POP_GROWTH"),       UserStringNop("PLANETS_SUBMENU")}]= StringCastedValueRef<double>("NextTurnPopGrowth");
            col_types[{UserStringNop("DISTANCE_FROM_ORIGINAL_TYPE"),UserStringNop("PLANETS_SUBMENU")}]= StringCastedValueRef<double>("DistanceFromOriginalType");
            col_types[{UserStringNop("PLANET_TYPE"),                UserStringNop("PLANETS_SUBMENU")}]= UserStringCastedValueRef<PlanetType>("PlanetType");
            col_types[{UserStringNop("ORIGINAL_TYPE"),              UserStringNop("PLANETS_SUBMENU")}]= UserStringCastedValueRef<PlanetType>("OriginalType");
            col_types[{UserStringNop("NEXT_TOWARDS_ORIGINAL_TYPE"), UserStringNop("PLANETS_SUBMENU")}]= UserStringCastedValueRef<PlanetType>("NextCloserToOriginalPlanetType");
            col_types[{UserStringNop("PLANET_SIZE"),                UserStringNop("PLANETS_SUBMENU")}]= UserStringCastedValueRef<PlanetSize>("PlanetSize");
            col_types[{UserStringNop("PLANET_ENVIRONMENT"),         UserStringNop("PLANETS_SUBMENU")}]= UserStringCastedValueRef<PlanetEnvironment>("PlanetEnvironment");
            col_types[{UserStringNop("SUPPLY_RANGE"),               UserStringNop("PLANETS_SUBMENU")}]= StringCastedValueRef<double>("PropagatedSupplyRange");
            col_types[{UserStringNop("AVAILABLE_FOCI"),             UserStringNop("PLANETS_SUBMENU")}]= UserStringVecValueRef("AvailableFoci");

            // ship/fleet
            col_types[{UserStringNop("SPECIES"),                    UserStringNop("FLEETS_SUBMENU")}] = UserStringValueRef("Species");
            col_types[{UserStringNop("DESIGN_WND_DESIGN_NAME"),     UserStringNop("FLEETS_SUBMENU")}] = DesignNameValueRef("DesignID");
            col_types[{UserStringNop("LAST_TURN_ACTIVE_IN_BATTLE"), UserStringNop("FLEETS_SUBMENU")}] = StringCastedValueRef<int>("LastTurnActiveInBattle");
            col_types[{UserStringNop("ARRIVED_ON_TURN"),            UserStringNop("FLEETS_SUBMENU")}] = StringCastedValueRef<int>("ArrivedOnTurn");
            col_types[{UserStringNop("ETA"),                        UserStringNop("FLEETS_SUBMENU")}] = StringCastedValueRef<int>("ETA");
            col_types[{UserStringNop("FINAL_DEST"),                 UserStringNop("FLEETS_SUBMENU")}] = ObjectNameValueRef("FinalDestinationID");
            col_types[{UserStringNop("NEXT_SYSTEM"),                UserStringNop("FLEETS_SUBMENU")}] = ObjectNameValueRef("NextSystemID");
            col_types[{UserStringNop("PREV_SYSTEM"),                UserStringNop("FLEETS_SUBMENU")}] = ObjectNameValueRef("PreviousSystemID");
            col_types[{UserStringNop("NEAREST_SYSTEM"),             UserStringNop("FLEETS_SUBMENU")}] = ObjectNameValueRef("NearestSystemID");
            col_types[{UserStringNop("HULL"),                       UserStringNop("FLEETS_SUBMENU")}] = UserStringValueRef("Hull");
            col_types[{UserStringNop("PARTS"),                      UserStringNop("FLEETS_SUBMENU")}] = UserStringVecValueRef("Parts");

            for (MeterType meter = MeterType(0); meter <= METER_SPEED;  // the meter(s) after METER_SPEED are part-specific
                 meter = MeterType(meter + 1))
            {
                col_types[{boost::lexical_cast<std::string>(meter),   UserStringNop("METERS_SUBMENU")}] = StringCastedValueRef<double>(ValueRef::MeterToName(meter));
            }
        }
        return col_types;
    }

    const ValueRef::ValueRefBase<std::string>* GetValueRefByName(const std::string& name) {
        for (const std::map<std::pair<std::string, std::string>, ValueRef::ValueRefBase<std::string>*>::value_type& entry : AvailableColumnTypes()) {
            if (entry.first.first == name)
                return entry.second;
        }
        return nullptr;
    }

    int GetColumnWidth(int column) {
        if (column < 0)
            return ClientUI::Pts()*4;   // size for first (non-reference) column
        std::string option_name = "UI.objects-list-width-col-" + std::to_string(column);
        if (GetOptionsDB().OptionExists(option_name))
            return GetOptionsDB().Get<int>(option_name);
        return ClientUI::Pts()*10;
    }

    void SetColumnWidth(int column, int width) {
        if (column < 0)
            return;
        std::string option_name = "UI.objects-list-width-col-" + std::to_string(column);
        if (GetOptionsDB().OptionExists(option_name))
            GetOptionsDB().Set(option_name, width);
    }

    std::string GetColumnName(int column) {
        if (column < 0)
            return "";
        std::string option_name = "UI.objects-list-info-col-" + std::to_string(column);
        if (GetOptionsDB().OptionExists(option_name))
            return GetOptionsDB().Get<std::string>(option_name);
        return "";
    }

    void SetColumnName(int column, const std::string& name) {
        if (column < 0)
            return;
        std::string option_name = "UI.objects-list-info-col-" + std::to_string(column);
        if (GetOptionsDB().OptionExists(option_name))
            GetOptionsDB().Set(option_name, name);
    }

    const ValueRef::ValueRefBase<std::string>* GetColumnValueRef(int column) {
        if (column < 0)
            return nullptr;
        std::string option_name = "UI.objects-list-info-col-" + std::to_string(column);
        if (!GetOptionsDB().OptionExists(option_name))
            return nullptr;
        std::string column_ref_name = GetOptionsDB().Get<std::string>(option_name);
        return GetValueRefByName(column_ref_name);
    }

    const int DATA_PANEL_BORDER = 1;

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
    const std::string HASGROWTHSPECIAL_CONDITION(UserStringNop("CONDITION_HAS_GROWTH_SPECIAL"));
    const std::string GGWITHPTYPE_CONDITION(UserStringNop("CONDITION_PTYPE_W_GG"));
    const std::string ASTWITHPTYPE_CONDITION(UserStringNop("CONDITION_PTYPE_W_AST"));

    const std::string FILTER_OPTIONS_WND_NAME = "object-list-filter";

    template <class enumT>
    ValueRef::ValueRefBase<enumT>*  CopyEnumValueRef(const ValueRef::ValueRefBase<enumT>* const value_ref) {
        if (const ValueRef::Constant<enumT>* constant =
            dynamic_cast<const ValueRef::Constant<enumT>*>(value_ref))
        { return new ValueRef::Constant<enumT>(constant->Value()); }
        return new ValueRef::Constant<enumT>(enumT(-1));
    }

    std::map<std::string, std::string> object_list_cond_description_map;

    const std::string&              ConditionClassName(const Condition::ConditionBase* const condition) {
        if (dynamic_cast<const Condition::All* const>(condition))
            return ALL_CONDITION;
        else if (dynamic_cast<const Condition::EmpireAffiliation* const>(condition))
            return EMPIREAFFILIATION_CONDITION;
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

        std::map< std::string, std::string >::iterator desc_it = object_list_cond_description_map.find(condition->Description());
        if (desc_it != object_list_cond_description_map.end())
            return desc_it->second;

        return EMPTY_STRING;
    }

    template <typename enumT>
    std::vector<std::string>        StringsFromEnums(const std::vector<enumT>& enum_vals) {
        std::vector<std::string> retval;
        for (const enumT& enum_val : enum_vals)
            retval.push_back(boost::lexical_cast<std::string>(enum_val));
        return retval;
    }
}

////////////////////////////////////////////////
// ConditionWidget
////////////////////////////////////////////////
const GG::X CONDITION_WIDGET_WIDTH(380);

class ConditionWidget : public GG::Control {
public:
    ConditionWidget(GG::X x, GG::Y y, const Condition::ConditionBase* initial_condition = nullptr) :
        GG::Control(x, y, CONDITION_WIDGET_WIDTH, GG::Y1, GG::INTERACTIVE),
        m_class_drop(nullptr),
        m_string_drop(nullptr),
        m_param_spin1(nullptr),
        m_param_spin2(nullptr)
    {
        if (!initial_condition) {
            Condition::ConditionBase* init_condition = new Condition::All();
            Init(init_condition);
            delete init_condition;
        } else {
            Init(initial_condition);
        }
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
            for (std::map<int, Empire*>::value_type& entry : Empires()) {
                if (entry.second->Name() == empire_name) {
                    empire_id = entry.first;
                    break;
                }
            }
            return new Condition::EmpireAffiliation(new ValueRef::Constant<int>(empire_id), affil);

        } else if (condition_key == HOMEWORLD_CONDITION) {
            const std::string& species_name = GetString();
            if (species_name.empty())
                return new Condition::Homeworld();
            std::vector<ValueRef::ValueRefBase<std::string>*> names;
            names.push_back(new ValueRef::Constant<std::string>(species_name));
            return new Condition::Homeworld(names);

        } else if (condition_key == CANCOLONIZE_CONDITION) {
            return new Condition::CanColonize();

        } else if (condition_key == CANPRODUCESHIPS_CONDITION) {
            return new Condition::CanProduceShips();

        } else if (condition_key == HASSPECIAL_CONDITION) {
            return new Condition::HasSpecial(GetString());

        } else if (condition_key == HASGROWTHSPECIAL_CONDITION) {
            std::vector<Condition::ConditionBase*> operands;
            // determine sitrep order
            std::istringstream template_stream(UserString("FUNCTIONAL_GROWTH_SPECIALS_LIST"));
            for (std::istream_iterator<std::string> stream_it = std::istream_iterator<std::string>(template_stream);
                 stream_it != std::istream_iterator<std::string>(); stream_it++)
            {
                operands.push_back(new Condition::HasSpecial(*stream_it));
            }
            Condition::Or* this_cond =  new Condition::Or(operands);
            object_list_cond_description_map[this_cond->Description()] = HASGROWTHSPECIAL_CONDITION;
            return this_cond;

        } else if (condition_key == ASTWITHPTYPE_CONDITION) { // And [Planet PlanetType PT_ASTEROIDS ContainedBy And [System Contains PlanetType X]]
            std::vector<Condition::ConditionBase*> operands1;
            operands1.push_back(new Condition::Type(new ValueRef::Constant<UniverseObjectType>(OBJ_PLANET)));
            const std::string& text = GetString();
            if (text == UserString("CONDITION_ANY")) {
                std::vector<ValueRef::ValueRefBase<PlanetType>*> copytype;
                copytype.push_back(new ValueRef::Constant<PlanetType>(PT_ASTEROIDS));
                operands1.push_back(new Condition::Not(new Condition::PlanetType(copytype)));
            } else {
                operands1.push_back(new Condition::PlanetType(GetEnumValueRefVec< ::PlanetType>()));
            }
            std::vector<Condition::ConditionBase*> operands2;
            operands2.push_back(new Condition::Type(new ValueRef::Constant<UniverseObjectType> (OBJ_SYSTEM)));
            std::vector<ValueRef::ValueRefBase<PlanetType>*> maintype;
            maintype.push_back(new ValueRef::Constant<PlanetType>(PT_ASTEROIDS));
            operands2.push_back(new Condition::Contains(new Condition::PlanetType(maintype)));
            operands1.push_back(new Condition::ContainedBy(new Condition::And(operands2)));
            Condition::And* this_cond =  new Condition::And(operands1);
            object_list_cond_description_map[this_cond->Description()] = ASTWITHPTYPE_CONDITION;
            return this_cond;

        } else if (condition_key == GGWITHPTYPE_CONDITION) { // And [Planet PlanetType PT_GASGIANT ContainedBy And [System Contains PlanetType X]]
            std::vector<Condition::ConditionBase*> operands1;
            const std::string& text = GetString();
            if (text == UserString("CONDITION_ANY")) {
                std::vector<ValueRef::ValueRefBase<PlanetType>*> copytype;
                copytype.push_back(new ValueRef::Constant<PlanetType>(PT_GASGIANT));
                operands1.push_back(new Condition::Not(new Condition::PlanetType(copytype)));
            } else
                operands1.push_back(new Condition::PlanetType(GetEnumValueRefVec< ::PlanetType>()));
            std::vector<Condition::ConditionBase*> operands2;
            operands2.push_back(new Condition::Type(new ValueRef::Constant<UniverseObjectType>(OBJ_SYSTEM)));
            std::vector<ValueRef::ValueRefBase<PlanetType>*> maintype;
            maintype.push_back(new ValueRef::Constant<PlanetType>(PT_GASGIANT));
            operands2.push_back(new Condition::Contains(new Condition::PlanetType(maintype)));
            operands1.push_back(new Condition::ContainedBy(new Condition::And(operands2)));
            Condition::And* this_cond =  new Condition::And(operands1);
            object_list_cond_description_map[this_cond->Description()] = GGWITHPTYPE_CONDITION;
            return this_cond;

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

    void Render() override
    { GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

private:
    class ConditionRow : public GG::ListBox::Row {
    public:
        ConditionRow(const std::string& key, GG::Y row_height) :
            GG::ListBox::Row(GG::X1, row_height, ""),
            m_condition_key(key)
        {
            SetChildClippingMode(ClipToClient);
            push_back(new CUILabel(UserString(m_condition_key), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP));
        }
        const std::string&  GetKey() const { return m_condition_key; }
    private:
        std::string m_condition_key;
    };

    class StringRow : public GG::ListBox::Row  {
    public:
        StringRow(const std::string& text, GG::Y row_height, bool stringtable_lookup = true) :
            GG::ListBox::Row(GG::X1, row_height, ""),
            m_string(text)
        {
            SetChildClippingMode(ClipToClient);
            const std::string& label = (text.empty() ? EMPTY_STRING :
                (stringtable_lookup ? UserString(text) : text));
            push_back(new CUILabel(label, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP));
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

    std::vector<ValueRef::ValueRefBase<std::string>*> GetStringValueRefVec() {
        std::vector<ValueRef::ValueRefBase<std::string>*> retval;
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
            ErrorLogger() << "ConditionWidget::GetEnum unable to convert text to enum type: " << text;
        }
        return enum_val;
    }

    template <typename T>
    ValueRef::ValueRefBase<T>*                              GetEnumValueRef()
    { return new ValueRef::Constant<T>(GetEnum<T>()); }

    template <typename T>
    std::vector<ValueRef::ValueRefBase<T>*>                 GetEnumValueRefVec()
    {
        std::vector<ValueRef::ValueRefBase<T>*> retval;
        retval.push_back(GetEnumValueRef<T>());
        return retval;
    }

    GG::X   DropListWidth() const
    { return GG::X(ClientUI::Pts()*15); }

    GG::X   ParamsDropListWidth() const
    { return CONDITION_WIDGET_WIDTH - DropListWidth(); }

    GG::X   SpinDropListWidth() const
    { return GG::X(ClientUI::Pts()*16); }

    GG::Y   DropListHeight() const
    { return GG::Y(ClientUI::Pts() + 4); }

    int     DropListDropHeight() const
    { return 12; }

    void    Init(const Condition::ConditionBase* init_condition) {
        // fill droplist with basic types of conditions and select appropriate row
        m_class_drop = new CUIDropDownList(DropListDropHeight());
        m_class_drop->Resize(GG::Pt(DropListWidth(), DropListHeight()));
        m_class_drop->SetStyle(GG::LIST_NOSORT);
        AttachChild(m_class_drop);

        std::vector<std::string> row_keys ={ALL_CONDITION,              PLANETTYPE_CONDITION,       PLANETSIZE_CONDITION,
                                            HASGROWTHSPECIAL_CONDITION, GGWITHPTYPE_CONDITION,      ASTWITHPTYPE_CONDITION,
                                            FOCUSTYPE_CONDITION,        STARTYPE_CONDITION,         HASTAG_CONDITION,
                                            HASSPECIAL_CONDITION,       EMPIREAFFILIATION_CONDITION,MONSTER_CONDITION,
                                            ARMED_CONDITION,            STATIONARY_CONDITION,       CANPRODUCESHIPS_CONDITION,
                                            CANCOLONIZE_CONDITION,      HOMEWORLD_CONDITION,        METERVALUE_CONDITION,
                                            CAPITAL_CONDITION };

        SetMinSize(m_class_drop->Size());
        GG::ListBox::iterator select_row_it = m_class_drop->end();
        const std::string& init_condition_key = ConditionClassName(init_condition);

        // fill droplist with rows for the available condition classes to be selected
        for (const std::string& key : row_keys) {
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
        delete m_string_drop;
        m_string_drop = nullptr;
        delete m_param_spin1;
        m_param_spin1 = nullptr;
        delete m_param_spin2;
        m_param_spin2 = nullptr;

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
            condition_key == MONSTER_CONDITION ||
            condition_key == HASGROWTHSPECIAL_CONDITION)
        {
            // no params
            param_widget_top += m_class_drop->Height();

        } else if (condition_key == HOMEWORLD_CONDITION ||
                   condition_key == SPECIES_CONDITION)
        {
            // droplist of valid species
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(ParamsDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // add empty row, allowing for matching any species
            GG::ListBox::iterator row_it = m_string_drop->Insert(new StringRow("", GG::Y(ClientUI::Pts())));
            m_string_drop->Select(row_it);

            for (const std::map<std::string, Species*>::value_type& entry : GetSpeciesManager()) {
                const std::string& species_name = entry.first;
                row_it = m_string_drop->Insert(new StringRow(species_name, GG::Y(ClientUI::Pts())));
            }

        } else if (condition_key == HASSPECIAL_CONDITION) {
            // droplist of valid specials
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(ParamsDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // add empty row, allowing for matching any special
            GG::ListBox::iterator row_it = m_string_drop->Insert(new StringRow("", GG::Y(ClientUI::Pts())));
            m_string_drop->Select(row_it);

            for (const std::string& special_name : SpecialNames()) {
                row_it = m_string_drop->Insert(new StringRow(special_name, GG::Y(ClientUI::Pts())));
            }

        } else if (condition_key == HASTAG_CONDITION) {
            // droplist of valid tags
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(ParamsDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // collect all valid tags on any object in universe
            std::set<std::string> all_tags;

            for (std::shared_ptr<const UniverseObject> obj : GetUniverse().Objects().FindObjects<UniverseObject>()) {
                std::set<std::string> tags = obj->Tags();
                all_tags.insert(tags.begin(), tags.end());
            }

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (const std::string& tag : all_tags) {
                row_it = m_string_drop->Insert(new StringRow(tag, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == PLANETSIZE_CONDITION) {
            // droplist of valid sizes
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(ParamsDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            std::vector< ::PlanetSize> planet_sizes;
            for (::PlanetSize size = SZ_TINY; size != NUM_PLANET_SIZES; size = ::PlanetSize(size + 1))
                planet_sizes.push_back(size);

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (const std::string& text : StringsFromEnums(planet_sizes)) {
                row_it = m_string_drop->Insert(new StringRow(text, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == PLANETTYPE_CONDITION ||
                   condition_key == GGWITHPTYPE_CONDITION ||
                   condition_key == ASTWITHPTYPE_CONDITION ) {
            // droplist of valid types
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(ParamsDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);

            std::vector< ::PlanetType> planet_types;
            for (::PlanetType type = PT_SWAMP; type != NUM_PLANET_TYPES; type = ::PlanetType(type + 1))
                planet_types.push_back(type);

            GG::ListBox::iterator row_it = m_string_drop->end();
            if (condition_key == GGWITHPTYPE_CONDITION || condition_key == ASTWITHPTYPE_CONDITION )
                row_it = m_string_drop->Insert(new StringRow(UserString("CONDITION_ANY"), GG::Y(ClientUI::Pts()), false));
            for (const std::string& text : StringsFromEnums(planet_types)) {
                row_it = m_string_drop->Insert(new StringRow(text, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == STARTYPE_CONDITION) {
            // droplist of valid types
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(ParamsDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            std::vector< ::StarType> star_types;
            for (::StarType type = STAR_BLUE; type != NUM_STAR_TYPES; type = ::StarType(type + 1))
                star_types.push_back(type);

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (const std::string& text : StringsFromEnums(star_types)) {
                row_it = m_string_drop->Insert(new StringRow(text, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == FOCUSTYPE_CONDITION) {
            // droplist of valid foci
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(ParamsDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // collect all valid foci on any object in universe
            std::set<std::string> all_foci;
            for (std::shared_ptr<const Planet> planet : Objects().FindObjects<Planet>()) {
                std::vector<std::string> obj_foci = planet->AvailableFoci();
                std::copy(obj_foci.begin(), obj_foci.end(), std::inserter(all_foci, all_foci.end()));
            }

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (const std::string& focus : all_foci) {
                row_it = m_string_drop->Insert(new StringRow(focus, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

        } else if (condition_key == METERVALUE_CONDITION) {
            // droplist of meter types
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(ParamsDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);
            param_widget_left = GG::X0;
            param_widget_top = m_string_drop->Height() + GG::Y(Value(PAD));

            std::vector< ::MeterType> meter_types;
            for (::MeterType type = METER_TARGET_POPULATION; type != NUM_METER_TYPES; type = ::MeterType(type + 1))
                meter_types.push_back(type);

            GG::ListBox::iterator row_it = m_string_drop->end();
            for (const std::string& text : StringsFromEnums(meter_types)) {
                row_it = m_string_drop->Insert(new StringRow(text, GG::Y(ClientUI::Pts())));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);

            m_param_spin1 = new CUISpin<int>(0, 1, 0, 1000, true);
            m_param_spin1->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_param_spin1->Resize(GG::Pt(SpinDropListWidth(), m_param_spin1->Height()));
            AttachChild(m_param_spin1);
            param_widget_left = SpinDropListWidth() + PAD;

            m_param_spin2 = new CUISpin<int>(0, 1, 0, 1000, true);
            m_param_spin2->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_param_spin2->Resize(GG::Pt(SpinDropListWidth(), m_param_spin2->Height()));
            AttachChild(m_param_spin2);

            param_widget_top += m_param_spin1->Height();
        } else if (condition_key == EMPIREAFFILIATION_CONDITION) {
            // droplist of empires
            m_string_drop = new CUIDropDownList(DropListDropHeight());
            m_string_drop->MoveTo(GG::Pt(param_widget_left, param_widget_top));
            m_string_drop->Resize(GG::Pt(SpinDropListWidth(), DropListHeight()));
            AttachChild(m_string_drop);

            param_widget_top += m_string_drop->Height();

            // add rows for empire names
            GG::ListBox::iterator row_it = m_string_drop->end();
            for (const std::map<int, Empire*>::value_type& entry : Empires()) {
                const std::string& empire_name = entry.second->Name();
                row_it = m_string_drop->Insert(new StringRow(empire_name, GG::Y(ClientUI::Pts()), false));
            }
            if (!m_string_drop->Empty())
                m_string_drop->Select(0);
        }

        Resize(GG::Pt(Width(), param_widget_top));
    }

    GG::DropDownList*   m_class_drop;
    GG::DropDownList*   m_string_drop;
    GG::Spin<int>*      m_param_spin1;
    GG::Spin<int>*      m_param_spin2;
};

////////////////////////////////////////////////
// FilterDialog                               //
////////////////////////////////////////////////
class FilterDialog : public CUIWnd {
public:
    FilterDialog(const std::map<UniverseObjectType, std::set<VIS_DISPLAY>>& vis_filters,
                 const Condition::ConditionBase* const condition_filter) :
        CUIWnd(UserString("FILTERS"),
               GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL,
               FILTER_OPTIONS_WND_NAME),
        m_vis_filters(vis_filters),
        m_filter_buttons(),
        m_accept_changes(false),
        m_filters_layout(nullptr),
        m_cancel_button(nullptr),
        m_apply_button(nullptr)
    { Init(condition_filter); }

    bool    ChangesAccepted()
    { return m_accept_changes; }

    std::map<UniverseObjectType, std::set<VIS_DISPLAY>> GetVisibilityFilters() const
    { return m_vis_filters; }

    // caller takes ownership of returned ConditionBase*
    Condition::ConditionBase*                           GetConditionFilter()
    { return m_condition_widget->GetCondition(); }

protected:
    GG::Rect CalculatePosition() const override
    { return GG::Rect(GG::X(100), GG::Y(100), GG::X(500), GG::Y(350)); }

private:
    void    Init(const Condition::ConditionBase* const condition_filter) {
        if (m_filters_layout)
            delete m_filters_layout;
        m_filters_layout = new GG::Layout(GG::X0, GG::Y0, GG::X(390), GG::Y(90), 4, 7);
        AttachChild(m_filters_layout);

        m_filters_layout->SetMinimumColumnWidth(0, GG::X(ClientUI::Pts()*8));
        m_filters_layout->SetColumnStretch(0, 0.0);

        GG::X button_width = GG::X(ClientUI::Pts()*8);
        GG::Button* label = nullptr;

        label = new CUIButton(UserString("VISIBLE"));
        label->Resize(GG::Pt(button_width, label->MinUsableSize().y));
        m_filters_layout->SetMinimumRowHeight(0, label->MinUsableSize().y);
        m_filters_layout->Add(label, 1, 0, GG::ALIGN_CENTER);
        GG::Connect(label->LeftClickedSignal,
                    boost::bind(&FilterDialog::UpdateVisFilterFromVisibilityButton, this, SHOW_VISIBLE));

        label = new CUIButton(UserString("PREVIOUSLY_VISIBLE"));
        label->Resize(GG::Pt(button_width, label->MinUsableSize().y));
        m_filters_layout->Add(label, 2, 0, GG::ALIGN_CENTER);
        GG::Connect(label->LeftClickedSignal,
                    boost::bind(&FilterDialog::UpdateVisFilterFromVisibilityButton, this, SHOW_PREVIOUSLY_VISIBLE));

        label = new CUIButton(UserString("DESTROYED"));
        label->Resize(GG::Pt(button_width, label->MinUsableSize().y));
        m_filters_layout->Add(label, 3, 0, GG::ALIGN_CENTER);
        GG::Connect(label->LeftClickedSignal,
                    boost::bind(&FilterDialog::UpdateVisFilterFromVisibilityButton, this, SHOW_DESTROYED));

        m_filters_layout->SetMinimumRowHeight(0, label->MinUsableSize().y);
        m_filters_layout->SetMinimumRowHeight(1, label->MinUsableSize().y);
        m_filters_layout->SetMinimumRowHeight(2, label->MinUsableSize().y);
        m_filters_layout->SetMinimumRowHeight(3, label->MinUsableSize().y);

        int col = 1;
        for (std::map<UniverseObjectType, std::set<VIS_DISPLAY>>::value_type& entry : m_vis_filters) {
            const UniverseObjectType& uot = entry.first;
            const std::set<VIS_DISPLAY>& vis_display = entry.second;

            m_filters_layout->SetColumnStretch(col, 1.0);

            label = new CUIButton(" " + UserString(boost::lexical_cast<std::string>(uot)) + " ");
            m_filters_layout->Add(label, 0, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(label->LeftClickedSignal,
                        boost::bind(&FilterDialog::UpdateVisFiltersFromObjectTypeButton, this, uot));

            GG::StateButton* button = new CUIStateButton(" ", GG::FORMAT_CENTER, std::make_shared<CUICheckBoxRepresenter>());
            button->SetCheck(vis_display.find(SHOW_VISIBLE) != vis_display.end());
            m_filters_layout->Add(button, 1, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisFiltersFromStateButtons,    this);
            m_filter_buttons[uot][SHOW_VISIBLE] = button;

            button = new CUIStateButton(" ", GG::FORMAT_CENTER, std::make_shared<CUICheckBoxRepresenter>());
            button->SetCheck(vis_display.find(SHOW_PREVIOUSLY_VISIBLE) != vis_display.end());
            m_filters_layout->Add(button, 2, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisFiltersFromStateButtons,    this);
            m_filter_buttons[uot][SHOW_PREVIOUSLY_VISIBLE] = button;

            button = new CUIStateButton(" ", GG::FORMAT_CENTER, std::make_shared<CUICheckBoxRepresenter>());
            button->SetCheck(vis_display.find(SHOW_DESTROYED) != vis_display.end());
            m_filters_layout->Add(button, 3, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisFiltersFromStateButtons,    this);
            m_filter_buttons[uot][SHOW_DESTROYED] = button;

            ++col;
        }


        // TODO: Add multiple condition widgets initialized for input condition
        m_condition_widget = new ConditionWidget(GG::X(3), m_filters_layout->Height() + GG::Y(3), condition_filter);
        m_cancel_button = new CUIButton(UserString("CANCEL"));
        m_apply_button = new CUIButton(UserString("APPLY"));

        AttachChild(m_condition_widget);
        AttachChild(m_cancel_button);
        AttachChild(m_apply_button);

        GG::Connect(m_cancel_button->LeftClickedSignal, &FilterDialog::CancelClicked,   this);
        GG::Connect(m_apply_button->LeftClickedSignal, &FilterDialog::AcceptClicked,   this);

        ResetDefaultPosition();

        GG::Pt button_lr = this->ClientSize();
        m_cancel_button->Resize(GG::Pt(button_width, m_cancel_button->MinUsableSize().y));
        m_cancel_button->MoveTo(button_lr - m_cancel_button->Size());
        button_lr = button_lr - GG::Pt(m_cancel_button->Width() + GG::X(3), GG::Y0);
        m_apply_button->Resize(GG::Pt(button_width, m_apply_button->MinUsableSize().y));
        m_apply_button->MoveTo(button_lr - m_apply_button->Size());
    }

    void    AcceptClicked() {
        m_accept_changes = true;
        m_done = true;
    }

    void    CancelClicked() {
        m_accept_changes = false;
        m_done = true;
    }

    void    UpdateStateButtonsFromVisFilters() {
        // set state button checks to match current visibility filter settings
        for (std::map<UniverseObjectType, std::map<VIS_DISPLAY, GG::StateButton*>>::value_type& entry : m_filter_buttons) {
            UniverseObjectType uot = entry.first;

            // find visibilities for this object type
            std::map<UniverseObjectType, std::set<VIS_DISPLAY>>::iterator uot_it = m_vis_filters.find(uot);
            const std::set<VIS_DISPLAY>& shown_vis = (uot_it != m_vis_filters.end() ? uot_it->second : std::set<VIS_DISPLAY>());

            // set all button checks depending on whether that buttons visibility is to be shown
            for (std::map<VIS_DISPLAY, GG::StateButton*>::value_type& button : entry.second) {
                if (!button.second)
                    continue;
                button.second->SetCheck(shown_vis.find(button.first) != shown_vis.end());
            }
        }
    }

    void    UpdateVisFiltersFromStateButtons(bool button_checked) {
        m_vis_filters.clear();
        // set all filters based on state button settings
        for (std::map<UniverseObjectType, std::map<VIS_DISPLAY, GG::StateButton*>>::value_type& entry : m_filter_buttons) {
            UniverseObjectType uot = entry.first;

            for (std::map<VIS_DISPLAY, GG::StateButton*>::value_type& button : entry.second) {
                if (!button.second)
                    continue;
                if (button.second->Checked())
                    m_vis_filters[uot].insert(button.first);
            }
        }
    }

    void    UpdateVisFiltersFromObjectTypeButton(UniverseObjectType type) {
        // toggle visibilities for this object type

        // if all on, turn all off. otherwise, turn all on
        bool all_on = (m_vis_filters[type].size() == 3);
        if (!all_on) {
            m_vis_filters[type].insert(SHOW_VISIBLE);
            m_vis_filters[type].insert(SHOW_PREVIOUSLY_VISIBLE);
            m_vis_filters[type].insert(SHOW_DESTROYED);
        } else {
            m_vis_filters[type].clear();
        }
        UpdateStateButtonsFromVisFilters();
    }

    void    UpdateVisFilterFromVisibilityButton(VIS_DISPLAY vis) {
        // toggle types for this visibility

        // determine if all types are already on for requested visibility
        bool all_on = true;
        for (std::map<UniverseObjectType, std::map<VIS_DISPLAY, GG::StateButton*>>::value_type& entry : m_filter_buttons) {
            std::set<VIS_DISPLAY>& type_vis = m_vis_filters[entry.first];
            if (type_vis.find(vis) == type_vis.end()) {
                all_on = false;
                break;
            }
        }
        // if all on, turn all off. otherwise, turn all on
        for (std::map<UniverseObjectType, std::map<VIS_DISPLAY, GG::StateButton*>>::value_type& entry : m_filter_buttons) {
            std::set<VIS_DISPLAY>& type_vis = m_vis_filters[entry.first];
            if (!all_on)
                type_vis.insert(vis);
            else
                type_vis.erase(vis);
        }

        UpdateStateButtonsFromVisFilters();
    }

    std::map<UniverseObjectType, std::set<VIS_DISPLAY>>     m_vis_filters;
    std::map<UniverseObjectType,
             std::map<VIS_DISPLAY, GG::StateButton*>>       m_filter_buttons;
    bool                                                    m_accept_changes;

    ConditionWidget*    m_condition_widget;
    GG::Layout*         m_filters_layout;
    GG::Button*         m_cancel_button;
    GG::Button*         m_apply_button;
};

namespace {
    std::vector<std::shared_ptr<GG::Texture>> ObjectTextures(std::shared_ptr<const UniverseObject> obj) {
        std::vector<std::shared_ptr<GG::Texture>> retval;

        if (obj->ObjectType() == OBJ_SHIP) {
            std::shared_ptr<const Ship> ship = std::dynamic_pointer_cast<const Ship>(obj);
            if (ship) {
                if (const ShipDesign* design = ship->Design())
                    retval.push_back(ClientUI::ShipDesignIcon(design->ID()));
            }
            if (retval.empty()) {
                retval.push_back(ClientUI::ShipDesignIcon(INVALID_OBJECT_ID));  // default icon
            }
        } else if (obj->ObjectType() == OBJ_FLEET) {
            if (std::shared_ptr<const Fleet> fleet = std::dynamic_pointer_cast<const Fleet>(obj)) {
                std::shared_ptr<GG::Texture> size_icon = FleetSizeIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
                if (size_icon)
                    retval.push_back(size_icon);
                std::vector<std::shared_ptr<GG::Texture>> head_icons = FleetHeadIcons(fleet, FleetButton::FLEET_BUTTON_LARGE);
                std::copy(head_icons.begin(), head_icons.end(), std::back_inserter(retval));
            }
        } else if (obj->ObjectType() == OBJ_SYSTEM) {
            if (std::shared_ptr<const System> system = std::dynamic_pointer_cast<const System>(obj)) {
                StarType star_type = system->GetStarType();
                ClientUI* ui = ClientUI::GetClientUI();
                std::shared_ptr<GG::Texture> disc_texture = ui->GetModuloTexture(
                    ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[star_type], system->ID());
                if (disc_texture)
                    retval.push_back(disc_texture);
                std::shared_ptr<GG::Texture> halo_texture = ui->GetModuloTexture(
                    ClientUI::ArtDir() / "stars", ClientUI::HaloStarTypeFilePrefixes()[star_type], system->ID());
                if (halo_texture)
                    retval.push_back(halo_texture);
            }
        } else if (obj->ObjectType() == OBJ_PLANET) {
            if (std::shared_ptr<const Planet> planet = std::dynamic_pointer_cast<const Planet>(obj))
                retval.push_back(ClientUI::PlanetIcon(planet->Type()));
        } else if (obj->ObjectType() == OBJ_BUILDING) {
            if (std::shared_ptr<const Building> building = std::dynamic_pointer_cast<const Building>(obj))
                retval.push_back(ClientUI::BuildingIcon(building->BuildingTypeName()));
        } else if (obj->ObjectType() == OBJ_FIELD) {
            if (std::shared_ptr<const Field> field = std::dynamic_pointer_cast<const Field>(obj))
                retval.push_back(ClientUI::FieldTexture(field->FieldTypeName()));
        }
        if (retval.empty())
            retval.push_back(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "generic_object.png", true));
        return retval;
    }

    const GG::X PAD(3);
}

////////////////////////////////////////////////
// ObjectPanel
////////////////////////////////////////////////
class ObjectPanel : public GG::Control {
public:
    ObjectPanel(GG::X w, GG::Y h, std::shared_ptr<const UniverseObject> obj,
                bool expanded, bool has_contents, int indent) :
        Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
        m_initialized(false),
        m_object_id(obj ? obj->ID() : INVALID_OBJECT_ID),
        m_indent(indent),
        m_expanded(expanded),
        m_has_contents(has_contents),
        m_expand_button(nullptr),
        m_dot(nullptr),
        m_icon(nullptr),
        m_column_val_cache(),
        m_selected(false)
    {
        SetChildClippingMode(ClipToClient);
        std::shared_ptr<const ResourceCenter> rcobj = std::dynamic_pointer_cast<const ResourceCenter>(obj);
        if (rcobj)
            GG::Connect(rcobj->ResourceCenterChangedSignal, &ObjectPanel::ResourceCenterChanged, this);
    }

    void                ResourceCenterChanged() {
        RefreshCache();
        RequirePreRender();
    }

    std::string         SortKey(std::size_t column) const {
        if (column >= m_column_val_cache.size())
            RefreshCache();
        if (column >= m_column_val_cache.size())
            return "";
        return m_column_val_cache[column];
    }

    int                 ObjectID() const { return m_object_id; }

    void PreRender() override {
        GG::Control::PreRender();
        RefreshLayout();
    }

    void Render() override {
        if (!m_initialized)
            Init();

        const GG::Clr& background_colour = ClientUI::WndColor();
        const GG::Clr& unselected_colour = ClientUI::WndOuterBorderColor();
        const GG::Clr& selected_colour = ClientUI::WndInnerBorderColor();
        GG::Clr border_colour = m_selected ? selected_colour : unselected_colour;
        if (Disabled())
            border_colour = DisabledColor(border_colour);

        GG::FlatRectangle(UpperLeft(), LowerRight(), background_colour, border_colour, DATA_PANEL_BORDER);
    }

    void                Select(bool b)
    { m_selected = b; }

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        const GG::Pt old_size = Size();
        GG::Control::SizeMove(ul, lr);
        if (old_size != Size())
            DoLayout();
    }

    void                RefreshLayout() {
        if (!m_initialized)
            return;
        GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER |
                                            GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE;

        delete m_dot;
        m_dot = nullptr;
        delete m_expand_button;
        m_expand_button = nullptr;
        delete m_icon;
        m_icon = nullptr;

        if (m_has_contents) {
            boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

            if (m_expanded) {
                m_expand_button = new CUIButton(
                    GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "minusnormal.png"     , true)),
                    GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "minusclicked.png"    , true)),
                    GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "minusmouseover.png"  , true)));
            } else {
                m_expand_button = new CUIButton(
                    GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "plusnormal.png"   , true)),
                    GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "plusclicked.png"  , true)),
                    GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "plusmouseover.png", true)));
            }

            AttachChild(m_expand_button);
            GG::Connect(m_expand_button->LeftClickedSignal, &ObjectPanel::ExpandCollapseButtonPressed, this);
        } else {
            m_dot = new GG::StaticGraphic(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "dot.png", true), style);
            AttachChild(m_dot);
        }

        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(m_object_id);
        std::vector<std::shared_ptr<GG::Texture>> textures = ObjectTextures(obj);

        m_icon = new MultiTextureStaticGraphic(textures,
                                               std::vector<GG::Flags<GG::GraphicStyle>>(textures.size(), style));
        AttachChild(m_icon);

        for (GG::Control* control : m_controls)
        { DeleteChild(control); }
        m_controls.clear();

        for (GG::Control* control : GetControls()) {
            m_controls.push_back(control);
            AttachChild(control);
        }

        DoLayout();
    }

    void                SetHasContents(bool has_contents)
    { m_has_contents = has_contents; }

    mutable boost::signals2::signal<void ()>  ExpandCollapseSignal;

private:
    void                DoLayout() {
        if (!m_initialized)
            return;

        const GG::X ICON_WIDTH(Value(ClientHeight()));

        GG::X indent(ICON_WIDTH * m_indent);
        GG::X left = indent;
        GG::Y top(GG::Y0);
        GG::Y bottom(ClientHeight());

        if (m_expand_button) {
            m_expand_button->SizeMove(GG::Pt(left, top), GG::Pt(left + ICON_WIDTH, bottom));
        } else if (m_dot) {
            m_dot->SizeMove(GG::Pt(left, top), GG::Pt(left + ICON_WIDTH, bottom));
        }
        left += ICON_WIDTH + PAD;

        if (m_icon) {
            m_icon->SizeMove(GG::Pt(left, top), GG::Pt(left + ICON_WIDTH, bottom));
            left += ICON_WIDTH + PAD;
        }

        // loop through m_controls, positioning according to column widths.
        // first column position dependent on indent (ie. left at start of loop)
        // second column position fixed equal to first column width value.
        // ie. reset left, not dependent on current left.
        GG::Control* ctrl = m_controls[0];
        GG::X width(GetColumnWidth(static_cast<int>(-1)) + GetColumnWidth(static_cast<int>(0)));
        GG::X right = width;
        ctrl->SizeMove(GG::Pt(left, top), GG::Pt(right, bottom));
        left = right + 2*PAD;

        for (std::size_t i = 1; i < m_controls.size(); ++i) {
            right = left + GetColumnWidth(static_cast<int>(i));
            if ((ctrl = m_controls[i]))
                ctrl->SizeMove(GG::Pt(left, top), GG::Pt(right, bottom));
            left = right + PAD;
        }
    }

    void                ExpandCollapseButtonPressed() {
        m_expanded = !m_expanded;
        ExpandCollapseSignal();
    }

    void                Init() {
        if (m_initialized)
            return;
        m_initialized = true;
        RefreshLayout();
    }

    void                RefreshCache() const {
        m_column_val_cache.clear();
        m_column_val_cache.reserve(NUM_COLUMNS);
        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(m_object_id);
        ScriptingContext context(obj);

        // get currently displayed column value refs, put values into this panel's cache
        for (unsigned int i = 0; i < NUM_COLUMNS; ++i) {
            const ValueRef::ValueRefBase<std::string>* ref = GetColumnValueRef(static_cast<int>(i));
            if (ref)
                m_column_val_cache.push_back(ref->Eval(context));
            else
                m_column_val_cache.push_back("");
        }
    }

    std::vector<GG::Control*>   GetControls() {
        std::vector<GG::Control*> retval;

        RefreshCache();

        for (unsigned int i = 0; i < NUM_COLUMNS; ++i) {
            std::string col_val = m_column_val_cache[i];
            GG::Label* control = new CUILabel(col_val, GG::FORMAT_LEFT);
            control->Resize(GG::Pt(GG::X(GetColumnWidth(i)), ClientHeight()));
            retval.push_back(control);
        }

        return retval;
    }

    bool    m_initialized;
    int     m_object_id;
    int     m_indent;
    bool    m_expanded;
    bool    m_has_contents;

    GG::Button*                     m_expand_button;
    GG::StaticGraphic*              m_dot;
    MultiTextureStaticGraphic*      m_icon;
    std::vector<GG::Control*>       m_controls;

    mutable std::vector<std::string>m_column_val_cache;

    bool                            m_selected;
};

////////////////////////////////////////////////
// ObjectRow
////////////////////////////////////////////////
class ObjectRow : public GG::ListBox::Row {
public:
    ObjectRow(GG::X w, GG::Y h, std::shared_ptr<const UniverseObject> obj, bool expanded,
              int container_object_panel, const std::set<int>& contained_object_panels,
              int indent) :
        GG::ListBox::Row(w, h, "", GG::ALIGN_CENTER, 1),
        m_panel(nullptr),
        m_container_object_panel(container_object_panel),
        m_contained_object_panels(contained_object_panels),
        m_obj_init(obj),
        m_expanded_init(expanded),
        m_indent_init(indent)
    {
        SetName("ObjectRow");
        SetChildClippingMode(ClipToClient);
        Init();
    }

    void Init() {
        m_panel = new ObjectPanel(ClientWidth() - GG::X(2 * GetLayout()->BorderMargin()),
                                  ClientHeight() - GG::Y(2 * GetLayout()->BorderMargin()),
                                  m_obj_init, m_expanded_init, !m_contained_object_panels.empty(), m_indent_init);
        push_back(m_panel);
        GG::Connect(m_panel->ExpandCollapseSignal,  &ObjectRow::ExpandCollapseClicked, this);

        GG::Pt border(GG::X(2 * GetLayout()->BorderMargin()), GG::Y(2 * GetLayout()->BorderMargin()));
        m_panel->Resize(Size() - border);
    }

    GG::ListBox::Row::SortKeyType SortKey(std::size_t column) const override
    { return m_panel ? m_panel->SortKey(column) : ""; }

    int                     ObjectID() const {
        if (m_panel)
            return m_panel->ObjectID();
        return INVALID_OBJECT_ID;
    }

    int                     ContainedByPanel() const
    { return m_container_object_panel; }

    const std::set<int>&    ContainedPanels() const
    { return m_contained_object_panels; }

    void                    SetContainedPanels(const std::set<int>& contained_object_panels) {
        m_contained_object_panels = contained_object_panels;
        m_panel->SetHasContents(!m_contained_object_panels.empty());
        m_panel->RequirePreRender();
    }

    void                    Update()
    { m_panel->RequirePreRender(); }

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        const GG::Pt old_size = Size();
        GG::ListBox::Row::SizeMove(ul, lr);
        if (!empty() && old_size != Size() && m_panel){
            GG::Pt border(GG::X(2 * GetLayout()->BorderMargin()), GG::Y(2 * GetLayout()->BorderMargin()));
            m_panel->Resize(lr - ul - border);
        }
    }

    void                    ExpandCollapseClicked()
    { ExpandCollapseSignal(m_panel ? m_panel->ObjectID() : INVALID_OBJECT_ID); }

    mutable boost::signals2::signal<void (int)>   ExpandCollapseSignal;
private:
    ObjectPanel*        m_panel;
    int                 m_container_object_panel;
    std::set<int>       m_contained_object_panels;
    std::shared_ptr<const UniverseObject> m_obj_init;
    bool                m_expanded_init;
    int                 m_indent_init;
};

////////////////////////////////////////////////
// ObjectHeaderPanel
////////////////////////////////////////////////
class ObjectHeaderPanel : public GG::Control {
public:
    ObjectHeaderPanel(GG::X w, GG::Y h) :
        Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
        m_controls()
    {
        SetChildClippingMode(ClipToClient);
    }

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        const GG::Pt old_size = Size();
        GG::Control::SizeMove(ul, lr);
        if (old_size != Size())
            DoLayout();
    }

    void Render() override
    {}

    void            Refresh() {
        for (GG::Button* button : m_controls)
        { DeleteChild(button); }
        m_controls.clear();

        std::vector<GG::Button*> controls = GetControls();
        for (int i = 0; i < static_cast<int>(controls.size()); ++i) {
            m_controls.push_back(controls[i]);
            AttachChild(controls[i]);
            GG::Connect(controls[i]->LeftClickedSignal, boost::bind(&ObjectHeaderPanel::ButtonLeftClicked, this, i-1));
            if (i > 0)
                GG::Connect(controls[i]->RightClickedSignal, boost::bind(&ObjectHeaderPanel::ButtonRightClicked, this, i-1));
        }

        DoLayout();
    }

    mutable boost::signals2::signal<void (int)> ColumnButtonLeftClickSignal;// column clicked, indicating that sorting should be redone
    mutable boost::signals2::signal<void ()>    ColumnsChangedSignal;       // column contents or widths changed, requiring refresh of list

private:
    void                        DoLayout() {
        GG::X left(GG::X0);
        GG::Y top(GG::Y0);
        GG::Y bottom(ClientHeight());

        // loop through m_controls, positioning according to column widths.
        for (std::size_t i = 0; i < m_controls.size(); ++i) {
            GG::Button* ctrl = m_controls[i];
            GG::X width(GetColumnWidth(static_cast<int>(i)-1));

            GG::X right = left + width;

            if (ctrl)
                ctrl->SizeMove(GG::Pt(left, top), GG::Pt(right, bottom));

            left = right + PAD;
        }
    }

    void                        ButtonLeftClicked(int column_id)
    { ColumnButtonLeftClickSignal(column_id); }

    void                        ButtonRightClicked(int column_id) {
        if (column_id < 0 || column_id >= static_cast<int>(m_controls.size()))
            return;
        GG::Button* clicked_button = m_controls[column_id+1];
        if (!clicked_button)
            return;

        std::string current_column_type = GetColumnName(column_id);

        const std::map<std::pair<std::string, std::string>, ValueRef::ValueRefBase<std::string>*>&
            available_column_types = AvailableColumnTypes();

        std::map<int, std::string> menu_index_templates;
        int index = 1;

        GG::MenuItem menu_contents;
        menu_contents.next_level.push_back(GG::MenuItem("", 0, false, current_column_type.empty()));

        GG::MenuItem meters_submenu(UserString("METERS_SUBMENU"),  -1, false, false);
        GG::MenuItem planets_submenu(UserString("PLANETS_SUBMENU"),-2, false, false);
        GG::MenuItem fleets_submenu(UserString("FLEETS_SUBMENU"),  -3, false, false);

        for (const std::map<std::pair<std::string, std::string>, ValueRef::ValueRefBase<std::string>*>::value_type& entry : available_column_types) {
            menu_index_templates[index] = entry.first.first;
            bool check = (current_column_type == entry.first.first);
            const std::string& menu_label = UserString(entry.first.first);
            // put meters into root or submenus...
            if (entry.first.second.empty())
                menu_contents.next_level.push_back(GG::MenuItem(menu_label, index, false, check));
            else if (entry.first.second == "METERS_SUBMENU")
                meters_submenu.next_level.push_back(GG::MenuItem(menu_label, index, false, check));
            else if (entry.first.second == "PLANETS_SUBMENU")
                planets_submenu.next_level.push_back(GG::MenuItem(menu_label, index, false, check));
            else if (entry.first.second == "FLEETS_SUBMENU")
                fleets_submenu.next_level.push_back(GG::MenuItem(menu_label, index, false, check));
            ++index;
        }
        menu_contents.next_level.push_back(meters_submenu);
        menu_contents.next_level.push_back(planets_submenu);
        menu_contents.next_level.push_back(fleets_submenu);


        CUIPopupMenu popup(clicked_button->Left(), clicked_button->Bottom(), menu_contents);
        if (!popup.Run())
            return;
        int selected_menu_item = popup.MenuID();
        if (selected_menu_item < 0) // submenus
            return;
        if (selected_menu_item == 0)// empty column
            SetColumnName(column_id, "");

        // set clicked column to show the selected column type info
        const std::string& selected_type = menu_index_templates[selected_menu_item];
        SetColumnName(column_id, selected_type);

        ColumnsChangedSignal();
    }

    std::vector<GG::Button*>    GetControls() {
        std::vector<GG::Button*> retval;

        GG::Button* control = new CUIButton("-");
        retval.push_back(control);

        for (unsigned int i = 0; i < NUM_COLUMNS; ++i) {
            std::string text;
            const std::string& header_name = GetColumnName(static_cast<int>(i));
            if (!header_name.empty())
                text = UserString(header_name);
            control = new CUIButton(text);
            retval.push_back(control);
        }

        return retval;
    }

    std::vector<GG::Button*>    m_controls;
};

////////////////////////////////////////////////
// ObjectHeaderRow
////////////////////////////////////////////////
class ObjectHeaderRow : public GG::ListBox::Row {
public:
    ObjectHeaderRow(GG::X w, GG::Y h) :
        GG::ListBox::Row(w, h, "", GG::ALIGN_CENTER, 1),
        m_panel(nullptr)
    {
        m_panel = new ObjectHeaderPanel(w, h);
        push_back(m_panel);
        GG::Connect(m_panel->ColumnButtonLeftClickSignal,   ColumnHeaderLeftClickSignal);
        GG::Connect(m_panel->ColumnsChangedSignal,          ColumnsChangedSignal);
    }

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        const GG::Pt old_size = Size();
        GG::ListBox::Row::SizeMove(ul, lr);
        //std::cout << "ObjectRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (!empty() && old_size != Size() && m_panel)
            m_panel->Resize(Size());
    }

    void                    Update()
    { m_panel->Refresh(); }

    mutable boost::signals2::signal<void (int)> ColumnHeaderLeftClickSignal;// column clicked, indicating that sorting should be redone
    mutable boost::signals2::signal<void ()>    ColumnsChangedSignal;       // column contents or widths changed, requiring refresh of list

private:
     ObjectHeaderPanel* m_panel;
};

namespace {
    const std::locale& GetLocale() {
        static boost::locale::generator gen;
        static std::locale loc = gen("en_US.UTF-8");    // should sort accented latin letters reasonably
        return loc;
    }

    struct CustomRowCmp {
        bool operator()(const GG::ListBox::Row& lhs, const GG::ListBox::Row& rhs, std::size_t column) {
            const std::string& lhs_key = lhs.SortKey(column);
            const std::string& rhs_key = rhs.SortKey(column);
            try {
                // attempt to cast sort keys to floats, so that number-aware
                // sorting can be done for columns that contain numbers
                float lhs_val = lhs_key.empty() ? 0.0f : boost::lexical_cast<float>(lhs_key);
                float rhs_val = rhs_key.empty() ? 0.0f : boost::lexical_cast<float>(rhs_key);
                return lhs_val < rhs_val;
            } catch (...) {
                return GetLocale().operator()(static_cast<const ObjectRow&>(lhs).SortKey(column),
                                              static_cast<const ObjectRow&>(rhs).SortKey(column));
            }
        }
    };
}

////////////////////////////////////////////////
// ObjectListBox
////////////////////////////////////////////////
class ObjectListBox : public CUIListBox {
public:
    ObjectListBox() :
        CUIListBox(),
        m_object_change_connections(),
        m_collapsed_objects(),
        m_filter_condition(nullptr),
        m_visibilities(),
        m_header_row(nullptr)
    {
        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        ManuallyManageColProps();
        SetNumCols(1);
        NormalizeRowsOnInsert(false);
        SetSortCmp(CustomRowCmp());

        SetVScrollWheelIncrement(Value(ListRowHeight())*4);

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

        m_header_row = new ObjectHeaderRow(GG::X1, ListRowHeight());
        SetColHeaders(m_header_row); // Gives ownership

        GG::Connect(m_header_row->ColumnsChangedSignal,         &ObjectListBox::Refresh,                this);
        GG::Connect(m_header_row->ColumnHeaderLeftClickSignal,  &ObjectListBox::SortingClicked,         this);
        m_obj_deleted_connection = GG::Connect(GetUniverse().UniverseObjectDeleteSignal,   &ObjectListBox::UniverseObjectDeleted,  this);
    }

    virtual         ~ObjectListBox() {
        delete m_filter_condition;
    }

    void            SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        const GG::Pt old_size = Size();
        Wnd::SizeMove(ul, lr);
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            for (GG::ListBox::Row* row : *this)
                row->Resize(row_size);
            m_header_row->Resize(row_size);
            ListBox::AdjustScrolls(true);
        }
    }

    GG::Pt          ListRowSize() const
    { return GG::Pt(ClientWidth(), ListRowHeight()); }

    static GG::Y    ListRowHeight()
    { return GG::Y(ClientUI::Pts() * 2); }

    const Condition::ConditionBase* const                       FilterCondition() const
    { return m_filter_condition; }

    const std::map<UniverseObjectType, std::set<VIS_DISPLAY>>   Visibilities() const
    { return m_visibilities; }

    void            CollapseObject(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID) {
            for (GG::ListBox::Row* row : *this)
                if (const ObjectRow* object_row = dynamic_cast<const ObjectRow*>(row))
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
        delete m_filter_condition;
        m_filter_condition = condition;
        Refresh();
    }

    void            SetVisibilityFilters(const std::map<UniverseObjectType, std::set<VIS_DISPLAY>>& vis) {
        if (vis != m_visibilities) {
            m_visibilities = vis;
            Refresh();
        }
    }

    void            ClearContents() {
        Clear();
        for (std::map<int, boost::signals2::connection>::value_type& entry : m_object_change_connections)
        { entry.second.disconnect(); }
        m_object_change_connections.clear();
    }

    bool            ObjectShown(std::shared_ptr<const UniverseObject> obj, bool assume_visible_without_checking = false) {
        if (!obj)
            return false;

        if (m_filter_condition && !m_filter_condition->Eval(obj))
            return false;

        int object_id = obj->ID();
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        UniverseObjectType type = obj->ObjectType();

        if (GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id).find(object_id) != GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id).end())
            return m_visibilities[type].find(SHOW_DESTROYED) != m_visibilities[type].end();

        if (assume_visible_without_checking || GetUniverse().GetObjectVisibilityByEmpire(object_id, client_empire_id) >= VIS_PARTIAL_VISIBILITY)
            return m_visibilities[type].find(SHOW_VISIBLE) != m_visibilities[type].end();

        return m_visibilities[type].find(SHOW_PREVIOUSLY_VISIBLE) != m_visibilities[type].end();
    }

    void            Refresh() {
        std::size_t first_visible_queue_row = std::distance(this->begin(), this->FirstRowShown());
        ClearContents();

        m_header_row->Update();

        // sort objects by containment associations
        std::set<int>                   systems;
        std::map<int, std::set<int>>    system_fleets;
        std::map<int, std::set<int>>    fleet_ships;
        std::map<int, std::set<int>>    system_planets;
        std::map<int, std::set<int>>    planet_buildings;
        std::set<int>                   fields;

        for (std::shared_ptr<const UniverseObject> obj : GetUniverse().Objects()) {
            if (!ObjectShown(obj))
                continue;

            int object_id = obj->ID();

            if (obj->ObjectType() == OBJ_SYSTEM) {
                systems.insert(object_id);
            } else if (obj->ObjectType() == OBJ_FIELD) {
                fields.insert(object_id);
            } else if (std::shared_ptr<const Fleet> fleet = std::dynamic_pointer_cast<const Fleet>(obj)) {
                system_fleets[fleet->SystemID()].insert(object_id);
            } else if (std::shared_ptr<const Ship> ship = std::dynamic_pointer_cast<const Ship>(obj)) {
                fleet_ships[ship->FleetID()].insert(object_id);
            } else if (std::shared_ptr<const Planet> planet = std::dynamic_pointer_cast<const Planet>(obj)) {
                system_planets[planet->SystemID()].insert(object_id);
            } else if (std::shared_ptr<const Building> building = std::dynamic_pointer_cast<const Building>(obj)) {
                planet_buildings[building->PlanetID()].insert(object_id);
            }
        }

        int indent = 0;


        // add system rows
        for (int system_id : systems) {
            std::map<int, std::set<int>>::iterator sp_it = system_planets.find(system_id);
            std::map<int, std::set<int>>::iterator sf_it = system_fleets.find(system_id);
            std::set<int> system_contents;
            if (sp_it != system_planets.end())
                system_contents = sp_it->second;
            if (sf_it != system_fleets.end())
                system_contents.insert(sf_it->second.begin(), sf_it->second.end());

            AddObjectRow(system_id, INVALID_OBJECT_ID, system_contents, indent);
            ++indent;

            // add planet rows in this system
            if (sp_it != system_planets.end()) {
                for (int planet_id : sp_it->second) {
                    std::map<int, std::set<int>>::iterator pb_it = planet_buildings.find(planet_id);

                    if (!ObjectCollapsed(system_id)) {
                        AddObjectRow(planet_id, system_id,
                                        pb_it != planet_buildings.end() ? pb_it->second : std::set<int>(),
                                        indent);
                        ++indent;
                    }

                    // add building rows on this planet
                    if (pb_it != planet_buildings.end()) {
                        if (!ObjectCollapsed(planet_id) && !ObjectCollapsed(system_id)) {
                            for (int building_id : pb_it->second) {
                                AddObjectRow(building_id, planet_id, std::set<int>(), indent);
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
                for (int fleet_id : sf_it->second) {
                    std::map<int, std::set<int>>::iterator fs_it = fleet_ships.find(fleet_id);

                    if (!ObjectCollapsed(system_id)) {
                        AddObjectRow(fleet_id, system_id, 
                                        fs_it != fleet_ships.end() ? fs_it->second : std::set<int>(),
                                        indent);
                        ++indent;
                    }

                    // add ship rows in this fleet
                    if (fs_it != fleet_ships.end()) {
                        if (!ObjectCollapsed(fleet_id) && !ObjectCollapsed(system_id)) {
                            for (int ship_id : fs_it->second) {
                                AddObjectRow(ship_id, fleet_id, std::set<int>(), indent);
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
        for (const std::map<int, std::set<int>>::value_type& sp : system_planets) {
            for (int planet_id : sp.second) {
                std::map<int, std::set<int>>::iterator pb_it = planet_buildings.find(planet_id);

                AddObjectRow(planet_id, INVALID_OBJECT_ID,
                                pb_it != planet_buildings.end() ? pb_it->second : std::set<int>(),
                                indent);
                ++indent;

                // add building rows on this planet
                if (pb_it != planet_buildings.end()) {
                    if (!ObjectCollapsed(planet_id)) {
                        for (int building_id : pb_it->second) {
                            AddObjectRow(building_id, planet_id, std::set<int>(), indent);
                        }
                    }
                    planet_buildings.erase(pb_it);
                }

                --indent;
            }
        }
        system_planets.clear();


        // add buildings not in a shown planet
        for (const std::map<int, std::set<int>>::value_type& pb : planet_buildings) {
            for (int building_id : pb.second) {
                AddObjectRow(building_id, INVALID_OBJECT_ID, std::set<int>(), indent);
            }
        }
        planet_buildings.clear();


        // add fleets not in shown systems
        for (const std::map<int, std::set<int>>::value_type& sf : system_fleets) {
            for (int fleet_id : sf.second) {
                std::map<int, std::set<int>>::iterator fs_it = fleet_ships.find(fleet_id);

                AddObjectRow(fleet_id, INVALID_OBJECT_ID,
                                fs_it != fleet_ships.end() ? fs_it->second : std::set<int>(),
                                indent);
                ++indent;

                // add ship rows on this fleet
                if (fs_it != fleet_ships.end()) {
                    if (!ObjectCollapsed(fleet_id)) {
                        for (int ship_id : fs_it->second) {
                            AddObjectRow(ship_id, fleet_id, std::set<int>(), indent);
                        }
                    }
                    fleet_ships.erase(fs_it);
                }
                indent--;
            }
        }
        system_fleets.clear();


        // add any remaining ships not in shown fleets
        for (const std::map<int, std::set<int>>::value_type& fs : fleet_ships) {
            for (int ship_id : fs.second) {
                AddObjectRow(ship_id, INVALID_OBJECT_ID, std::set<int>(), indent);
            }
        }
        fleet_ships.clear();

        for (int field_id : fields)
            AddObjectRow(field_id, INVALID_OBJECT_ID, std::set<int>(), indent);

        if (!this->Empty())
            this->BringRowIntoView(--this->end());
        if (first_visible_queue_row < this->NumRows())
            this->BringRowIntoView(std::next(this->begin(), first_visible_queue_row));
    }

    void            UpdateObjectPanel(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        for (GG::ListBox::Row* row : *this) {
            if (ObjectRow* orow = dynamic_cast<ObjectRow*>(row)) {
                orow->Update();
                return;
            }
        }
    }

    void            SortingClicked(int clicked_column) {
        int                         old_sort_col =  this->SortCol();
        GG::Flags<GG::ListBoxStyle> old_style =     this->Style();

        if (clicked_column < 0) {
            this->SetStyle(GG::LIST_NOSORT);

            // Sorting and nesting don't really work well together. The user may have turned sorting off
            // to get nesting to work. So let's rebuild the world to make sure nesting works again.
            Refresh();
            //std::cout << "col -1 : set style to no sort" << std::endl;

        } else if (!GetColumnName(clicked_column).empty()) { // empty columns are not sort-worthy
            this->SetSortCol(clicked_column);
            if (old_sort_col == clicked_column) {
                // if previously descending sorting, switch to normal sort
                // if previously no sorting, switch to descending sort
                // if previously normal sort, switch to descending sort
                if (old_style & GG::LIST_SORTDESCENDING) {
                    this->SetStyle(GG::LIST_NONE);
                } else {
                    this->SetStyle(GG::LIST_SORTDESCENDING);
                }
            } else {
                // no previously sorting set on this column, so default to descending sort
                this->SetStyle(GG::LIST_SORTDESCENDING);
            }
        }
    }

    mutable boost::signals2::signal<void ()> ExpandCollapseSignal;

private:
    void            AddObjectRow(int object_id, int container, const std::set<int>& contents, int indent)
    { AddObjectRow(object_id, container, contents, indent, this->end()); }

    void            AddObjectRow(int object_id, int container, const std::set<int>& contents,
                                 int indent, GG::ListBox::iterator it)
    {
        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj)
            return;
        const GG::Pt row_size = ListRowSize();
        ObjectRow* object_row = new ObjectRow(row_size.x, row_size.y, obj, !ObjectCollapsed(object_id),
                                              container, contents, indent);
        this->Insert(object_row, it);
        object_row->Resize(row_size);
        GG::Connect(object_row->ExpandCollapseSignal,   &ObjectListBox::ObjectExpandCollapseClicked,
                    this, boost::signals2::at_front);
        m_object_change_connections[obj->ID()].disconnect();
        m_object_change_connections[obj->ID()] = GG::Connect(obj->StateChangedSignal,
            boost::bind(&ObjectListBox::ObjectStateChanged, this, obj->ID()), boost::signals2::at_front);
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
        for (GG::ListBox::Row* row : *this) {
            if (ObjectRow* object_row = dynamic_cast<ObjectRow*>(row)) {
                if (object_row->ObjectID() == container_object_id) {
                    std::set<int> new_contents;
                    for (int content_id : object_row->ContainedPanels()) {
                        if (content_id != object_id)
                            new_contents.insert(content_id);
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
        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(object_id);
        DebugLogger() << "ObjectListBox::ObjectStateChanged: " << obj->Name();
        if (!obj)
            return;

        UniverseObjectType type = obj->ObjectType();
        if (type == OBJ_SHIP || type == OBJ_BUILDING)
            UpdateObjectPanel(object_id);
        else if (type == OBJ_FLEET || type == OBJ_PLANET || type == OBJ_SYSTEM)
            Refresh();
    }

    void            UniverseObjectDeleted(std::shared_ptr<const UniverseObject> obj)
    { if (obj) RemoveObjectRow(obj->ID()); }

    std::map<int, boost::signals2::connection>          m_object_change_connections;
    std::set<int>                                       m_collapsed_objects;
    Condition::ConditionBase*                           m_filter_condition;
    std::map<UniverseObjectType, std::set<VIS_DISPLAY>> m_visibilities;
    ObjectHeaderRow*                                    m_header_row;
    boost::signals2::connection m_obj_deleted_connection;
};

////////////////////////////////////////////////
// ObjectListWnd
////////////////////////////////////////////////
ObjectListWnd::ObjectListWnd(const std::string& config_name) :
    CUIWnd(UserString("MAP_BTN_OBJECTS"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE,
           config_name, false),
    m_list_box(nullptr),
    m_filter_button(nullptr),
    m_collapse_button(nullptr)
{
    m_list_box = new ObjectListBox();
    m_list_box->SetHiliteColor(GG::CLR_ZERO);
    m_list_box->SetStyle(GG::LIST_NOSORT);

    GG::Connect(m_list_box->SelChangedSignal,           &ObjectListWnd::ObjectSelectionChanged, this);
    GG::Connect(m_list_box->DoubleClickedSignal,        &ObjectListWnd::ObjectDoubleClicked,    this);
    GG::Connect(m_list_box->RightClickedSignal,         &ObjectListWnd::ObjectRightClicked,     this);
    GG::Connect(m_list_box->ExpandCollapseSignal,       &ObjectListWnd::DoLayout,               this);
    AttachChild(m_list_box);

    m_filter_button = new CUIButton(UserString("FILTERS"));
    GG::Connect(m_filter_button->LeftClickedSignal,     &ObjectListWnd::FilterClicked,          this);
    AttachChild(m_filter_button);

    m_collapse_button = new CUIButton(UserString("COLLAPSE_ALL"));
    GG::Connect(m_collapse_button->LeftClickedSignal,   &ObjectListWnd::CollapseExpandClicked,  this);
    AttachChild(m_collapse_button);

    DoLayout();
}

void ObjectListWnd::DoLayout() {
    GG::X BUTTON_WIDTH(ClientUI::Pts()*7);
    GG::Y BUTTON_HEIGHT = m_filter_button->MinUsableSize().y;
    int PAD(3);

    GG::Pt button_ul(GG::X0, ClientHeight() - BUTTON_HEIGHT);

    m_filter_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_collapse_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);

    m_list_box->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(ClientWidth(), button_ul.y));

    SetMinSize(GG::Pt(3*BUTTON_WIDTH, 6*BUTTON_HEIGHT));

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

void ObjectListWnd::ObjectSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    // mark as selected all ObjectPanel that are in \a rows and mark as not
    // selected all ObjectPanel that aren't in \a rows
    for (GG::ListBox::iterator it = m_list_box->begin(); it != m_list_box->end(); ++it) {
        bool select_this_row = (rows.find(it) != rows.end());

        GG::ListBox::Row* row = *it;
        if (!row) {
            ErrorLogger() << "ObjectListWnd::ObjectSelectionChanged couldn't get row";
            continue;
        }
        if (row->empty()) {
            ErrorLogger() << "ObjectListWnd::ObjectSelectionChanged got empty row";
            continue;
        }
        GG::Control* control = !row->empty() ? row->at(0) : nullptr;
        if (!control) {
            ErrorLogger() << "ObjectListWnd::ObjectSelectionChanged couldn't get control from row";
            continue;
        }
        ObjectPanel* data_panel = dynamic_cast<ObjectPanel*>(control);
        if (!data_panel) {
            ErrorLogger() << "ObjectListWnd::ObjectSelectionChanged couldn't get ObjectPanel from control";
            continue;
        }
        data_panel->Select(select_this_row);
    }

    SelectedObjectsChangedSignal();
}

void ObjectListWnd::ObjectDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    int object_id = ObjectInRow(it);
    if (object_id != INVALID_OBJECT_ID)
        ObjectDoubleClickedSignal(object_id);
    ClientUI::GetClientUI()->ZoomToObject(object_id);
}

std::set<int> ObjectListWnd::SelectedObjectIDs() const {
    std::set<int> sel_ids;
    const GG::ListBox::SelectionSet sel = m_list_box->Selections();
    for (const GG::ListBox::SelectionSet::value_type& entry : m_list_box->Selections()) {
        ObjectRow *row = dynamic_cast<ObjectRow *>(*entry);
        if (row) {
            int selected_object_id = row->ObjectID();
            if (selected_object_id != INVALID_OBJECT_ID)
                sel_ids.insert(selected_object_id);
        }
    }
    return sel_ids;
}

void ObjectListWnd::SetSelectedObjects(std::set<int> sel_ids) {
    for (GG::ListBox::iterator it = m_list_box->begin(); it != m_list_box->end(); ++it) {
        ObjectRow *row = dynamic_cast<ObjectRow *>(*it);
        if (row) {
            int selected_object_id = row->ObjectID();
            if (selected_object_id != INVALID_OBJECT_ID) {
                if (sel_ids.find(selected_object_id) != sel_ids.end()) {
                    m_list_box->SelectRow(it);
                }
            }
        }
    }
}

void ObjectListWnd::ObjectRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    int object_id = ObjectInRow(it);
    if (object_id == INVALID_OBJECT_ID)
        return;
    HumanClientApp* app = HumanClientApp::GetApp();
    ClientNetworking& net = app->Networking();
    bool moderator = false;
    if (app->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        moderator = true;

    // Right click on an unselected row should automatically select it
    m_list_box->SelectRow(it, true);

    // create popup menu with object commands in it
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("DUMP"), 1, false, false));

    std::shared_ptr<const UniverseObject> obj = GetUniverseObject(object_id);
    //DebugLogger() << "ObjectListBox::ObjectStateChanged: " << obj->Name();
    if (!obj)
        return;

    const int MENUITEM_SET_FOCUS_BASE = 20;
    const int MENUITEM_SET_SHIP_BASE = 50;
    const int MENUITEM_SET_BUILDING_BASE = 250;
    int menuitem_id = MENUITEM_SET_FOCUS_BASE;
    int ship_menuitem_id = MENUITEM_SET_SHIP_BASE;
    int bld_menuitem_id = MENUITEM_SET_BUILDING_BASE;
    std::map<std::string, int> all_foci, avail_blds;
    std::map<int, int> avail_designs;
    UniverseObjectType type = obj->ObjectType();
    Empire* cur_empire = GetEmpire(app->EmpireID());
    if (type == OBJ_PLANET) {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("SP_PLANET_SUITABILITY"), 2, false, false));

        for (const GG::ListBox::SelectionSet::value_type& entry : m_list_box->Selections()) {
            ObjectRow *row = dynamic_cast<ObjectRow *>(*entry);
            if (row) {
                std::shared_ptr<Planet> one_planet = GetPlanet(row->ObjectID());
                if (one_planet && one_planet->OwnedBy(app->EmpireID())) {
                    for (const std::string& planet_focus : one_planet->AvailableFoci())
                        all_foci[planet_focus]++;

                    for (int ship_design : cur_empire->AvailableShipDesigns()) {
                        if (cur_empire->ProducibleItem(BT_SHIP, ship_design, row->ObjectID()))
                            avail_designs[ship_design]++;
                    }

                    for (const std::string& building_type : cur_empire->AvailableBuildingTypes()) {
                        if (cur_empire->EnqueuableItem(BT_BUILDING, building_type, row->ObjectID()) &&
                            cur_empire->ProducibleItem(BT_BUILDING, building_type, row->ObjectID()))
                        {
                            avail_blds[building_type]++;
                        }
                    }
                }
            }
        }
        GG::MenuItem focusMenuItem(UserString("MENUITEM_SET_FOCUS"), 3, false, false);
        for (std::map<std::string, int>::value_type& entry : all_foci) {
            menuitem_id++;
            std::stringstream out;
            out << UserString(entry.first) << " (" << entry.second << ")";
            focusMenuItem.next_level.push_back(GG::MenuItem(out.str(), menuitem_id, false, false));
        }
        if (menuitem_id > MENUITEM_SET_FOCUS_BASE)
            menu_contents.next_level.push_back(focusMenuItem);

        GG::MenuItem ship_menu_item(UserString("MENUITEM_ENQUEUE_SHIPDESIGN"), 4, false, false);
        for (std::map<int, int>::iterator it = avail_designs.begin();
             it != avail_designs.end() && ship_menuitem_id < MENUITEM_SET_BUILDING_BASE; ++it)
        {
            ship_menuitem_id++;
            std::stringstream out;
            out << GetShipDesign(it->first)->Name() << " (" << it->second << ")";
            ship_menu_item.next_level.push_back(GG::MenuItem(out.str(), ship_menuitem_id, false, false));
        }

        if (ship_menuitem_id > MENUITEM_SET_SHIP_BASE)
            menu_contents.next_level.push_back(ship_menu_item);

        GG::MenuItem building_menu_item(UserString("MENUITEM_ENQUEUE_BUILDING"), 5, false, false);
        for (std::map<std::string, int>::value_type& entry : avail_blds) {
            bld_menuitem_id++;
            std::stringstream out;
            out << UserString(entry.first) << " (" << entry.second << ")";
            building_menu_item.next_level.push_back(GG::MenuItem(out.str(), bld_menuitem_id, false, false));
        }

        if (bld_menuitem_id > MENUITEM_SET_BUILDING_BASE)
            menu_contents.next_level.push_back(building_menu_item);
    }
    // moderator actions...
    if (moderator) {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("MOD_DESTROY"),      10, false, false));
        menu_contents.next_level.push_back(GG::MenuItem(UserString("MOD_SET_OWNER"),    11, false, false));
    }

    // run popup and respond
    CUIPopupMenu popup(pt.x, pt.y, menu_contents);
    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: {
            ObjectDumpSignal(object_id);
            break;
        }
        case 2: {
            ClientUI::GetClientUI()->ZoomToPlanetPedia(object_id);
            break;
        }
        case 3: {
                // should never happen, Set Focus parent menu item is disabled
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
        default: {
            int id = popup.MenuID();
            if (id > MENUITEM_SET_FOCUS_BASE && id <= menuitem_id) {
                std::string focus = std::next(all_foci.begin(), id - MENUITEM_SET_FOCUS_BASE - 1)->first;
                for (const GG::ListBox::SelectionSet::value_type& entry : m_list_box->Selections()) {
                    ObjectRow *row = dynamic_cast<ObjectRow *>(*entry);
                    if (row) {
                        std::shared_ptr<Planet> one_planet = GetPlanet(row->ObjectID());
                        if (one_planet && one_planet->OwnedBy(app->EmpireID())) {
                            one_planet->SetFocus(focus);
                            app->Orders().IssueOrder(OrderPtr(new ChangeFocusOrder(app->EmpireID(), one_planet->ID(), focus)));
                         }
                    }
                }
            } else if (id > MENUITEM_SET_SHIP_BASE && id <= ship_menuitem_id) {
                int ship_design = std::next(avail_designs.begin(), id - MENUITEM_SET_SHIP_BASE - 1)->first;
                bool needs_queue_update(false);
                for (const GG::ListBox::SelectionSet::value_type& entry : m_list_box->Selections()) {
                    ObjectRow *row = dynamic_cast<ObjectRow *>(*entry);
                    if (!row)
                        continue;
                    std::shared_ptr<Planet> one_planet = GetPlanet(row->ObjectID());
                    if (!one_planet || !one_planet->OwnedBy(app->EmpireID()) || !cur_empire->ProducibleItem(BT_SHIP, ship_design, row->ObjectID()))
                        continue;
                    ProductionQueue::ProductionItem ship_item(BT_SHIP, ship_design);
                    app->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(app->EmpireID(), ship_item, 1, row->ObjectID())));
                    needs_queue_update = true;
                }
                if (needs_queue_update)
                    cur_empire->UpdateProductionQueue();
            } else if (id > MENUITEM_SET_BUILDING_BASE && id <= bld_menuitem_id) {
                std::string bld = std::next(avail_blds.begin(), id - MENUITEM_SET_BUILDING_BASE - 1)->first;
                bool needs_queue_update(false);
                for (const GG::ListBox::SelectionSet::value_type& entry : m_list_box->Selections()) {
                    ObjectRow *row = dynamic_cast<ObjectRow *>(*entry);
                    if (!row)
                        continue;
                    std::shared_ptr<Planet> one_planet = GetPlanet(row->ObjectID());
                    if (!one_planet || !one_planet->OwnedBy(app->EmpireID())
                       || !cur_empire->EnqueuableItem(BT_BUILDING, bld, row->ObjectID())
                       || !cur_empire->ProducibleItem(BT_BUILDING, bld, row->ObjectID()))
                    {
                        continue;
                    }
                    ProductionQueue::ProductionItem bld_item(BT_BUILDING, bld);
                    app->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(app->EmpireID(), bld_item, 1, row->ObjectID())));
                    needs_queue_update = true;
                }
                if (needs_queue_update)
                    cur_empire->UpdateProductionQueue();
            }

            std::set<int> sel_ids = SelectedObjectIDs();
            Refresh();
            SetSelectedObjects(sel_ids);
            ObjectSelectionChanged(m_list_box->Selections());
            break;
        }
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
    FilterDialog dlg(m_list_box->Visibilities(), m_list_box->FilterCondition());
    dlg.Run();

    if (dlg.ChangesAccepted()) {
        m_list_box->SetVisibilityFilters(dlg.GetVisibilityFilters());
        m_list_box->SetFilterCondition(dlg.GetConditionFilter());
    }
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
