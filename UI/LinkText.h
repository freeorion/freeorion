// -*- C++ -*-
#ifndef _LinkText_h_
#define _LinkText_h_

#ifndef _GGTextControl_h_
#include "GGTextControl.h"
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
    <tech ID>
    <encyclopedia [string]>\endverbatim
    The ID parameters refer to the UniverseObjects that should be zoomed to for each link; encyclopedia entries are refered
    to by strings.
    <br><br>Note that to save and load this class using GG's automatic serialization, LinkText must be added to the 
    app's XMLObjectFactory. Note also that for link tags to be correctly handled, they must not overlap each other at all, 
    even though overlap with regular GG::Font tags if fine. */
class LinkText : public GG::TextControl
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<bool (int)>                IDSignalType;     ///< emitted when a link that refers to an ID number is clicked
    typedef boost::signal<bool (const std::string&)> StringSignalType; ///< emitted when a link that refers to an string is clicked
    //@}

    /** \name Slot Types */ //@{
    typedef IDSignalType::slot_type     IDSlotType;       ///< type of functor(s) invoked on a IDSignalType
    typedef StringSignalType::slot_type StringSlotType;   ///< type of functor(s) invoked on a StringSignalType
    //@}

    /** \name Structors */ //@{
    LinkText(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<GG::Font>& font, Uint32 text_fmt = 0, GG::Clr color = GG::CLR_BLACK, Uint32 flags = CLICKABLE); ///< ctor taking a font directly
    LinkText(int x, int y, int w, int h, const std::string& str, const std::string& font_filename, int pts, Uint32 text_fmt = 0, GG::Clr color = GG::CLR_BLACK, Uint32 flags = CLICKABLE); ///< ctor taking a font filename and font point size

    /** ctor that does not require window size.
        Window size is determined from the string and font; the window will be large enough to fit the text as rendered, 
        and no larger.  \see DynamicText::DynamicText() */
    LinkText(int x, int y, const std::string& str, const boost::shared_ptr<GG::Font>& font, GG::Clr color = GG::CLR_BLACK, Uint32 flags = CLICKABLE);
   
    /** ctor that does not require window size.
        Window size is determined from the string and font; the window will be large enough to fit the text as rendered, 
        and no larger.  \see DynamicText::DynamicText() */
    LinkText(int x, int y, const std::string& str, const std::string& font_filename, int pts, GG::Clr color = GG::CLR_BLACK, Uint32 flags = CLICKABLE);
   
    LinkText(const GG::XMLElement& elem); ///< ctor that constructs a LinkText object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a LinkText object
    //@}

    /** \name Mutators */ //@{
    virtual void   LButtonDown(const GG::Pt& pt, Uint32 keys);
    virtual void   LButtonUp(const GG::Pt& pt, Uint32 keys);
    virtual void   LClick(const GG::Pt& pt, Uint32 keys);
    virtual void   MouseHere(const GG::Pt& pt, Uint32 keys);
    virtual void   MouseLeave(const GG::Pt& pt, Uint32 keys);
   
    /** sets the text to \a str; may resize the window.  If the window was constructed to fit the size of the text 
        (i.e. if the second ctor type was used), calls to this function cause the window to be resized to whatever 
        space the newly rendered text occupies. */
    virtual void   SetText(const std::string& str);

    IDSignalType&     PlanetLinkSignal()       const {return m_planet_sig;}       ///< returns the planet link signal object for this LinkText
    IDSignalType&     SystemLinkSignal()       const {return m_system_sig;}       ///< returns the system link signal object for this LinkText
    IDSignalType&     FleetLinkSignal()        const {return m_fleet_sig;}        ///< returns the fleet link signal object for this LinkText
    IDSignalType&     ShipLinkSignal()         const {return m_ship_sig;}         ///< returns the ship link signal object for this LinkText
    IDSignalType&     TechLinkSignal()         const {return m_tech_sig;}         ///< returns the tech link signal object for this LinkText
    StringSignalType& EncyclopediaLinkSignal() const {return m_encyclopedia_sig;} ///< returns the encyclopedia link signal object for this LinkText

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a LinkText object
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

    mutable IDSignalType m_planet_sig;
    mutable IDSignalType m_system_sig;
    mutable IDSignalType m_fleet_sig;
    mutable IDSignalType m_ship_sig;
    mutable IDSignalType m_tech_sig;
    mutable StringSignalType m_encyclopedia_sig;

    static bool s_link_tags_registered;
};

inline std::pair<std::string, std::string> LinkTextRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _LinkText_h_
