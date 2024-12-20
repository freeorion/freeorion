#ifndef _LinkText_h_
#define _LinkText_h_

#include <GG/TextControl.h>

#include <boost/signals2/signal.hpp>


class TextLinker {
public:
    enum class DecoratorType : uint8_t {
        Default,
        ColorByOwner,
        ColorByEmpire,
        PathType,
        ValueRef
    };

    TextLinker();

    /// Sets the link decorator for a link type.
    /// \param link_type The link type (tag) to be decorated. Eg. "planet"
    /// \param decorator The decorator to use. Assumes ownership.
    void SetDecorator(std::string_view link_type, DecoratorType dt);

    ///< link clicked signals: first string is the link type, second string is the specific item clicked
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkDoubleClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkRightClickedSignal;

    static constexpr std::string_view ENCYCLOPEDIA_TAG = "encyclopedia";
    static constexpr std::string_view GRAPH_TAG = "graph";
    static constexpr std::string_view URL_TAG = "url";
    /** Tag for clickable link to open users file manager at a specified directory */
    static constexpr std::string_view BROWSE_PATH_TAG = "browsepath";

protected:
    void Render_();
    void LClick_(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys);
    void RClick_(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys);
    void LDoubleClick_(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys);
    void MouseHere_(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys);
    void MouseLeave_();

    virtual const GG::Font::LineVec&         GetLineData() const noexcept = 0;
    virtual const std::shared_ptr<GG::Font>& GetFont() const noexcept = 0;

    virtual GG::Pt              TextUpperLeft() const = 0;
    virtual GG::Pt              TextLowerRight() const = 0;
    virtual void                SetLinkedText(std::string str) = 0;
    virtual const std::string&  RawText() const noexcept = 0;    ///< returns text being displayed before any link formatting is added

    void FindLinks();                ///< finds the links in the text, with which to populate m_links.
    void LocateLinks();              ///< calculates the physical locations of the links in m_links
    void MarkLinks();                ///< wraps text for each link in text formatting tags so that the links appear visually distinct from other text
    int  GetLinkUnderPt(GG::Pt pt);  ///< returns the index of the link under screen coordinate \a pt, or -1 if none

private:
    struct Link {
        std::string           type;           ///< contents of type field of link tag (eg "planet" in <planet 3>)
        std::string           data;           ///< contents of data field of link tag (eg "3" in <planet 3>)
        std::vector<GG::Rect> rects;          ///< the rectangles in which this link falls, in window coordinates (some links may span more than one line)
        std::pair<int, int>   text_posn;      ///< the index of the first (.first) and last + 1 (.second) characters in the raw link text
        std::pair<int, int>   real_text_posn; ///< the index of the first and last + 1 characters in the current (potentially decorated) content string
    };

    std::string LinkDefaultFormatTag(const Link& link, const std::string& content) const;
    std::string LinkRolloverFormatTag(const Link& link, const std::string& content) const;

    using LinkTypeDecoratorType = std::pair<std::string_view, DecoratorType>;

    std::vector<Link>                  m_links;
    std::vector<LinkTypeDecoratorType> m_decorators;
    int                                m_rollover_link = -1;
};

/** Allows text that the user sees to emit signals when clicked, and indicates
  * to the user visually which text represents a link.  There is one type of
  * signal for each type of ZoomTo*() method in ClientUI.  This allows any text
  * that refers to game elements to be tagged as such and clicked by the user,
  * with the appropriate ClientUI response function called.
  * The followig tags are currently supported:
  * \verbatim
  * <planet ID>
  * <system ID>
  * <fleet ID>
  * <ship ID>
  * <building ID>
  * <empire ID>
  * <tech [string]>
  * <buildingtype [string]>
  * <special [string]>
  * <shiphull [string]>
  * <shippart [string]>
  * <species [string]>
  * <encyclopedia [string]>\endverbatim
  * The ID parameters refer to the UniverseObjects that should be zoomed to for
  * each link.  Encyclopedia entries and content items are referred to by strings.
  * <br><br>Note that for link tags to be correctly handled, they must not
  * overlap each other at all, even though overlap with regular GG::Font tags
  * is fine. */
class LinkText : public GG::TextControl, public TextLinker {
public:
    LinkText(GG::X x, GG::Y y, GG::X w, std::string str, std::shared_ptr<GG::Font> font,
             GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE, GG::Clr color = GG::CLR_BLACK);

    /** ctor that does not require window size.
        Window size is determined from the string and font; the window will be large enough to fit the text as rendered,
        and no larger.  \see DynamicText::DynamicText() */
    LinkText(GG::X x, GG::Y y, std::string str, std::shared_ptr<GG::Font> font, GG::Clr color = GG::CLR_BLACK);

    GG::Pt TextUpperLeft() const override;
    GG::Pt TextLowerRight() const override;

    const GG::Font::LineVec& GetLineData() const noexcept override { return GG::TextControl::GetLineData(); }
    const std::shared_ptr<GG::Font>& GetFont() const noexcept override { return GG::TextControl::GetFont(); }

    /** Returns text displayed before link formatting is added. */
    const std::string& RawText() const noexcept override { return m_raw_text; }

    void Render() override;
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseHere(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    /** sets the text to \a str; may resize the window.  If the window was
        constructed to fit the size of the text (i.e. if the second ctor type
        was used), calls to this function cause the window to be resized to
        whatever space the newly rendered text occupies. */
    void SetText(std::string str) override;

private:
    void SetLinkedText(std::string str) override;

    std::string     m_raw_text;
};


/// Helper for generating a link string with content from a stringtable entry
std::string LinkTaggedText(std::string_view tag, std::string_view stringtable_entry);

/// Helper for generating a link string
std::string LinkTaggedIDText(std::string_view tag, int id, std::string_view text);

/// Helper for generating a link string with preset display text (not to be looked up in stringtable)
std::string LinkTaggedPresetText(std::string_view tag, std::string_view stringtable_entry,
                                 std::string_view display_text);

/**
 * Helper for generating a link string for a raw string with unknown tag
 * by searching the various content managers. Returns unlinkified raw
 * string if no match can be found.
 */
std::string LinkStringIfPossible(std::string_view raw, std::string_view user_string);

/**
 * Helper method for generating comma seperated lists of pedia links
 * for the given vector of strings. Every single string will be fed
 * into the above helper method to make a link if possible.
 */
std::string LinkList(const std::vector<std::string>& strings);
std::string LinkList(const std::vector<std::string_view>& strings);
std::string LinkList(const std::set<std::string>& strings);

/// Free function to register link tags that TextLinker knows of.  This allows GG::Font to remove
/// them so that they will not be rendered.  Must be called at least once before text with embedded
/// XML tags is handled by GG::Font
void RegisterLinkTags();

/// Helper for resolving <value> to the valueref value
std::string ValueRefLinkText(std::string text, const bool add_explanation);

#endif
