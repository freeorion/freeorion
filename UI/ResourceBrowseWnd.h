#ifndef _ResourceBrowseWnd_h_
#define _ResourceBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>


/** A popup tooltop for display when mousing over in-game icons.  A title and some detail text.*/
class ResourceBrowseWnd : public GG::BrowseInfoWnd {
public:
    ResourceBrowseWnd(const std::string& title_text, const std::string& unit_label, float used, float output, float target_output);
    void CompleteConstruction() override;
    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;
    void Render() override;

private:
    virtual void  InitBuffer();

    GG::GL2DVertexBuffer        m_buffer;
    std::shared_ptr<GG::Label>  m_title_text;

    std::shared_ptr<GG::Label>  m_used_points_label;
    std::shared_ptr<GG::Label>  m_used_points;
    std::shared_ptr<GG::Label>  m_used_points_P_label;
    std::shared_ptr<GG::Label>  m_output_points_label;
    std::shared_ptr<GG::Label>  m_output_points;
    std::shared_ptr<GG::Label>  m_output_points_P_label;
    std::shared_ptr<GG::Label>  m_target_points_label;
    std::shared_ptr<GG::Label>  m_target_points;
    std::shared_ptr<GG::Label>  m_target_points_P_label;

    GG::Pt      m_offset;
};

#endif
