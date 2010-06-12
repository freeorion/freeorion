// -*- C++ -*-
#ifndef _ProductionWnd_h_
#define _ProductionWnd_h_

#include "CUIWnd.h"
#include "../universe/Enums.h"

#include <GG/ListBox.h>

class QueueListBox;
class ProductionInfoPanel;
class BuildDesignatorWnd;

/** Contains a BuildDesignatorWnd, some stats on the empire-wide production queue, and the queue itself. */
class ProductionWnd : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    ProductionWnd(GG::X w, GG::Y h);
    ~ProductionWnd();
    //@}

    /** \name Mutators */ //@{
    virtual bool    InWindow(const GG::Pt& pt) const;
    virtual bool    InClient(const GG::Pt& pt) const;

    void            Render();

    void            Refresh();
    void            Reset();
    void            Update();

    /** Shows \a building_type in production encyclopedia window */
    void            ShowBuildingTypeInEncyclopedia(const std::string& building_type);

    /** Centres map wnd on location of item on queue with index \a queue_idx
      * and displays info about that item in encyclopedia window. */
    void            CenterOnBuild(int queue_idx);

    /** Programatically sets this Wnd's selected system.
      * Does not emit a SystemSelectedSignal. */
    void            SelectSystem(int system_id);

    /** Programatically sets this Wnd's selected planet.
      * Does not emit a PlanetSelectedSignal. */
    void            SelectPlanet(int planet_id);

    /** Attempts to find a planet to select, and if successful, selects that
      * planet */
    void            SelectDefaultPlanet();

    void            Sanitize();
    //@}

    mutable boost::signal<void (int)> SystemSelectedSignal; ///< emitted when the user changes the selected system in the production screen
    mutable boost::signal<void (int)> PlanetSelectedSignal; ///< emitted when the user changes the selected planet in the production screen

private:
    void    ProductionQueueChangedSlot();
    void    UpdateQueue();     ///< Clears and repopulates queue list with listitems corresponding to contents of empire's production queue
    void    UpdateInfoPanel(); ///< Updates production summary at top left of production screen, and signals that the empire's minerals resource pool has changed (propegates to the mapwnd to update indicator)

    void    AddBuildToQueueSlot(BuildType build_type, const std::string& name, int number, int location);
    void    AddBuildToQueueSlot(BuildType build_type, int design_id, int number, int location);

    void    ChangeBuildQuantitySlot(int queue_idx, int quantity);

    void    QueueItemMoved(GG::ListBox::Row* row, std::size_t position);
    void    QueueItemDeletedSlot(GG::ListBox::iterator it);
    void    QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt);
    void    QueueItemDoubleClickedSlot(GG::ListBox::iterator it);

    ProductionInfoPanel* m_production_info_panel;
    QueueListBox*        m_queue_lb;
    BuildDesignatorWnd*  m_build_designator_wnd;

    boost::signals::connection  m_empire_connection;
};

#endif // _ProductionWnd_h_
