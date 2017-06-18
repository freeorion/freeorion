#ifndef _FieldIcon_h_
#define _FieldIcon_h_

#include <GG/GGFwd.h>
#include <GG/Control.h>

#include <boost/signals2/signal.hpp>


/** A control that allows interaction with a field. This class allows user
  * interaction with fields on the galaxy map.  It contains the graphic to
  * display the field, along with the object ID of the UniverseObject
    associated with it. */
class FieldIcon : public GG::Control {
public:
    //! \name Structors //!@{
    FieldIcon(int field_id);

    ~FieldIcon();
    //!@}

    //! \name Accessors //!@{
    /** Checks to see if point lies inside in-system fleet buttons before
        checking parent InWindow method. */
    bool InWindow(const GG::Pt& pt) const override;

    int                 FieldID() const;                        //!< returns ID of system this icon represents

    /** Returns the field texture. */
    const std::shared_ptr<GG::Texture>& FieldTexture() const;
    //!@}

    //! \name Mutators //!@{
    void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) override;
    void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Render() override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void MouseLeave() override;

    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    void            Refresh();

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

    /** Field texture. */
    std::shared_ptr<GG::Texture> m_texture;

    std::shared_ptr<GG::DynamicGraphic>             m_selection_indicator;      //!< shown to indicate system is selected in sidepanel;
    std::shared_ptr<GG::StaticGraphic>              m_mouseover_indicator;      //!< shown when the mouse cursor is over the system;
    bool                            m_selected;                 //!< is this icon presently selected / should it show m_selected_indicator
    std::shared_ptr<GG::Label>                      m_name;                     //!< the control that holds the name of the system;
};

#endif // _SystemIcon_h_
