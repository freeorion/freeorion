#ifndef _CensusBrowseWnd_h_
#define _CensusBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>


/** A popup tooltop for display when mousing over in-game icons.  A title and some detail text.*/
class CensusBrowseWnd : public GG::BrowseInfoWnd {
public:
    CensusBrowseWnd(std::string title_text,
                    std::map<std::string, float> population_counts,
                    std::map<std::string, float> tag_counts,
                    std::vector<std::string> census_order);

    void CompleteConstruction() override;
    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;

    void Render() override;

private:
    virtual void  InitBuffer();

    GG::GL2DVertexBuffer            m_buffer;
    std::shared_ptr<GG::Label>      m_title_text;
    std::shared_ptr<GG::Label>      m_species_text;
    std::shared_ptr<GG::ListBox>    m_list;
    std::shared_ptr<GG::Label>      m_tags_text;
    std::shared_ptr<GG::ListBox>    m_tags_list;
    GG::Pt                          m_offset;
    std::map<std::string, float>    m_population_counts;
    std::map<std::string, float>    m_tag_counts;
    std::vector<std::string>        m_census_order;

    void InitRowSizes();
};

#endif
