// -*- C++ -*-
//FieldIcon.h
#ifndef _FieldIcon_h_
#define _FieldIcon_h_

#include "CUIDrawUtil.h"
#include <GG/Control.h>

class CUILabel;
namespace GG {
    class StaticGraphic;
    class DynamicGraphic;
    class Texture;
}

/** A control that allows interaction with a field. This class allows user
  * interaction with fields on the galaxy map.  It contains the graphic to
  * display the field, along with the object ID of the UniverseObject
    associated with it. */
class FieldIcon : public GG::Control {
public:
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
    virtual void    LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
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

    mutable boost::signals2::signal<void (int)>   MouseEnteringSignal;
    mutable boost::signals2::signal<void (int)>   MouseLeavingSignal;
    mutable boost::signals2::signal<void (int)>   LeftClickedSignal;
    mutable boost::signals2::signal<void (int)>   RightClickedSignal;
    mutable boost::signals2::signal<void (int)>   LeftDoubleClickedSignal;
    mutable boost::signals2::signal<void (int)>   RightDoubleClickedSignal;
    //!@}

private:
    void            PositionFieldName();

    int                             m_field_id;                 //!< the Field associated with this FieldIcon
    boost::shared_ptr<GG::Texture>  m_texture;                  //!< field texture
    GG::DynamicGraphic*             m_selection_indicator;      //!< shown to indicate system is selected in sidepanel
    GG::StaticGraphic*              m_mouseover_indicator;      //!< shown when the mouse cursor is over the system
    bool                            m_selected;                 //!< is this icon presently selected / should it show m_selected_indicator
    CUILabel*                       m_name;                     //!< the control that holds the name of the system
};

#endif // _SystemIcon_h_
