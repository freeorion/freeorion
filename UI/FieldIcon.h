#ifndef _FieldIcon_h_
#define _FieldIcon_h_

#include <GG/GGFwd.h>
#include <GG/Control.h>

#include <boost/signals2/signal.hpp>


/** A control that allows interaction with a field. This class allows user
  * interaction with fields on the galaxy map.  It contains the graphic to
  * display the field, along with the object ID of the UniverseObject
    associated with it. */
class FieldIcon final : public GG::Control {
public:
    FieldIcon(int field_id);
    ~FieldIcon() = default;
    void CompleteConstruction() override;

    /** Checks to see if point lies inside in-system fleet buttons before
        checking parent InWindow method. */
    bool InWindow(GG::Pt pt) const noexcept override;
    int  FieldID() const noexcept { return m_field_id; }

    /** Returns the field texture. */
    const auto& FieldTexture() const noexcept { return m_texture; }

    void LButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) override;
    void LButtonUp(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void Render() override {}
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void Refresh();
    void SetSelected(bool selected = true);   //!< shows/hides the selection indicator over this field

    mutable boost::signals2::signal<void (int)> MouseEnteringSignal;
    mutable boost::signals2::signal<void (int)> MouseLeavingSignal;
    mutable boost::signals2::signal<void (int)> LeftClickedSignal;
    mutable boost::signals2::signal<void (int)> RightClickedSignal;
    mutable boost::signals2::signal<void (int)> LeftDoubleClickedSignal;
    mutable boost::signals2::signal<void (int)> RightDoubleClickedSignal;

private:
    int                                 m_field_id;             //!< the Field associated with this FieldIcon
    std::shared_ptr<GG::Texture>        m_texture;
    std::shared_ptr<GG::DynamicGraphic> m_selection_indicator;  //!< shown to indicate system is selected in sidepanel;
    std::shared_ptr<GG::StaticGraphic>  m_mouseover_indicator;  //!< shown when the mouse cursor is over the system;
    bool                                m_selected = false;     //!< is this icon presently selected / should it show m_selected_indicator
    std::shared_ptr<GG::Label>          m_name;                 //!< the control that holds the name of the system;
};


#endif
