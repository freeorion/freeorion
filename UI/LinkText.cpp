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

    // format tag to use when a link is not moused over, to indicate it is a link as distinct from normal text
    static const std::string LINK_DEFAULT_FORMAT_TAG = "<rgba 80 255 128 255>";
    static const std::string LINK_DEFAULT_FORMAT_CLOSE = "</rgba>";
    // format tag to use when a link is moused over, to respond to the user and indicate that the link is clickable
    static const std::string LINK_ROLLOVER_FORMAT_TAG = "<rgba 192 80 255 255>";
    static const std::string LINK_ROLLOVER_FORMAT_CLOSE = "</rgba>";

    static bool link_tags_registered = false;
    void RegisterLinkTags() {
        if (link_tags_registered)
            return;
        link_tags_registered = true;

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
        GG::Font::RegisterKnownTag(VarText::SPECIES_TAG);

        GG::Font::RegisterKnownTag(ENCYCLOPEDIA_TAG);
    }
}

///////////////////////////////////////
// LinkText
///////////////////////////////////////
LinkText::LinkText(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font,
                   GG::Flags<GG::TextFormat> format/* = GG::FORMAT_NONE*/, GG::Clr color/* = GG::CLR_BLACK*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) : 
    GG::TextControl(x, y, w, GG::Y1, str, font, color, format, flags),
    TextLinker(),
    m_raw_text(str)
{
    Resize(TextLowerRight() - TextUpperLeft());
    FindLinks();
    MarkLinks();
}

LinkText::LinkText(GG::X x, GG::Y y, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   GG::Clr color/* = GG::CLR_BLACK*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) :
    GG::TextControl(x, y, str, font, color, GG::FORMAT_NONE, flags),
    TextLinker(),
    m_raw_text(str)
{
    FindLinks();
    MarkLinks();
}

void LinkText::Render()
{
    GG::TextControl::Render();
    TextLinker::Render_();
}

void LinkText::SetText(const std::string& str)
{
    m_raw_text = str;
    GG::TextControl::SetText(m_raw_text);   // so that line data is updated for use in FindLinks
    FindLinks();
    MarkLinks();
}

void LinkText::SetLinkedText(const std::string& str)
{
    GG::TextControl::SetText(str);
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

const std::string& LinkText::RawText() const
{
    return m_raw_text;
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
    m_rollover_link(-1)
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

void TextLinker::LClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    using boost::lexical_cast;
    int sel_link = GetLinkUnderPt(pt);
    if (sel_link == -1)
        return;
    if (sel_link < 0 || sel_link >= static_cast<int>(m_links.size())) {
        Logger().errorStream() << "TextLinker::LClick_ found out of bounds sel_link!";
        return;
    }

    const std::string& LINK_TYPE = m_links[sel_link].type;
    const std::string& DATA = m_links[sel_link].data;

    try {
        if (LINK_TYPE == VarText::PLANET_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToPlanet(lexical_cast<int>(DATA));
        } else if (LINK_TYPE == VarText::SYSTEM_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToSystem(lexical_cast<int>(DATA));
        } else if (LINK_TYPE == VarText::FLEET_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToFleet(lexical_cast<int>(DATA));
        } else if (LINK_TYPE == VarText::SHIP_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToShip(lexical_cast<int>(DATA));
        } else if (LINK_TYPE == VarText::BUILDING_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToBuilding(lexical_cast<int>(DATA));

        } else if (LINK_TYPE == VarText::TECH_TAG) {
            ClientUI::GetClientUI()->ZoomToTech(DATA);
        } else if (LINK_TYPE == VarText::BUILDING_TYPE_TAG) {
            ClientUI::GetClientUI()->ZoomToBuildingType(DATA);
        } else if (LINK_TYPE == VarText::SPECIAL_TAG) {
            ClientUI::GetClientUI()->ZoomToSpecial(DATA);
        } else if (LINK_TYPE == VarText::SHIP_HULL_TAG) {
            ClientUI::GetClientUI()->ZoomToShipHull(DATA);
        } else if (LINK_TYPE == VarText::SHIP_PART_TAG) {
            ClientUI::GetClientUI()->ZoomToShipPart(DATA);
        } else if (LINK_TYPE == VarText::SPECIES_TAG) {
            ClientUI::GetClientUI()->ZoomToSpecies(DATA);

        } else if (LINK_TYPE == ENCYCLOPEDIA_TAG) {
            ClientUI::GetClientUI()->ZoomToEncyclopediaEntry(DATA);
        }
    } catch (const boost::bad_lexical_cast&) {
        Logger().errorStream() << "TextLiner::LClick_: caught lexical cast exception for link type: " << LINK_TYPE << " and data: " << DATA;
    }
}

void TextLinker::MouseHere_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    int rollover_link = GetLinkUnderPt(pt);
    if (rollover_link != m_rollover_link) {
        m_rollover_link = rollover_link;
        MarkLinks();
    }
}

void TextLinker::MouseLeave_()
{
    m_rollover_link = -1;
    MarkLinks();
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
                    tag->tag_name == VarText::SPECIES_TAG ||
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

void TextLinker::MarkLinks()
{
    const std::string& raw_text = RawText();

    if (m_links.empty()) {
        SetLinkedText(raw_text);
        return;
    }

    //std::cout << "TextLinker::MarkLinks:" << std::endl;
    //std::cout << "raw text:" << raw_text << std::endl;

    int copy_start_index = 0;
    std::string::const_iterator raw_text_start_it = raw_text.begin();

    std::string marked_text;

    // copy text from current copy_start_index up to just before start of next link
    for (int i = 0; i < static_cast<int>(m_links.size()); ++i) {
        const Link& link = m_links[i];
        int link_start_index = link.text_posn.first;
        int link_end_index = link.text_posn.second;

        // copy raw text up to start of first link
        std::copy(raw_text_start_it + copy_start_index, raw_text_start_it + link_start_index, std::back_inserter(marked_text));

        // add link markup open tag
        if (i == m_rollover_link)
            marked_text += LINK_ROLLOVER_FORMAT_TAG;
        else
            marked_text += LINK_DEFAULT_FORMAT_TAG;

        // copy link text itself
        std::copy(raw_text_start_it + link_start_index, raw_text_start_it + link_end_index, std::back_inserter(marked_text));

        // add link markup close tag
        if (i == m_rollover_link)
            marked_text += LINK_ROLLOVER_FORMAT_CLOSE;
         else
            marked_text += LINK_DEFAULT_FORMAT_CLOSE;

        // update copy point for following text
        copy_start_index = link_end_index;
    }

    // copy remaining text after last link
    std::copy(raw_text_start_it + copy_start_index, raw_text.end(), std::back_inserter(marked_text));

    // set underlying UI control text
    SetLinkedText(marked_text);
    //std::cout << "marktext:" << marked_text << std::endl << std::endl;
}
