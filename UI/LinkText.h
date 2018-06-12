#ifndef _LinkText_h_
#define _LinkText_h_

#include <GG/TextControl.h>

#include <boost/signals2/signal.hpp>


/// A class that can be subclassed to give types of links decorations in link text.
/// "Decorations" here mean mostly wrapping the text in some styling tag or other,
/// but a decorator is free to manipulate the decorated text in any way.
class LinkDecorator {
public:
    /** \name Structors */ //@{
    LinkDecorator() {};
    virtual ~LinkDecorator() {};
    //@}

    /// Gets called for each link of the type this decorator is assigned to.
    /// The return value is shown to the user as the link.
    /// The default implementation wraps content in an rgba tag that colors it by ClientUI::DefaultLinkColor
    /// \param target The target of the link. Usually an id of something or the name of an encyclopedia entry.
    /// \param content The text the link tag was wrapped around.
    /// \returns The text that should be shown to the user in the place of content
    virtual std::string Decorate(const std::string& target, const std::string& content) const;

    /// Gets called when the mouse hovers over a link of the type this decorator is assigned to.
    /// The return value is shown to the user as the link.
    /// The default implementation wraps content in an rgba tag that colors it by ClientUI::RolloverLinkColor
    /// \param target The target of the link. Usually an id of something or the name of an encyclopedia entry.
    /// \param content The text the link tag was wrapped around.
    /// \returns The text that should be shown to the user in the place of content
    virtual std::string DecorateRollover(const std::string& target, const std::string& content) const;

protected:
    /// Try to convert str to int. Returns -1 if \param str conversion to an
    /// int fails.  Helper for interpreting \param str as an ID of an object
    /// in the game universe.
    static int CastStringToInt(const std::string& str);
};

// Should be unique_ptr, but we don't have c++11
typedef std::shared_ptr<LinkDecorator> LinkDecoratorPtr;

class ColorByOwner: public LinkDecorator {
public:
    std::string Decorate(const std::string& object_id_str, const std::string& content) const override;
};

class PathTypeDecorator : public LinkDecorator {
public:
    std::string Decorate(const std::string& path_type, const std::string& content) const override;
    std::string DecorateRollover(const std::string& path_type, const std::string& content) const override;
};

class TextLinker {
public:
    /** \name Structors */ //@{
    TextLinker();
    virtual ~TextLinker();
    //@}

    /// Sets the link decorator for a link type.
    /// \param link_type The link type (tag) to be decorated. Eg. "planet"
    /// \param decorator The decorator to use. Assumes ownership.
    void SetDecorator(const std::string& link_type, LinkDecorator* decorator);

    ///< link clicked signals: first string is the link type, second string is the specific item clicked
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkDoubleClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkRightClickedSignal;

    static const std::string ENCYCLOPEDIA_TAG;
    static const std::string GRAPH_TAG;
    static const std::string URL_TAG;
    /** Tag for clickable link to open users file manager at a specified directory */
    static const std::string BROWSE_PATH_TAG;

protected:
    void Render_();
    void LClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void RClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void LDoubleClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void MouseHere_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void MouseLeave_();

    virtual const std::vector<GG::Font::LineData>&  GetLineData() const = 0;
    virtual const std::shared_ptr<GG::Font>&        GetFont() const = 0;

    virtual GG::Pt              TextUpperLeft() const = 0;
    virtual GG::Pt              TextLowerRight() const = 0;
    virtual void                SetLinkedText(const std::string& str) = 0;
    virtual const std::string&  RawText() const = 0;    ///< returns text being displayed before any link formatting is added

    void FindLinks();                       ///< finds the links in the text, with which to populate m_links.
    void LocateLinks();                     ///< calculates the physical locations of the links in m_links
    void MarkLinks();                       ///< wraps text for each link in text formatting tags so that the links appear visually distinct from other text
    int  GetLinkUnderPt(const GG::Pt& pt);  ///< returns the index of the link under screen coordinate \a pt, or -1 if none
private:
    struct Link;

    std::string LinkDefaultFormatTag(const Link& link, const std::string& content) const;
    std::string LinkRolloverFormatTag(const Link& link, const std::string& content) const;

    std::vector<Link>                       m_links;
    int                                     m_rollover_link;
    std::map<std::string, LinkDecoratorPtr> m_decorators;
    static const LinkDecorator DEFAULT_DECORATOR;
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
    /** \name Structors */ //@{
    LinkText(GG::X x, GG::Y y, GG::X w, const std::string& str, const std::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE, GG::Clr color = GG::CLR_BLACK); ///< ctor taking a font directly

    /** ctor that does not require window size.
        Window size is determined from the string and font; the window will be large enough to fit the text as rendered,
        and no larger.  \see DynamicText::DynamicText() */
    LinkText(GG::X x, GG::Y y, const std::string& str, const std::shared_ptr<GG::Font>& font, GG::Clr color = GG::CLR_BLACK);
    //@}

    /** \name Accessors */ //@{
    GG::Pt TextUpperLeft() const override;
    GG::Pt TextLowerRight() const override;

    const std::vector<GG::Font::LineData>& GetLineData() const override;
    const std::shared_ptr<GG::Font>& GetFont() const override;

    /** Returns text displayed before link formatting is added. */
    const std::string& RawText() const override;
    //@}

    /** \name Mutators */ //@{
    void Render() override;
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    /** sets the text to \a str; may resize the window.  If the window was
        constructed to fit the size of the text (i.e. if the second ctor type
        was used), calls to this function cause the window to be resized to
        whatever space the newly rendered text occupies. */
    void SetText(const std::string& str) override;
    //@}

private:
    void SetLinkedText(const std::string& str) override;

    std::string     m_raw_text;
};


/// Helper for generating a link string with content from a stringtable entry
std::string LinkTaggedText(const std::string& tag, const std::string& stringtable_entry);

/// Helper for generating a link string
std::string LinkTaggedIDText(const std::string& tag, int id, const std::string& text);

/// Helper for generating a link string with preset display text (not to be looked up in stringtable)
std::string LinkTaggedPresetText(const std::string& tag, const std::string& stringtable_entry, const std::string& display_text);

/// Free function to register link tags that TextLinker knows of.  This allows GG::Font to remove
/// them so that they will not be rendered.  Must be called at least once before text with embedded
/// XML tags is handled by GG::Font
void RegisterLinkTags();

#endif // _LinkText_h_
