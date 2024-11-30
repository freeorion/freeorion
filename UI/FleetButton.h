#ifndef _FleetButton_h_
#define _FleetButton_h_


#include <GG/Button.h>
#include <GG/StaticGraphic.h>

#include <memory>


class Fleet;
class RotatingGraphic;
class ScanlineControl;

/** represents one or more fleets of an empire at a location on the map. */
class FleetButton final : public GG::Button {
public:
    enum class SizeType : uint8_t {
        NONE,
        TINY,
        SMALL,
        MEDIUM,
        LARGE
    };

    FleetButton(std::vector<int> fleet_IDs, SizeType size_type);
    FleetButton(int fleet_id, SizeType size_type);
    void CompleteConstruction() override;
    ~FleetButton() = default;

    void Refresh(SizeType size_type);

    /** Returns true if \a pt is within or over the button. */
    bool InWindow(GG::Pt pt) const noexcept override;

    const auto& Fleets() const noexcept  { return m_fleets; }    ///< returns the fleets represented by this control
    bool        Selected() const noexcept { return m_selected; } ///< returns whether this button has been marked selected
    void        MouseHere(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void        SizeMove(GG::Pt ul, GG::Pt lr) override;
    void        SetSelected(bool selected = true);               ///< sets selection status of button.  if selected = true, marks button as selected.  otherwise marks button as not selected

    static void PlayFleetButtonOpenSound();
    static void PlayFleetButtonRolloverSound();

protected:
    void RenderUnpressed() override;
    void RenderPressed() override;
    void RenderRollover() override;

private:
    void LayoutIcons();

    std::vector<int>                                m_fleets;   ///< the fleets represented by this button
    SizeType                                        m_size = SizeType::NONE;
    std::vector<std::shared_ptr<GG::StaticGraphic>> m_icons;
    std::shared_ptr<RotatingGraphic>                m_selection_indicator;
    std::shared_ptr<ScanlineControl>                m_scanline_control;
    bool                                            m_selected = false; ///< should this button render itself specially to show that it is selected?
    bool                                            m_fleet_blockaded = false;
};

/* returns head icon for passed fleet at passed icon size */
std::vector<std::shared_ptr<GG::Texture>> FleetHeadIcons(const Fleet* fleet,
                                                         FleetButton::SizeType size_type);

/* returns head icon for passed fleets at passed icon size */
std::vector<std::shared_ptr<GG::Texture>> FleetHeadIcons(const std::vector<const Fleet*>& fleets,
                                                         FleetButton::SizeType size_type);

/* returns size icon for passed fleet at passed icon size */
std::shared_ptr<GG::Texture> FleetSizeIcon(const Fleet* fleet, FleetButton::SizeType size_type);

/* returns head icon for passed fleet size at passed icon size */
std::shared_ptr<GG::Texture> FleetSizeIcon(unsigned int fleet_size, FleetButton::SizeType size_type);

/* returns blockade icon if a fleet is being blockaded */
std::shared_ptr<GG::Texture> FleetBlockadedIcon(FleetButton::SizeType size_type);

/* returns icon for indication fleet icon selection */
std::shared_ptr<GG::Texture> FleetSelectionIndicatorIcon();


#endif
