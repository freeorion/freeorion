#include "LinkText.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../Empire/Empire.h"
#include "../universe/NamedValueRefManager.h"
#include "../universe/Species.h"
#include "../universe/UniverseObject.h"
#include "../universe/ValueRefs.h"
#include "../util/AppInterface.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/VarText.h"
#include "../util/i18n.h"

#include <GG/WndEvent.h>
#include <GG/GUI.h>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string.hpp>


class Tech;
class Policy;
class BuildingType;
class Special;
class FieldType;
class ShipHull;
class ShipPart;
const Tech*         GetTech(std::string_view name);
const Policy*       GetPolicy(std::string_view name);
const BuildingType* GetBuildingType(std::string_view name);
const Special*      GetSpecial(std::string_view name);
const FieldType*    GetFieldType(std::string_view name);
const ShipHull*     GetShipHull(std::string_view name);
const ShipPart*     GetShipPart(std::string_view name);


namespace {
    constexpr bool RENDER_DEBUGGING_LINK_RECTS = false;
    constexpr std::string_view LINK_FORMAT_CLOSE = "</rgba>"; // closing format tag

    std::string AllParamsAsString(const std::vector<GG::Font::Substring>& params) {
        std::string retval;
        for (const auto& param : params) {
            if (!retval.empty())
                retval.append(" ");
            retval.append(param);
        }
        return retval;
    }

    std::string ResolveNestedPathTypes(std::string text) {
        if (text.empty())
            return text;

        const auto& path_type_strings = PathTypeStrings();
        const auto& path_types = PathTypes();

        constexpr auto pts_sz = std::tuple_size_v<std::decay_t<decltype(path_type_strings)>>;
        constexpr auto pt_sz = std::tuple_size_v<std::decay_t<decltype(path_types)>>;
        static_assert(pts_sz == pt_sz);

        for (std::size_t idx = 0; idx < pts_sz; ++idx)
            boost::replace_all(text, path_type_strings[idx], PathToString(GetPath(path_types[idx])));

        return text;
    }

    namespace xpr = boost::xpressive;
    const xpr::sregex REGEX_NON_BRACKET = *~(xpr::set= '<', '>');
    const std::string BROWSEPATH_TAG_OPEN_PRE{std::string{"<"}.append(TextLinker::BROWSE_PATH_TAG)};
    const std::string BROWSEPATH_TAG_CLOSE{std::string{"</"}.append(TextLinker::BROWSE_PATH_TAG).append(">")};
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
        auto invalid_path_str = PathTypeToString(PathType::PATH_INVALID);

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

/** Parses VarText::FOCS_VALUE_TAG%s within @p text, replacing value ref name%s with
 *  the evaluation result of that value ref.
 *  Given tag content (i.e. the stringtable content for the tag name) gets added as explanation.
 *  If the tag content is empty or @p add_explanation is true,
 *  the value ref description gets added as explanation instead. */
std::string ValueRefLinkText(std::string text, const bool add_explanation) {
    static const std::string FOCS_VALUE_TAG_CLOSE{std::string{"</"}.append(VarText::FOCS_VALUE_TAG).append(">")};
    if (!boost::contains(text, FOCS_VALUE_TAG_CLOSE))
        return text;

    auto text_it = text.begin();
    xpr::smatch match;
    const xpr::sregex FOCS_VALUE_SEARCH =
        (std::string{"<"}.append(VarText::FOCS_VALUE_TAG)) >> xpr::_s >> (xpr::s1 = REGEX_NON_BRACKET) >> ">" >>
        (xpr::s2 = REGEX_NON_BRACKET) >> (std::string{"</"}.append(VarText::FOCS_VALUE_TAG).append(">"));

    while (true) {
        if (!xpr::regex_search(text_it, text.end(), match, FOCS_VALUE_SEARCH, xpr::regex_constants::match_default))
            break;

        std::string value_ref_name{match[1]};
        auto* value_ref = GetValueRefBase(value_ref_name);
        auto value_str{value_ref ? value_ref->EvalAsString() : value_ref_name};
        // Explanation: match[2] contains the localized UserString. Result looks like e.g. " (per planet size: 2.5 * 0.2000)"
        // Using UserStringExists to get rid of lookup errors is kind of redundant. It would be better if UserString substitution had not happened before.
        std::string explanation_str{
            (value_ref && add_explanation)
            ? " (" + ((match[2].length()==0 || !UserStringExists(value_ref_name)) ? "" : match[2] + ": ") + value_ref->Description() + ")"
            : ""};

        auto resolved_tooltip = std::string{"<"}.append(VarText::FOCS_VALUE_TAG).append(" ")
                               .append(value_ref_name).append(">").append(value_str).append(explanation_str)
                               .append("</").append(VarText::FOCS_VALUE_TAG).append(">");

        text.replace(text_it + match.position(), text_it + match.position() + match.length(), resolved_tooltip);

        text_it = text.end() - match.suffix().length();
    }

    return text;
}

///////////////////////////////////////
// LinkText
///////////////////////////////////////
LinkText::LinkText(GG::X x, GG::Y y, GG::X w, std::string str, std::shared_ptr<GG::Font> font,
                   GG::Flags<GG::TextFormat> format, GG::Clr color) :
    GG::TextControl(x, y, w, GG::Y1, str, std::move(font), color, format, GG::INTERACTIVE),
    TextLinker(),
    m_raw_text(std::move(str))
{
    Resize(TextLowerRight() - TextUpperLeft());
    FindLinks();
    MarkLinks();
}

LinkText::LinkText(GG::X x, GG::Y y, std::string str, std::shared_ptr<GG::Font> font, GG::Clr color) :
    GG::TextControl(x, y, GG::X1, GG::Y1, str, std::move(font), color, GG::FORMAT_NOWRAP, GG::INTERACTIVE),
    TextLinker(),
    m_raw_text(std::move(str))
{
    FindLinks();
    MarkLinks();
}

void LinkText::Render() {
    GG::TextControl::Render();
    TextLinker::Render_();
}

void LinkText::SetText(std::string str) {
    m_raw_text = std::move(str);
    FindLinks();
    MarkLinks();
}

void LinkText::SetLinkedText(std::string str)
{ GG::TextControl::SetText(std::move(str)); }

GG::Pt LinkText::TextUpperLeft() const
{ return GG::TextControl::TextUpperLeft(); }

GG::Pt LinkText::TextLowerRight() const
{ return GG::TextControl::TextLowerRight(); }

void LinkText::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ TextLinker::LClick_(pt, mod_keys); }

void LinkText::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
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

void LinkText::MouseHere(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ TextLinker::MouseHere_(pt, mod_keys); }

void LinkText::MouseLeave()
{ TextLinker::MouseLeave_(); }

void LinkText::SizeMove(GG::Pt ul, GG::Pt lr) {
    bool resized = Size() != (lr-ul);
    GG::TextControl::SizeMove(ul, lr);
    if (resized)
        LocateLinks();
}

///////////////////////////////////////
// LinkDecorator
///////////////////////////////////////
namespace {
    std::string LinkDecoratorDefaultDecorate(const std::string& target, const std::string& content)
    { return GG::RgbaTag(ClientUI::DefaultLinkColor()).append(content).append(LINK_FORMAT_CLOSE); }

    std::string LinkDecoratorDefaultDecorateRollover(const std::string& target, const std::string& content)
    { return GG::RgbaTag(ClientUI::RolloverLinkColor()).append(content).append(LINK_FORMAT_CLOSE); }

    int CastStringToInt(const std::string& str) {
        // Try to convert str to int. Returns -1 if str conversion to an int fails.
        std::stringstream ss;
        ss << str;
        int retval = -1;
        ss >> retval;

        if (ss.eof())
            return retval;

        return -1;
    }

    std::string ColorByEmpireDecorate(const std::string& empire_id, const std::string& content) {
        const int id = CastStringToInt(empire_id);
        const Empire* empire = GetEmpire(id);
        const GG::Clr color = empire ? empire->Color() : ClientUI::DefaultLinkColor();
        return GG::RgbaTag(color) + content + "</rgba>";
    }

    std::string ColorByOwnerDecorate(const std::string& object_id_str, const std::string& content) {
        GG::Clr color = ClientUI::DefaultLinkColor();
        // get object indicated by object_id, and then get object's owner, if any
        int object_id = CastStringToInt(object_id_str);
        const auto& context = IApp::GetApp()->GetContext();
        auto object = context.ContextObjects().get(object_id);
        if (object && !object->Unowned())
            if (auto empire = context.GetEmpire(object->Owner()))
                color = empire->Color();
        return GG::RgbaTag(color).append(content).append("</rgba>");
    }

    std::string PathTypeDecorate(const std::string& path_type, const std::string& content)
    { return LinkDecoratorDefaultDecorate(path_type, BrowsePathLinkText(content)); }

    std::string PathTypeDecorateRollover(const std::string& path_type, const std::string& content)
    { return LinkDecoratorDefaultDecorateRollover(path_type, BrowsePathLinkText(content)); }

    std::string ValueRefDecorate(const std::string& value_ref_name, const std::string& content)
    { return GG::RgbaTag(ClientUI::DefaultTooltipColor()).append(::ValueRefLinkText(content, false)).append(LINK_FORMAT_CLOSE); }

    std::string ValueRefDecorateRollover(const std::string& value_ref_name, const std::string& content)
    { return GG::RgbaTag(ClientUI::RolloverTooltipColor()).append(::ValueRefLinkText(content, true)).append(LINK_FORMAT_CLOSE); }

    /// The return value is shown to the user as the link.
    /// The default wraps content in an rgba tag that colors it by ClientUI::DefaultLinkColor
    /// \param target The target of the link. Usually an id of something or the name of an encyclopedia entry.
    /// \param content The text the link tag was wrapped around.
    /// \returns The text that should be shown to the user in the place of content
    std::string Decorate(TextLinker::DecoratorType type, const std::string& target, const std::string& content) {
        switch (type) {
        case TextLinker::DecoratorType::ColorByOwner:
            return ColorByOwnerDecorate(target, content);
            break;
        case TextLinker::DecoratorType::ColorByEmpire:
            return ColorByEmpireDecorate(target, content);
            break;
        case TextLinker::DecoratorType::PathType:
            return PathTypeDecorate(target, content);
            break;
        case TextLinker::DecoratorType::ValueRef:
            return ValueRefDecorate(target, content);
            break;
        default: [[likely]]
            return LinkDecoratorDefaultDecorate(target, content);
        }
    }

    /// Gets called when the mouse hovers over a link of the type this decorator is assigned to.
    /// The return value is shown to the user as the link.
    /// The default wraps content in an rgba tag that colors it by ClientUI::RolloverLinkColor
    /// \param target The target of the link. Usually an id of something or the name of an encyclopedia entry.
    /// \param content The text the link tag was wrapped around.
    /// \returns The text that should be shown to the user in the place of content
    std::string DecorateRollover(TextLinker::DecoratorType type, const std::string& target, const std::string& content) {
        switch (type) {
        case TextLinker::DecoratorType::PathType:
            return PathTypeDecorateRollover(target, content);
            break;
        case TextLinker::DecoratorType::ValueRef:
            return ValueRefDecorateRollover(target, content);
            break;
        default: [[likely]]
            return LinkDecoratorDefaultDecorateRollover(target, content);
        }
    }
}


///////////////////////////////////////
// TextLinker
///////////////////////////////////////
TextLinker::TextLinker()
{ RegisterLinkTags(); }

void TextLinker::SetDecorator(std::string_view link_type, DecoratorType dt) {
    auto it = std::find_if(m_decorators.begin(), m_decorators.end(),
                           [link_type](const auto& deco) { return deco.first == link_type; });
    if (it != m_decorators.end())
        it->second = dt;
    else
        m_decorators.emplace_back(link_type, dt);
    MarkLinks();
}

std::string TextLinker::LinkDefaultFormatTag(const Link& link, const std::string& content) const {
    const auto it = std::find_if(m_decorators.begin(), m_decorators.end(),
                                 [dt{link.type}](const auto& deco) { return deco.first == dt; });
    const auto dt = (it != m_decorators.end() ? it->second : DecoratorType::Default);
    return Decorate(dt, link.data, content);
}

std::string TextLinker::LinkRolloverFormatTag(const Link& link, const std::string& content) const {
    const auto it = std::find_if(m_decorators.begin(), m_decorators.end(),
                                 [dt{link.type}](const auto& deco) { return deco.first == dt; });
    const auto dt = (it != m_decorators.end() ? it->second : DecoratorType::Default);
    return DecorateRollover(dt, link.data, content);
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
            const GG::Rect r = TextUpperLeft() + rect;
            FlatRectangle(r.ul, r.lr, GG::CLR_ZERO, GG::CLR_RED, 1);
        }
    }
}

void TextLinker::LClick_(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    int sel_link = GetLinkUnderPt(pt);
    if (sel_link == -1)
        return;
    if (sel_link < 0 || sel_link >= static_cast<int>(m_links.size())) {
        ErrorLogger() << "TextLinker::LClick_ found out of bounds sel_link!";
        return;
    }

    const auto& LINK = m_links[sel_link];
    LinkClickedSignal(LINK.type, LINK.data);
}

void TextLinker::RClick_(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    const int sel_link = GetLinkUnderPt(pt);
    if (sel_link == -1)
        return;
    if (sel_link < 0 || sel_link >= static_cast<int>(m_links.size())) {
        ErrorLogger() << "TextLinker::RClick_ found out of bounds sel_link!";
        return;
    }

    const auto& LINK = m_links[sel_link];
    LinkClickedSignal(LINK.type, LINK.data);
}

void TextLinker::LDoubleClick_(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    const int sel_link = GetLinkUnderPt(pt);
    if (sel_link == -1)
        return;
    if (sel_link < 0 || sel_link >= static_cast<int>(m_links.size())) {
        ErrorLogger() << "TextLinker::DoubleClick_ found out of bounds sel_link!";
        return;
    }

    const auto& LINK = m_links[sel_link];
    LinkClickedSignal(LINK.type, LINK.data);
}

void TextLinker::MouseHere_(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
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

    Link link;

    // control text needs to be updated so that the line data is calculated from
    // the raw text and not the marked text set in MarkText().
    SetLinkedText(RawText());

    static constexpr auto is_link_tag = [](const auto& tag) -> bool {
        static constexpr std::array<std::string_view, 24> link_tags = {
            VarText::PLANET_ID_TAG,         VarText::SYSTEM_ID_TAG,   VarText::SHIP_ID_TAG,
            VarText::FLEET_ID_TAG,          VarText::BUILDING_ID_TAG, VarText::FIELD_ID_TAG,
            VarText::COMBAT_ID_TAG,         VarText::EMPIRE_ID_TAG,   VarText::DESIGN_ID_TAG,
            VarText::PREDEFINED_DESIGN_TAG, VarText::TECH_TAG,        VarText::POLICY_TAG,
            VarText::BUILDING_TYPE_TAG,     VarText::SPECIAL_TAG,     VarText::SHIP_HULL_TAG,
            VarText::SHIP_PART_TAG,         VarText::SPECIES_TAG,     VarText::FIELD_TYPE_TAG,
            VarText::METER_TYPE_TAG,        VarText::FOCS_VALUE_TAG,  TextLinker::ENCYCLOPEDIA_TAG,
            TextLinker::GRAPH_TAG,          TextLinker::URL_TAG,      TextLinker::BROWSE_PATH_TAG};
        return std::find(link_tags.begin(), link_tags.end(), tag) != link_tags.end();
    };

    for (const auto& curr_line : GetLineData()) {
        for (const auto& curr_char : curr_line.char_data) {
            for (const auto& tag : curr_char.tags) {
                if (!is_link_tag(tag.tag_name))
                    continue;

                link.type = tag.tag_name;
                if (tag.IsCloseTag()) {
                    link.text_posn.second = Value(curr_char.string_index);
                    m_links.emplace_back(std::move(link));
                    link = Link();
                } else {
                    if (!tag.params.empty()) {
                        if (tag.tag_name == TextLinker::BROWSE_PATH_TAG) {
                            auto all_param_str(AllParamsAsString(tag.params));
                            link.data = ResolveNestedPathTypes(all_param_str);
                            // BROWSE_PATH_TAG requires a PathType within param
                            if (link.data == all_param_str) {
                                ErrorLogger() << "Invalid param \"" << link.data
                                              << "\" for tag " << TextLinker::BROWSE_PATH_TAG;
                                link.data = PathTypeToString(PathType::PATH_INVALID);
                            }
                        } else {
                            link.data = tag.params[0];
                        }
                    } else {
                        link.data.clear();
                    }
                    link.text_posn.first = Value(curr_char.string_index);
                    for (auto& itag : curr_char.tags)
                        link.text_posn.first -= Value(itag.StringSize());
                }
                // Before decoration, the real positions are the same as the raw ones
                link.real_text_posn = link.text_posn;
            }
        }
    }

    LocateLinks();
}

void TextLinker::LocateLinks() {
    if (m_links.empty())
        return;

    GG::Y y_posn(GG::Y0); // y-coordinate of the top of the current line
    const auto& font = GetFont();

    // We assume that links are stored in m_links in the order they appear in the text.
    // We shall iterate through the text, updating the rectangles of a link whenever we know we are inside it
    auto current_link = m_links.begin();
    bool inside_link = false;

    for (const auto& curr_line : GetLineData()) {
        // if the last line ended without the current tag ending
        if (inside_link)
            current_link->rects.emplace_back(GG::X0, y_posn, GG::X0, y_posn + font->Height());

        for (unsigned int i = 0; i < curr_line.char_data.size(); ++i) {
            const GG::X x_posn = (i > 0) ? curr_line.char_data.at(static_cast<std::size_t>(i - 1)).extent : GG::X0;
            // The link text_posn is at the beginning of the tag, whereas
            // char_data jumps over tags. That is why we cannot test for precise equality
            const auto cdsi = Value(curr_line.char_data.at(i).string_index);
            const auto [first, lastplusone] = current_link->real_text_posn;

            if (!inside_link &&
                std::cmp_greater_equal(cdsi, first) &&
                std::cmp_less(cdsi, lastplusone))
            {
                inside_link = true;
                // Clear out the old rectangles
                current_link->rects.clear();
                current_link->rects.emplace_back(x_posn, y_posn, GG::X0, y_posn + font->Height());

            } else if (inside_link && std::cmp_greater_equal(cdsi, current_link->real_text_posn.second)) {
                inside_link = false;
                current_link->rects.back().lr.x = x_posn;
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

int TextLinker::GetLinkUnderPt(GG::Pt pt) {
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
    if (m_links.empty()) {
        SetLinkedText(RawText());
        return;
    }

    int copy_start_index = 0;
    const std::string& raw_text = RawText();
    auto raw_text_start_it = raw_text.begin();

    std::string marked_text;
    marked_text.reserve(raw_text.size()); // possibly underestimate

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
    SetLinkedText(std::move(marked_text));
}

namespace {
    auto get_species = [](std::string_view sv) { return GetSpeciesManager().GetSpecies(sv); };
}

std::string LinkStringIfPossible(std::string_view raw, std::string_view user_string) {
    if      (GetBuildingType(raw)) return LinkTaggedPresetText(VarText::BUILDING_TYPE_TAG, raw, user_string);
    else if (get_species(raw))     return LinkTaggedPresetText(VarText::SPECIES_TAG,       raw, user_string);
    else if (GetSpecial(raw))      return LinkTaggedPresetText(VarText::SPECIAL_TAG,       raw, user_string);
    else if (GetPolicy(raw))       return LinkTaggedPresetText(VarText::POLICY_TAG,        raw, user_string);
    else if (GetShipHull(raw))     return LinkTaggedPresetText(VarText::SHIP_HULL_TAG,     raw, user_string);
    else if (GetShipPart(raw))     return LinkTaggedPresetText(VarText::SHIP_PART_TAG,     raw, user_string);
    else if (GetTech(raw))         return LinkTaggedPresetText(VarText::TECH_TAG,          raw, user_string);
    else if (GetFieldType(raw))    return LinkTaggedPresetText(VarText::FIELD_TYPE_TAG,    raw, user_string);
    else return std::string{user_string};
}

std::string LinkList(const std::vector<std::string>& strings) {
    std::string s;
    bool first = true;
    for (std::string string : strings) {
        if (first)
            first = false;
        else
            s.append(",  ");
        s.append(LinkStringIfPossible(string, UserString(string)));
    }
    return s;
}

std::string LinkList(const std::vector<std::string_view>& strings) {
    std::string s;
    bool first = true;
    for (std::string_view string : strings) {
        if (first)
            first = false;
        else
            s.append(",  ");
        s.append(LinkStringIfPossible(string, UserString(string)));
    }
    return s;
}

std::string LinkList(const std::set<std::string>& strings) {
    std::string s;
    bool first = true;
    for (const auto& string : strings) {
        if (first)
            first = false;
        else
            s.append(",  ");
        s.append(LinkStringIfPossible(string, UserString(string)));
    }
    return s;
}

std::string LinkTaggedText(std::string_view tag, std::string_view stringtable_entry)
{ return LinkTaggedPresetText(tag, stringtable_entry, UserString(stringtable_entry)); }

std::string LinkTaggedIDText(std::string_view tag, int id, std::string_view display_text)
{ return LinkTaggedPresetText(tag, std::to_string(id), display_text); }

std::string LinkTaggedPresetText(std::string_view tag, std::string_view stringtable_entry,
                                 std::string_view display_text)
{
    std::string retval;
    retval.reserve(10 + display_text.length() + 2*tag.length() + stringtable_entry.length());
    retval.append("<").append(tag).append(" ").append(stringtable_entry)
          .append(">").append(display_text).append("</").append(tag).append(">");
    return retval;
}

namespace {
    bool link_tags_registered = false;
}

void RegisterLinkTags() {
    if (link_tags_registered)
        return;
    link_tags_registered = true;

    // need to register the tags that link text uses so GG::Font will know how to (not) render them
    GG::Font::RegisterKnownTags({VarText::PLANET_ID_TAG, VarText::SYSTEM_ID_TAG, VarText::SHIP_ID_TAG,
                                 VarText::FLEET_ID_TAG, VarText::BUILDING_ID_TAG, VarText::FIELD_ID_TAG,
                                 VarText::COMBAT_ID_TAG,
                                 VarText::EMPIRE_ID_TAG,
                                 VarText::DESIGN_ID_TAG, VarText::PREDEFINED_DESIGN_TAG,
                                 VarText::TECH_TAG, VarText::POLICY_TAG, VarText::BUILDING_TYPE_TAG,
                                 VarText::SPECIAL_TAG, VarText::SHIP_HULL_TAG, VarText::SHIP_PART_TAG,
                                 VarText::SPECIES_TAG, VarText::FIELD_TYPE_TAG, VarText::METER_TYPE_TAG,
                                 VarText::FOCS_VALUE_TAG,
                                 TextLinker::ENCYCLOPEDIA_TAG, TextLinker::GRAPH_TAG,
                                 TextLinker::URL_TAG, TextLinker::BROWSE_PATH_TAG});
}
