#include "MeterBrowseWnd.h"

#include <GG/DrawUtil.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/Building.h"
#include "../universe/Effect.h"
#include "../universe/Planet.h"
#include "../universe/PopCenter.h"
#include "../universe/UniverseObject.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIDrawUtil.h"
#include "CUIControls.h"

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

    const GG::X     METER_BROWSE_LABEL_WIDTH(300);
    const GG::X     METER_BROWSE_VALUE_WIDTH(50);
}

MeterBrowseWnd::MeterBrowseWnd(int object_id, MeterType primary_meter_type, MeterType secondary_meter_type/* = INVALID_METER_TYPE*/) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH, GG::Y1),
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
        return UserString(EnumToString(meter_type));
    }
}

void MeterBrowseWnd::Initialize() {
    m_row_height = GG::Y(ClientUI::Pts()*3/2);
    const GG::X TOTAL_WIDTH = METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH;

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
        m_current_label->Resize(GG::Pt(METER_BROWSE_LABEL_WIDTH, m_row_height));
        AttachChild(m_current_label);

        m_current_value = new CUILabel("");
        m_current_value->MoveTo(GG::Pt(METER_BROWSE_LABEL_WIDTH, top));
        m_current_value->Resize(GG::Pt(METER_BROWSE_VALUE_WIDTH, m_row_height));
        AttachChild(m_current_value);
        top += m_row_height;

        m_next_turn_label = new CUILabel(UserString("TT_NEXT_TURN"), GG::FORMAT_RIGHT);
        m_next_turn_label->MoveTo(GG::Pt(GG::X0, top));
        m_next_turn_label->Resize(GG::Pt(METER_BROWSE_LABEL_WIDTH, m_row_height));
        AttachChild(m_next_turn_label);

        m_next_turn_value = new CUILabel("");
        m_next_turn_value->MoveTo(GG::Pt(METER_BROWSE_LABEL_WIDTH, top));
        m_next_turn_value->Resize(GG::Pt(METER_BROWSE_VALUE_WIDTH, m_row_height));
        AttachChild(m_next_turn_value);
        top += m_row_height;

        m_change_label = new CUILabel(UserString("TT_CHANGE"), GG::FORMAT_RIGHT);
        m_change_label->MoveTo(GG::Pt(GG::X0, top));
        m_change_label->Resize(GG::Pt(METER_BROWSE_LABEL_WIDTH, m_row_height));
        AttachChild(m_change_label);

        m_change_value = new CUILabel("");
        m_change_value->MoveTo(GG::Pt(METER_BROWSE_LABEL_WIDTH, top));
        m_change_value->Resize(GG::Pt(METER_BROWSE_VALUE_WIDTH, m_row_height));
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

    Resize(GG::Pt(METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH, top));

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

void MeterBrowseWnd::UpdateSummary() {
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_object_id);
    if (!obj)
        return;

    const Meter* primary_meter = obj->GetMeter(m_primary_meter_type);
    const Meter* secondary_meter = obj->GetMeter(m_secondary_meter_type);

    float breakdown_total = 0.0f;
    std::string breakdown_meter_name;

    if (primary_meter && secondary_meter) {
        if (!m_current_value || !m_next_turn_value || !m_change_value) {
            ErrorLogger() << "MeterBrowseWnd::UpdateSummary has primary and secondary meters, but is missing one or more controls to display them";
            return;
        }

        // Primary meter holds value from turn to turn and changes slow each turn.
        // The current value of the primary meter doesn't change with focus changes
        // so its growth from turn to turn is important to show
        float primary_current = obj->InitialMeterValue(m_primary_meter_type);
        float primary_next = obj->NextTurnCurrentMeterValue(m_primary_meter_type);
        float primary_change = primary_next - primary_current;

        m_current_value->SetText(DoubleToString(primary_current, 3, false));
        m_next_turn_value->SetText(DoubleToString(primary_next, 3, false));
        m_change_value->SetText(ColouredNumber(primary_change));

        // target or max meter total for breakdown summary
        breakdown_total = obj->CurrentMeterValue(m_secondary_meter_type);
        breakdown_meter_name = MeterToUserString(m_secondary_meter_type);

    } else if (primary_meter) {
        // unpaired meter total for breakdown summary
        breakdown_total = obj->InitialMeterValue(m_primary_meter_type);
        breakdown_meter_name = MeterToUserString(m_primary_meter_type);
    } else {
        ErrorLogger() << "MeterBrowseWnd::UpdateSummary can't get primary meter";
        return;
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


    // get object and meter, aborting if not valid
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_object_id);
    if (!obj) {
        ErrorLogger() << "MeterBrowseWnd::UpdateEffectLabelsAndValues couldn't get object with id " << m_object_id;
        return;
    }


    // get effect accounting info for this MeterBrowseWnd's object, aborting if non available
    const Universe& universe = GetUniverse();
    const Effect::AccountingMap& effect_accounting_map = universe.GetEffectAccountingMap();
    Effect::AccountingMap::const_iterator map_it = effect_accounting_map.find(m_object_id);
    if (map_it == effect_accounting_map.end())
        return;
    const std::map<MeterType, std::vector<Effect::AccountingInfo> >& meter_map = map_it->second;


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


    // get accounting info for this MeterBrowseWnd's meter type, aborting if none available
    std::map<MeterType, std::vector<Effect::AccountingInfo> >::const_iterator meter_it = meter_map.find(accounting_displayed_for_meter);
    if (meter_it == meter_map.end() || meter_it->second.empty())
        return; // couldn't find appropriate meter type, or there were no entries for that meter.
    const std::vector<Effect::AccountingInfo>& info_vec = meter_it->second;


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
            if (empire = GetEmpire(source->Owner()))
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

        case ECT_UNKNOWN_CAUSE: {
        default:
            const std::string& label_template = (info_it->custom_label.empty()
                ? UserString("TT_UNKNOWN")
                : UserString(info_it->custom_label));
            text += label_template;
        }
        }

        CUILabel* label = new CUILabel(text, GG::FORMAT_RIGHT);
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(METER_BROWSE_LABEL_WIDTH, m_row_height));
        AttachChild(label);

        CUILabel* value = new CUILabel(ColouredNumber(info_it->meter_change));
        value->MoveTo(GG::Pt(METER_BROWSE_LABEL_WIDTH, top));
        value->Resize(GG::Pt(METER_BROWSE_VALUE_WIDTH, m_row_height));
        AttachChild(value);
        m_effect_labels_and_values.push_back(std::pair<CUILabel*, CUILabel*>(label, value));

        top += m_row_height;
    }
}
