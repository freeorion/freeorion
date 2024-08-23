#ifndef _TechTreeWnd_h_
#define _TechTreeWnd_h_


#include <boost/signals2/signal.hpp>
#include <GG/Wnd.h>
#include "../Empire/Empire.h"


class Tech;
class EncyclopediaDetailPanel;

/** Returns a browse wnd for tech entries */
std::shared_ptr<GG::BrowseInfoWnd> TechRowBrowseWnd(const std::string& tech_name, int empire_id);

/** Contains the tech graph layout, some controls to control what is visible in
  * the tech layout, the tech navigator, and the tech detail window. */
class TechTreeWnd : public GG::Wnd {
public:
    using TechSignalType = boost::signals2::signal<void (std::string)>;
    using TechClickSignalType = boost::signals2::signal<void (std::string, GG::Flags<GG::ModKey>)>;
    using QueueAddTechsSignalType = boost::signals2::signal<void (std::vector<std::string>, int)>;

    /** TechTreeWnd constructor is usually called before client has
        access to techs.  Attempting to show the tech tree takes a long
        time and generates errors.  If \p initially_hidden is true then the
        tech categories are not parsed until the first time Show() is
        called, speeding up the constructor and preventing spurious errors.*/
    TechTreeWnd(GG::X w, GG::Y h, bool initially_hidden = true);
    void CompleteConstruction() override;

    double Scale() const;
    bool   PediaVisible();

    /** If tech @p tech_name is currently visible */
    bool TechIsVisible(const std::string& tech_name) const;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void Show() override;
    void Update();
    void Clear();
    void Reset();

    void ShowCategory(std::string category);
    void ShowAllCategories();
    void HideCategory(const std::string& category);
    void HideAllCategories();
    void ToggleAllCategories();

    void SetTechStatus(const TechStatus status, const bool state);

    void ToggleViewType(bool show_list_view);
    void ShowTreeView();
    void ShowListView();

    void CenterOnTech(const std::string& tech_name);
    void SetEncyclopediaTech(std::string tech_name);
    void SelectTech(std::string tech_name);

    void ShowPedia();
    void HidePedia();
    void TogglePedia();

    mutable TechSignalType          TechSelectedSignal;
    mutable QueueAddTechsSignalType AddTechsToQueueSignal;

private:
    class TechTreeControls;
    class LayoutPanel;
    class TechListBox;

    void TechLeftClickedSlot(std::string tech_name, GG::Flags<GG::ModKey> modkeys);
    void AddTechToResearchQueue(std::string tech_name, bool to_front);
    void TechPediaDisplaySlot(std::string tech_name);

    void InitializeWindows();

    std::shared_ptr<TechTreeControls>        m_tech_tree_controls;
    std::shared_ptr<EncyclopediaDetailPanel> m_enc_detail_panel;
    std::shared_ptr<LayoutPanel>             m_layout_panel;
    std::shared_ptr<TechListBox>             m_tech_list;

    /// If m_init_flag is true tech categories are not parsed until the
    /// first time Show() is called.  TechTreeWnd is constructed before the
    /// tech categories are available to be parsed.
    bool m_init_flag = true;
};


#endif
