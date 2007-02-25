// -*- C++ -*-
#ifndef _ProductionWnd_h_
#define _ProductionWnd_h_

#include "CUIWnd.h"
#include "../universe/Enums.h"

#include <GG/ListBox.h>

class CUIListBox;
class ProductionInfoPanel;
class BuildDesignatorWnd;

/** Contains a BuildDesignatorWnd, some stats on the empire-wide production queue, and the queue itself. */
class ProductionWnd : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    ProductionWnd(int w, int h);
    //@}

    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> SystemSelectedSignalType; ///< emitted when something in the productionwnd wants to change the selected system
    //@}

    /** \name Mutators */ //@{
    bool InWindow(const GG::Pt& pt) const;
    bool InClient(const GG::Pt& pt) const;

    void Render();

    void Reset();
    void CenterOnBuild(int queue_idx);
    void SelectSystem(int system);
    void SelectPlanet(int planet);
    void QueueItemMoved(int row_idx, GG::ListBox::Row* row);
    void Sanitize();
    //@}

    mutable SystemSelectedSignalType SystemSelectedSignal;

private:
    void UpdateQueue();     ///< Clears and repopulates queue list with listitems corresponding to contents of empire's production queue
    void ResetInfoPanel();  ///< Updates production summary at top left of production screen, and signals that the empire's minerals resource pool has changed (propegates to the mapwnd to update indicator)
    void AddBuildToQueueSlot(BuildType build_type, const std::string& name, int number, int location);
    void ChangeBuildQuantitySlot(int queue_idx, int quantity);
    void QueueItemDeletedSlot(int row_idx, GG::ListBox::Row* row);
    void QueueItemClickedSlot(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt);
    void QueueItemDoubleClickedSlot(int row_idx, GG::ListBox::Row* row);

    ProductionInfoPanel* m_production_info_panel;
    CUIListBox*          m_queue_lb;
    BuildDesignatorWnd*  m_build_designator_wnd;
};

#endif // _ProductionWnd_h_
