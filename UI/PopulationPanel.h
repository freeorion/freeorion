// -*- C++ -*-
#ifndef _PopulationPanel_h_
#define _PopulationPanel_h_

#include "AccordionPanel.h"
#include "../universe/TemporaryPtr.h"

class MultiIconValueIndicator;
class MultiMeterStatusBar;
class PopCenter;
class StatisticIcon;

/** Shows population with meter bars */
class PopulationPanel : public AccordionPanel {
public:
    /** \name Structors */ //@{
    PopulationPanel(GG::X w, int object_id);            ///< basic ctor
    ~PopulationPanel();
    //@}

    /** \name Accessors */ //@{
    int                 PopCenterID() const {return m_popcenter_id;}
    //@}

    /** \name Mutators */ //@{
    void                ExpandCollapse(bool expanded);  ///< expands or collapses panel to show details or just summary info

    virtual void        Render();
    virtual void        MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    bool                EventFilter(GG::Wnd* w, const GG::WndEvent& event);

    void                Update();                       ///< updates indicators with values of associated object.  Does not do layout and resizing.
    void                Refresh();                      ///< updates, redoes layout, resizes indicator

    /** Enables, or disables if \a enable is false, issuing orders via this PopulationPanel. */
    void                EnableOrderIssuing(bool enable = true);
    //@}

private:
    void                ExpandCollapseButtonPressed();      ///< toggles panel expanded or collapsed

    void                DoLayout();                         ///< resizes panel and positions widgets

    TemporaryPtr<const PopCenter>   GetPopCenter() const;   ///< returns the PopCenter object with id m_popcenter_id

    int                             m_popcenter_id;         ///< object id for the UniverseObject that is also a PopCenter which is being displayed in this panel

    StatisticIcon*                  m_pop_stat;             ///< icon and number of population
    StatisticIcon*                  m_happiness_stat;       ///< icon and number of happiness

    MultiIconValueIndicator*        m_multi_icon_value_indicator;   ///< textually / numerically indicates population
    MultiMeterStatusBar*            m_multi_meter_status_bar;       ///< graphically indicates meter values

    static std::map<int, bool>      s_expanded_map;         ///< map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false)
};

#endif
