#ifndef _ResearchWnd_h_
#define _ResearchWnd_h_

#include "CUIWnd.h"

#include <GG/ListBox.h>

class TechTreeWnd;
class ResourceInfoPanel;
class ResearchQueueWnd;

/** Contains a TechTreeWnd, some stats on the empire-wide research queue, and the queue itself. */
class ResearchWnd : public GG::Wnd {
public:
    /** \name Structors */ //@{
    ResearchWnd(GG::X w, GG::Y h, bool initially_hidden = true);
    ~ResearchWnd();
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    int     ShownEmpireID() const;
    bool    PediaVisible();
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Render() override;

    void    SetEmpireShown(int empire_id);

    void    Refresh();
    void    Reset();
    void    Update();

    void    CenterOnTech(const std::string& tech_name);
    /** Set encyclopedia entry to tech article for @p tech_name
     *  Selects and centers tech @p tech_name if tech is initially visible
     *  or if @p force is true(default). */
    void    ShowTech(const std::string& tech_name, bool force = true);
    void    QueueItemMoved(const GG::ListBox::iterator& row_it, const GG::ListBox::iterator& original_position_it);
    void    Sanitize();

    void    ShowPedia();
    void    HidePedia();
    void    TogglePedia();

    /** Enables, or disables if \a enable is false, issuing orders via this ResearchWnd. */
    void    EnableOrderIssuing(bool enable = true);
    //@}

private:
    void    DoLayout(bool init = false);
    void    ResearchQueueChangedSlot();
    void    UpdateQueue();
    void    UpdateInfoPanel();     ///< Updates research summary at top left of production screen, and signals that the empire's minerals research pool has changed (propagates to the mapwnd to update indicator)
    void    DeleteQueueItem(GG::ListBox::iterator it);
    void    AddTechsToQueueSlot(const std::vector<std::string>& tech_vec, int pos = -1);
    void    QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    QueueItemDoubleClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    QueueItemPaused(GG::ListBox::iterator it, bool pause);

    std::shared_ptr<ResourceInfoPanel>  m_research_info_panel;
    std::shared_ptr<ResearchQueueWnd>   m_queue_wnd;
    std::shared_ptr<TechTreeWnd>        m_tech_tree_wnd;

    bool                        m_enabled;
    int                         m_empire_shown_id;
    boost::signals2::connection m_empire_connection;
};

#endif // _ResearchWnd_h_
