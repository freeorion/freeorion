#include "GraphControl.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/i18n.h"
#include "../util/ranges.h"

#include <GG/ClrConstants.h>


GraphControl::GraphControl() :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1)
{
    //std::vector<std::pair<double, double>> test_data = {{0.0, 1.0}, {1.0, 3.2}, {4.2, -1}, {0, 0}, {-1, 1}};
    //m_data.push_back({test_data, GG::CLR_CYAN});

    //test_data = {{1.0, 1.0}, {2.0, 3.2}, {3.2, -1}, {4, 0}, {5, 1}};
    //m_data.push_back({test_data, GG::CLR_YELLOW});

    AutoSetRange();
}

void GraphControl::AddSeries(std::vector<std::pair<double, double>> data, GG::Clr clr) {
    if (!data.empty()) {
        m_data.emplace_back(std::move(data), clr);
        DoLayout();
    }
}

void GraphControl::Clear() {
    m_data.clear();
    DoLayout();
}

void GraphControl::SetXMin(double x_min) {
    double old_x_min = x_min;
    m_x_min = x_min;
    if (m_x_min != old_x_min)
        DoLayout();
}

void GraphControl::SetYMin(double y_min) {
    double old_y_min = y_min;
    m_y_min = y_min;
    if (m_y_min != old_y_min)
        DoLayout();
}

void GraphControl::SetXMax(double x_max) {
    double old_x_max = x_max;
    m_x_max = x_max;
    if (m_x_max != old_x_max)
        DoLayout();
}

void GraphControl::SetYMax(double y_max) {
    double old_y_max = y_max;
    m_y_max = y_max;
    if (m_y_max != old_y_max)
        DoLayout();
}

void GraphControl::SetRange(double x1, double x2, double y1, double y2) {
    if (m_x_min != x1 || m_y_min != y1 || m_x_max != x2 || m_y_max != y2) {
        m_x_min = x1;
        m_y_min = y1;
        m_x_max = x2;
        m_y_max = y2;

        DoLayout();
    }
}

void GraphControl::AutoSetRange() {
    if (m_data.empty())
        return;

    // large default values that are expected to be overwritten by the first seen value
    double x_min = std::numeric_limits<double>::infinity();
    double y_min = std::numeric_limits<double>::infinity();
    double x_max = -std::numeric_limits<double>::infinity();
    double y_max = -std::numeric_limits<double>::infinity();

    for (const auto& curve : m_data | range_keys) { // wanted to do this with ranges join and minmax_element, but boost ranges doesn't have an implementation of that and not all target platforms support C++20 ranges yet
        for (const auto& [x, y] : curve) {
            x_min = std::min(x, x_min);
            x_max = std::max(x, x_max);
            y_min = std::min(y, y_min);
            y_max = std::max(y, y_max);
        }
    }

    SetRange(x_min, x_max, y_min, y_max);
}

void GraphControl::ShowPoints(bool show) {
    if (std::exchange(m_show_points, show) != show)
        DoLayout();
}

void GraphControl::ShowLines(bool show) {
    if (std::exchange(m_show_lines, show) != show)
        DoLayout();
}

void GraphControl::ShowScale(bool show) {
    if (std::exchange(m_show_scale, show) != show)
        DoLayout();
}

void GraphControl::UseLogScale(bool log) {
    if (std::exchange(m_log_scale, log) != log)
        DoLayout();
}

void GraphControl::ScaleToZero(bool zero) {
    if (std::exchange(m_zero_in_range, zero) != zero)
        DoLayout();
}

void GraphControl::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_sz = Size();
    GG::Control::SizeMove(ul, lr);
    if (Size() != old_sz)
        DoLayout();
}

void GraphControl::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    auto set_log_scale = [this]()
    { UseLogScale(true); };
    auto set_linear_scale = [this]()
    { UseLogScale(false); };
    popup->AddMenuItem(GG::MenuItem(UserString("USE_LINEAR_SCALE"), false, !m_log_scale, set_linear_scale));
    popup->AddMenuItem(GG::MenuItem(UserString("USE_LOG_SCALE"), false, m_log_scale, set_log_scale));

    auto set_zero_limit = [this]()
    { ScaleToZero(true); };
    auto free_limit = [this]()
    { ScaleToZero(false); };
    popup->AddMenuItem(GG::MenuItem(UserString("SCALE_TO_ZERO"), false, m_zero_in_range, set_zero_limit));
    popup->AddMenuItem(GG::MenuItem(UserString("SCALE_FREE"), false, !m_zero_in_range, free_limit));

    auto show_scale = [this]()
    { ShowScale(true); };
    auto hide_scale = [this]()
    { ShowScale(false); };
    popup->AddMenuItem(GG::MenuItem(UserString("SHOW_SCALE"), false, m_show_scale, show_scale));
    popup->AddMenuItem(GG::MenuItem(UserString("HIDE_SCALE"), false, !m_show_scale, hide_scale));

    auto show_lines = [this]() {
        ShowLines(true);
        ShowPoints(false);
    };
    auto show_points = [this]() {
        ShowLines(false);
        ShowPoints(true);
    };
    popup->AddMenuItem(GG::MenuItem(UserString("SHOW_LINES"), false, m_show_lines, show_lines));
    popup->AddMenuItem(GG::MenuItem(UserString("SHOW_POINTS"), false, m_show_points, show_points));

    popup->Run();
}

void GraphControl::Render() {
    // background
    FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);

    if (!m_show_lines && !m_show_points)
        return;

    static constexpr int PAD = 1;
    const int WIDTH = Value(Width()) - 2*PAD;
    const int HEIGHT = Value(Height()) - 2*PAD;

    GG::Pt ul = UpperLeft() + GG::Pt(GG::X(PAD), GG::Y(PAD));
    GG::Pt lr = ul + GG::Pt(GG::X(WIDTH), GG::Y(HEIGHT));

    GG::BeginScissorClipping(ul, lr);

    if (m_show_scale && !m_y_scale_ticks.empty()) {
        glEnable(GL_TEXTURE_2D);
        const auto font = ClientUI::GetFont();
        GG::Font::RenderState rs{ClientUI::TextColor()};
        for (auto label : m_y_scale_ticks) {
            const auto roundedlabel = boost::format("%|1$.12|") % label.second;
            font->RenderText(GG::Pt{ul.x + GG::X1, lr.y + label.first}, roundedlabel.str(), rs);
        }
    }

    glPushMatrix();
    glTranslatef(Value(ul.x), Value(lr.y), 0.0f);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glLineWidth(2.0f);
    glPointSize(5.0f);

    m_vert_buf.activate();
    m_colour_buf.activate();

    if (m_show_lines)
        glDrawArrays(GL_LINES, 0, m_vert_buf.size());

    if (m_show_points)
        glDrawArrays(GL_POINTS, 0, m_vert_buf.size());

    glLineWidth(1.0f);
    glPointSize(1.0f);

    glPopClientAttrib();
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();

    GG::EndScissorClipping();
}

void GraphControl::DoLayout() {
    static constexpr int PAD = 1;
    const int WIDTH = Value(Width()) - 2*PAD;
    const int HEIGHT = Value(Height()) - 2*PAD;
    m_vert_buf.clear();
    m_colour_buf.clear();
    m_y_scale_ticks.clear();

    // X range to plot
    double shown_x_max = m_x_max + 1;
    double shown_x_min = m_x_min - 1;
    double x_range = shown_x_max - shown_x_min;

    // Y range to plot
    double shown_y_max = m_y_max;
    double shown_y_min = m_y_min;
    double step = (shown_y_max - shown_y_min) / 10.0;
    if (m_log_scale) {
        double effective_max_val = std::max(1.0, std::max(std::abs(m_y_max), std::abs(m_y_min)));
        double effective_min_val = std::max(1.0, std::min(std::abs(m_y_max), std::abs(m_y_min)));
        // force log scale to start at 0.0 or larger
        shown_y_min = std::max(0.0, std::floor(std::log10(effective_min_val)));
        // take larger of abs of signed min and max y values as max of range, or at least miminum 0 = log10(1.0)
        shown_y_max = std::ceil(std::log10(effective_max_val));
        step = 1.0;
    } else {
        // plot from or to 0 ?
        double effective_max_val = m_y_max;// > 0.0 ? m_y_max : 0.0;
        double effective_min_val = m_y_min;// > 0.0 ? 0.0 : m_y_min;
        if (m_zero_in_range) {
            effective_max_val = m_y_max > 0.0 ? m_y_max : 0.0;
            effective_min_val = m_y_min > 0.0 ? 0.0 : m_y_min;
        }

        // pick step between scale lines based on magnitude of range and next bigger round number
        double effective_range = std::abs(effective_max_val - effective_min_val);
        double pow10_below = std::round(std::pow(10.0, std::floor(std::log10(effective_range))));
        double ratio_above_pow10 = effective_range / pow10_below;
        if (ratio_above_pow10 <= 2)
            step = 0.2 * pow10_below;   // eg. effective_range is 18, pow10_below = 10, ratio_above_pow10 = 1.8, step = 2
        else if (ratio_above_pow10 <= 5)
            step = 0.5 * pow10_below;   // eg. effective_range is 32, pow10_below = 10, ratio_above_pow10 = 3.2, step = 5
        else
            step = pow10_below;         // eg. effective_range is 87, pow10_below = 10, ratio_above_pow10 = 8.7, step = 10
        // round min down and max up to nearest multiple of step
        shown_y_min = std::round(step * std::floor(effective_min_val / step));
        shown_y_max = std::round(step * std::ceil(effective_max_val / step));
    }
    double y_range = shown_y_max - shown_y_min;


    // plot horizontal scale marker lines
    if (m_show_scale) {
        for (double line_y = shown_y_min + step; line_y <= shown_y_max; line_y = line_y + step) {
            const auto& screen_y = static_cast<float>((line_y - shown_y_min) * HEIGHT / y_range);
            m_vert_buf.store(GG::X1, -screen_y);    // OpenGL is positive down / negative up
            m_colour_buf.store(GG::LightenClr(ClientUI::WndColor()));
            m_vert_buf.store(GG::X(WIDTH - 1), -screen_y);
            m_colour_buf.store(GG::LightenClr(ClientUI::WndColor()));

            if (m_log_scale)
                m_y_scale_ticks[GG::Y(-screen_y)] = std::pow(10.0, std::round(line_y));
            else
                m_y_scale_ticks[GG::Y(-screen_y)] = line_y;
        }
    }

    // plot lines connecting successive data points
    for (auto& curve : m_data) {
        auto curve_pts = curve.first;
        if (curve_pts.empty())
            continue;
        if (m_log_scale) {
            for (auto& pt : curve_pts)
                pt.second = std::log10(std::max(1.0, std::abs(pt.second)));   // ignore sign of points, trunscate log scale at minimum 0 = log10(1.0)
        }

        const auto& curve_colour = curve.second;

        // pairs of n and n+1 point, starting with first-second, to the second-last-last
        for (auto curve_it = curve_pts.begin(); curve_it != curve_pts.end(); ++curve_it) {
            auto next_it = curve_it;
            ++next_it;
            if (next_it == curve_pts.end())
                break;

            {
                const auto& curve_x = curve_it->first;
                const auto& screen_x = static_cast<float>((curve_x - shown_x_min) * WIDTH / x_range);
                const auto& curve_y = curve_it->second;
                const auto& screen_y = static_cast<float>((curve_y - shown_y_min) * HEIGHT / y_range);
                m_vert_buf.store(screen_x, -screen_y);  // OpenGL is positive down / negative up
                m_colour_buf.store(curve_colour);
            }

            {
                const auto& curve_x = next_it->first;
                const auto& screen_x = static_cast<float>((curve_x - shown_x_min) * WIDTH / x_range);
                const auto& curve_y = next_it->second;
                const auto& screen_y = static_cast<float>((curve_y - shown_y_min) * HEIGHT / y_range);
                m_vert_buf.store(screen_x, -screen_y);  // OpenGL is positive down / negative up
                m_colour_buf.store(curve_colour);
            }
        }
    }

    m_vert_buf.createServerBuffer();
    m_colour_buf.createServerBuffer();

    // todo: determine screen and data points at which to draw scale ticks
    //
    // ticks should be placed at increments of powers of 10 multiplied by 2, 5, or 10.
    // eg. 4,6,8,10,12,14; -5,0,5,10,15,20; 0,10,20,30,40,50
}
