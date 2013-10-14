// -*- C++ -*-
#ifndef _GraphControl_h_
#define _GraphControl_h_

#include <GG/Control.h>
#include <vector>

namespace GG {
    struct Clr;
}

class GraphControl : public GG::Control {
public:
    //! \name Structors //@{
    GraphControl(GG::X x, GG::Y y, GG::X w, GG::Y h);
    //@}

    //! \name Mutators //@{
    void            AddSeries(const std::vector<std::pair<double, double> >& data, const GG::Clr& clr);
    void            Clear();

    void            SetXMin(double x_min);
    void            SetYMin(double y_min);
    void            SetXMax(double x_max);
    void            SetYMax(double y_max);
    void            SetRange(double x1, double x2, double y1, double y2);
    void            AutoSetRange();

    void            ShowPoints(bool show = true);
    void            ShowLines(bool show = true);
    void            ShowScale(bool show = true);

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    Render();
    //@}

private:
    void            DoLayout();

    bool    m_show_points, m_show_lines, m_show_scale;
    double  m_x_min, m_x_max, m_y_min, m_y_max;
    std::vector<std::pair<std::vector<std::pair<double, double> >,  GG::Clr> > m_data;
    std::vector<std::pair<std::vector<std::pair<int, int> >,        GG::Clr> > m_render_pts;
    std::map<int, double>   m_x_scale_ticks;
    std::map<int, double>   m_y_scale_ticks;
};

#endif
