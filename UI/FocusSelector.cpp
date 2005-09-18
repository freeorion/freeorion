#include "FocusSelector.h"

#include "../universe/ResourceCenter.h"
#include "../util/OptionsDB.h"
#include "../UI/ClientUI.h"

#include "GGApp.h"
#include "GGDrawUtil.h"
#include "GGLayout.h"

#include <boost/format.hpp>

namespace {
    const char* const IMAGES[] = {
        0,
        0,
        "icons/farming.png",
        "icons/industry.png",
        "icons/mining.png",
        "icons/research.png",
        "icons/trade.png"
    };

    const char* const MONO_IMAGES[] = {
        0,
        0,
        "icons/farming_mono.png",
        "icons/industry_mono.png",
        "icons/mining_mono.png",
        "icons/research_mono.png",
        "icons/trade_mono.png"
    };

    GG::Clr METER_STATUS_BAR_COLOR(193, 193, 193, 255);
    GG::Clr METER_STATUS_BAR_GROWTH_COLOR(151, 151, 151, 200);
    GG::Clr METER_STATUS_BAR_SHRINK_COLOR(255, 0, 0, 255);

    const GG::Clr FOCUS_SELECTOR_COLOR(127, 127, 127, 255);
    const GG::Clr FOCUS_SELECTOR_SECONDARY_OUTLINE_COLOR(191, 191, 191, 255);
    const int FOCUS_SELECTOR_METER_BAR_HEIGHT = 18;
    const int FOCUS_SELECTOR_TEXT_HEIGHT = 14;
    const FocusType FOCUS_SELECTOR_FIRST_FOCUS = FOCUS_FARMING;
    const FocusType FOCUS_SELECTOR_LAST_FOCUS = NUM_FOCI;
}

FocusButton::FocusButton(GG::Clr color, const boost::shared_ptr<GG::Texture>& image) :
    Button(0, 0, 1, 1, "", "", 0, color),
    m_texture(image)
{}

bool FocusButton::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_texture) {
        glColor4ubv(GG::CLR_WHITE.v);
        m_texture->OrthoBlit(ul, lr, 0, false);
    }
    return true;
}

void FocusButton::RClick(const GG::Pt& pt, Uint32 keys)
{
    RightClickedSignal();
}

void FocusButton::SetImage(const boost::shared_ptr<GG::Texture>& image)
{
    m_texture = image;
}


MeterStatusBar::MeterStatusBar(int w, int h, double initial_max, double initial_current, double max, double current) :
    GG::Wnd(0, 0, w, h, 0),
    m_initial_max(initial_max),
    m_initial_current(initial_current),
    m_projected_max(max),
    m_projected_current(current)
{
    assert(Meter::METER_MIN <= m_projected_current && m_projected_current <= Meter::METER_MAX);
    assert(Meter::METER_MIN <= m_projected_max && m_projected_max <= Meter::METER_MAX);
}

bool MeterStatusBar::Render()
{
    assert(Height() % 2); // non-odd heights look messed up
    const GG::Pt MARGIN = GG::Pt(1, 2);
    GG::Pt working_space = Size() - GG::Pt(2 * MARGIN.x, 2 * MARGIN.y);
    GG::Pt max_ul = UpperLeft() + MARGIN;
    GG::Pt max_lr = max_ul + GG::Pt(working_space.x, working_space.y / 2);
    GG::Pt curr_lr = LowerRight() - MARGIN;
    GG::Pt curr_ul = curr_lr - GG::Pt(working_space.x, working_space.y / 2);
    int w = working_space.x;
    if (m_projected_max < m_initial_max) {
        GG::Rect rect(max_ul.x, max_ul.y, max_ul.x + static_cast<int>(w * m_projected_max / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), max_lr.y);
        GG::FlatRectangle(rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y,
                          METER_STATUS_BAR_COLOR, GG::CLR_ZERO, 0);
        GG::Rect shrink_rect(rect.lr.x, rect.ul.y, max_ul.x + static_cast<int>(w * m_initial_max / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), max_lr.y);
        GG::FlatRectangle(shrink_rect.ul.x, shrink_rect.ul.y, shrink_rect.lr.x, shrink_rect.lr.y,
                          METER_STATUS_BAR_SHRINK_COLOR, GG::CLR_ZERO, 0);
    } else {
        GG::Rect rect(max_ul.x, max_ul.y, max_ul.x + static_cast<int>(w * m_initial_max / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), max_lr.y);
        GG::FlatRectangle(rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y,
                          METER_STATUS_BAR_COLOR, GG::CLR_ZERO, 0);
        GG::Rect growth_rect(rect.lr.x, rect.ul.y, max_ul.x + static_cast<int>(w * m_projected_max / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), max_lr.y);
        GG::FlatRectangle(growth_rect.ul.x, growth_rect.ul.y, growth_rect.lr.x, growth_rect.lr.y,
                          METER_STATUS_BAR_GROWTH_COLOR, GG::CLR_ZERO, 0);
    }
    if (m_projected_current < m_initial_current) {
        GG::Rect rect(curr_ul.x, curr_ul.y, curr_ul.x + static_cast<int>(w * m_projected_current / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), curr_lr.y);
        GG::FlatRectangle(rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y,
                          METER_STATUS_BAR_COLOR, GG::CLR_ZERO, 0);
        GG::Rect shrink_rect(rect.lr.x, rect.ul.y, curr_ul.x + static_cast<int>(w * m_initial_current / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), curr_lr.y);
        GG::FlatRectangle(shrink_rect.ul.x, shrink_rect.ul.y, shrink_rect.lr.x, shrink_rect.lr.y,
                          METER_STATUS_BAR_SHRINK_COLOR, GG::CLR_ZERO, 0);
    } else {
        GG::Rect rect(curr_ul.x, curr_ul.y, curr_ul.x + static_cast<int>(w * m_initial_current / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), curr_lr.y);
        GG::FlatRectangle(rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y,
                          METER_STATUS_BAR_COLOR, GG::CLR_ZERO, 0);
        GG::Rect growth_rect(rect.lr.x, rect.ul.y, curr_ul.x + static_cast<int>(w * m_projected_current / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), curr_lr.y);
        GG::FlatRectangle(growth_rect.ul.x, growth_rect.ul.y, growth_rect.lr.x, growth_rect.lr.y,
                          METER_STATUS_BAR_GROWTH_COLOR, GG::CLR_ZERO, 0);
    }
    return true;
}

void MeterStatusBar::SetProjectedCurrent(double current)
{
    assert(Meter::METER_MIN <= current && current <= Meter::METER_MAX);
    m_projected_current = current;
}

void MeterStatusBar::SetProjectedMax(double max)
{
    assert(Meter::METER_MIN <= max && max <= Meter::METER_MAX);
    m_projected_max = max;
}


FocusSelector::FocusClickFunctor::FocusClickFunctor(int id, bool primary_, FocusSelector* wnd_) :
    button_id(id),
    primary(primary_),
    wnd(wnd_)
{}

void FocusSelector::FocusClickFunctor::operator()()
{
    if (primary)
        wnd->PrimaryFocusClicked(button_id);
    else
        wnd->SecondaryFocusClicked(button_id);
}

FocusSelector::FocusSelector(int w, const ResourceCenter& resource_center) :
    Control(0, 0, w, w / (FOCUS_SELECTOR_LAST_FOCUS - FOCUS_SELECTOR_FIRST_FOCUS) + FOCUS_SELECTOR_METER_BAR_HEIGHT + FOCUS_SELECTOR_TEXT_HEIGHT),
    m_primary_focus(resource_center.PrimaryFocus()),
    m_secondary_focus(resource_center.SecondaryFocus()),
    m_first_button_focus(FOCUS_SELECTOR_FIRST_FOCUS)
{
    SetColor(FOCUS_SELECTOR_COLOR);
    GG::Layout* layout = new GG::Layout(0, 0, Width(), Height(), 3, (FOCUS_SELECTOR_LAST_FOCUS - FOCUS_SELECTOR_FIRST_FOCUS), 1);
    layout->SetRowStretch(0, 1);
    layout->SetMinimumRowHeight(1, FOCUS_SELECTOR_METER_BAR_HEIGHT);
    layout->SetMinimumRowHeight(2, FOCUS_SELECTOR_TEXT_HEIGHT);
    for (FocusType i = FOCUS_SELECTOR_FIRST_FOCUS; i < FOCUS_SELECTOR_LAST_FOCUS; i = FocusType(i + 1)) {
        boost::shared_ptr<GG::Texture> texture;
        if (MonoImage(m_focus_buttons.size() - 1))
            texture = GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + MONO_IMAGES[i], true);
        else
            texture = GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + IMAGES[i], true);
        m_focus_buttons.push_back(new FocusButton(GG::CLR_WHITE, texture));
        GG::Connect(m_focus_buttons.back()->ClickedSignal, FocusClickFunctor(m_focus_buttons.size() - 1, true, this));
        GG::Connect(m_focus_buttons.back()->RightClickedSignal, FocusClickFunctor(m_focus_buttons.size() - 1, false, this));
        layout->Add(m_focus_buttons.back(), 0, m_focus_buttons.size() - 1);

        const Meter* meter = resource_center.GetMeter(FocusToMeter(i));
        assert(meter);
        m_meter_status_bars[i] = new MeterStatusBar(1, 1, meter->InitialMax(), meter->InitialCurrent(),
                                                    meter->Max(), meter->Current());
        layout->Add(m_meter_status_bars[i], 1, m_meter_status_bars.size() - 1);

        double delta = meter->DeltaCurrent();
        GG::Clr delta_color = GetOptionsDB().Get<StreamableColor>("UI.text-color").ToClr();
        std::stringstream delta_str;
        if (0.0 < delta) {
            delta_color = GG::CLR_GREEN;
            delta_str << '+';
        } else if (delta < 0.0) {
            delta_color = GG::CLR_RED;
        }
        if (delta <= -1.0 || 1.0 <= delta)
            delta_str << boost::format("%.2g") % delta;
        else
            delta_str << boost::format("%.1g") % delta;
        m_meter_deltas.push_back(new GG::TextControl(0, 0, 1, 1, delta_str.str(), "Vera.ttf", 11, delta_color, GG::TF_TOP)); // TODO: use ClientUI font
        layout->Add(m_meter_deltas.back(), 2, m_meter_deltas.size() - 1);
    }
    glDisable(GL_TEXTURE_2D);
    AttachChild(layout);
}

bool FocusSelector::Render()
{
    for (unsigned int i = 0; i < m_focus_buttons.size(); ++i) {
        GG::Pt ul = m_focus_buttons[i]->UpperLeft() - GG::Pt(1, 1),
            lr = m_focus_buttons[i]->LowerRight() + GG::Pt(1, 1);
        GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, Color(), 1);
    }
    GG::Pt ul, lr;
    if (m_secondary_focus == FOCUS_BALANCED) {
        ul = m_focus_buttons[0]->UpperLeft() - GG::Pt(1, 1);
        lr = m_focus_buttons.back()->LowerRight() + GG::Pt(1, 1);
    } else {
        int button = m_secondary_focus - m_first_button_focus;
        ul = m_focus_buttons[button]->UpperLeft() - GG::Pt(1, 1);
        lr = m_focus_buttons[button]->LowerRight() + GG::Pt(1, 1);
    }
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, FOCUS_SELECTOR_SECONDARY_OUTLINE_COLOR, 1);
    return true;
}

void FocusSelector::LClick(const GG::Pt& pt, Uint32 keys)
{
    PrimaryFocusChangedSignal(FOCUS_BALANCED);
}

void FocusSelector::RClick(const GG::Pt& pt, Uint32 keys)
{
    SecondaryFocusChangedSignal(FOCUS_BALANCED);
}

void FocusSelector::Update(const ResourceCenter& resource_center)
{
    m_primary_focus = resource_center.PrimaryFocus();
    m_secondary_focus = resource_center.SecondaryFocus();
    AdjustButtonImages();
    AdjustMeterStatusBars(resource_center);
}

FocusType FocusSelector::ButtonFocus(int button) const
{
    return FocusType(m_first_button_focus + button);
}

bool FocusSelector::MonoImage(int button) const
{
    FocusType button_focus = ButtonFocus(button);
    return m_primary_focus != FOCUS_BALANCED && m_primary_focus != button_focus;
}

void FocusSelector::AdjustButtonImages()
{
    for (unsigned int i = 0; i < m_focus_buttons.size(); ++i) {
        if (MonoImage(i))
            m_focus_buttons[i]->SetImage(GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + MONO_IMAGES[ButtonFocus(i)]));
        else
            m_focus_buttons[i]->SetImage(GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + IMAGES[ButtonFocus(i)]));
    }
}

void FocusSelector::AdjustMeterStatusBars(const ResourceCenter& resource_center)
{
    for (std::map<FocusType, MeterStatusBar*>::iterator it = m_meter_status_bars.begin(); it != m_meter_status_bars.end(); ++it) {// FocusType i = FOCUS_SELECTOR_FIRST_FOCUS; i < FOCUS_SELECTOR_LAST_FOCUS; i = FocusType(i + 1)) {
        MeterType meter_type = FocusToMeter(it->first);
        const Meter* meter = resource_center.GetMeter(meter_type);
        assert(meter);
        it->second->SetProjectedCurrent(resource_center.ProjectedCurrent(meter_type));
        it->second->SetProjectedMax(meter->Max());
    }
}

void FocusSelector::PrimaryFocusClicked(int button)
{
    FocusType focus = FocusType(m_first_button_focus + button);
    if (focus == m_primary_focus)
        return;
    PrimaryFocusChangedSignal(focus);
}

void FocusSelector::SecondaryFocusClicked(int button)
{
    FocusType focus = FocusType(m_first_button_focus + button);
    if (focus == m_secondary_focus)
        return;
    SecondaryFocusChangedSignal(focus);
}
