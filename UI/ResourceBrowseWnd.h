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
                      bool show_stockpile = false, float stockpile_use = 0.0f,
                      float stockpile = 0.0f, float stockpile_change = 0.0f,
                      bool show_stockpile_limit = false,
                      float stockpile_use_limit = 0.0f);
    void CompleteConstruction() override;
    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;
    void Render() override;

private:
    virtual void  InitBuffer();

    GG::GL2DVertexBuffer        m_buffer;
    std::shared_ptr<GG::Label>  m_title_text;

    bool                        m_show_points = false;
    std::shared_ptr<GG::Label>  m_used_points_label;
    std::shared_ptr<GG::Label>  m_used_points;
    std::shared_ptr<GG::Label>  m_used_points_P_label;
    std::shared_ptr<GG::Label>  m_output_points_label;
    std::shared_ptr<GG::Label>  m_output_points;
    std::shared_ptr<GG::Label>  m_output_points_P_label;
    std::shared_ptr<GG::Label>  m_target_points_label;
    std::shared_ptr<GG::Label>  m_target_points;
    std::shared_ptr<GG::Label>  m_target_points_P_label;
    bool                        m_show_stockpile = false;
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
    bool                        m_show_stockpile_limit = false;
};

class WastedStockpiledResourceBrowseWnd : public GG::BrowseInfoWnd {
public:
    WastedStockpiledResourceBrowseWnd(const std::string& title_text,
                                      const std::string& unit_label,
                                      float capacity, float excess,
                                      bool show_stockpile = false,
                                      float stockpile_effic = 0.0f,
                                      float to_stockpile = 0.0f,
                                      float wasted = 0.0f,
                                      const std::string& bottom_message = "");
    void CompleteConstruction() override;
    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;
    void Render() override;

private:
    virtual void  InitBuffer();

    GG::GL2DVertexBuffer        m_buffer;
    std::shared_ptr<GG::Label>  m_title_text;

    std::shared_ptr<GG::Label>  m_output_label;
    std::shared_ptr<GG::Label>  m_output;
    std::shared_ptr<GG::Label>  m_output_P_label;
    std::shared_ptr<GG::Label>  m_used_label;
    std::shared_ptr<GG::Label>  m_used;
    std::shared_ptr<GG::Label>  m_used_P_label;
    std::shared_ptr<GG::Label>  m_excess_label;
    std::shared_ptr<GG::Label>  m_excess;
    std::shared_ptr<GG::Label>  m_excess_P_label;

    bool                        m_show_stockpile;
    std::shared_ptr<GG::Label>  m_stockpile_label;
    std::shared_ptr<GG::Label>  m_stockpile;
    std::shared_ptr<GG::Label>  m_stockpile_P_label;

    std::shared_ptr<GG::Label>  m_wasted_label;
    std::shared_ptr<GG::Label>  m_wasted;
    std::shared_ptr<GG::Label>  m_wasted_P_label;

    std::shared_ptr<GG::Label>  m_bottom_message_text;

    GG::Pt                      m_offset;
};

#endif
