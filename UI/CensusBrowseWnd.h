// -*- C++ -*-
#ifndef _CensusBrowseWnd_h_
#define _CensusBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>


/** A popup tooltop for display when mousing over in-game icons.  A title and some detail text.*/
class CensusBrowseWnd : public GG::BrowseInfoWnd {
public:
    CensusBrowseWnd(const std::string& title_text, const std::map<std::string, float>& population_counts, const std::map<std::string, float>& tag_counts);
    virtual bool    WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const;
    virtual void    Render();
    void            DoLayout();

private:
    virtual void  InitBuffer();

    GG::GL2DVertexBuffer    m_buffer;

    GG::Label*      m_title_text;
    GG::Label*      m_species_text;
    GG::ListBox*    m_list;
    GG::Label*      m_tags_text;
    GG::ListBox*    m_tags_list;
    GG::Pt          m_offset;
    GG::Y           m_row_height;

    void InitRowSizes();
};

#endif
