#include "LinkText.h"

#include "../util/MultiplayerCommon.h"

#include <GG/DrawUtil.h>

#include <boost/lexical_cast.hpp>


#define RENDER_DEBUGGING_LINK_RECTS 0


// initialize static(s)
bool LinkText::s_link_tags_registered = false;


LinkText::LinkText(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   Uint32 text_fmt/* = 0*/, GG::Clr color/* = GG::CLR_BLACK*/, GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) : 
    GG::TextControl(x, y, w, 1, str, font, color, text_fmt, flags),
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
{
    Resize(TextLowerRight() - TextUpperLeft());
    Init();
}

LinkText::LinkText(int x, int y, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   GG::Clr color/* = GG::CLR_BLACK*/, GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) : 
    GG::TextControl(x, y, str, font, color, 0, flags),
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
{
    Init();
}

void LinkText::Render()
{
#if RENDER_DEBUGGING_LINK_RECTS
    GG::Rect bounds(TextUpperLeft(), TextLowerRight());
    FlatRectangle(bounds.ul.x, bounds.ul.y, bounds.lr.x, bounds.lr.y, GG::CLR_ZERO, GG::CLR_YELLOW, 1);
    for (unsigned int i = 0; i < m_links.size(); ++i) {
        for (unsigned int j = 0; j < m_links[i].rects.size(); ++j) {
            GG::Rect r = TextUpperLeft() + m_links[i].rects[j];
            FlatRectangle(r.ul.x, r.ul.y, r.lr.x, r.lr.y, GG::CLR_ZERO, GG::CLR_RED, 1);
        }
    }
#endif
    GG::TextControl::Render();
}

void LinkText::LButtonDown(const GG::Pt& pt, Uint32 keys)
{
    m_old_sel_link = GetLinkUnderPt(pt);
}

void LinkText::LButtonUp(const GG::Pt& pt, Uint32 keys)
{
    m_old_sel_link = -1;
    ClearOldRollover();
}

void LinkText::LClick(const GG::Pt& pt, Uint32 keys)
{
    using boost::lexical_cast;
    int sel_link = GetLinkUnderPt(pt);
    if (sel_link == m_old_sel_link && sel_link != -1) {
        if (m_links[sel_link].type == "planet") {
            PlanetLinkSignal(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "system") {
            SystemLinkSignal(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "fleet") {
            FleetLinkSignal(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "ship") {
            ShipLinkSignal(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "tech") {
            TechLinkSignal(m_links[sel_link].data);
        } else if (m_links[sel_link].type == "building") {
            BuildingLinkSignal(m_links[sel_link].data);
        } else if (m_links[sel_link].type == "encyclopedia") {
            EncyclopediaLinkSignal(m_links[sel_link].data);
        }
    }
    m_old_sel_link = -1;
}

void LinkText::MouseHere(const GG::Pt& pt, Uint32 keys)
{
    int rollover_link = GetLinkUnderPt(pt);
    if (rollover_link != m_old_rollover_link) {
        ClearOldRollover();
        if (rollover_link != -1) {
            std::string text = WindowText();
            text.insert(m_links[rollover_link].text_posn.first, "<u>");
            text.insert(m_links[rollover_link].text_posn.second + 3, "</u>");
            SetText(text);
            m_old_rollover_link = rollover_link;
        }
    }
}

void LinkText::MouseLeave()
{
    ClearOldRollover();
}
   
void LinkText::SetText(const std::string& str)
{
    GG::TextControl::SetText(str);
    FindLinks();
}

void LinkText::Init()
{
    // if this is the first LinkText created, it needs to register the tags that it knows about
    if (!s_link_tags_registered) {
        GG::Font::RegisterKnownTag("planet");
        GG::Font::RegisterKnownTag("system");
        GG::Font::RegisterKnownTag("fleet");
        GG::Font::RegisterKnownTag("ship");
        GG::Font::RegisterKnownTag("tech");
        GG::Font::RegisterKnownTag("building");
        GG::Font::RegisterKnownTag("encyclopedia");
        s_link_tags_registered = true;
        SetText(WindowText());
    }
    FindLinks();
}

void LinkText::FindLinks()
{
    m_links.clear();

    const std::vector<GG::Font::LineData>& line_data = GetLineData();
    const boost::shared_ptr<GG::Font>& font = GetFont();

    int y_posn = 0; // y-coordinate of the top of the current line
    Link link;

    for (std::vector<GG::Font::LineData>::const_iterator it = line_data.begin(); 
         it != line_data.end();
         ++it, y_posn += font->Lineskip()) {
        const GG::Font::LineData& curr_line = *it;

        // if the last line ended without the current tag ending
        if (link.type != "") {
            link.rects.push_back(GG::Rect(0, y_posn, 0, y_posn + font->Height()));
        }

        for (unsigned int i = 0; i < curr_line.char_data.size(); ++i) {
            for (unsigned int j = 0; j < curr_line.char_data[i].tags.size(); ++j) {
                const boost::shared_ptr<GG::Font::FormattingTag>& tag = curr_line.char_data[i].tags[j];
                if (tag->text == "planet" || tag->text == "system" || tag->text == "fleet" ||
                    tag->text == "ship" || tag->text == "tech" || tag->text == "building" || tag->text == "encyclopedia") {
                    link.type = tag->text;
                    if (tag->close_tag) {
                        link.text_posn.second = curr_line.char_data[i].original_char_index;
                        link.rects.back().lr.x = i ? curr_line.char_data[i - 1].extent : 0;
                        m_links.push_back(link);
                        link = Link();
                    } else {
                        link.data = tag->params[0];
                        link.text_posn.first = curr_line.char_data[i].original_char_index;
                        for (unsigned int k = 0; k < curr_line.char_data[i].tags.size(); ++k) {
                            link.text_posn.first -= curr_line.char_data[i].tags[k]->OriginalStringChars();
                        }
                        link.rects.push_back(GG::Rect(i ? curr_line.char_data[i - 1].extent : 0,
                                                      y_posn,
                                                      0,
                                                      y_posn + font->Height()));
                    }
                }
            }
        }

        // if a line is ending without the current tag ending
        if (link.type != "") {
            link.rects.back().lr.x = curr_line.char_data.empty() ? 0 : curr_line.char_data.back().extent;
        }
    }
}

int LinkText::GetLinkUnderPt(const GG::Pt& pt)
{
    int retval = -1;
    for (unsigned int i = 0; i < m_links.size(); ++i) {
        for (unsigned int j = 0; j < m_links[i].rects.size(); ++j) {
            GG::Rect r = TextUpperLeft() + m_links[i].rects[j];
            if (r.Contains(pt)) {
                retval = i;
                break;
            }
        }
    }
    return retval;
}

void LinkText::ClearOldRollover()
{
    if (m_old_rollover_link != -1) {
        std::string text = WindowText();
        std::string::size_type pos = text.find("<u>", m_links[m_old_rollover_link].text_posn.first);
        text.erase(pos, 3);
        pos = text.find("</u>", m_links[m_old_rollover_link].text_posn.first);
        text.erase(pos, 4);
        SetText(text);
    }
    m_old_rollover_link = -1;
}


