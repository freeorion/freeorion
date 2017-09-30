#include "LinkText.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../Empire/Empire.h"
#include "../universe/UniverseObject.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/VarText.h"
#include "../util/i18n.h"
#include "../util/Directories.h"
#include "../universe/Enums.h"

#include <GG/DrawUtil.h>
#include <GG/WndEvent.h>
#include <GG/GUI.h>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string.hpp>

// TextLinker static(s)
const std::string TextLinker::ENCYCLOPEDIA_TAG("encyclopedia");
const std::string TextLinker::GRAPH_TAG("graph");
const std::string TextLinker::URL_TAG("url");
const std::string TextLinker::BROWSE_PATH_TAG("browsepath");

namespace {
    static const bool RENDER_DEBUGGING_LINK_RECTS = false;

    // closing format tag
    static const std::string LINK_FORMAT_CLOSE = "</rgba>";

    std::string AllParamsAsString(const std::vector<GG::Font::Substring>& params) {
        std::string retval;
        for (const auto& param : params) {
            if (!retval.empty())
                retval.append(" ");
            retval.append(param);
        }
        return retval;
    }

    std::string ResolveNestedPathTypes(const std::string& text) {
        if (text.empty())
            return text;
        std::string new_text = text;
        for (const auto& path_type : PathTypeStrings()) {
            std::string path_string = PathToString(GetPath(path_type));
            boost::replace_all(new_text, path_type, path_string);
        }
        return new_text;
    }

    namespace xpr = boost::xpressive;
    const xpr::sregex REGEX_NON_BRACKET = *~(xpr::set= '<', '>');
    const std::string BROWSEPATH_TAG_OPEN_PRE("<" + TextLinker::BROWSE_PATH_TAG);
    const std::string BROWSEPATH_TAG_CLOSE("</" + TextLinker::BROWSE_PATH_TAG + ">");
    const xpr::sregex BROWSEPATH_SEARCH = BROWSEPATH_TAG_OPEN_PRE >> xpr::_s >> (xpr::s1 = REGEX_NON_BRACKET) >> ">" >>
                                          (xpr::s2 = REGEX_NON_BRACKET) >> BROWSEPATH_TAG_CLOSE;

    /** Parses TextLinker::BROWSE_PATH_TAG%s within @p text, replacing string representation of PathType%s with
     *  current path for that PathType.  If link label is empty, inserts resolved link argument as label */
    std::string BrowsePathLinkText(const std::string& text) {
        if (!boost::contains(text, BROWSEPATH_TAG_CLOSE))
            return text;

        std::string retval(text);
        auto text_it = retval.begin();
        xpr::smatch match;
        auto invalid_path_str = PathTypeToString(PATH_INVALID);

        while (true) {
            if (!xpr::regex_search(text_it, retval.end(), match, BROWSEPATH_SEARCH, xpr::regex_constants::match_default))
                break;

            auto link_arg = ResolveNestedPathTypes(match[1]);
            std::string link_label(match[2]);
            if (link_label.empty())
                link_label = link_arg;
            else
                link_label = ResolveNestedPathTypes(link_label);

            // Only support argument containing at least one valid PathType
            if (link_arg == match[1].str() || boost::contains(link_arg, invalid_path_str)) {
                link_arg = invalid_path_str;
                link_label = UserString("ERROR") + ": " + link_label;
            }

            auto resolved_link = BROWSEPATH_TAG_OPEN_PRE + " " + link_arg + ">" + link_label + BROWSEPATH_TAG_CLOSE;

            retval.replace(text_it + match.position(), text_it + match.position() + match.length(), resolved_link);

            text_it = retval.end() - match.suffix().length();
        }

        return retval;
    }
}

///////////////////////////////////////
// LinkText
///////////////////////////////////////
LinkText::LinkText(GG::X x, GG::Y y, GG::X w, const std::string& str, const std::shared_ptr<GG::Font>& font,
                   GG::Flags<GG::TextFormat> format/* = GG::FORMAT_NONE*/, GG::Clr color/* = GG::CLR_BLACK*/) :
    GG::TextControl(x, y, w, GG::Y1, str, font, color, format, GG::INTERACTIVE),
    TextLinker(),
    m_raw_text(str)
{
    Resize(TextLowerRight() - TextUpperLeft());
    FindLinks();
    MarkLinks();
}

LinkText::LinkText(GG::X x, GG::Y y, const std::string& str, const std::shared_ptr<GG::Font>& font,
                   GG::Clr color/* = GG::CLR_BLACK*/) :
    GG::TextControl(x, y, GG::X1, GG::Y1, str, font, color, GG::FORMAT_NOWRAP, GG::INTERACTIVE),
    TextLinker(),
    m_raw_text(str)
{
    FindLinks();
    MarkLinks();
}

void LinkText::Render() {
    GG::TextControl::Render();
    TextLinker::Render_();
}

void LinkText::SetText(const std::string& str) {
    m_raw_text = str;
    FindLinks();
    MarkLinks();
}

void LinkText::SetLinkedText(const std::string& str)
{ GG::TextControl::SetText(str); }

GG::Pt LinkText::TextUpperLeft() const
{ return GG::TextControl::TextUpperLeft(); }

GG::Pt LinkText::TextLowerRight() const
{ return GG::TextControl::TextLowerRight(); }

const std::vector<GG::Font::LineData>& LinkText::GetLineData() const
{ return GG::TextControl::GetLineData(); }

const std::shared_ptr<GG::Font>& LinkText::GetFont() const
{ return GG::TextControl::GetFont(); }

const std::string& LinkText::RawText() const
{ return m_raw_text; }

void LinkText::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ TextLinker::LClick_(pt, mod_keys); }

void LinkText::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {

    auto rclick_action = [this, pt, mod_keys]() { TextLinker::RClick_(pt, mod_keys); };
    auto copy_action = [this]() { GG::GUI::GetGUI()->CopyWndText(this); };

    // create popup menu
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    if (GetLinkUnderPt(pt) != -1) {
        popup->AddMenuItem(GG::MenuItem(UserString("OPEN"),     false, false, rclick_action));
        popup->AddMenuItem(GG::MenuItem(true)); // separator
    }
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_COPY"),  false, false, copy_action));

    popup->Run();
}

void LinkText::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ TextLinker::MouseHere_(pt, mod_keys); }

void LinkText::MouseLeave()
{ TextLinker::MouseLeave_(); }

void LinkText::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    bool resized = Size() != (lr-ul);
    GG::TextControl::SizeMove(ul, lr);
    if (resized)
        LocateLinks();
}

///////////////////////////////////////
// LinkDecorator
///////////////////////////////////////

std::string LinkDecorator::Decorate(const std::string& target, const std::string& content) const{
    return GG::RgbaTag(ClientUI::DefaultLinkColor()) + content + LINK_FORMAT_CLOSE;
}

std::string LinkDecorator::DecorateRollover(const std::string& target, const std::string& content) const{
    return GG::RgbaTag(ClientUI::RolloverLinkColor()) + content + LINK_FORMAT_CLOSE;
}

int LinkDecorator::CastStringToInt(const std::string& str) {
    std::stringstream ss;
    ss << str;
    int retval = -1;
    ss >> retval;

    if (ss.eof())
        return retval;

    return -1;
}

std::string ColorByOwner::Decorate(const std::string& object_id_str, const std::string& content) const {
    GG::Clr color = ClientUI::DefaultLinkColor();
    const Empire* empire = nullptr;
    // get object indicated by object_id, and then get object's owner, if any
    int object_id = CastStringToInt(object_id_str);
    auto object = Objects().Object(object_id);
    if (object && !object->Unowned())
        empire = GetEmpire(object->Owner());
    if (empire)
        color = empire->Color();
    return GG::RgbaTag(color) + content + "</rgba>";
}

std::string PathTypeDecorator::Decorate(const std::string& path_type, const std::string& content) const {
    return LinkDecorator::Decorate(path_type, BrowsePathLinkText(content));
}

std::string PathTypeDecorator::DecorateRollover(const std::string& path_type, const std::string& content) const {
    return LinkDecorator::DecorateRollover(path_type, BrowsePathLinkText(content));
}


///////////////////////////////////////
// TextLinker::Link
///////////////////////////////////////
struct TextLinker::Link {
    std::string             type;           ///< contents of type field of link tag (eg "planet" in <planet 3>)
    std::string             data;           ///< contents of data field of link tag (eg "3" in <planet 3>)
    std::vector<GG::Rect>   rects;          ///< the rectangles in which this link falls, in window coordinates (some links may span more than one line)
    std::pair<int, int>     text_posn;      ///< the index of the first (.first) and last + 1 (.second) characters in the raw link text
    std::pair<int, int>     real_text_posn; ///< the index of the first and last + 1 characters in the current (potentially decorated) content string
};


///////////////////////////////////////
// TextLinker
///////////////////////////////////////
TextLinker::TextLinker() :
    m_links(),
    m_rollover_link(-1)
{
    RegisterLinkTags();
}

TextLinker::~TextLinker()
{}

void TextLinker::SetDecorator(const std::string& link_type, LinkDecorator* decorator) {
    m_decorators[link_type] = std::shared_ptr<LinkDecorator>(decorator);
    MarkLinks();
}

std::string TextLinker::LinkDefaultFormatTag(const Link& link, const std::string& content) const {
    const LinkDecorator* decorator = &DEFAULT_DECORATOR;

    auto it = m_decorators.find(link.type);
    if (it != m_decorators.end()){
        decorator = it->second.get();
    }

    return decorator->Decorate(link.data, content);
}

std::string TextLinker::LinkRolloverFormatTag(const Link& link, const std::string& content) const {
    const LinkDecorator* decorator = &DEFAULT_DECORATOR;

    auto it = m_decorators.find(link.type);
    if (it != m_decorators.end())
        decorator = it->second.get();

    return decorator->DecorateRollover(link.data, content);
}

void TextLinker::Render_() {
    if (!RENDER_DEBUGGING_LINK_RECTS)
        return;

    // draw yellow box around whole text block
    GG::Rect bounds(TextUpperLeft(), TextLowerRight());
    FlatRectangle(bounds.ul, bounds.lr, GG::CLR_ZERO, GG::CLR_YELLOW, 1);

    // draw red box around individual linkified bits of text within block
    for (const Link& link : m_links) {
        for (const GG::Rect& rect : link.rects) {
            GG::Rect r = TextUpperLeft() + rect;
            FlatRectangle(r.ul, r.lr, GG::CLR_ZERO, GG::CLR_RED, 1);
        }
    }
}

void TextLinker::LClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    int sel_link = GetLinkUnderPt(pt);
    if (sel_link == -1)
        return;
    if (sel_link < 0 || sel_link >= static_cast<int>(m_links.size())) {
        ErrorLogger() << "TextLinker::LClick_ found out of bounds sel_link!";
        return;
    }

    const std::string LINK_TYPE = m_links[sel_link].type;
    const std::string DATA = m_links[sel_link].data;

    LinkClickedSignal(LINK_TYPE, DATA);
}

void TextLinker::RClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    int sel_link = GetLinkUnderPt(pt);
    if (sel_link == -1)
        return;
    if (sel_link < 0 || sel_link >= static_cast<int>(m_links.size())) {
        ErrorLogger() << "TextLinker::RClick_ found out of bounds sel_link!";
        return;
    }

    const std::string& LINK_TYPE = m_links[sel_link].type;
    const std::string& DATA = m_links[sel_link].data;

    LinkRightClickedSignal(LINK_TYPE, DATA);
}

void TextLinker::LDoubleClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    int sel_link = GetLinkUnderPt(pt);
    if (sel_link == -1)
        return;
    if (sel_link < 0 || sel_link >= static_cast<int>(m_links.size())) {
        ErrorLogger() << "TextLinker::DoubleClick_ found out of bounds sel_link!";
        return;
    }

    const std::string& LINK_TYPE = m_links[sel_link].type;
    const std::string& DATA = m_links[sel_link].data;

    LinkDoubleClickedSignal(LINK_TYPE, DATA);
}

void TextLinker::MouseHere_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    int rollover_link = GetLinkUnderPt(pt);
    if (rollover_link != m_rollover_link) {
        m_rollover_link = rollover_link;
        MarkLinks();
    }
}

void TextLinker::MouseLeave_() {
    m_rollover_link = -1;
    MarkLinks();
}

void TextLinker::FindLinks() {
    m_links.clear();

    GG::Y y_posn(0); // y-coordinate of the top of the current line
    Link link;

    // control text needs to be updated so that the line data is calculated from
    // the raw text and not the marked text set in MarkText().
    SetLinkedText(RawText());

    for (const auto& curr_line : GetLineData()) {
        for (const auto& curr_char : curr_line.char_data) {
            for (const auto& tag : curr_char.tags) {
                if (tag->tag_name == VarText::PLANET_ID_TAG ||
                    tag->tag_name == VarText::SYSTEM_ID_TAG ||
                    tag->tag_name == VarText::SHIP_ID_TAG ||
                    tag->tag_name == VarText::FLEET_ID_TAG ||
                    tag->tag_name == VarText::BUILDING_ID_TAG ||
                    tag->tag_name == VarText::FIELD_ID_TAG ||
                    tag->tag_name == VarText::COMBAT_ID_TAG ||
                    tag->tag_name == VarText::EMPIRE_ID_TAG ||
                    tag->tag_name == VarText::DESIGN_ID_TAG ||
                    tag->tag_name == VarText::PREDEFINED_DESIGN_TAG ||
                    tag->tag_name == VarText::TECH_TAG ||
                    tag->tag_name == VarText::BUILDING_TYPE_TAG ||
                    tag->tag_name == VarText::SPECIAL_TAG ||
                    tag->tag_name == VarText::SHIP_HULL_TAG ||
                    tag->tag_name == VarText::SHIP_PART_TAG ||
                    tag->tag_name == VarText::SPECIES_TAG ||
                    tag->tag_name == VarText::FIELD_TYPE_TAG ||
                    tag->tag_name == VarText::METER_TYPE_TAG ||
                    tag->tag_name == TextLinker::ENCYCLOPEDIA_TAG ||
                    tag->tag_name == TextLinker::GRAPH_TAG ||
                    tag->tag_name == TextLinker::URL_TAG ||
                    tag->tag_name == TextLinker::BROWSE_PATH_TAG)
                {
                    link.type = tag->tag_name;
                    if (tag->close_tag) {
                        link.text_posn.second = Value(curr_char.string_index);
                        m_links.push_back(link);
                        link = Link();
                    } else {
                        if (!tag->params.empty()) {
                            if (tag->tag_name == TextLinker::BROWSE_PATH_TAG) {
                                auto all_param_str(AllParamsAsString(tag->params));
                                link.data = ResolveNestedPathTypes(all_param_str);
                                // BROWSE_PATH_TAG requires a PathType within param
                                if (link.data == all_param_str) {
                                    ErrorLogger() << "Invalid param \"" << link.data << "\" for tag "
                                                  << TextLinker::BROWSE_PATH_TAG;
                                    link.data = PathTypeToString(PATH_INVALID);
                                }
                            } else {
                                link.data = tag->params[0];
                            }
                        } else {
                            link.data.clear();
                        }
                        link.text_posn.first = Value(curr_char.string_index);
                        for (auto& itag : curr_char.tags) {
                            link.text_posn.first -= Value(itag->StringSize());
                        }
                    }
                    // Before decoration, the real positions are the same as the raw ones
                    link.real_text_posn = link.text_posn;
                }
            }
        }
    }

    LocateLinks();
}

void TextLinker::LocateLinks() {
    if (m_links.empty())
        return;

    GG::Y y_posn(0); // y-coordinate of the top of the current line
    const auto& font = GetFont();

    // We assume that links are stored in m_links in the order they appear in the text.
    // We shall iterate through the text, updating the rectangles of a link whenever we know we are inside it
    auto current_link = m_links.begin();
    bool inside_link = false;

    for (const auto& curr_line : GetLineData()) {
        // if the last line ended without the current tag ending
        if (inside_link)
            current_link->rects.push_back(GG::Rect(GG::X0, y_posn, GG::X0,
                                                   y_posn + font->Height()));

        for (unsigned int i = 0; i < curr_line.char_data.size(); ++i) {
            // The link text_posn is at the beginning of the tag, whereas
            // char_data jumps over tags. That is why we cannot test for precise equality
            if (!inside_link && curr_line.char_data[i].string_index >= current_link->real_text_posn.first &&
                curr_line.char_data[i].string_index < current_link->real_text_posn.second)
            {
                inside_link = true;
                // Clear out the old rectangles
                current_link->rects.clear();
                current_link->rects.push_back(GG::Rect(i ? curr_line.char_data[i - 1].extent : GG::X0,
                                              y_posn, GG::X0, y_posn + font->Height()));
            } else if (inside_link && curr_line.char_data[i].string_index >= current_link->real_text_posn.second) {
                inside_link = false;
                current_link->rects.back().lr.x = i ? curr_line.char_data[i - 1].extent : GG::X0;
                ++current_link;
                if (current_link == m_links.end())
                    return;
            }
        }

        // if a line is ending without the current tag ending
        if (inside_link)
            current_link->rects.back().lr.x = curr_line.char_data.empty() ? GG::X0 : curr_line.char_data.back().extent;

        y_posn += font->Lineskip();
    }
}

int TextLinker::GetLinkUnderPt(const GG::Pt& pt) {
    std::vector<Link> links;
    try {
        links = m_links;
    } catch (...) {
        ErrorLogger() << "exception caught copying links in GetLinkUnderPt";
        return -1;
    }

    GG::Pt tex_ul = TextUpperLeft();

    for (unsigned int i = 0; i < links.size(); ++i) {
        const Link& link = links[i];

        for (const GG::Rect& link_rect : link.rects) {
            GG::Rect r = tex_ul + link_rect;
            if (r.Contains(pt))
                return i;
        }
    }
    return -1;  // no link found
}

void TextLinker::MarkLinks() {
    const std::string& raw_text = RawText();

    if (m_links.empty()) {
        SetLinkedText(raw_text);
        return;
    }

    //std::cout << "TextLinker::MarkLinks:" << std::endl;
    //std::cout << "raw text:" << raw_text << std::endl;

    int copy_start_index = 0;
    auto raw_text_start_it = raw_text.begin();

    std::string marked_text;

    // copy text from current copy_start_index up to just before start of next link
    for (int i = 0; i < static_cast<int>(m_links.size()); ++i) {
        Link& link = m_links[i];
        int link_start_index = link.text_posn.first;
        int link_end_index = link.text_posn.second;

        // copy raw text up to start of first link
        std::copy(raw_text_start_it + copy_start_index, raw_text_start_it + link_start_index, std::back_inserter(marked_text));

        std::string content = std::string(raw_text_start_it + link_start_index, raw_text_start_it + link_end_index);

        int length_before = marked_text.size();
        // add link markup open tag
        if (i == m_rollover_link)
            marked_text += LinkRolloverFormatTag(link, content);
        else
            marked_text += LinkDefaultFormatTag(link, content);

        // update copy point for following text
        copy_start_index = link_end_index;
        // update m_links to know the real positions of the links in the decorated text
        // this makes you able to call LocateLinks without resetting the text.
        link.real_text_posn.first = length_before;
        link.real_text_posn.second = marked_text.size();
    }

    // copy remaining text after last link
    std::copy(raw_text_start_it + copy_start_index, raw_text.end(), std::back_inserter(marked_text));

    // set underlying UI control text
    SetLinkedText(marked_text);
    //std::cout << "marktext:" << marked_text << std::endl << std::endl;
}

const LinkDecorator TextLinker::DEFAULT_DECORATOR;

std::string LinkTaggedText(const std::string& tag, const std::string& stringtable_entry)
{ return "<" + tag + " " + stringtable_entry + ">" + UserString(stringtable_entry) + "</" + tag + ">"; }

std::string LinkTaggedIDText(const std::string& tag, int id, const std::string& text)
{ return "<" + tag + " " + std::to_string(id) + ">" + text + "</" + tag + ">"; }

std::string LinkTaggedPresetText(const std::string& tag, const std::string& stringtable_entry, const std::string& display_text)
{ return "<" + tag + " " + stringtable_entry + ">" + display_text + "</" + tag + ">"; }

namespace {
    static bool link_tags_registered = false;
}

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
    GG::Font::RegisterKnownTag(VarText::FIELD_ID_TAG);

    GG::Font::RegisterKnownTag(VarText::COMBAT_ID_TAG);

    GG::Font::RegisterKnownTag(VarText::EMPIRE_ID_TAG);
    GG::Font::RegisterKnownTag(VarText::DESIGN_ID_TAG);
    GG::Font::RegisterKnownTag(VarText::PREDEFINED_DESIGN_TAG);

    GG::Font::RegisterKnownTag(VarText::TECH_TAG);
    GG::Font::RegisterKnownTag(VarText::BUILDING_TYPE_TAG);
    GG::Font::RegisterKnownTag(VarText::SPECIAL_TAG);
    GG::Font::RegisterKnownTag(VarText::SHIP_HULL_TAG);
    GG::Font::RegisterKnownTag(VarText::SHIP_PART_TAG);
    GG::Font::RegisterKnownTag(VarText::SPECIES_TAG);
    GG::Font::RegisterKnownTag(VarText::FIELD_TYPE_TAG);
    GG::Font::RegisterKnownTag(VarText::METER_TYPE_TAG);

    GG::Font::RegisterKnownTag(TextLinker::ENCYCLOPEDIA_TAG);
    GG::Font::RegisterKnownTag(TextLinker::GRAPH_TAG);
    GG::Font::RegisterKnownTag(TextLinker::URL_TAG);
    GG::Font::RegisterKnownTag(TextLinker::BROWSE_PATH_TAG);
}
