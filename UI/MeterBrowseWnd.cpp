#include "MeterBrowseWnd.h"

#include <GG/DrawUtil.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/Building.h"
#include "../universe/Effect.h"
#include "../universe/Planet.h"
#include "../universe/PopCenter.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/UniverseObject.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIDrawUtil.h"
#include "CUIControls.h"

#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/math/special_functions/round.hpp>

namespace {
    /** Returns text wrapped in GG RGBA tags for specified colour */
    std::string ColourWrappedtext(const std::string& text, const GG::Clr colour)
    { return GG::RgbaTag(colour) + text + "</rgba>"; }

    /** Returns text representation of number wrapped in GG RGBA tags for
      * colour depending on whether number is positive, negative or 0.0 */
    std::string ColouredNumber(double number) {
        GG::Clr clr = ClientUI::TextColor();
        if (number > 0.0)
            clr = ClientUI::StatIncrColor();
        else if (number < 0.0)
            clr = ClientUI::StatDecrColor();
        return ColourWrappedtext(DoubleToString(number, 3, true), clr);
    }

    const int       EDGE_PAD(3);

    GG::X MeterBrowseLabelWidth()
    { return GG::X(30*ClientUI::Pts()); }

    GG::X MeterBrowseValueWidth()
    { return GG::X(4*ClientUI::Pts()); }

    GG::X FighterBrowseLabelWidth()
    { return GG::X(10*ClientUI::Pts()); }

}

MeterBrowseWnd::MeterBrowseWnd(int object_id, MeterType primary_meter_type, MeterType secondary_meter_type/* = INVALID_METER_TYPE*/) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, MeterBrowseLabelWidth() + MeterBrowseValueWidth(), GG::Y1),
    m_primary_meter_type(primary_meter_type),
    m_secondary_meter_type(secondary_meter_type),
    m_object_id(object_id),
    m_summary_title(0),
    m_current_label(0), m_current_value(0),
    m_next_turn_label(0), m_next_turn_value(0),
    m_change_label(0), m_change_value(0),
    m_meter_title(0),
    m_row_height(1),
    m_initialized(false)
{}

bool MeterBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void MeterBrowseWnd::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    // main background
    GG::FlatRectangle(ul, lr, OpaqueColor(ClientUI::WndColor()), ClientUI::WndOuterBorderColor(), 1);

    // top title filled background
    if (m_summary_title)
        GG::FlatRectangle(m_summary_title->UpperLeft(), m_summary_title->LowerRight() + GG::Pt(GG::X(EDGE_PAD), GG::Y0), ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);

    // middle title filled background
    if (m_meter_title)
        GG::FlatRectangle(m_meter_title->UpperLeft(), m_meter_title->LowerRight() + GG::Pt(GG::X(EDGE_PAD), GG::Y0), ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);
}

namespace {
    const std::string& MeterToUserString(MeterType meter_type) {
        return UserString(boost::lexical_cast<std::string>(meter_type));
    }
}

void MeterBrowseWnd::Initialize() {
    m_row_height = GG::Y(ClientUI::Pts()*3/2);
    const GG::X TOTAL_WIDTH = MeterBrowseLabelWidth() + MeterBrowseValueWidth();

    // get objects and meters to verify that they exist
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_object_id);
    if (!obj) {
        ErrorLogger() << "MeterBrowseWnd couldn't get object with id " << m_object_id;
        return;
    }
    const Meter* primary_meter = obj->GetMeter(m_primary_meter_type);
    const Meter* secondary_meter = obj->GetMeter(m_secondary_meter_type);

    GG::Y top = GG::Y0;

    // create controls and do layout
    if (primary_meter && secondary_meter) {
        // both primary and secondary meter exist.  display a current value
        // summary at top with current and next turn values

        // special case for meters: use species name
        std::string summary_title_text;
        if (m_primary_meter_type == METER_POPULATION) {
            std::string human_readable_species_name;
            if (TemporaryPtr<const PopCenter> pop = boost::dynamic_pointer_cast<const PopCenter>(obj)) {
                const std::string& species_name = pop->SpeciesName();
                if (!species_name.empty())
                    human_readable_species_name = UserString(species_name);
            }
            summary_title_text = boost::io::str(FlexibleFormat(UserString("TT_SPECIES_POPULATION")) % human_readable_species_name);
        } else {
            summary_title_text = MeterToUserString(m_primary_meter_type);
        }

        m_summary_title = new CUILabel(summary_title_text, GG::FORMAT_RIGHT);
        m_summary_title->MoveTo(GG::Pt(GG::X0, top));
        m_summary_title->Resize(GG::Pt(TOTAL_WIDTH - EDGE_PAD, m_row_height));
        m_summary_title->SetFont(ClientUI::GetBoldFont());
        AttachChild(m_summary_title);
        top += m_row_height;

        m_current_label = new CUILabel(UserString("TT_THIS_TURN"), GG::FORMAT_RIGHT);
        m_current_label->MoveTo(GG::Pt(GG::X0, top));
        m_current_label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(m_current_label);

        m_current_value = new CUILabel("");
        m_current_value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        m_current_value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(m_current_value);
        top += m_row_height;

        m_next_turn_label = new CUILabel(UserString("TT_NEXT_TURN"), GG::FORMAT_RIGHT);
        m_next_turn_label->MoveTo(GG::Pt(GG::X0, top));
        m_next_turn_label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(m_next_turn_label);

        m_next_turn_value = new CUILabel("");
        m_next_turn_value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        m_next_turn_value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(m_next_turn_value);
        top += m_row_height;

        m_change_label = new CUILabel(UserString("TT_CHANGE"), GG::FORMAT_RIGHT);
        m_change_label->MoveTo(GG::Pt(GG::X0, top));
        m_change_label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(m_change_label);

        m_change_value = new CUILabel("");
        m_change_value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        m_change_value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(m_change_value);
        top += m_row_height;
    }

    if (primary_meter) {
        // effect accounting meter breakdown total / summary.  Shows "Meter Name: Value"
        // above a list of effects.  Actual text is set in UpdateSummary() but
        // the control is created here.
        m_meter_title = new CUILabel("", GG::FORMAT_RIGHT);
        m_meter_title->MoveTo(GG::Pt(GG::X0, top));
        m_meter_title->Resize(GG::Pt(TOTAL_WIDTH - EDGE_PAD, m_row_height));
        m_meter_title->SetFont(ClientUI::GetBoldFont());
        AttachChild(m_meter_title);
        top += m_row_height;
    }

    UpdateSummary();

    UpdateEffectLabelsAndValues(top);

    Resize(GG::Pt(MeterBrowseLabelWidth() + MeterBrowseValueWidth(), top));

    m_initialized = true;
}

void MeterBrowseWnd::UpdateImpl(std::size_t mode, const Wnd* target) {
    // because a MeterBrowseWnd's contents depends only on the meters of a single object, if that object doesn't
    // change between showings of the meter browse wnd, it's not necessary to fully recreate the MeterBrowseWnd,
    // and it can be just reshown.without being altered.  To refresh a MeterBrowseWnd, recreate it by assigning
    // a new one as the moused-over object's BrowseWnd in this Wnd's place
    if (!m_initialized)
        Initialize();
}

namespace {
    /** Return the vector of accounting information from \p obj_id of \p meter_type.*/
    boost::optional<const std::vector<Effect::AccountingInfo>& > GetAccountingInfo(
        int obj_id, const MeterType& meter_type)
    {
        // get object and meter, aborting if not valid
        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(obj_id);
        if (!obj) {
            ErrorLogger() << "Couldn't get object with id " << obj_id;
            return boost::none;
        }

        // get effect accounting info for this MeterBrowseWnd's object, aborting if non available
        const Universe& universe = GetUniverse();
        const Effect::AccountingMap& effect_accounting_map = universe.GetEffectAccountingMap();
        Effect::AccountingMap::const_iterator map_it = effect_accounting_map.find(obj_id);
        if (map_it == effect_accounting_map.end())
            return boost::none;
        const std::map<MeterType, std::vector<Effect::AccountingInfo> >& meter_map = map_it->second;

        // get accounting info for this MeterBrowseWnd's meter type, aborting if none available
        std::map<MeterType, std::vector<Effect::AccountingInfo> >::const_iterator meter_it = meter_map.find(meter_type);
        if (meter_it == meter_map.end() || meter_it->second.empty())
            return boost::none;

        const std::vector<Effect::AccountingInfo>& info_vec = meter_it->second;
        return info_vec;
    }
}

namespace DualMeter {

    /** Return the triplet of {Current, Projected, Target} meter value for the pair of meters \p
        actual_meter_type and \p target_meter_type associated with \p obj. */
    boost::tuple<float, float, float> CurrentProjectedTarget(
        const UniverseObject& obj, const MeterType& actual_meter_type, const MeterType& target_meter_type)
    {
        const Meter* actual_meter = obj.GetMeter(actual_meter_type);

        float current = Meter::INVALID_VALUE;
        float projected = Meter::INVALID_VALUE;
        if (actual_meter) {
            current = actual_meter->Initial();
            projected = obj.NextTurnCurrentMeterValue(actual_meter_type);

            // If there is accounting info, correct the projected result by including the
            // results of all known effects in addition to the default meter change.
            if (boost::optional<const std::vector<Effect::AccountingInfo>&>
                maybe_info_vec = GetAccountingInfo(obj.ID(), actual_meter_type))
            {
                const std::vector<Effect::AccountingInfo>& info_vec = *maybe_info_vec;

                projected -= current;
                for (std::vector<Effect::AccountingInfo>::const_iterator info_it = info_vec.begin();
                     info_it != info_vec.end(); ++info_it)
                {
                    if ((info_it->cause_type == ECT_UNKNOWN_CAUSE)
                        || (info_it->cause_type == INVALID_EFFECTS_GROUP_CAUSE_TYPE))
                    {
                        continue;
                    }
                    projected += info_it->meter_change;
                }
            }
        }

        const Meter* target_meter = obj.GetMeter(target_meter_type);
        const float target = target_meter ? target_meter->Current() : Meter::INVALID_VALUE;

        // Clamp projected value with the target value
        if (actual_meter && target_meter
            && ((current <= target && target < projected)
                || (projected < target && target <= current)))
        {
            projected = target;
        }

        // Clamp when there is no target.
        if (!target_meter)
            projected = current;

        return boost::make_tuple(current, projected, target);
    }
}

void MeterBrowseWnd::UpdateSummary() {
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_object_id);
    if (!obj)
        return;

    const Meter* primary_meter = obj->GetMeter(m_primary_meter_type);
    const Meter* secondary_meter = obj->GetMeter(m_secondary_meter_type);

    if (!primary_meter) {
        ErrorLogger() << "MeterBrowseWnd::UpdateSummary can't get primary meter";
        return;
    }

    float breakdown_total = 0.0f;
    std::string breakdown_meter_name;

    if (secondary_meter) {
        if (!m_current_value || !m_next_turn_value || !m_change_value) {
            ErrorLogger() << "MeterBrowseWnd::UpdateSummary has primary and secondary meters, but is missing one or more controls to display them";
            return;
        }

        boost::tuple<float, float, float> current_projected_target = DualMeter::CurrentProjectedTarget(
            *obj, m_primary_meter_type, m_secondary_meter_type);

        m_current_value->SetText(DoubleToString(boost::get<0>(current_projected_target), 3, false));
        m_next_turn_value->SetText(DoubleToString(boost::get<1>(current_projected_target), 3, false));
        float primary_change = boost::get<1>(current_projected_target) - boost::get<0>(current_projected_target);
        m_change_value->SetText(ColouredNumber(primary_change));

        // target or max meter total for breakdown summary
        breakdown_total = boost::get<2>(current_projected_target);
        breakdown_meter_name = MeterToUserString(m_secondary_meter_type);
    } else {
        boost::tuple<float, float, float> current_projected_target = DualMeter::CurrentProjectedTarget(
            *obj, m_primary_meter_type, m_secondary_meter_type);

        // unpaired meter total for breakdown summary
        breakdown_total = boost::get<0>(current_projected_target);
        breakdown_meter_name = MeterToUserString(m_primary_meter_type);
    }

    // set accounting breakdown total / summary
    if (m_meter_title)
        m_meter_title->SetText(boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY")) %
                                              breakdown_meter_name %
                                              DoubleToString(breakdown_total, 3, false)));
}

void MeterBrowseWnd::UpdateEffectLabelsAndValues(GG::Y& top) {
    // clear existing labels
    for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
        DeleteChild(m_effect_labels_and_values[i].first);
        DeleteChild(m_effect_labels_and_values[i].second);
    }
    m_effect_labels_and_values.clear();

    // select which meter type to display accounting for.  if there is a valid
    // secondary meter type, then this is probably the target or max meter and
    // should have accounting displayed.  if there is no valid secondary meter
    // (and there is a valid primary meter) then that is probably an unpaired
    // meter and should have accounting displayed
    MeterType accounting_displayed_for_meter = INVALID_METER_TYPE;
    if (m_secondary_meter_type != INVALID_METER_TYPE)
        accounting_displayed_for_meter = m_secondary_meter_type;
    else if (m_primary_meter_type != INVALID_METER_TYPE)
        accounting_displayed_for_meter = m_primary_meter_type;
    if (accounting_displayed_for_meter == INVALID_METER_TYPE)
        return; // nothing to display

    boost::optional<const std::vector<Effect::AccountingInfo>&>
        maybe_info_vec = GetAccountingInfo(m_object_id, accounting_displayed_for_meter);
    if (!maybe_info_vec)
        return;
    const std::vector<Effect::AccountingInfo>& info_vec = *maybe_info_vec;

    // add label-value pairs for each alteration recorded for this meter
    for (std::vector<Effect::AccountingInfo>::const_iterator info_it = info_vec.begin(); info_it != info_vec.end(); ++info_it) {
        TemporaryPtr<const UniverseObject> source = GetUniverseObject(info_it->source_id);

        const Empire*   empire = 0;
        TemporaryPtr<const Building> building;
        TemporaryPtr<const Planet>   planet;
        //TemporaryPtr<const Ship>     ship = 0;
        std::string     text;
        std::string     name;
        if (source)
            name = source->Name();

        switch (info_it->cause_type) {
        case ECT_TECH: {
            name.clear();
            if ((empire = GetEmpire(source->Owner())))
                name = empire->Name();
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_TECH")
                : UserString(info_it->custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info_it->specific_cause));
            break;
        }
        case ECT_BUILDING: {
            name.clear();
            if (building = boost::dynamic_pointer_cast<const Building>(source))
                if (planet = GetPlanet(building->PlanetID()))
                    name = planet->Name();
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_BUILDING")
                : UserString(info_it->custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info_it->specific_cause));
            break;
        }
        case ECT_FIELD: {
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_FIELD")
                : UserString(info_it->custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info_it->specific_cause));
            break;
        }
        case ECT_SPECIAL: {
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_SPECIAL")
                : UserString(info_it->custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info_it->specific_cause));
            break;
        }
        case ECT_SPECIES: {
            //DebugLogger() << "Effect Species Meter Browse Wnd effect cause " << info_it->specific_cause << " custom label: " << info_it->custom_label;
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_SPECIES")
                : UserString(info_it->custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info_it->specific_cause));
            break;
        }
        case ECT_SHIP_HULL: {
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_SHIP_HULL")
                : UserString(info_it->custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info_it->specific_cause));
            break;
        }
        case ECT_SHIP_PART: {
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_SHIP_PART")
                : UserString(info_it->custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info_it->specific_cause));
            break;
        }
        case ECT_INHERENT:
            text += UserString("TT_INHERENT");
            break;

        case ECT_UNKNOWN_CAUSE:
        default:
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_UNKNOWN")
                : UserString(info_it->custom_label));
            text += label_template;
        }

        GG::Label* label = new CUILabel(text, GG::FORMAT_RIGHT);
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(label);

        GG::Label* value = new CUILabel(ColouredNumber(info_it->meter_change));
        value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(value);
        m_effect_labels_and_values.push_back(std::make_pair(label, value));

        top += m_row_height;
    }
}


ShipDamageBrowseWnd::ShipDamageBrowseWnd(int object_id, MeterType primary_meter_type) :
    MeterBrowseWnd(object_id, primary_meter_type)
{}

void ShipDamageBrowseWnd::Initialize() {
    m_row_height = GG::Y(ClientUI::Pts()*3/2);
    const GG::X TOTAL_WIDTH = MeterBrowseLabelWidth() + MeterBrowseValueWidth();

    // get objects and meters to verify that they exist
    TemporaryPtr<const UniverseObject> ship = GetShip(m_object_id);
    if (!ship) {
        ErrorLogger() << "ShipDamageBrowseWnd couldn't get ship with id " << m_object_id;
        return;
    }
    GG::Y top = GG::Y0;

    // create controls and do layout

    // effect accounting meter breakdown total / summary.  Shows "Meter Name: Value"
    // above a list of effects.  Actual text is set in UpdateSummary() but
    // the control is created here.
    m_meter_title = new CUILabel("", GG::FORMAT_RIGHT);
    m_meter_title->MoveTo(GG::Pt(GG::X0, top));
    m_meter_title->Resize(GG::Pt(TOTAL_WIDTH - EDGE_PAD, m_row_height));
    m_meter_title->SetFont(ClientUI::GetBoldFont());
    AttachChild(m_meter_title);
    top += m_row_height;

    UpdateSummary();

    UpdateEffectLabelsAndValues(top);

    Resize(GG::Pt(MeterBrowseLabelWidth() + MeterBrowseValueWidth(), top));

    m_initialized = true;
}

void ShipDamageBrowseWnd::UpdateImpl(std::size_t mode, const Wnd* target) {
    // because a ShipDamageBrowseWnd's contents depends only on the meters of a single object, if that object doesn't
    // change between showings of the meter browse wnd, it's not necessary to fully recreate the ShipDamageBrowseWnd,
    // and it can be just reshown.without being altered.  To refresh a ShipDamageBrowseWnd, recreate it by assigning
    // a new one as the moused-over object's BrowseWnd in this Wnd's place
    if (!m_initialized)
        Initialize();
}

void ShipDamageBrowseWnd::UpdateSummary() {
    TemporaryPtr<const Ship> ship = GetShip(m_object_id);
    if (!ship)
        return;

    // unpaired meter total for breakdown summary
    float breakdown_total = ship->TotalWeaponsDamage(0.0f, false);
    std::string breakdown_meter_name = UserString("SHIP_DAMAGE_STAT_TITLE");


    // set accounting breakdown total / summary
    if (m_meter_title)
        m_meter_title->SetText(boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY")) %
                                              breakdown_meter_name %
                                              DoubleToString(breakdown_total, 3, false)));
}

void ShipDamageBrowseWnd::UpdateEffectLabelsAndValues(GG::Y& top) {
    // clear existing labels
    for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
        DeleteChild(m_effect_labels_and_values[i].first);
        DeleteChild(m_effect_labels_and_values[i].second);
    }
    m_effect_labels_and_values.clear();

    // get object and meter, aborting if not valid
    TemporaryPtr<const Ship> ship = GetShip(m_object_id);
    if (!ship) {
        ErrorLogger() << "ShipDamageBrowseWnd::UpdateEffectLabelsAndValues couldn't get ship with id " << m_object_id;
        return;
    }

    const ShipDesign* design = GetShipDesign(ship->DesignID());
    if (!design)
        return;
    const std::vector<std::string>& parts = design->Parts();

    std::string     name = ship->Name();
    const std::string& label_template = UserString("TT_SHIP_PART");

    // for each weapon part, get its damage meter value
    for (std::vector<std::string>::const_iterator part_it = parts.begin();
         part_it != parts.end(); ++part_it)
    {
        const std::string& part_name = *part_it;
        const PartType* part = GetPartType(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();
        if (!(part_class == PC_DIRECT_WEAPON))
            continue;

        // get the attack power for each weapon part
        float part_attack = ship->CurrentPartMeterValue(METER_CAPACITY, part_name) *
                            ship->CurrentPartMeterValue(METER_SECONDARY_STAT, part_name);
        std::string text = boost::io::str(FlexibleFormat(label_template) % name % UserString(part_name));

        GG::Label* label = new CUILabel(text, GG::FORMAT_RIGHT);
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(label);

        GG::Label* value = new CUILabel(ColouredNumber(part_attack));
        value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(value);
        m_effect_labels_and_values.push_back(std::make_pair(label, value));

        top += m_row_height;
    }

}

namespace {
    class ShipFightersBrowseRow : public GG::ListBox::Row {
    public:
        ShipFightersBrowseRow(const std::string& label, double value, double base_value = 0.0f) :
                GG::ListBox::Row(FighterBrowseLabelWidth() + MeterBrowseValueWidth(), GG::Y1, "")
        {
            CUILabel* label_control = new CUILabel(label, GG::FORMAT_RIGHT);
            push_back(label_control);
            label_control->Resize(GG::Pt(FighterBrowseLabelWidth(), GG::Y(ClientUI::Pts()*3/2)));

            std::string value_text;
            if (base_value > 0.0f) {
                std::string base_value_text = ColourWrappedtext(boost::lexical_cast<std::string>(base_value),
                                                                ClientUI::StatIncrColor());
                value_text = boost::io::str(FlexibleFormat(UserString("TT_N_OF_N"))
                                            % boost::lexical_cast<std::string>(value) % base_value_text);
            } else {
                value_text = ColourWrappedtext(boost::lexical_cast<std::string>(value), ClientUI::StatIncrColor());
            }
            CUILabel* value_control = new CUILabel(value_text);
            push_back(value_control);
            value_control->Resize(GG::Pt(MeterBrowseValueWidth(), GG::Y(ClientUI::Pts()*3/2)));

            Resize(GG::Pt(label_control->Width() + value_control->Width(),
                          std::max(label_control->Height(), value_control->Height())));
        }
    };
}

ShipFightersBrowseWnd::ShipFightersBrowseWnd(int object_id, MeterType primary_meter_type, bool show_all_bouts /* = false*/) :
    MeterBrowseWnd(object_id, primary_meter_type),
    m_bay_list(0),
    m_hangar_list(0),
    m_show_all_bouts(show_all_bouts)
{}

void ShipFightersBrowseWnd::Initialize() {
    m_row_height = GG::Y(ClientUI::Pts()*3/2);
    const GG::X LABEL_WIDTH = FighterBrowseLabelWidth();
    const GG::X VALUE_WIDTH = MeterBrowseValueWidth();
    const GG::X TOTAL_WIDTH = ((LABEL_WIDTH + VALUE_WIDTH) * 2) + (EDGE_PAD * 3);
    const GG::Clr BG_COLOR = ClientUI::WndColor();

    // get objects and meters to verify that they exist
    TemporaryPtr<const UniverseObject> ship = GetShip(m_object_id);
    if (!ship) {
        ErrorLogger() << "Couldn't get ship with id " << m_object_id;
        return;
    }
    GG::Y top = GG::Y0;

    // create controls and do layout
    m_meter_title = new CUILabel("", GG::FORMAT_RIGHT);
    m_meter_title->MoveTo(GG::Pt(GG::X0, top));
    m_meter_title->Resize(GG::Pt(TOTAL_WIDTH, m_row_height));
    m_meter_title->SetFont(ClientUI::GetBoldFont());
    AttachChild(m_meter_title);

    top += m_row_height;

    m_bay_list = new CUIListBox();
    m_bay_list->SetColor(BG_COLOR);
    m_bay_list->SetInteriorColor(BG_COLOR);
    m_bay_list->ManuallyManageColProps();
    m_bay_list->SetNumCols(2);
    m_bay_list->SetColWidth(0, LABEL_WIDTH - EDGE_PAD);
    m_bay_list->SetColWidth(1, VALUE_WIDTH - EDGE_PAD);
    m_bay_list->MoveTo(GG::Pt(GG::X0, top));
    AttachChild(m_bay_list);

    m_hangar_list = new CUIListBox();
    m_hangar_list->SetColor(BG_COLOR);
    m_hangar_list->SetInteriorColor(BG_COLOR);
    m_hangar_list->ManuallyManageColProps();
    m_hangar_list->SetNumCols(2);
    m_hangar_list->SetColWidth(0, LABEL_WIDTH - EDGE_PAD);
    m_hangar_list->SetColWidth(1, VALUE_WIDTH - EDGE_PAD);
    m_hangar_list->MoveTo(GG::Pt(LABEL_WIDTH + VALUE_WIDTH + EDGE_PAD, top));
    AttachChild(m_hangar_list);

    UpdateSummary();

    UpdateEffectLabelsAndValues(top);

    Resize(GG::Pt(TOTAL_WIDTH + EDGE_PAD, top + EDGE_PAD));

    m_initialized = true;
}

void ShipFightersBrowseWnd::UpdateImpl(std::size_t mode, const Wnd* target) {
    if (!m_initialized)
        Initialize();
}

void ShipFightersBrowseWnd::UpdateSummary() {
    TemporaryPtr<const Ship> ship = GetShip(m_object_id);
    if (!ship)
        return;

    // unpaired meter total for breakdown summary
    float breakdown_total = ship->FighterCount();
    std::string breakdown_meter_name = UserString("SHIP_FIGHTERS_TITLE");


    // set accounting breakdown total / summary
    if (m_meter_title)
        m_meter_title->SetText(boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY")) %
                                              breakdown_meter_name %
                                              DoubleToString(breakdown_total, 3, false)));
}

namespace {
    /** Number of fighters that participated for @p bout and total damage inflicted
     *  If @p bout is less than 1 (or unassigned), total damage is summed for all rounds
     */
    std::pair<int, float> FightersDamageForBout(int total_bay_capacity, int current_docked_fighters,
                                                float fighter_damage, int bout = -1)
    {
        int last_bout = bout < 1 ? ShipFightersBrowseWnd::NUM_COMBAT_BOUTS : bout;
        std::pair<int, float> retval = { 0, 0.0f };
        int fighters_next_bout = 0;
        int remaining_docked_fighters = current_docked_fighters;

        for (int i = 1; i <= last_bout; ++i) {
            retval.first += fighters_next_bout;
            if (bout == last_bout)
                retval.second = retval.first * fighter_damage;
            else 
                retval.second += retval.first * fighter_damage;

            // launch fighters
            fighters_next_bout = std::min(total_bay_capacity, remaining_docked_fighters);
            remaining_docked_fighters -= fighters_next_bout;
        }

        return retval;
    }
}

void ShipFightersBrowseWnd::UpdateEffectLabelsAndValues(GG::Y& top) {
    // clear existing labels
    for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
        DeleteChild(m_effect_labels_and_values[i].first);
        DeleteChild(m_effect_labels_and_values[i].second);
    }
    m_effect_labels_and_values.clear();
    m_bay_list->DeleteChildren();
    m_hangar_list->DeleteChildren();

    // early return if no valid ship, ship design, or no parts in the design
    TemporaryPtr<const Ship> ship = GetShip(m_object_id);
    if (!ship) {
        ErrorLogger() << "Couldn't get ship with id " << m_object_id;
        return;
    }
    const ShipDesign* design = GetShipDesign(ship->DesignID());
    if (!design)
        return;
    const std::vector<std::string>& parts = design->Parts();
    if (parts.empty())
        return;

    const GG::Clr DAMAGE_COLOR = GG::CLR_ORANGE;
    const GG::Clr HIGHLIGHT_COLOR = GG::CLR_YELLOW;
    const GG::X LABEL_WIDTH = FighterBrowseLabelWidth();
    const GG::X VALUE_WIDTH = MeterBrowseValueWidth();
    const GG::X ROW_WIDTH = LABEL_WIDTH + EDGE_PAD + VALUE_WIDTH;

    float fighter_damage = 0.0f;
    std::pair<std::string, int> hangar_part;  // name, count
    std::multimap<std::string, int> bay_parts;  // name, capacity
    int hangar_current_fighters = 0;
    int hangar_total_capacity = 0;
    int bay_total_capacity = 0;

    // populate values from hangars and bays
    for (std::string part_name : parts) {
        const PartType* part = GetPartType(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();

        if (part_class == PC_FIGHTER_BAY) {
            float current_capacity = ship->CurrentPartMeterValue(METER_CAPACITY, part_name);
            // store the current part and capacity, increase total bay capacity
            bay_parts.insert(std::make_pair(part_name, current_capacity));
            bay_total_capacity += current_capacity;
            // TODO Account for METER_MAX_CAPACITY if this control is expanded for more detail

        } else if (part_class == PC_FIGHTER_HANGAR) {
            if (hangar_part.first.empty()) {
                hangar_part.first = part_name;
            } else if (hangar_part.first != part_name) {
                ErrorLogger() << "Ship " << ship->ID() << "contains different hangar parts: "
                              << hangar_part.first << ", " << part_name;
            }
            // set the current and total fighter capacity
            hangar_current_fighters = ship->CurrentPartMeterValue(METER_CAPACITY, part_name);
            hangar_total_capacity = ship->CurrentPartMeterValue(METER_MAX_CAPACITY, part_name);
            fighter_damage = ship->CurrentPartMeterValue(METER_SECONDARY_STAT, part_name);
            // hangars share the same ship meter, increase the count for later processing
            hangar_part.second++;
        }
    }

    //  no fighters, early return
    if (hangar_part.second <= 0)
        return;

    // Add list summary labels
    std::string bay_value_text = boost::lexical_cast<std::string>(bay_total_capacity);
    std::string hangar_current_text = boost::lexical_cast<std::string>(hangar_current_fighters);

    // highlight the lower value if not showing all bouts
    if (!m_show_all_bouts) {
        if (hangar_current_fighters <= bay_total_capacity)
            hangar_current_text = ColourWrappedtext(hangar_current_text, HIGHLIGHT_COLOR);
        else
            bay_value_text = ColourWrappedtext(bay_value_text, HIGHLIGHT_COLOR);
    }

    std::string hangar_total_text = boost::lexical_cast<std::string>(hangar_total_capacity);
    std::string hangar_value_text = boost::io::str(FlexibleFormat(UserString("TT_N_OF_N"))
                                                   % hangar_current_text % hangar_total_text);

    GG::Label* bay_summary = new CUILabel(boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY"))
                                                         % UserString("SHIP_FIGHTER_BAY_SUMMARY") % bay_value_text));
    bay_summary->SetFont(ClientUI::GetBoldFont());
    bay_summary->MoveTo(GG::Pt(GG::X0, top));
    bay_summary->Resize(GG::Pt(LABEL_WIDTH + VALUE_WIDTH, m_row_height));
    AttachChild(bay_summary);

    GG::Label* hangar_summary = new CUILabel(boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY"))
                                                            % UserString("SHIP_FIGHTER_HANGAR_SUMMARY") % hangar_value_text));
    hangar_summary->SetFont(ClientUI::GetBoldFont());
    hangar_summary->MoveTo(GG::Pt(ROW_WIDTH, top));
    hangar_summary->Resize(GG::Pt(LABEL_WIDTH + VALUE_WIDTH, m_row_height));
    AttachChild(hangar_summary);
    m_effect_labels_and_values.push_back(std::make_pair(bay_summary, hangar_summary));

    top += m_row_height;

    std::string fighter_damage_text = ColourWrappedtext(DoubleToString(fighter_damage, 3, false), DAMAGE_COLOR);

    if (!m_show_all_bouts) {
        // Show damage for first wave (2nd combat round)
        std::pair<int, float> first_wave = FightersDamageForBout(bay_total_capacity, hangar_current_fighters, fighter_damage, 2);
        std::string launch_text = ColourWrappedtext(boost::lexical_cast<std::string>(first_wave.first), HIGHLIGHT_COLOR);
        std::string damage_label_text = boost::io::str(FlexibleFormat(UserString("TT_FIGHTER_DAMAGE"))
                                                       % launch_text % fighter_damage_text);
        GG::Label* damage_label = new CUILabel(damage_label_text, GG::FORMAT_RIGHT);
        damage_label->MoveTo(GG::Pt(GG::X0, top));
        damage_label->Resize(GG::Pt(LABEL_WIDTH * 2 + EDGE_PAD, m_row_height));
        AttachChild(damage_label);

        std::string damage_value_text = DoubleToString(first_wave.second, 3, false);
        GG::Label* damage_value = new CUILabel(ColourWrappedtext(damage_value_text, DAMAGE_COLOR));
        damage_value->MoveTo(GG::Pt(LABEL_WIDTH * 2, top));
        damage_value->Resize(GG::Pt(VALUE_WIDTH * 2, m_row_height));
        damage_value->SetFont(ClientUI::GetBoldFont());
        AttachChild(damage_value);
        m_effect_labels_and_values.push_back(std::make_pair(damage_label, damage_value));

        top += m_row_height;
    } else {
        // add labels for bay parts
        for (std::multimap<std::string, int>::value_type& bay_part : bay_parts) {
            const std::string& part_name = UserString(bay_part.first);
            int fighter_total = bay_part.second;
            m_bay_list->Insert(new ShipFightersBrowseRow(part_name, fighter_total));
        }

        // add a label for each hangar part, distributing the total fighters equally
        int unaccounted_current_fighters = hangar_current_fighters;
        int unaccounted_total_fighters = hangar_total_capacity;
        const std::string& hangar_name = UserString(hangar_part.first);
        for (int i = 0; i < hangar_part.second; ++i) {
            int max_fighters = std::min(unaccounted_total_fighters, hangar_total_capacity / hangar_part.second);
            // top heavy when current fighters do not distribute equally
            int cur_fighters = std::min(unaccounted_current_fighters, std::min(max_fighters,
                boost::math::iround(static_cast<float>(hangar_current_fighters) / hangar_part.second)));
            unaccounted_current_fighters -= cur_fighters;
            unaccounted_total_fighters -= max_fighters;
            m_hangar_list->Insert(new ShipFightersBrowseRow(hangar_name, cur_fighters, max_fighters));
        }
        // validate that all fighters were accounted for
        if (unaccounted_current_fighters != 0 || unaccounted_total_fighters != 0) {
            DebugLogger() << "Display of number of fighters mismatched.  Current: "
                          << unaccounted_current_fighters << "/" << hangar_current_fighters
                          << " Total: " << unaccounted_total_fighters << "/" << hangar_total_capacity
                          << "  For ship " << ship->ID() << " with design: \n" << design->Dump();
        }

        int max_rows = static_cast<int>(std::max(m_bay_list->NumRows(), m_hangar_list->NumRows()));
        int list_vpad = max_rows > 0 ? 10 : 0;
        const GG::Y LIST_HEIGHT = (m_row_height * max_rows) + list_vpad;

        m_bay_list->MoveTo(GG::Pt(GG::X0, top));
        m_bay_list->Resize(GG::Pt(ROW_WIDTH, LIST_HEIGHT));
        m_hangar_list->MoveTo(GG::Pt(ROW_WIDTH + EDGE_PAD, top));
        m_hangar_list->Resize(GG::Pt(ROW_WIDTH, LIST_HEIGHT));

        top += LIST_HEIGHT;

        // Damage summary labels
        float total_dmg = FightersDamageForBout(bay_total_capacity, hangar_current_fighters, fighter_damage).second;
        std::string avg_val =  ColourWrappedtext(DoubleToString(total_dmg / NUM_COMBAT_BOUTS, 3, false), DAMAGE_COLOR);
        std::string detail_avg_text = boost::io::str(FlexibleFormat(UserString("SHIP_FIGHTERS_DAMAGE_AVERAGE")) % avg_val);
        std::string total_val = ColourWrappedtext(DoubleToString(total_dmg, 3, false), DAMAGE_COLOR);
        std::string detail_value_text = boost::io::str(FlexibleFormat(UserString("SHIP_FIGHTERS_DAMAGE_TOTAL")) % total_val);

        GG::Label* detail_summary_avg = new CUILabel(detail_avg_text, GG::FORMAT_RIGHT);
        detail_summary_avg->MoveTo(GG::Pt(GG::X0, top));
        detail_summary_avg->Resize(GG::Pt(LABEL_WIDTH + VALUE_WIDTH, m_row_height));
        AttachChild(detail_summary_avg);

        GG::Label* detail_summary_total = new CUILabel(detail_value_text, GG::FORMAT_RIGHT);
        detail_summary_total->MoveTo(GG::Pt(ROW_WIDTH, top));
        detail_summary_total->Resize(GG::Pt(LABEL_WIDTH + VALUE_WIDTH, m_row_height));
        detail_summary_total->SetFont(ClientUI::GetBoldFont());
        AttachChild(detail_summary_total);
        m_effect_labels_and_values.push_back(std::make_pair(detail_summary_avg, detail_summary_total));

        top += m_row_height;

        // Damage labels for each combat round
        for (int i = 2; i <= NUM_COMBAT_BOUTS; ++i) {
            std::pair<int, float> current_bout = FightersDamageForBout(bay_total_capacity, hangar_current_fighters,
                                                                       fighter_damage, i);
            std::string launch_text = boost::lexical_cast<std::string>(current_bout.first);
            std::string damage_label_text = boost::io::str(FlexibleFormat(UserString("TT_FIGHTER_DAMAGE"))
                                                           % launch_text % fighter_damage_text);
            std::string bout_text = boost::io::str(FlexibleFormat(UserString("TT_COMBAT_ROUND"))
                                                   % boost::lexical_cast<std::string>(i));
            damage_label_text = boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY"))
                                               % bout_text % damage_label_text);

            GG::Label* bout_label = new CUILabel(damage_label_text, GG::FORMAT_RIGHT);
            bout_label->MoveTo(GG::Pt(GG::X0, top));
            bout_label->Resize(GG::Pt(LABEL_WIDTH * 2 + EDGE_PAD, m_row_height));
            AttachChild(bout_label);

            std::string bout_value_text = DoubleToString(current_bout.second, 3, false);
            GG::Label* bout_value = new CUILabel(ColourWrappedtext(bout_value_text, DAMAGE_COLOR));
            bout_value->MoveTo(GG::Pt(LABEL_WIDTH * 2, top));
            bout_value->Resize(GG::Pt(VALUE_WIDTH * 2, m_row_height));
            AttachChild(bout_value);
            m_effect_labels_and_values.push_back(std::make_pair(bout_label, bout_value));

            top += m_row_height;
        }
    }
}
