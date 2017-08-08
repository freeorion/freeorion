#ifndef _IconTextBrowseWnd_h_
#define _IconTextBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>


/** A popup tooltop for display when mousing over in-game icons.  Has an icon and title and some detail text.*/
class IconTextBrowseWnd : public GG::BrowseInfoWnd {
public:
    IconTextBrowseWnd(const std::shared_ptr<GG::Texture> texture, const std::string& title_text,
                      const std::string& main_text);

    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;

    void PreRender() override;

    void Render() override;

private:
    std::shared_ptr<GG::StaticGraphic>                   m_icon;
    std::shared_ptr<GG::Label>                           m_title_text_label;
    std::shared_ptr<GG::Label>                           m_main_text_label;
    const std::shared_ptr<GG::Texture> m_texture;
    const std::string                    m_title_text;
    const std::string                    m_main_text;
};

#endif
