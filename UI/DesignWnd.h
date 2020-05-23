#ifndef _DesignWnd_h_
#define _DesignWnd_h_

#include <GG/Wnd.h>
#include <boost/uuid/uuid.hpp>

class EncyclopediaDetailPanel;
struct SaveGameUIData;

/** ShipDesignManager tracks information known to the client about ShipDesigns.
  * This includes the order of designs to be shown DesignWnd and the
  * BuildDesignator. */
class ShipDesignManager {
public:
    /** Designs provides ordered lists of designs for display in the UI.
     Derived classes provide an implementation for the pure virtual
     OrderedIDs(), which provides ship design ids in display order. They may
     also provide additional functionality used by the DesignWnd. */
    class Designs {
    public:
        virtual std::vector<int> OrderedIDs() const = 0;
    };

    ShipDesignManager();
    virtual ~ShipDesignManager();

    virtual void StartGame(int empire_id, bool is_new_game);
    virtual void Load(const SaveGameUIData& data);
    virtual void Save(SaveGameUIData& data) const;

    /** DisplayedDesigns are design currently producible by the empire, in the ProductionWnd.*/
    virtual Designs* DisplayedDesigns();
    /** SavedDesigns are designs that the player has saved on their own machine
        for future use.  They may/may not also be in use in the current game. */
    virtual Designs* SavedDesigns();

private:
    std::unique_ptr<Designs>    m_displayed_designs;// designs shown to the player in the UI
    std::unique_ptr<Designs>    m_saved_designs;    // loaded from saved design script files, can be added to empire by player or automatically
};

/** Lets the player design ships */
class DesignWnd : public GG::Wnd {
public:
    /** \name Structors */ //@{
    DesignWnd(GG::X w, GG::Y h);
    //@}
    void CompleteConstruction() override;

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void Render() override;
    void Reset();
    void Sanitize();

    //! Shows @a ship_part in design encyclopedia window
    void ShowShipPartInEncyclopedia(const std::string& ship_part);

    //! Shows @p ship_hull in design encyclopedia window
    void ShowShipHullInEncyclopedia(const std::string& ship_hull);

    /** Shows ship design with id \a design_id in design encyclopedia window */
    void ShowShipDesignInEncyclopedia(int design_id);

    /** Enables, or disables if \a enable is false, issuing orders via this DesignWnd. */
    void EnableOrderIssuing(bool enable = true);
    //@}

private:
    class BaseSelector;     // has tabs to pick empty hull, previously-saved design or an autodesign template to start new design
    class PartPalette;      // shows parts that can be clicked for detailed or dragged on slots in design
    class MainPanel;        // shows image of hull, slots and parts, design name and description entry boxes, confirm button

    void DesignChanged();
    void DesignNameChanged();
    void InitializeWindows();

    std::shared_ptr<EncyclopediaDetailPanel>    m_detail_panel;
    std::shared_ptr<BaseSelector>               m_base_selector;
    std::shared_ptr<PartPalette>                m_part_palette;
    std::shared_ptr<MainPanel>                  m_main_panel;
};

#endif // _DesignWnd_h_
