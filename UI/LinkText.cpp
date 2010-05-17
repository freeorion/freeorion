#include "LinkText.h"

#include "ClientUI.h"
#include "../util/MultiplayerCommon.h"
#include "../util/VarText.h"

#include <GG/DrawUtil.h>
#include <GG/WndEvent.h>

#include <boost/lexical_cast.hpp>

namespace {
    static const bool RENDER_DEBUGGING_LINK_RECTS = false;

    static const std::string ENCYCLOPEDIA_TAG = "encyclopedia";

    static bool link_tags_registered = false;
    void RegisterLinkTags() {
        if (link_tags_registered)
            return;
        // need to register the tags that link text uses so GG::Font will know how to (not) render them
        GG::Font::RegisterKnownTag(VarText::PLANET_ID_TAG);
        GG::Font::RegisterKnownTag(VarText::SYSTEM_ID_TAG);
        GG::Font::RegisterKnownTag(VarText::SHIP_ID_TAG);
        GG::Font::RegisterKnownTag(VarText::FLEET_ID_TAG);
        GG::Font::RegisterKnownTag(VarText::BUILDING_ID_TAG);

        GG::Font::RegisterKnownTag(VarText::EMPIRE_ID_TAG);

        GG::Font::RegisterKnownTag(VarText::TECH_TAG);
        GG::Font::RegisterKnownTag(VarText::BUILDING_TYPE_TAG);
        GG::Font::RegisterKnownTag(VarText::SPECIAL_TAG);
        GG::Font::RegisterKnownTag(VarText::SHIP_HULL_TAG);
        GG::Font::RegisterKnownTag(VarText::SHIP_PART_TAG);
        //GG::Font::RegisterKnownTag(VarText::SPECIES_TAG);

        GG::Font::RegisterKnownTag(ENCYCLOPEDIA_TAG);
        link_tags_registered = true;
    }
}

///////////////////////////////////////
// LinkText
///////////////////////////////////////
LinkText::LinkText(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   GG::Flags<GG::TextFormat> format/* = GG::FORMAT_NONE*/, GG::Clr color/* = GG::CLR_BLACK*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) : 
    GG::TextControl(x, y, w, GG::Y1, str, font, color, format, flags),
    TextLinker()
{
    Resize(TextLowerRight() - TextUpperLeft());
    FindLinks();
}

LinkText::LinkText(GG::X x, GG::Y y, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   GG::Clr color/* = GG::CLR_BLACK*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) : 
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

std::string LinkText::Text_() const
{
    return GG::TextControl::Text();
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
// TextLinker::Link
///////////////////////////////////////
struct TextLinker::Link
{
    std::string             type;           ///< contents of type field of link tag (eg "planet" in <planet 3>)
    std::string             data;           ///< contents of data field of link tag (eg "3" in <planet 3>)
    std::vector<GG::Rect>   rects;          ///< the rectangles in which this link falls, in window coordinates (some links may span more than one line)
    std::pair<int, int>     text_posn;      ///< the index of the first (.first) and last + 1 (.second) characters in the link text
};


///////////////////////////////////////
// TextLinker
///////////////////////////////////////
TextLinker::TextLinker() : 
    m_old_sel_link(-1),
    m_old_rollover_link(-1)
{
    RegisterLinkTags();
}

TextLinker::~TextLinker()
{}

void TextLinker::Render_()
{
    if (!RENDER_DEBUGGING_LINK_RECTS)
        return;

    // draw yellow box around whole text block
    GG::Rect bounds(TextUpperLeft(), TextLowerRight());
    FlatRectangle(bounds.ul, bounds.lr, GG::CLR_ZERO, GG::CLR_YELLOW, 1);

    // draw red box around individual linkified bits of text within block
    for (unsigned int i = 0; i < m_links.size(); ++i) {
        for (unsigned int j = 0; j < m_links[i].rects.size(); ++j) {
            GG::Rect r = TextUpperLeft() + m_links[i].rects[j];
            FlatRectangle(r.ul, r.lr, GG::CLR_ZERO, GG::CLR_RED, 1);
        }
    }
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
            std::string text = Text_();
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
                if (tag->tag_name == VarText::PLANET_ID_TAG ||
                    tag->tag_name == VarText::SYSTEM_ID_TAG ||
                    tag->tag_name == VarText::SHIP_ID_TAG ||
                    tag->tag_name == VarText::FLEET_ID_TAG ||
                    tag->tag_name == VarText::BUILDING_ID_TAG ||
                    tag->tag_name == VarText::EMPIRE_ID_TAG ||
                    tag->tag_name == VarText::TECH_TAG ||
                    tag->tag_name == VarText::BUILDING_TYPE_TAG ||
                    tag->tag_name == VarText::SPECIAL_TAG ||
                    tag->tag_name == VarText::SHIP_HULL_TAG ||
                    tag->tag_name == VarText::SHIP_PART_TAG ||
//                    tag->tag_name == VarText::SPECIES_TAG ||
                    tag->tag_name == ENCYCLOPEDIA_TAG)
                {
                    link.type = tag->tag_name;
                    if (tag->close_tag) {
                        link.text_posn.second = Value(curr_line.char_data[i].string_index);
                        link.rects.back().lr.x = i ? curr_line.char_data[i - 1].extent : GG::X0;
                        m_links.push_back(link);
                        link = Link();
                    } else {
                        link.data = tag->params[0];
                        link.text_posn.first = Value(curr_line.char_data[i].string_index);
                        for (unsigned int k = 0; k < curr_line.char_data[i].tags.size(); ++k) {
                            link.text_posn.first -= Value(curr_line.char_data[i].tags[k]->StringSize());
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
        std::string text = Text_();
        std::string::size_type pos = text.find("<u>", m_links[m_old_rollover_link].text_posn.first);
        text.erase(pos, 3);
        pos = text.find("</u>", m_links[m_old_rollover_link].text_posn.first);
        text.erase(pos, 4);
        SetLinkedText(text);
    }
    m_old_rollover_link = -1;
}
