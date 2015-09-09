#include "GraphControl.h"

#include "ClientUI.h"

#include <GG/DrawUtil.h>
#include <GG/ClrConstants.h>


GraphControl::GraphControl() :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
    m_show_points(true),
    m_show_lines(true),
    m_show_scale(true),
    m_x_min(0.0),
    m_x_max(0.0),
    m_y_min(1.0),
    m_y_max(1.0),
    m_data(),
    m_vert_buf(),
    m_colour_buf(),
    m_x_scale_ticks(),
    m_y_scale_ticks()
{
    std::vector<std::pair<double, double> > test_data;
    test_data.push_back(std::make_pair(0.0, 1.0));
    test_data.push_back(std::make_pair(1.0, 3.2));
    test_data.push_back(std::make_pair(4.2, -1));
    test_data.push_back(std::make_pair(0, 0));
    test_data.push_back(std::make_pair(-1, 1));
    m_data.push_back(std::make_pair(test_data, GG::CLR_CYAN));

    test_data.clear();
    test_data.push_back(std::make_pair(1.0, 1.0));
    test_data.push_back(std::make_pair(2.0, 3.2));
    test_data.push_back(std::make_pair(3.2, -1));
    test_data.push_back(std::make_pair(4, 0));
    test_data.push_back(std::make_pair(5, 1));
    m_data.push_back(std::make_pair(test_data, GG::CLR_YELLOW));

    AutoSetRange();
}

void GraphControl::AddSeries(const std::vector<std::pair<double, double> >& data, const GG::Clr& clr) {
    if (!data.empty()) {
        m_data.push_back(std::make_pair(data, clr));
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
    double x_min = 99999999.9;
    double y_min = 99999999.9;
    double x_max = -99999999.9;
    double y_max = -99999999.9;

    for (std::vector<std::pair<std::vector<std::pair<double, double> >, GG::Clr> >::const_iterator
         it = m_data.begin(); it != m_data.end(); ++it)
    {
        const std::vector<std::pair<double, double> >& curve_pts = it->first;
        for (std::vector<std::pair<double, double> >::const_iterator curve_it = curve_pts.begin();
             curve_it != curve_pts.end(); ++curve_it)
        {
            if (curve_it->first < x_min)
                x_min = curve_it->first;
            if (curve_it->first > x_max)
                x_max = curve_it->first;
            if (curve_it->second < y_min)
                y_min = curve_it->second;
            if (curve_it->second > y_max)
                y_max = curve_it->second;
        }
    }

    SetRange(x_min, x_max, y_min, y_max);
}

void GraphControl::ShowPoints(bool show/* = true*/) {
    bool old_show_points = m_show_points;
    m_show_points = show;
    if (show != old_show_points)
        DoLayout();
}

void GraphControl::ShowLines(bool show/* = true*/) {
    bool old_show_lines = m_show_lines;
    m_show_lines = show;
    if (show != old_show_lines)
        DoLayout();
}

void GraphControl::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_sz = Size();
    GG::Control::SizeMove(ul, lr);
    if (Size() != old_sz)
        DoLayout();
}

void GraphControl::Render() {
    // background
    FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);

    if (!m_show_lines && !m_show_points)
        return;

    const int PAD = 1;
    const int WIDTH = Value(Width()) - 2*PAD;
    const int HEIGHT = Value(Height()) - 2*PAD;

    GG::Pt ul = UpperLeft() + GG::Pt(GG::X(PAD), GG::Y(PAD));
    GG::Pt lr = ul + GG::Pt(GG::X(WIDTH), GG::Y(HEIGHT));

    GG::BeginScissorClipping(ul, lr);

    glPushMatrix();
    glTranslatef(Value(ul.x), Value(lr.y), 0.0f);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

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
    const int PAD = 1;
    const int WIDTH = Value(Width()) - 2*PAD;
    const int HEIGHT = Value(Height()) - 2*PAD;

    double x_range = m_x_max - m_x_min;
    double y_range = m_y_max - m_y_min;

    m_vert_buf.clear();
    m_colour_buf.clear();


    for (std::vector<std::pair<std::vector<std::pair<double, double> >, GG::Clr> >::const_iterator
         it = m_data.begin(); it != m_data.end(); ++it)
    {
        const std::vector<std::pair<double, double> >& curve_pts = it->first;
        if (curve_pts.empty())
            continue;

        GG::Clr curve_colour = it->second;
        double curve_x, curve_y;
        float screen_x, screen_y;

        // pairs of n and n+1 point, starting with first-second, to the second-last-last
        std::vector<std::pair<double, double> >::const_iterator curve_it = curve_pts.begin();
        for (; curve_it != curve_pts.end(); ++curve_it) {
            std::vector<std::pair<double, double> >::const_iterator next_it = curve_it;
            ++next_it;
            if (next_it == curve_pts.end())
                break;

            curve_x = curve_it->first;
            screen_x = static_cast<float>((curve_x - m_x_min) * WIDTH / x_range);
            curve_y = curve_it->second;
            screen_y = static_cast<float>((curve_y - m_y_min) * HEIGHT / y_range);
            m_vert_buf.store(screen_x, -screen_y);  // OpenGL is positive down / negative up
            m_colour_buf.store(curve_colour);

            curve_x = next_it->first;
            screen_x = static_cast<float>((curve_x - m_x_min) * WIDTH / x_range);
            curve_y = next_it->second;
            screen_y = static_cast<float>((curve_y - m_y_min) * HEIGHT / y_range);
            m_vert_buf.store(screen_x, -screen_y);  // OpenGL is positive down / negative up
            m_colour_buf.store(curve_colour);
        }
    }

    m_vert_buf.createServerBuffer();
    m_colour_buf.createServerBuffer();

    // todo: determine screen and data points at which to draw scale ticks
    //
    // ticks should be placed at increments of powers of 10 multiplied by 2, 5, or 10.
    // eg. 4,6,8,10,12,14; -5,0,5,10,15,20; 0,10,20,30,40,50
}
