#include "LinkText.h"

#include "../util/MultiplayerCommon.h"

#include <boost/lexical_cast.hpp>


namespace {
    bool temp_header_bool = RecordHeaderFile(LinkTextRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


// initialize static(s)
bool LinkText::s_link_tags_registered = false;


LinkText::LinkText(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   Uint32 text_fmt/* = 0*/, GG::Clr color/* = GG::CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) : 
    GG::TextControl(x, y, w, h, str, font, color, text_fmt, flags),
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
{
    Init();
}

LinkText::LinkText(int x, int y, int w, int h, const std::string& str, const std::string& font_filename, int pts, 
                   Uint32 text_fmt/* = 0*/, GG::Clr color/* = GG::CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) : 
    GG::TextControl(x, y, w, h, str, font_filename, pts, color, text_fmt, flags),
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
{
    Init();
}

LinkText::LinkText(int x, int y, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   GG::Clr color/* = GG::CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) : 
    GG::TextControl(x, y, str, font, color, 0, flags),
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
{
    Init();
}

LinkText::LinkText(int x, int y, const std::string& str, const std::string& font_filename, int pts, 
                   GG::Clr color/* = GG::CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) : 
    GG::TextControl(x, y, str, font_filename, pts, color, 0, flags),
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
{
    Init();
}

LinkText::LinkText(const GG::XMLElement& elem) : 
    GG::TextControl(elem.Child("GG::TextControl")),
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
{
    if (elem.Tag() != "LinkText")
        throw std::invalid_argument("Attempted to construct a LinkText from an XMLElement that had a tag other than \"LinkText\"");
   
    Init();
}

void LinkText::LButtonDown(const GG::Pt& pt, Uint32 keys)
{
    m_old_sel_link = GetLinkUnderPt(pt);
}

void LinkText::LButtonUp(const GG::Pt& pt, Uint32 keys)
{
    m_old_sel_link = -1;
}

void LinkText::LClick(const GG::Pt& pt, Uint32 keys)
{
    using boost::lexical_cast;
    int sel_link = GetLinkUnderPt(pt);
    if (sel_link == m_old_sel_link && sel_link != -1) {
        if (m_links[sel_link].type == "planet") {
            m_planet_sig(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "system") {
            m_system_sig(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "fleet") {
            m_fleet_sig(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "ship") {
            m_ship_sig(lexical_cast<int>(m_links[sel_link].data));
        } else if (m_links[sel_link].type == "tech") {
            m_tech_sig(m_links[sel_link].data);
        } else if (m_links[sel_link].type == "encyclopedia") {
            m_encyclopedia_sig(m_links[sel_link].data);
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

void LinkText::MouseLeave(const GG::Pt& pt, Uint32 keys)
{
    ClearOldRollover();
}
   
void LinkText::SetText(const std::string& str)
{
    GG::TextControl::SetText(str);
    FindLinks();
}

GG::XMLElement LinkText::XMLEncode() const
{
    GG::XMLElement retval("LinkText");
    retval.AppendChild(GG::TextControl::XMLEncode());
    return retval;
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
            if (!curr_line.char_data[i].tags.empty()) {
                for (unsigned int j = 0; j < curr_line.char_data[i].tags.size(); ++j) {
                    const boost::shared_ptr<GG::Font::FormattingTag>& tag = curr_line.char_data[i].tags[j];
                    if (tag->text == "planet" || tag->text == "system" || tag->text == "fleet" ||
                        tag->text == "ship" || tag->text == "tech" || tag->text == "encyclopedia") {
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
            if (m_links[i].rects[j].Contains(ScreenToWindow(pt))) {
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
        pos = text.find("</u>", std::min(0, m_links[m_old_rollover_link].text_posn.first - 3));
        text.erase(pos, 4);
        SetText(text);
    }
    m_old_rollover_link = -1;
}


