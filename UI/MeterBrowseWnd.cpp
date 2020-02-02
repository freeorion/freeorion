#include "MeterBrowseWnd.h"

#include <GG/DrawUtil.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/GameRules.h"
#include "../universe/Building.h"
#include "../universe/Effect.h"
#include "../universe/Planet.h"
#include "../universe/PopCenter.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/UniverseObject.h"
#include "../universe/Enums.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIDrawUtil.h"
#include "CUIControls.h"

#include <boost/optional.hpp>

namespace {
    /** Returns text wrapped in GG RGBA tags for specified colour */
    std::string ColourWrappedtext(const std::string& text, const GG::Clr colour)
    { return GG::RgbaTag(colour) + text + "</rgba>"; }

    /** Returns text representation of number wrapped in GG RGBA tags for
      * colour depending on whether number is positive, negative or 0.0 */
    std::string ColouredNumber(double number, unsigned int digits = 3) {
        GG::Clr clr = ClientUI::TextColor();
        if (number > 0.0)
            clr = ClientUI::StatIncrColor();
        else if (number < 0.0)
            clr = ClientUI::StatDecrColor();
        return ColourWrappedtext(DoubleToString(number, digits, true), clr);
    }

    /** Cast int to string, prepend sign if requested */
    std::string IntToString(int number, bool prepend = false) {
        std::string prepend_text = "";
        if (prepend) {
            if (number >= 0)
                prepend_text = "+";
            else if (number < 0)
                prepend_text = "-";
        }
        return prepend_text + std::to_string(number);
    }

    /** Returns a string representation of @p number wrapped in GG RGBA tags for @p clr.
     *  If @p number is less than 0, wraps with @p neg_clr instead. */
    std::string ColouredInt(int number, bool prepend = true, GG::Clr clr = ClientUI::StatIncrColor(),
                            GG::Clr neg_clr = ClientUI::StatDecrColor())
    {
        if (number < 0)
            clr = neg_clr;
        return ColourWrappedtext(IntToString(number, prepend), clr);
    }

    const int EDGE_PAD(3);

    GG::X MeterBrowseLabelWidth()
    { return GG::X(30*ClientUI::Pts()); }

    GG::X MeterBrowseQtyWidth()
    { return GG::X(3*ClientUI::Pts()); }

    GG::X MeterBrowseValueWidth()
    { return GG::X(4*ClientUI::Pts()); }

    const GG::Y MeterBrowseRowHeight()
    { return GG::Y(ClientUI::Pts()*3/2); }

    GG::X FighterBrowseLabelWidth()
    { return GG::X(10*ClientUI::Pts()); }

    GG::X FighterBrowseListWidth()
    { return FighterBrowseLabelWidth() + MeterBrowseQtyWidth() + MeterBrowseValueWidth() + (EDGE_PAD * 4); }
}

MeterBrowseWnd::MeterBrowseWnd(int object_id, MeterType primary_meter_type) :
    MeterBrowseWnd(object_id, primary_meter_type, INVALID_METER_TYPE)
{}

MeterBrowseWnd::MeterBrowseWnd(int object_id, MeterType primary_meter_type,
                               MeterType secondary_meter_type) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, MeterBrowseLabelWidth() + MeterBrowseValueWidth(), GG::Y1),
    m_primary_meter_type(primary_meter_type),
    m_secondary_meter_type(secondary_meter_type),
    m_object_id(object_id)
{}

bool MeterBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void MeterBrowseWnd::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    // main background
    GG::FlatRectangle(ul, lr, OpaqueColor(ClientUI::WndColor()),
                      ClientUI::WndOuterBorderColor(), 1);

    // top title filled background
    if (m_summary_title)
        GG::FlatRectangle(m_summary_title->UpperLeft(),
                          m_summary_title->LowerRight() + GG::Pt(GG::X(EDGE_PAD), GG::Y0),
                          ClientUI::WndOuterBorderColor(),
                          ClientUI::WndOuterBorderColor(), 0);

    // middle title filled background
    if (m_meter_title)
        GG::FlatRectangle(m_meter_title->UpperLeft(),
                          m_meter_title->LowerRight() + GG::Pt(GG::X(EDGE_PAD), GG::Y0),
                          ClientUI::WndOuterBorderColor(),
                          ClientUI::WndOuterBorderColor(), 0);
}

namespace {
    const std::string& MeterToUserString(MeterType meter_type) {
        return UserString(boost::lexical_cast<std::string>(meter_type));
    }
}

void MeterBrowseWnd::Initialize() {
    m_row_height = MeterBrowseRowHeight();
    const GG::X TOTAL_WIDTH = MeterBrowseLabelWidth() + MeterBrowseValueWidth();

    // get objects and meters to verify that they exist
    auto obj = GetUniverseObject(m_object_id);
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
            if (auto pop = std::dynamic_pointer_cast<const PopCenter>(obj)) {
                const std::string& species_name = pop->SpeciesName();
                if (!species_name.empty())
                    human_readable_species_name = UserString(species_name);
            }
            summary_title_text = boost::io::str(FlexibleFormat(UserString("TT_SPECIES_POPULATION")) % human_readable_species_name);
        } else {
            summary_title_text = MeterToUserString(m_primary_meter_type);
        }

        m_summary_title = GG::Wnd::Create<CUILabel>(summary_title_text, GG::FORMAT_RIGHT);
        m_summary_title->MoveTo(GG::Pt(GG::X0, top));
        m_summary_title->Resize(GG::Pt(TOTAL_WIDTH - EDGE_PAD, m_row_height));
        m_summary_title->SetFont(ClientUI::GetBoldFont());
        AttachChild(m_summary_title);
        top += m_row_height;

        m_current_label = GG::Wnd::Create<CUILabel>(UserString("TT_THIS_TURN"), GG::FORMAT_RIGHT);
        m_current_label->MoveTo(GG::Pt(GG::X0, top));
        m_current_label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(m_current_label);

        m_current_value = GG::Wnd::Create<CUILabel>("");
        m_current_value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        m_current_value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(m_current_value);
        top += m_row_height;

        m_next_turn_label = GG::Wnd::Create<CUILabel>(UserString("TT_NEXT_TURN"), GG::FORMAT_RIGHT);
        m_next_turn_label->MoveTo(GG::Pt(GG::X0, top));
        m_next_turn_label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(m_next_turn_label);

        m_next_turn_value = GG::Wnd::Create<CUILabel>("");
        m_next_turn_value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        m_next_turn_value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(m_next_turn_value);
        top += m_row_height;

        m_change_label = GG::Wnd::Create<CUILabel>(UserString("TT_CHANGE"), GG::FORMAT_RIGHT);
        m_change_label->MoveTo(GG::Pt(GG::X0, top));
        m_change_label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(m_change_label);

        m_change_value = GG::Wnd::Create<CUILabel>("");
        m_change_value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        m_change_value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(m_change_value);
        top += m_row_height;
    }

    if (primary_meter) {
        // effect accounting meter breakdown total / summary.  Shows "Meter Name: Value"
        // above a list of effects.  Actual text is set in UpdateSummary() but
        // the control is created here.
        m_meter_title = GG::Wnd::Create<CUILabel>("", GG::FORMAT_RIGHT);
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
    // because a MeterBrowseWnd's contents depends only on the meters of a
    // single object, if that object doesn't change between showings of the
    // meter browse wnd, it's not necessary to fully recreate the
    // MeterBrowseWnd, and it can be just reshown.without being altered. To
    // refresh a MeterBrowseWnd, recreate it by assigning a new one as the
    // moused-over object's BrowseWnd in this Wnd's place
    if (!m_initialized)
        Initialize();
}

namespace {
    /** Return the vector of accounting information from \p obj_id of \p meter_type.*/
    boost::optional<const std::vector<Effect::AccountingInfo>&> GetAccountingInfo(
        int obj_id, const MeterType& meter_type)
    {
        // get object and meter, aborting if not valid
        auto obj = GetUniverseObject(obj_id);
        if (!obj) {
            ErrorLogger() << "Couldn't get object with id " << obj_id;
            return boost::none;
        }

        // get effect accounting info for this MeterBrowseWnd's object, aborting if non available
        const Universe& universe = GetUniverse();
        const auto& effect_accounting_map = universe.GetEffectAccountingMap();
        auto map_it = effect_accounting_map.find(obj_id);
        if (map_it == effect_accounting_map.end())
            return boost::none;
        const auto& meter_map = map_it->second;

        // get accounting info for this MeterBrowseWnd's meter type, aborting if none available
        auto meter_it = meter_map.find(meter_type);
        if (meter_it == meter_map.end() || meter_it->second.empty())
            return boost::none;

        return meter_it->second;
    }
}

void MeterBrowseWnd::UpdateSummary() {
    auto obj = GetUniverseObject(m_object_id);
    if (!obj)
        return;

    const Meter* primary_meter = obj->GetMeter(m_primary_meter_type);
    if (!primary_meter) {
        ErrorLogger() << "MeterBrowseWnd::UpdateSummary can't get primary meter";
        return;
    }
    const Meter* secondary_meter = obj->GetMeter(m_secondary_meter_type);

    float breakdown_total = 0.0f;
    std::string breakdown_meter_name;

    if (secondary_meter) {
        if (!m_current_value || !m_next_turn_value || !m_change_value) {
            ErrorLogger() << "MeterBrowseWnd::UpdateSummary has primary and secondary meters, but is missing one or more controls to display them";
            return;
        }

        m_current_value->SetText(DoubleToString(primary_meter->Initial(), 3, false));
        m_next_turn_value->SetText(DoubleToString(primary_meter->Current(), 3, false));
        float primary_change = primary_meter->Current() - primary_meter->Initial();
        m_change_value->SetText(ColouredNumber(primary_change, 3));

        // target or max meter total for breakdown summary
        breakdown_total = secondary_meter->Current();
        breakdown_meter_name = MeterToUserString(m_secondary_meter_type);

    } else {    // no secondary meter
        // unpaired meter total for breakdown summary
        breakdown_total = primary_meter->Current();
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
    for (const auto& effect_label : m_effect_labels_and_values) {
        DetachChild(effect_label.first);
        DetachChild(effect_label.second);
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

    auto maybe_info_vec = GetAccountingInfo(m_object_id, accounting_displayed_for_meter);
    if (!maybe_info_vec)
        return;

    // add label-value pairs for each alteration recorded for this meter
    for (const auto& info : *maybe_info_vec) {
        auto source = GetUniverseObject(info.source_id);

        std::string text;
        std::string name;
        if (source)
            name = source->Name();

        switch (info.cause_type) {
        case ECT_TECH: {
            name.clear();
            if (const auto empire = GetEmpire(source->Owner()))
                name = empire->Name();
            const std::string& label_template = (info.custom_label.empty()
                ? UserString("TT_TECH")
                : UserString(info.custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info.specific_cause));
            break;
        }
        case ECT_BUILDING: {
            name.clear();
            if (const auto& building = std::dynamic_pointer_cast<const Building>(source))
                if (const auto& planet = GetPlanet(building->PlanetID()))
                    name = planet->Name();
            const std::string& label_template = (info.custom_label.empty()
                ? UserString("TT_BUILDING")
                : UserString(info.custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info.specific_cause));
            break;
        }
        case ECT_FIELD: {
            const std::string& label_template = (info.custom_label.empty()
                ? UserString("TT_FIELD")
                : UserString(info.custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info.specific_cause));
            break;
        }
        case ECT_SPECIAL: {
            const std::string& label_template = (info.custom_label.empty()
                ? UserString("TT_SPECIAL")
                : UserString(info.custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info.specific_cause));
            break;
        }
        case ECT_SPECIES: {
            //DebugLogger() << "Effect Species Meter Browse Wnd effect cause " << info.specific_cause << " custom label: " << info.custom_label;
            const std::string& label_template = (info.custom_label.empty()
                ? UserString("TT_SPECIES")
                : UserString(info.custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info.specific_cause));
            break;
        }
        case ECT_SHIP_HULL: {
            const std::string& label_template = (info.custom_label.empty()
                ? UserString("TT_SHIP_HULL")
                : UserString(info.custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info.specific_cause));
            break;
        }
        case ECT_SHIP_PART: {
            const std::string& label_template = (info.custom_label.empty()
                ? UserString("TT_SHIP_PART")
                : UserString(info.custom_label));
            text += boost::io::str(FlexibleFormat(label_template)
                % name
                % UserString(info.specific_cause));
            break;
        }
        case ECT_INHERENT:
            text += UserString("TT_INHERENT");
            break;

        case ECT_UNKNOWN_CAUSE:
        default:
            const std::string& label_template = (info.custom_label.empty()
                ? UserString("TT_UNKNOWN")
                : UserString(info.custom_label));
            text += label_template;
        }

        auto label = GG::Wnd::Create<CUILabel>(text, GG::FORMAT_RIGHT);
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(label);

        auto value = GG::Wnd::Create<CUILabel>(ColouredNumber(info.meter_change, 3));
        value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(value);
        m_effect_labels_and_values.push_back({label, value});

        top += m_row_height;
    }
}

ShipDamageBrowseWnd::ShipDamageBrowseWnd(int object_id, MeterType primary_meter_type) :
    MeterBrowseWnd(object_id, primary_meter_type)
{}

void ShipDamageBrowseWnd::Initialize() {
    m_row_height = MeterBrowseRowHeight();
    const GG::X TOTAL_WIDTH = MeterBrowseLabelWidth() + MeterBrowseValueWidth();

    // get objects and meters to verify that they exist
    auto ship = GetShip(m_object_id);
    if (!ship) {
        ErrorLogger() << "ShipDamageBrowseWnd couldn't get ship with id " << m_object_id;
        return;
    }
    GG::Y top = GG::Y0;

    // create controls and do layout

    // effect accounting meter breakdown total / summary.  Shows "Meter Name: Value"
    // above a list of effects.  Actual text is set in UpdateSummary() but
    // the control is created here.
    m_meter_title = GG::Wnd::Create<CUILabel>("", GG::FORMAT_RIGHT);
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
    auto ship = GetShip(m_object_id);
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
    for (const auto& effect_label : m_effect_labels_and_values) {
        DetachChild(effect_label.first);
        DetachChild(effect_label.second);
    }
    m_effect_labels_and_values.clear();

    // get object and meter, aborting if not valid
    auto ship = GetShip(m_object_id);
    if (!ship) {
        ErrorLogger() << "ShipDamageBrowseWnd::UpdateEffectLabelsAndValues couldn't get ship with id " << m_object_id;
        return;
    }

    const ShipDesign* design = GetShipDesign(ship->DesignID());
    if (!design)
        return;

    std::string     name = ship->Name();
    const std::string& label_template = UserString("TT_SHIP_PART");

    // for each weapon part, get its damage meter value
    for (const std::string& part_name : design->Parts()) {
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

        auto label = GG::Wnd::Create<CUILabel>(text, GG::FORMAT_RIGHT);
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(MeterBrowseLabelWidth(), m_row_height));
        AttachChild(label);

        auto value = GG::Wnd::Create<CUILabel>(ColouredNumber(part_attack));
        value->MoveTo(GG::Pt(MeterBrowseLabelWidth(), top));
        value->Resize(GG::Pt(MeterBrowseValueWidth(), m_row_height));
        AttachChild(value);
        m_effect_labels_and_values.push_back({label, value});

        top += m_row_height;
    }

}

namespace {
    class ShipFightersBrowseRow : public GG::ListBox::Row {
    public:
        /** Ctor
         * @param [in] label text for the descriptive label, aligned to right
         * @param [in] qty quantity of source effect in @p label, not displayed when less or equal to 1.
         * @param [in] value value for label positioned to right of @p qty
         * @param [in] base_value optional; If greater than 0.0f: the value label is formatted to "value of base_value"
         */
        ShipFightersBrowseRow(const std::string& label, int qty, double value, double base_value = 0.0f) :
            GG::ListBox::Row(FighterBrowseListWidth(), MeterBrowseRowHeight(), "")
        {
            const GG::Clr QTY_COLOR = GG::CLR_GRAY;

            m_label_control = GG::Wnd::Create<CUILabel>(label, GG::FORMAT_RIGHT);

            std::string qty_text;
            if (qty > 1)
                qty_text = ColourWrappedtext("* " + IntToString(qty), QTY_COLOR);
            m_qty_control = GG::Wnd::Create<CUILabel>(qty_text);

            std::string value_text;
            if (base_value > 0.0f) {
                value_text = boost::io::str(FlexibleFormat(UserString("TT_N_OF_N"))
                                            % IntToString(value) % IntToString(base_value));
            } else {
                value_text = IntToString(value);
            }
            m_value_control = GG::Wnd::Create<CUILabel>(value_text, GG::FORMAT_RIGHT);
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();

            SetMargin(EDGE_PAD);

            push_back(m_label_control);
            m_label_control->Resize(GG::Pt(FighterBrowseLabelWidth(), MeterBrowseRowHeight()));

            push_back(m_qty_control);
            m_qty_control->Resize(GG::Pt(MeterBrowseQtyWidth(), MeterBrowseRowHeight()));

            push_back(m_value_control);
            m_value_control->Resize(GG::Pt(MeterBrowseValueWidth(), MeterBrowseRowHeight()));

            // Resize(GG::Pt(FighterBrowseListWidth(), MeterBrowseRowHeight()));
        }

    private:
        std::shared_ptr<CUILabel> m_label_control;
        std::shared_ptr<CUILabel> m_qty_control;
        std::shared_ptr<CUILabel> m_value_control;
    };
}

ShipFightersBrowseWnd::ShipFightersBrowseWnd(int object_id, MeterType primary_meter_type, bool show_all_bouts /* = false*/) :
    MeterBrowseWnd(object_id, primary_meter_type),
    m_bay_list(nullptr),
    m_hangar_list(nullptr),
    m_show_all_bouts(show_all_bouts)
{}

void ShipFightersBrowseWnd::Initialize() {
    m_row_height = MeterBrowseRowHeight();
    const GG::X LABEL_WIDTH = FighterBrowseLabelWidth();
    const GG::X QTY_WIDTH = MeterBrowseQtyWidth();
    const GG::X VALUE_WIDTH = MeterBrowseValueWidth();
    const GG::X ROW_WIDTH = FighterBrowseListWidth();
    const GG::X TOTAL_WIDTH = ROW_WIDTH * 2 + EDGE_PAD;
    const GG::Clr BG_COLOR = ClientUI::WndColor();

    // get objects and meters to verify that they exist
    auto ship = GetShip(m_object_id);
    if (!ship) {
        ErrorLogger() << "Couldn't get ship with id " << m_object_id;
        return;
    }
    GG::Y top = GG::Y0;

    // create controls and do layout
    m_meter_title = GG::Wnd::Create<CUILabel>("", GG::FORMAT_RIGHT);
    m_meter_title->MoveTo(GG::Pt(GG::X0, top));
    m_meter_title->Resize(GG::Pt(TOTAL_WIDTH + EDGE_PAD, m_row_height));
    m_meter_title->SetFont(ClientUI::GetBoldFont());
    AttachChild(m_meter_title);

    top += m_row_height;

    // initialize formatting for list of bay part labels
    m_bay_list = GG::Wnd::Create<CUIListBox>();
    m_bay_list->SetColor(BG_COLOR);  // border
    m_bay_list->SetInteriorColor(BG_COLOR);  // background
    m_bay_list->ManuallyManageColProps();
    m_bay_list->SetNumCols(3);
    m_bay_list->SetColWidth(0, LABEL_WIDTH);
    m_bay_list->SetColWidth(1, QTY_WIDTH);
    m_bay_list->SetColWidth(2, VALUE_WIDTH);
    m_bay_list->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
    AttachChild(m_bay_list);

    // initialize formatting for list of hangar part labels
    m_hangar_list = GG::Wnd::Create<CUIListBox>();
    m_hangar_list->SetColor(BG_COLOR);
    m_hangar_list->SetInteriorColor(BG_COLOR);
    m_hangar_list->ManuallyManageColProps();
    m_hangar_list->SetNumCols(3);
    m_hangar_list->SetColWidth(0, LABEL_WIDTH);
    m_hangar_list->SetColWidth(1, QTY_WIDTH);
    m_hangar_list->SetColWidth(2, VALUE_WIDTH);
    m_hangar_list->MoveTo(GG::Pt(ROW_WIDTH + (EDGE_PAD * 2), top));
    AttachChild(m_hangar_list);

    UpdateSummary();

    UpdateEffectLabelsAndValues(top);

    Resize(GG::Pt(TOTAL_WIDTH + (EDGE_PAD * 2), top + EDGE_PAD));

    m_initialized = true;
}

void ShipFightersBrowseWnd::UpdateImpl(std::size_t mode, const Wnd* target) {
    if (!m_initialized)
        Initialize();
}

void ShipFightersBrowseWnd::UpdateSummary() {
    auto ship = GetShip(m_object_id);
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
    /** Container for return values of ResolveFighterBouts */
    struct FighterBoutInfo {
        struct StateQty {
            int         docked;
            int         attacking;
            int         launched;       // transitioning between docked and attacking
        };
        float           damage;
        float           total_damage;   // damage from all previous bouts, including this bout
        StateQty        qty;
    };

    /** Populate fighter state quantities and damages for each combat round until @p limit_to_bout */
    std::map<int, FighterBoutInfo> ResolveFighterBouts(int bay_capacity, int current_docked,
                                                       float fighter_damage, int limit_to_bout = -1)
    {
        std::map<int, FighterBoutInfo> retval;
        const int NUM_BOUTS = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS");
        int target_bout = limit_to_bout < 1 ? NUM_BOUTS : limit_to_bout;
        for (int bout = 1; bout <= target_bout; ++bout) {
            // init current fighters
            if (bout == 1) {
                retval[bout].qty.docked = current_docked;
                retval[bout].qty.attacking = 0;
                retval[bout].qty.launched = 0;
            } else {
                retval[bout].qty = retval[bout - 1].qty;
                retval[bout].qty.attacking += retval[bout].qty.launched;
                retval[bout].qty.launched = 0;
                retval[bout].total_damage = retval[bout - 1].total_damage;
            }
            // calc damage this bout, apply to total
            retval[bout].damage = retval[bout].qty.attacking * fighter_damage;
            retval[bout].total_damage += retval[bout].damage;
            // launch fighters
            if (bout < NUM_BOUTS) {
                retval[bout].qty.launched = std::min(bay_capacity, retval[bout].qty.docked);
                retval[bout].qty.docked -= retval[bout].qty.launched;
            }
        }
        return retval;
    }
}

void ShipFightersBrowseWnd::UpdateEffectLabelsAndValues(GG::Y& top) {
    // clear existing labels
    for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
        DetachChild(m_effect_labels_and_values[i].first);
        DetachChild(m_effect_labels_and_values[i].second);
    }
    m_effect_labels_and_values.clear();
    m_bay_list->DetachChildren();
    m_hangar_list->DetachChildren();

    // early return if no valid ship, ship design, or no parts in the design
    auto ship = GetShip(m_object_id);
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

    // colors used, these could be moved to OptionsDB if other controls utilize similar value types
    const GG::Clr DAMAGE_COLOR = GG::CLR_ORANGE;
    const GG::Clr BAY_COLOR = GG::Clr(0, 160, 255, 255);
    const GG::Clr HANGAR_COLOR = GG::CLR_YELLOW;

    const GG::X LABEL_WIDTH = FighterBrowseLabelWidth();
    const GG::X QTY_WIDTH = MeterBrowseQtyWidth();
    const GG::X VALUE_WIDTH = MeterBrowseValueWidth();
    const GG::X ROW_WIDTH = FighterBrowseListWidth();

    float fighter_damage = 0.0f;
    std::pair<std::string, int> hangar_part;  // name, count
    std::map<std::string, std::pair<int, int>> bay_parts;  // name, (count, capacity)
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
            bay_parts[part_name].first++;
            bay_parts[part_name].second = current_capacity;
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

    // summary of bay capacity
    std::string bay_text = boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY"))
                                          % UserString("SHIP_FIGHTER_BAY_SUMMARY")
                                          % ColouredInt(bay_total_capacity, false, BAY_COLOR));
    auto bay_summary = GG::Wnd::Create<CUILabel>(bay_text, GG::FORMAT_RIGHT);
    bay_summary->SetFont(ClientUI::GetBoldFont());
    bay_summary->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
    bay_summary->Resize(GG::Pt(ROW_WIDTH, m_row_height));
    AttachChild(bay_summary);

    // summary of hangar capacity
    std::string hangar_text = boost::io::str(FlexibleFormat(UserString("TT_N_OF_N"))
                                             % ColouredInt(hangar_current_fighters, false, HANGAR_COLOR)
                                             % ColouredInt(hangar_total_capacity, false, HANGAR_COLOR));
    hangar_text = boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY"))
                                 % UserString("SHIP_FIGHTER_HANGAR_SUMMARY") % hangar_text);
    auto hangar_summary = GG::Wnd::Create<CUILabel>(hangar_text, GG::FORMAT_RIGHT);
    hangar_summary->SetFont(ClientUI::GetBoldFont());
    hangar_summary->MoveTo(GG::Pt(ROW_WIDTH + EDGE_PAD, top));
    hangar_summary->Resize(GG::Pt(ROW_WIDTH, m_row_height));
    AttachChild(hangar_summary);
    m_effect_labels_and_values.push_back({bay_summary, hangar_summary});

    top += m_row_height;

    // text for display of fighter damage
    std::string fighter_damage_text = ColourWrappedtext(DoubleToString(fighter_damage, 3, false), DAMAGE_COLOR);

    if (!m_show_all_bouts) {
        // Show damage for first wave (2nd combat round)
        std::map<int, FighterBoutInfo> bout_info = ResolveFighterBouts(bay_total_capacity, hangar_current_fighters,
                                                                       fighter_damage, 2);
        FighterBoutInfo first_wave = bout_info.rbegin()->second;
        GG::Clr highlight_clr = bout_info[1].qty.launched < bay_total_capacity ? HANGAR_COLOR : BAY_COLOR;
        std::string launch_text = ColouredInt(first_wave.qty.attacking, false, highlight_clr);
        std::string damage_label_text = boost::io::str(FlexibleFormat(UserString("TT_FIGHTER_DAMAGE"))
                                                       % launch_text % fighter_damage_text);
        // damage formula label
        auto damage_label = GG::Wnd::Create<CUILabel>(damage_label_text, GG::FORMAT_RIGHT);
        damage_label->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
        damage_label->Resize(GG::Pt(LABEL_WIDTH + (QTY_WIDTH * 2) + (LABEL_WIDTH / 2) + (EDGE_PAD * 3), m_row_height));
        AttachChild(damage_label);

        std::string damage_value_text = DoubleToString(first_wave.damage, 3, false);
        // sum of damage formula label
        auto damage_value = GG::Wnd::Create<CUILabel>(ColourWrappedtext(damage_value_text, DAMAGE_COLOR), GG::FORMAT_RIGHT);
        damage_value->MoveTo(GG::Pt(LABEL_WIDTH + (QTY_WIDTH * 2) + (LABEL_WIDTH / 2) + (EDGE_PAD * 4), top));
        damage_value->Resize(GG::Pt(VALUE_WIDTH, m_row_height));
        damage_value->SetFont(ClientUI::GetBoldFont());
        AttachChild(damage_value);
        m_effect_labels_and_values.push_back({damage_label, damage_value});

        top += m_row_height;
    } else {
        // Show each effect for part capacity summaries

        // add labels for bay parts
        for (const auto& bay_part : bay_parts) {
            const std::string& part_name = UserString(bay_part.first);
            int part_qty = bay_part.second.first;
            int fighter_total = bay_part.second.second * part_qty;
            m_bay_list->Insert(GG::Wnd::Create<ShipFightersBrowseRow>(part_name, part_qty, fighter_total));
        }
        // TODO Append other potential effects for bay capacities

        // add label for hangar part
        m_hangar_list->Insert(GG::Wnd::Create<ShipFightersBrowseRow>(UserString(hangar_part.first), hangar_part.second,
                                                                     hangar_current_fighters, hangar_total_capacity));
        // TODO Append other potential effects for hangar capacities

        // calculate the required height to align both listboxes
        int max_rows = static_cast<int>(std::max(m_bay_list->NumRows(), m_hangar_list->NumRows()));
        int list_vpad = max_rows > 0 ? 10 : 0;
        const GG::Y LIST_HEIGHT = (m_row_height * max_rows) + list_vpad;

        // properly size and position the listboxes
        m_bay_list->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
        m_bay_list->Resize(GG::Pt(ROW_WIDTH, LIST_HEIGHT));
        m_hangar_list->MoveTo(GG::Pt(ROW_WIDTH + EDGE_PAD, top));
        m_hangar_list->Resize(GG::Pt(ROW_WIDTH, LIST_HEIGHT));

        top += LIST_HEIGHT;

        // Damage summary labels
        // TODO Add list of effects on hangar(fighter) damage

        std::map<int, FighterBoutInfo> bout_info = ResolveFighterBouts(bay_total_capacity, hangar_current_fighters,
                                                                       fighter_damage);
        const FighterBoutInfo& last_bout = bout_info.rbegin()->second;

        // damage summary text
        std::string detail_label_text = UserString("SHIP_FIGHTERS_DAMAGE_TOTAL");
        std::string detail_value_text = ColourWrappedtext(DoubleToString(last_bout.total_damage, 3, false), DAMAGE_COLOR);
        detail_label_text = boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY"))
                                           % detail_label_text % detail_value_text);
        auto detail_summary_damage = GG::Wnd::Create<CUILabel>(detail_label_text, GG::FORMAT_RIGHT);
        detail_summary_damage->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
        detail_summary_damage->Resize(GG::Pt(ROW_WIDTH + QTY_WIDTH + (LABEL_WIDTH / 2) + EDGE_PAD, m_row_height));
        detail_summary_damage->SetFont(ClientUI::GetBoldFont());
        AttachChild(detail_summary_damage);

        // empty space
        auto detail_summary_launch = GG::Wnd::Create<CUILabel>("", GG::FORMAT_RIGHT);
        detail_summary_launch->MoveTo(GG::Pt(ROW_WIDTH + QTY_WIDTH + (LABEL_WIDTH / 2) + (EDGE_PAD * 2), top));
        detail_summary_launch->Resize(GG::Pt(VALUE_WIDTH + EDGE_PAD + (LABEL_WIDTH / 2), m_row_height));
        detail_summary_launch->SetFont(ClientUI::GetBoldFont());
        AttachChild(detail_summary_launch);
        m_effect_labels_and_values.push_back({detail_summary_damage, detail_summary_launch});

        top += m_row_height;

        // Damage labels for each combat round
        GG::X left = GG::X0;
        int previous_docked = hangar_current_fighters;
        for (auto& current_bout : bout_info) {
            const FighterBoutInfo& bout = current_bout.second;
            // combat round label
            std::string bout_text = boost::io::str(FlexibleFormat(UserString("TT_COMBAT_ROUND"))
                                                   % IntToString(current_bout.first));
            auto bout_label = GG::Wnd::Create<CUILabel>(bout_text, GG::FORMAT_RIGHT);
            bout_label->MoveTo(GG::Pt(left, top));
            bout_label->Resize(GG::Pt((QTY_WIDTH * 2) + (EDGE_PAD * 3), m_row_height));
            AttachChild(bout_label);
            left += (QTY_WIDTH * 2) + (EDGE_PAD * 4);

            // damage formula label
            std::string formula_label_text = boost::io::str(FlexibleFormat(UserString("TT_FIGHTER_DAMAGE"))
                                                            % IntToString(bout.qty.attacking)
                                                            % fighter_damage_text);
            auto formula_label = GG::Wnd::Create<CUILabel>(formula_label_text, GG::FORMAT_RIGHT);
            formula_label->MoveTo(GG::Pt(left, top));
            formula_label->Resize(GG::Pt(LABEL_WIDTH + (LABEL_WIDTH / 2), m_row_height));
            AttachChild(formula_label);
            left += LABEL_WIDTH + (LABEL_WIDTH / 2) + EDGE_PAD;

            // sum of damage formula label
            std::string formula_value_text = DoubleToString(bout.damage, 3, false);
            auto formula_value = GG::Wnd::Create<CUILabel>(ColourWrappedtext(formula_value_text, DAMAGE_COLOR), GG::FORMAT_RIGHT);
            formula_value->MoveTo(GG::Pt(left, top));
            formula_value->Resize(GG::Pt(VALUE_WIDTH, m_row_height));
            AttachChild(formula_value);
            m_effect_labels_and_values.push_back({formula_label, formula_value});
            left += VALUE_WIDTH + EDGE_PAD;

            // launched fighters label
            auto launch_label = GG::Wnd::Create<CUILabel>(UserString("TT_FIGHTER_LAUNCH"), GG::FORMAT_RIGHT);
            launch_label->MoveTo(GG::Pt(left, top));
            launch_label->Resize(GG::Pt((LABEL_WIDTH / 2) + EDGE_PAD, m_row_height));
            AttachChild(launch_label);
            left += (LABEL_WIDTH / 2) + EDGE_PAD + EDGE_PAD;

            GG::Clr launch_clr = bout.qty.launched < bay_total_capacity ? HANGAR_COLOR : BAY_COLOR;
            std::string launch_text = boost::io::str(FlexibleFormat(UserString("TT_N_OF_N"))
                                                     % ColouredInt(bout.qty.launched, false, launch_clr)
                                                     % ColouredInt(previous_docked, false, HANGAR_COLOR));
            auto launch_value = GG::Wnd::Create<CUILabel>(launch_text, GG::FORMAT_RIGHT);
            launch_value->MoveTo(GG::Pt(left, top));
            launch_value->Resize(GG::Pt(VALUE_WIDTH, m_row_height));
            AttachChild(launch_value);
            m_effect_labels_and_values.push_back({launch_label, launch_value});

            top += m_row_height;
            left = GG::X0;
            previous_docked = bout.qty.docked;
        }
    }
}
