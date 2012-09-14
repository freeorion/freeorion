// -*- C++ -*-
//FieldIcon.h
#ifndef _FieldIcon_h_
#define _FieldIcon_h_

#include "CUIDrawUtil.h"
#include <GG/Control.h>

namespace GG {
    class StaticGraphic;
    class DynamicGraphic;
    class TextControl;
    class Texture;
}

/** A control that allows interaction with a field. This class allows user
  * interaction with fields on the galaxy map.  It contains the graphic to
  * display the field, along with the object ID of the UniverseObject
    associated with it. */
class FieldIcon : public GG::Control {
public:
    //! \name Signal Types //!@{
    typedef boost::signal<void (int)>   MouseEnteringSignalType;        //!< emitted when the user moves the cursor over the icon; returns the object id
    typedef boost::signal<void (int)>   MouseLeavingSignalType;         //!< emitted when the user moves the cursor off of the icon; returns the object id
    typedef boost::signal<void (int)>   LeftClickedSignalType;          //!< emitted when the user left clicks the icon; returns the objectID
    typedef boost::signal<void (int)>   RightClickedSignalType;         //!< emitted when the user right clicks the icon; returns the objectID
    typedef boost::signal<void (int)>   LeftDoubleClickedSignalType;    //!< emitted when the user left double-clicks the icon; returns the object id
    typedef boost::signal<void (int)>   RightDoubleClickedSignalType;   //!< emitted when the user left double-clicks the icon; returns the object id
    //!@}

    //! \name Structors //!@{
    FieldIcon(GG::X x, GG::Y y, int field_id);                  //!< basid ctor
    ~FieldIcon();                                               //!< dtor
    //!@}

    //! \name Accessors //!@{
    int                 FieldID() const;                        //!< returns ID of system this icon represents
    const boost::shared_ptr<GG::Texture>& FieldTexture() const; //!< returns the field texture

    virtual bool        InWindow(const GG::Pt& pt) const;       //!< Overrides GG::Wnd::InWindow. Checks to see if point lies inside in-system fleet buttons before checking main InWindow method.
    //!@}

    //! \name Mutators //!@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            Refresh();
    virtual void    Render();

    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();
    virtual void    MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);

    void            SetSelected(bool selected = true);   //!< shows/hides the selection indicator over this field

    mutable MouseEnteringSignalType         MouseEnteringSignal;
    mutable MouseLeavingSignalType          MouseLeavingSignal;
    mutable LeftClickedSignalType           LeftClickedSignal;
    mutable RightClickedSignalType          RightClickedSignal;
    mutable LeftDoubleClickedSignalType     LeftDoubleClickedSignal;
    mutable RightDoubleClickedSignalType    RightDoubleClickedSignal;
    //!@}

private:
    void            PositionFieldName();

    int                             m_field_id;                 //!< the Field associated with this FieldIcon
    boost::shared_ptr<GG::Texture>  m_texture;                  //!< field texture
    GG::DynamicGraphic*             m_selection_indicator;      //!< shown to indicate system is selected in sidepanel
    GG::StaticGraphic*              m_mouseover_indicator;      //!< shown when the mouse cursor is over the system
    bool                            m_selected;                 //!< is this icon presently selected / should it show m_selected_indicator
    GG::TextControl*                m_name;                     //!< the control that holds the name of the system
};

#endif // _SystemIcon_h_
