// -*- C++ -*-
#ifndef _LinkText_h_
#define _LinkText_h_

#include <GG/TextControl.h>

class TextLinker {
public:
    /** \name Structors */ //@{
    TextLinker();
    virtual ~TextLinker();
    //@}

    ///< link clicked signals: first string is the link type, second string is the specific item clicked
    mutable boost::signal<void (const std::string&, const std::string&)> LinkClickedSignal;
    mutable boost::signal<void (const std::string&, const std::string&)> LinkDoubleClickedSignal;
    mutable boost::signal<void (const std::string&, const std::string&)> LinkRightClickedSignal;

    static const std::string ENCYCLOPEDIA_TAG;

protected:
    void        Render_();
    void        LClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void        RClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void        LDoubleClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void        MouseHere_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void        MouseLeave_();

    virtual const std::vector<GG::Font::LineData>&  GetLineData() const = 0;
    virtual const boost::shared_ptr<GG::Font>&      GetFont() const = 0;
    virtual GG::Pt                                  TextUpperLeft() const = 0;
    virtual GG::Pt                                  TextLowerRight() const = 0;
    virtual void                                    SetLinkedText(const std::string& str) = 0;
    virtual const std::string&                      RawText() const = 0;      ///< returns text being displayed before any link formatting is added

    void        FindLinks();                        ///< finds the links in the text, with which to populate m_links
    void        MarkLinks();                        ///< wraps text for each link in text formatting tags so that the links appear visually distinct from other text
    int         GetLinkUnderPt(const GG::Pt& pt);   ///< returns the index of the link under screen coordinate \a pt, or -1 if none

private:
    struct Link;
    std::vector<Link>   m_links;
    int                 m_rollover_link;
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
    LinkText(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE, GG::Clr color = GG::CLR_BLACK, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE); ///< ctor taking a font directly

    /** ctor that does not require window size.
        Window size is determined from the string and font; the window will be large enough to fit the text as rendered,
        and no larger.  \see DynamicText::DynamicText() */
    LinkText(GG::X x, GG::Y y, const std::string& str, const boost::shared_ptr<GG::Font>& font, GG::Clr color = GG::CLR_BLACK, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE);
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt  TextUpperLeft() const;
    virtual GG::Pt  TextLowerRight() const;

    virtual const std::vector<GG::Font::LineData>&  GetLineData() const;
    virtual const boost::shared_ptr<GG::Font>&      GetFont() const;
    virtual const std::string&                      RawText() const;          ///< returns text displayed before link formatting is added
    //@}

    /** \name Mutators */ //@{
    virtual void    Render();
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();

    /** sets the text to \a str; may resize the window.  If the window was
        constructed to fit the size of the text (i.e. if the second ctor type
        was used), calls to this function cause the window to be resized to
        whatever space the newly rendered text occupies. */
    virtual void    SetText(const std::string& str);
    //@}

private:
    virtual void    SetLinkedText(const std::string& str);

    std::string     m_raw_text;
};

#endif // _LinkText_h_
