#ifndef _ResourceBrowseWnd_h_
#define _ResourceBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>


/** A popup tooltop for display when mousing over in-game icons.
  * A title and some detail text.*/
class ResourceBrowseWnd : public GG::BrowseInfoWnd {
public:
    ResourceBrowseWnd(const std::string& title_text, const std::string& unit_label,
                      float used, float output, float target_output,
                      bool show_stockpile = false,
                      float stockpile_use = 0.0f,
                      float stockpile = 0.0f, float stockpile_change = 0.0f);
    void CompleteConstruction() override;
    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;
    void Render() override;

private:
    virtual void  InitBuffer();

    GG::GL2DVertexBuffer        m_buffer;
    std::shared_ptr<GG::Label>  m_title_text;

    bool                        m_show_points;
    std::shared_ptr<GG::Label>  m_used_points_label;
    std::shared_ptr<GG::Label>  m_used_points;
    std::shared_ptr<GG::Label>  m_used_points_P_label;
    std::shared_ptr<GG::Label>  m_output_points_label;
    std::shared_ptr<GG::Label>  m_output_points;
    std::shared_ptr<GG::Label>  m_output_points_P_label;
    std::shared_ptr<GG::Label>  m_target_points_label;
    std::shared_ptr<GG::Label>  m_target_points;
    std::shared_ptr<GG::Label>  m_target_points_P_label;
    bool                        m_show_stockpile;
    std::shared_ptr<GG::Label>  m_stockpile_points_label;
    std::shared_ptr<GG::Label>  m_stockpile_points;
    std::shared_ptr<GG::Label>  m_stockpile_points_P_label;
    std::shared_ptr<GG::Label>  m_stockpile_used_points_label;
    std::shared_ptr<GG::Label>  m_stockpile_used_points;
    std::shared_ptr<GG::Label>  m_stockpile_used_points_P_label;
    std::shared_ptr<GG::Label>  m_stockpile_change_points_label;
    std::shared_ptr<GG::Label>  m_stockpile_change_points;
    std::shared_ptr<GG::Label>  m_stockpile_change_points_P_label;

    GG::Pt                      m_offset;
};

#endif
