// -*- C++ -*-
#ifndef _LinkText_h_
#define _LinkText_h_

#ifndef _GG_TextControl_h_
#include <GG/TextControl.h>
#endif

class TextLinker
{
private:
    struct BoolCombiner 
    {
        typedef bool result_type; 
        template<class InIt> result_type operator()(InIt first, InIt last) const;
    };

public:
    /** \name Structors */ //@{
    TextLinker();
    virtual ~TextLinker();
    //@}

protected:
    void        Render_();
    void        LButtonDown_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void        LButtonUp_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void        LClick_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void        MouseHere_(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void        MouseLeave_();

    virtual const std::vector<GG::Font::LineData>&  GetLineData() const = 0;
    virtual const boost::shared_ptr<GG::Font>&      GetFont() const = 0;
    virtual GG::Pt                                  TextUpperLeft() const = 0;
    virtual GG::Pt                                  TextLowerRight() const = 0;
    virtual void                                    SetLinkedText(const std::string& str) = 0;
    virtual const std::string&                      WindowText() const = 0;

    void                                            FindLinks();    ///< finds the links in the text and populates m_links

private:
    struct Link
    {
        std::string             type;           ///< contents of type field of link tag (eg "planet" in <planet 3>)
        std::string             data;           ///< contents of data field of link tag (eg "3" in <planet 3>)
        std::vector<GG::Rect>   rects;          ///< the rectangles in which this link falls, in window coordinates (some links may span more than one line)
        std::pair<int, int>     text_posn;      ///< the index of the first (.first) and last + 1 (.second) characters in the link text
    };

    int     GetLinkUnderPt(const GG::Pt& pt);   ///< returns the index of the link under screen coordinate \a pt, or -1 if none
    void    ClearOldRollover();

    std::vector<Link>   m_links;
    int                 m_old_sel_link;
    int                 m_old_rollover_link;

    static bool         s_link_tags_registered;
};

/** allows text that the user sees to emit signals when clicked, and indicates to the user visually which text 
    represents a link.  There is one type of signal for each type of ZoomTo*() method in ClientUI.  This allows
    any text that refers to game elements to be tagged as such and clicked by the user, emitting a signal of the 
    appropriate type.  These signals can be used to ZoomTo*() an appropriate screen, or take some other action.
    The folowig tags are currently supported:
    \verbatim
    <planet ID>
    <system ID>
    <fleet ID>
    <ship ID>
    <tech [string]>
    <building [string]>
    <encyclopedia [string]>\endverbatim
    The ID parameters refer to the UniverseObjects that should be zoomed to for each link; encyclopedia entries are refered
    to by strings.
    <br><br>Note that for link tags to be correctly handled, they must not overlap each other at all, even though
    overlap with regular GG::Font tags if fine. */
class LinkText : public GG::TextControl, public TextLinker
{
public:
    /** \name Structors */ //@{
    LinkText(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE, GG::Clr color = GG::CLR_BLACK, GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< ctor taking a font directly

    /** ctor that does not require window size.
        Window size is determined from the string and font; the window will be large enough to fit the text as rendered, 
        and no larger.  \see DynamicText::DynamicText() */
    LinkText(int x, int y, const std::string& str, const boost::shared_ptr<GG::Font>& font, GG::Clr color = GG::CLR_BLACK, GG::Flags<GG::WndFlag> flags = GG::CLICKABLE);
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt  TextUpperLeft() const;
    virtual GG::Pt  TextLowerRight() const;

    virtual const std::vector<GG::Font::LineData>&  GetLineData() const;
    virtual const boost::shared_ptr<GG::Font>&      GetFont() const;
    virtual const std::string&                      WindowText() const;
    //@}

    /** \name Mutators */ //@{
    virtual void    Render();
    virtual void    LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();


    /** sets the text to \a str; may resize the window.  If the window was constructed to fit the size of the text 
        (i.e. if the second ctor type was used), calls to this function cause the window to be resized to whatever 
        space the newly rendered text occupies. */
    virtual void    SetText(const std::string& str);
    //@}

private:
    virtual void    SetLinkedText(const std::string& str);
};

// template implementations
template<class InIt>
TextLinker::BoolCombiner::result_type TextLinker::BoolCombiner::operator()(InIt first, InIt last) const
{
    while (first != last)
        *first++;
    return true;
}

#endif // _LinkText_h_
