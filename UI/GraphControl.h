#ifndef _GraphControl_h_
#define _GraphControl_h_

#include <vector>
#include <GG/GGFwd.h>
#include <GG/Control.h>
#include <GG/GLClientAndServerBuffer.h>

class GraphControl : public GG::Control {
public:
    //! \name Structors //@{
    GraphControl();
    //@}

    //! \name Mutators //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void Render() override;

    void AddSeries(const std::vector<std::pair<double, double>>& data, const GG::Clr& clr);
    void Clear();

    void SetXMin(double x_min);
    void SetYMin(double y_min);
    void SetXMax(double x_max);
    void SetYMax(double y_max);
    void SetRange(double x1, double x2, double y1, double y2);
    void AutoSetRange();

    void ShowPoints(bool show = true);
    void ShowLines(bool show = true);
    void ShowScale(bool show = true);
    void UseLogScale(bool log = true);
    void ScaleToZero(bool zero = true);
    //@}

private:
    void DoLayout();

    bool    m_show_points = true;
    bool    m_show_lines = true;
    bool    m_show_scale = true;
    bool    m_log_scale = false;
    bool    m_zero_in_range = false;

    double  m_x_min = 0.0f;
    double  m_x_max = 1.0f;
    double  m_y_min = 0.0f;
    double  m_y_max = 1.0f;
    std::vector<std::pair<std::vector<std::pair<double, double>>, GG::Clr>> m_data;

    GG::GL2DVertexBuffer    m_vert_buf;
    GG::GLRGBAColorBuffer   m_colour_buf;
    std::map<GG::Y, double> m_y_scale_ticks;
};

#endif
