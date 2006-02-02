// -*- C++ -*-
#ifndef _LinkText_h_
#define _LinkText_h_

#ifndef _GG_TextControl_h_
#include <GG/TextControl.h>
#endif

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
    <br><br>Note that to save and load this class using GG's automatic serialization, LinkText must be added to the 
    app's XMLObjectFactory. Note also that for link tags to be correctly handled, they must not overlap each other at all, 
    even though overlap with regular GG::Font tags if fine. */
class LinkText : public GG::TextControl
{
private:
    struct BoolCombiner 
    {
        typedef bool result_type; 
        template<class InIt> result_type operator()(InIt first, InIt last) const;
    };

public:
    /** \name Signal Types */ //@{
    typedef boost::signal<bool (int), BoolCombiner>                IDSignalType;     ///< emitted when a link that refers to an ID number is clicked
    typedef boost::signal<bool (const std::string&), BoolCombiner> StringSignalType; ///< emitted when a link that refers to an string is clicked
    //@}

    /** \name Slot Types */ //@{
    typedef IDSignalType::slot_type     IDSlotType;       ///< type of functor(s) invoked on a IDSignalType
    typedef StringSignalType::slot_type StringSlotType;   ///< type of functor(s) invoked on a StringSignalType
    //@}

    /** \name Structors */ //@{
    LinkText(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font, Uint32 text_fmt = 0, GG::Clr color = GG::CLR_BLACK, Uint32 flags = GG::CLICKABLE); ///< ctor taking a font directly

    /** ctor that does not require window size.
        Window size is determined from the string and font; the window will be large enough to fit the text as rendered, 
        and no larger.  \see DynamicText::DynamicText() */
    LinkText(int x, int y, const std::string& str, const boost::shared_ptr<GG::Font>& font, GG::Clr color = GG::CLR_BLACK, Uint32 flags = GG::CLICKABLE);
    //@}

    /** \name Mutators */ //@{
    virtual void   Render();
    virtual void   LButtonDown(const GG::Pt& pt, Uint32 keys);
    virtual void   LButtonUp(const GG::Pt& pt, Uint32 keys);
    virtual void   LClick(const GG::Pt& pt, Uint32 keys);
    virtual void   MouseHere(const GG::Pt& pt, Uint32 keys);
    virtual void   MouseLeave(const GG::Pt& pt, Uint32 keys);
   
    /** sets the text to \a str; may resize the window.  If the window was constructed to fit the size of the text 
        (i.e. if the second ctor type was used), calls to this function cause the window to be resized to whatever 
        space the newly rendered text occupies. */
    virtual void   SetText(const std::string& str);

    mutable IDSignalType     PlanetLinkSignal;       ///< returns the planet link signal object for this LinkText
    mutable IDSignalType     SystemLinkSignal;       ///< returns the system link signal object for this LinkText
    mutable IDSignalType     FleetLinkSignal;        ///< returns the fleet link signal object for this LinkText
    mutable IDSignalType     ShipLinkSignal;         ///< returns the ship link signal object for this LinkText
    mutable StringSignalType TechLinkSignal;         ///< returns the tech link signal object for this LinkText
    mutable StringSignalType BuildingLinkSignal;     ///< returns the building link signal object for this LinkText
    mutable StringSignalType EncyclopediaLinkSignal; ///< returns the encyclopedia link signal object for this LinkText
    //@}
    
private:
    struct Link
    {
        std::string             type;       ///< contents of type field of link tag (eg "planet" in <planet 3>)
        std::string             data;       ///< contents of data field of link tag (eg "3" in <planet 3>)
        std::vector<GG::Rect>   rects;      ///< the rectangles in which this link falls, in window coordinates (some links may span more than one line)
        std::pair<int, int>     text_posn;  ///< the index of the first (.first) and last + 1 (.second) characters in the link text
    };

    void Init();
    void FindLinks(); ///< finds the links in the text and populates m_links
    int GetLinkUnderPt(const GG::Pt& pt); ///< returns the index of the link under screen coordinate \a pt, or -1 if none
    void ClearOldRollover();

    std::vector<Link> m_links;
    int               m_old_sel_link;
    int               m_old_rollover_link;

    static bool s_link_tags_registered;
};

// template implementations
template<class InIt>
LinkText::BoolCombiner::result_type LinkText::BoolCombiner::operator()(InIt first, InIt last) const
{
    while (first != last)
        *first++;
    return true;
}

inline std::string LinkTextRevision()
{return "$Id$";}

#endif // _LinkText_h_
