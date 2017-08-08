#ifndef _TextBrowseWnd_h_
#define _TextBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>



/** A popup tooltop for display when mousing over in-game icons.  A title and some detail text.*/
class TextBrowseWnd : public GG::BrowseInfoWnd {
public:
    TextBrowseWnd(const std::string& title_text, const std::string& main_text, GG::X w = GG::X(200));

    void CompleteConstruction() override;

    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;

    void Render() override;

private:
    std::shared_ptr<GG::Label>          m_title_text;
    std::shared_ptr<GG::Label>          m_main_text;
    GG::Pt              m_offset;
};

#endif
