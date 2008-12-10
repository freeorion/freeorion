#include "LinkText.h"

#include "ClientUI.h"
#include "../util/MultiplayerCommon.h"

#include <GG/DrawUtil.h>
#include <GG/WndEvent.h>

#include <boost/lexical_cast.hpp>

#define RENDER_DEBUGGING_LINK_RECTS 0


///////////////////////////////////////
// LinkText
///////////////////////////////////////
LinkText::LinkText(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   GG::Flags<GG::TextFormat> format/* = GG::FORMAT_NONE*/, GG::Clr color/* = GG::CLR_BLACK*/, GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) : 
    GG::TextControl(x, y, w, GG::Y1, str, font, color, format, flags),
    TextLinker()
{
    Resize(TextLowerRight() - TextUpperLeft());
    FindLinks();
}

LinkText::LinkText(GG::X x, GG::Y y, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   GG::Clr color/* = GG::CLR_BLACK*/, GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) : 
    GG::TextControl(x, y, str, font, color, GG::FORMAT_NONE, flags),
    TextLinker()
{
    FindLinks();
}

void LinkText::Render()
{
    GG::TextControl::Render();
    TextLinker::Render_();
}

void LinkText::SetText(const std::string& str)
{
    GG::TextControl::SetText(str);
    FindLinks();
}

void LinkText::SetLinkedText(const std::string& str)
{
    GG::TextControl::SetText(str);
}

void LinkText::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    TextLinker::LButtonDown_(pt, mod_keys);
}

void LinkText::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    TextLinker::LButtonUp_(pt, mod_keys);
}

GG::Pt LinkText::TextUpperLeft() const
{
    return GG::TextControl::TextUpperLeft();
}

GG::Pt LinkText::TextLowerRight() const
{
    return GG::TextControl::TextLowerRight();
}

const std::vector<GG::Font::LineData>& LinkText::GetLineData() const
{
    return GG::TextControl::GetLineData();
}

const boost::shared_ptr<GG::Font>& LinkText::GetFont() const
{
    return GG::TextControl::GetFont();
}

const std::string& LinkText::WindowText() const
{
    return GG::TextControl::WindowText();
}

void LinkText::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    TextLinker::LClick_(pt, mod_keys);
}

void LinkText::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    TextLinker::MouseHere_(pt, mod_keys);
}

void LinkText::MouseLeave()
{
    TextLinker::MouseLeave_();
}


///////////////////////////////////////
// TextLinker
///////////////////////////////////////
// initialize static(s)
bool TextLinker::s_link_tags_registered = false;

TextLinker::TextLinker() : 
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
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
    }
}

TextLinker::~TextLinker()
{}

void TextLinker::Render_()
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
}

void TextLinker::LButtonDown_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_old_sel_link = GetLinkUnderPt(pt);
}

void TextLinker::LButtonUp_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_old_sel_link = -1;
    ClearOldRollover();
}

void TextLinker::LClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    using boost::lexical_cast;
    int sel_link = GetLinkUnderPt(pt);
    if (sel_link == m_old_sel_link && sel_link != -1) {
        if (m_links[sel_link].type == "planet") {
            ClientUI::GetClientUI()->ZoomToPlanet(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "system") {
            ClientUI::GetClientUI()->ZoomToSystem(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "fleet") {
            ClientUI::GetClientUI()->ZoomToFleet(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "ship") {
            ClientUI::GetClientUI()->ZoomToShip(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "tech") {
            ClientUI::GetClientUI()->ZoomToTech(m_links[sel_link].data);
        } else if (m_links[sel_link].type == "building") {
            ClientUI::GetClientUI()->ZoomToBuildingType(m_links[sel_link].data);
        } else if (m_links[sel_link].type == "encyclopedia") {
            ClientUI::GetClientUI()->ZoomToEncyclopediaEntry(m_links[sel_link].data);
        }
    }
    m_old_sel_link = -1;
}

void TextLinker::MouseHere_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    int rollover_link = GetLinkUnderPt(pt);
    if (rollover_link != m_old_rollover_link) {
        ClearOldRollover();
        if (rollover_link != -1) {
            std::string text = WindowText();
            text.insert(m_links[rollover_link].text_posn.first, "<u>");
            text.insert(m_links[rollover_link].text_posn.second + 3, "</u>");
            SetLinkedText(text);
            m_old_rollover_link = rollover_link;
        }
    }
}

void TextLinker::MouseLeave_()
{
    ClearOldRollover();
}

void TextLinker::FindLinks()
{
    m_links.clear();

    const std::vector<GG::Font::LineData>& line_data = GetLineData();
    const boost::shared_ptr<GG::Font>& font = GetFont();

    GG::Y y_posn(0); // y-coordinate of the top of the current line
    Link link;

    for (std::vector<GG::Font::LineData>::const_iterator it = line_data.begin(); 
         it != line_data.end();
         ++it, y_posn += font->Lineskip()) {
        const GG::Font::LineData& curr_line = *it;

        // if the last line ended without the current tag ending
        if (link.type != "") {
            link.rects.push_back(GG::Rect(GG::X0, y_posn, GG::X0, y_posn + font->Height()));
        }

        for (unsigned int i = 0; i < curr_line.char_data.size(); ++i) {
            for (unsigned int j = 0; j < curr_line.char_data[i].tags.size(); ++j) {
                const boost::shared_ptr<GG::Font::FormattingTag>& tag = curr_line.char_data[i].tags[j];
                if (tag->text == "planet" || tag->text == "system" || tag->text == "fleet" ||
                    tag->text == "ship" || tag->text == "tech" || tag->text == "building" || tag->text == "encyclopedia") {
                    link.type = tag->text;
                    if (tag->close_tag) {
                        link.text_posn.second = curr_line.char_data[i].string_index;
                        link.rects.back().lr.x = i ? curr_line.char_data[i - 1].extent : GG::X0;
                        m_links.push_back(link);
                        link = Link();
                    } else {
                        link.data = tag->params[0];
                        link.text_posn.first = curr_line.char_data[i].string_index;
                        for (unsigned int k = 0; k < curr_line.char_data[i].tags.size(); ++k) {
                            link.text_posn.first -= curr_line.char_data[i].tags[k]->OriginalStringChars();
                        }
                        link.rects.push_back(GG::Rect(i ? curr_line.char_data[i - 1].extent : GG::X0,
                                                      y_posn,
                                                      GG::X0,
                                                      y_posn + font->Height()));
                    }
                }
            }
        }

        // if a line is ending without the current tag ending
        if (link.type != "") {
            link.rects.back().lr.x = curr_line.char_data.empty() ? GG::X0 : curr_line.char_data.back().extent;
        }
    }
}

int TextLinker::GetLinkUnderPt(const GG::Pt& pt)
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

void TextLinker::ClearOldRollover()
{
    if (m_old_rollover_link != -1) {
        std::string text = WindowText();
        std::string::size_type pos = text.find("<u>", m_links[m_old_rollover_link].text_posn.first);
        text.erase(pos, 3);
        pos = text.find("</u>", m_links[m_old_rollover_link].text_posn.first);
        text.erase(pos, 4);
        SetLinkedText(text);
    }
    m_old_rollover_link = -1;
}
