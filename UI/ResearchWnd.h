#ifndef _ResearchWnd_h_
#define _ResearchWnd_h_

#include <atomic>
#include "../universe/ConstantsFwd.h"
#include <GG/ListBox.h>
#include <GG/Wnd.h>

class TechTreeWnd;
class ResourceInfoPanel;
class ResearchQueueWnd;
struct ScriptingContext;

/** Contains a TechTreeWnd, some stats on the empire-wide research queue, and the queue itself. */
class ResearchWnd : public GG::Wnd {
public:
    ResearchWnd(GG::X w, GG::Y h, bool initially_hidden = true);
    void CompleteConstruction() override;

    int  ShownEmpireID() const { return m_empire_shown_id; };
    bool PediaVisible();

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Render() override;

    void SetEmpireShown(int empire_id, const ScriptingContext& context);

    void Refresh(const ScriptingContext& context);
    void Reset(const ScriptingContext& context);
    void Update(const ScriptingContext& context);

    void CenterOnTech(const std::string& tech_name);
    /** Set encyclopedia entry to tech article for @p tech_name
     *  Selects and centers tech @p tech_name if tech is initially visible
     *  or if @p force is true(default). */
    void ShowTech(std::string tech_name, bool force = true);
    void QueueItemMoved(GG::ListBox::iterator row_it, GG::ListBox::iterator original_position_it);
    void Sanitize();

    void ShowPedia();
    void HidePedia();
    void TogglePedia();

    /** Enables, or disables if \a enable is false, issuing orders via this ResearchWnd. */
    void EnableOrderIssuing(bool enable = true);

private:
    void DoLayout(bool init = false);
    void ResearchQueueChangedSlot();
    void UpdateQueue(const ScriptingContext& context);
    void UpdateInfoPanel(const ScriptingContext& context); ///< Updates research summary at top left of production screen, and signals that the empire's minerals research pool has changed (propagates to the mapwnd to update indicator)
    void DeleteQueueItem(GG::ListBox::iterator it);
    void AddTechsToQueueSlot(std::vector<std::string> tech_vec, int pos = -1) const;
    void QueueItemClickedSlot(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void QueueItemDoubleClickedSlot(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void QueueItemPaused(GG::ListBox::iterator it, bool pause);

    std::shared_ptr<ResourceInfoPanel>  m_research_info_panel;
    std::shared_ptr<ResearchQueueWnd>   m_queue_wnd;
    std::shared_ptr<TechTreeWnd>        m_tech_tree_wnd;
    boost::signals2::scoped_connection  m_empire_connection;
    int                                 m_empire_shown_id = ALL_EMPIRES;
    bool                                m_enabled = false;
    std::atomic<bool>                   m_refresh_needed = false;
};


#endif
