// -*- C++ -*-
#ifndef _IconTextBrowseWnd_h_
#define _IconTextBrowseWnd_h_

#include <GG/BrowseInfoWnd.h>

namespace GG {
    class StaticGraphic;
}

class CUILabel;

/** A popup tooltop for display when mousing over in-game icons.  Has an icon and title and some detail text.*/
class IconTextBrowseWnd : public GG::BrowseInfoWnd {
public:
    IconTextBrowseWnd(const boost::shared_ptr<GG::Texture> texture, const std::string& title_text,
                      const std::string& main_text);
    virtual bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const;
    virtual void Render();

private:
    GG::StaticGraphic*  m_icon;
    CUILabel*           m_title_text;
    CUILabel*           m_main_text;
};

#endif
