#include "MultiMeterStatusBar.h"
#include "MeterBrowseWnd.h"

#include <GG/ClrConstants.h>
#include <GG/DrawUtil.h>
#include <GG/Texture.h>
#include <GG/GLClientAndServerBuffer.h>

#include "../util/Logger.h"
#include "../universe/Meter.h"
#include "../universe/UniverseObject.h"
#include "../universe/Enums.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"

namespace {
    /** Returns GG::Clr with which to display programatically coloured things
      * (such as meter bars) for the indicated \a meter_type */
    GG::Clr MeterColor(MeterType meter_type) {
        switch (meter_type) {
        case METER_INDUSTRY:
        case METER_TARGET_INDUSTRY:
            return GG::Clr(240, 90, 0, 255);
            break;
        case METER_RESEARCH:
        case METER_TARGET_RESEARCH:
            return GG::Clr(0, 255, 255, 255);
            break;
        case METER_TRADE:
        case METER_TARGET_TRADE:
            return GG::Clr(255, 200, 0, 255);
            break;
        case METER_SHIELD:
        case METER_MAX_SHIELD:
            return GG::Clr(0, 255, 186, 255);
            break;
        case METER_DEFENSE:
        case METER_MAX_DEFENSE:
            return GG::Clr(255, 0, 0, 255);
            break;
        case METER_TROOPS:
        case METER_MAX_TROOPS:
        case METER_REBEL_TROOPS:
            return GG::Clr(168, 107, 0, 255);
            break;
        case METER_DETECTION:
            return GG::Clr(191, 255, 0, 255);
            break;
        case METER_STEALTH:
            return GG::Clr(174, 0, 255, 255);
            break;
        case METER_HAPPINESS:
        case METER_TARGET_HAPPINESS:
            return GG::Clr(255, 255, 0, 255);
        case METER_SUPPLY:
        case METER_MAX_SUPPLY:
        case METER_STOCKPILE:
        case METER_MAX_STOCKPILE:
        case METER_CONSTRUCTION:
        case METER_TARGET_CONSTRUCTION:
        case METER_POPULATION:
        case METER_TARGET_POPULATION:
        default:
            return GG::CLR_WHITE;
        }
        // For future:
        //Influence GG::Clr(255, 0, 255, 255);
    }

    const int       EDGE_PAD(3);

    const int       BAR_PAD(1);
    const GG::Y     BAR_HEIGHT(10);

    const double    MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE_INCREMENT = 100.0;
}

MultiMeterStatusBar::MultiMeterStatusBar(GG::X w, int object_id, const std::vector<std::pair<MeterType, MeterType>>& meter_types) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_bar_shading_texture(ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "meter_bar_shading.png")),
    m_meter_types(meter_types),
    m_initial_values(),
    m_projected_values(),
    m_target_max_values(),
    m_object_id(object_id),
    m_bar_colours()
{
    SetName("MultiMeterStatusBar");
    Update();
}

void MultiMeterStatusBar::Render() {
    GG::Clr DARY_GREY = GG::Clr(44, 44, 44, 255);
    GG::Clr HALF_GREY = GG::Clr(128, 128, 128, 128);

    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);

    const GG::X BAR_LEFT = ClientUpperLeft().x + EDGE_PAD - 1;
    const GG::X BAR_RIGHT = ClientLowerRight().x - EDGE_PAD + 1;
    const GG::X BAR_MAX_LENGTH = BAR_RIGHT - BAR_LEFT;
    const GG::Y TOP = ClientUpperLeft().y + EDGE_PAD - 1;
    GG::Y y = TOP;

    for (unsigned int i = 0; i < m_initial_values.size(); ++i) {
        // bar grey backgrounds
        GG::FlatRectangle(GG::Pt(BAR_LEFT, y), GG::Pt(BAR_RIGHT, y + BAR_HEIGHT), DARY_GREY, DARY_GREY, 0);

        y += BAR_HEIGHT + BAR_PAD;
    }

    // Find the largest value to be displayed to determine the scale factor.  Must be greater than
    // zero so that there is at least one segment drawn.
    double largest_value = 0.1;
    for (unsigned int i = 0; i < m_initial_values.size(); ++i) {
        if ((m_initial_values[i] != Meter::INVALID_VALUE) && (m_initial_values[i] > largest_value))
            largest_value = m_initial_values[i];
        if ((m_projected_values[i] != Meter::INVALID_VALUE) && (m_projected_values[i] > largest_value))
            largest_value = m_projected_values[i];
        if ((m_target_max_values[i] != Meter::INVALID_VALUE) && (m_target_max_values[i] > largest_value))
            largest_value = m_target_max_values[i];
    }

    double num_full_increments =
        std::ceil(largest_value / MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE_INCREMENT);
    double MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE =
        num_full_increments * MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE_INCREMENT;

    // lines for 20, 40, 60, 80 etc.
    GG::GL2DVertexBuffer bar_verts;
    unsigned int num_segments = num_full_increments * 5;
    bar_verts.reserve(num_segments - 1);
    for (unsigned int ii_div_line = 1; ii_div_line <= (num_segments - 1); ++ii_div_line) {
        bar_verts.store(Value(BAR_LEFT) + ii_div_line*Value(BAR_MAX_LENGTH)/num_segments, TOP);
        bar_verts.store(Value(BAR_LEFT) + ii_div_line*Value(BAR_MAX_LENGTH)/num_segments, y - BAR_PAD);
    }
    bar_verts.activate();

    glColor(HALF_GREY);
    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, bar_verts.size());
    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);

    // current, initial, and target/max horizontal bars for each pair of MeterType
    y = TOP;
    for (unsigned int i = 0; i < m_initial_values.size(); ++i) {
        GG::Clr clr;

        const GG::Y BAR_TOP = y;
        const GG::Y BAR_BOTTOM = BAR_TOP + BAR_HEIGHT;

        const bool SHOW_INITIAL = (m_initial_values[i] != Meter::INVALID_VALUE);
        const bool SHOW_PROJECTED = (m_projected_values[i] != Meter::INVALID_VALUE);
        const bool SHOW_TARGET_MAX = (m_target_max_values[i] != Meter::INVALID_VALUE);


        const GG::X INITIAL_RIGHT(BAR_LEFT + BAR_MAX_LENGTH * m_initial_values[i] / MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE);
        const GG::Y INITIAL_TOP(BAR_TOP);
        if (SHOW_INITIAL) {
            // initial value
            glColor(m_bar_colours[i]);
            m_bar_shading_texture->OrthoBlit(GG::Pt(BAR_LEFT, INITIAL_TOP), GG::Pt(INITIAL_RIGHT, BAR_BOTTOM));
            // black border
            GG::FlatRectangle(GG::Pt(BAR_LEFT, INITIAL_TOP), GG::Pt(INITIAL_RIGHT, BAR_BOTTOM), GG::CLR_ZERO, GG::CLR_BLACK, 1);
        }

        const GG::X PROJECTED_RIGHT(BAR_LEFT + BAR_MAX_LENGTH * m_projected_values[i] / MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE);
        const GG::Y PROJECTED_TOP(INITIAL_TOP);
        if (SHOW_PROJECTED) {
            // projected colour bar with black border
            if (PROJECTED_RIGHT > INITIAL_RIGHT) {
                GG::FlatRectangle(GG::Pt(INITIAL_RIGHT - 1, PROJECTED_TOP), GG::Pt(PROJECTED_RIGHT, BAR_BOTTOM), ClientUI::StatIncrColor(), GG::CLR_BLACK, 1);
            } else if (PROJECTED_RIGHT < INITIAL_RIGHT) {
                GG::FlatRectangle(GG::Pt(PROJECTED_RIGHT - 1, PROJECTED_TOP), GG::Pt(INITIAL_RIGHT, BAR_BOTTOM), ClientUI::StatDecrColor(), GG::CLR_BLACK, 1);
            }
        }

        const GG::X TARGET_MAX_RIGHT(BAR_LEFT + BAR_MAX_LENGTH * m_target_max_values[i] / MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE);
        if (SHOW_TARGET_MAX && TARGET_MAX_RIGHT > BAR_LEFT) {
            // max / target value
            //glColor(DarkColor(m_bar_colours[i]));
            //m_bar_shading_texture->OrthoBlit(GG::Pt(BAR_LEFT, BAR_TOP), GG::Pt(TARGET_MAX_RIGHT, BAR_BOTTOM));
            // black border
            GG::FlatRectangle(GG::Pt(BAR_LEFT, BAR_TOP), GG::Pt(TARGET_MAX_RIGHT, BAR_BOTTOM), GG::CLR_ZERO, m_bar_colours[i], 1);
        }

        // move down position of next bar, if any
        y += BAR_HEIGHT + BAR_PAD;
    }
}

void MultiMeterStatusBar::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void MultiMeterStatusBar::Update() {
    m_initial_values.clear();   // initial value of .first MeterTypes at the start of this turn
    m_projected_values.clear(); // projected current value of .first MeterTypes for the start of next turn
    m_target_max_values.clear();// current values of the .second MeterTypes in m_meter_types

    auto obj = GetUniverseObject(m_object_id);
    if (!obj) {
        ErrorLogger() << "MultiMeterStatusBar couldn't get object with id " << m_object_id;
        return;
    }

    int num_bars = 0;   // count number of valid bars' data added

    for (auto& meter_types_pair : m_meter_types) {
        const Meter* actual_meter = obj->GetMeter(meter_types_pair.first);
        const Meter* target_max_meter = obj->GetMeter(meter_types_pair.second);

        if (actual_meter || target_max_meter) {
            float initial = actual_meter ? actual_meter->Initial() : Meter::INVALID_VALUE;
            float projected = actual_meter ? actual_meter->Current() : Meter::INVALID_VALUE;
            float target = target_max_meter ? target_max_meter->Current() : Meter::INVALID_VALUE;

            ++num_bars;

            m_initial_values.push_back(initial);
            m_projected_values.push_back(projected);
            m_target_max_values.push_back(target);
            m_bar_colours.push_back(MeterColor(meter_types_pair.first));
        }
    }

    // calculate height from number of bars to be shown
    const GG::Y HEIGHT = num_bars*BAR_HEIGHT + (num_bars - 1)*BAR_PAD + 2*EDGE_PAD - 2;
    Resize(GG::Pt(Width(), HEIGHT));
}
