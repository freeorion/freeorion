#ifndef _DesignWnd_h_
#define _DesignWnd_h_

#include <GG/Wnd.h>

class EncyclopediaDetailPanel;
class SaveGameUIData;


/** ShipDesignManager tracks information known to the client about ShipDesigns.  This includes
    the order of designs in the UI. The information is currently used in the DesignWnd and the
    BuildDesignator. */
class ShipDesignManager {
public:

    /** Designs provides ordered lists of designs for display in the UI. */
    class Designs {
    public:
        virtual std::vector<int> OrderedIDs() const = 0;
    };

    ShipDesignManager();

    void StartGame(int empire_id);
    void Save(SaveGameUIData& data) const;
    void Load(const SaveGameUIData& data);

    Designs* CurrentDesigns();

private:
    std::unique_ptr<Designs>    m_current_designs;
};

/** Lets the player design ships */
class DesignWnd : public GG::Wnd {
public:
    /** \name Structors */ //@{
    DesignWnd(GG::X w, GG::Y h);
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Render() override;

    void            Reset();
    void            Sanitize();

    /** Shows \a part_type in design encyclopedia window */
    void            ShowPartTypeInEncyclopedia(const std::string& part_type);

    /** Shows \a hull_type in design encyclopedia window */
    void            ShowHullTypeInEncyclopedia(const std::string& hull_type);

    /** Shows ship design with id \a design_id in design encyclopedia window */
    void            ShowShipDesignInEncyclopedia(int design_id);

    /** Enables, or disables if \a enable is false, issuing orders via this DesignWnd. */
    void            EnableOrderIssuing(bool enable = true);
    //@}

private:
    class BaseSelector;     // has tabs to pick empty hull, previously-saved design or an autodesign template to start new design
    class PartPalette;      // shows parts that can be clicked for detailed or dragged on slots in design
    class MainPanel;        // shows image of hull, slots and parts, design name and description entry boxes, confirm button

    int     AddDesign();    ///< adds current design to those stored by this empire, allowing ships of this design to be produced
    void    ReplaceDesign();///< replace selected completed design with the current design in the stored designs of this empire
    void    DesignChanged();
    void    DesignNameChanged();

    void    InitializeWindows();

    EncyclopediaDetailPanel*    m_detail_panel;
    BaseSelector*               m_base_selector;
    PartPalette*                m_part_palette;
    MainPanel*                  m_main_panel;
};

#endif // _DesignWnd_h_
