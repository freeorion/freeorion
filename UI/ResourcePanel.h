// -*- C++ -*-
#ifndef _ResourcePanel_h_
#define _ResourcePanel_h_

#include "AccordionPanel.h"

class MultiIconValueIndicator;
class MultiMeterStatusBar;
class StatisticIcon;

/** Shows resource meters with meter-bars */
class ResourcePanel : public AccordionPanel {
public:
    /** \name Structors */ //@{
    ResourcePanel(GG::X w, int object_id);
    ~ResourcePanel();
    //@}

    /** \name Accessors */ //@{
    int             ResourceCenterID() const {return m_rescenter_id;}
    //@}

    /** \name Mutators */ //@{
    void            ExpandCollapse(bool expanded); ///< expands or collapses panel to show details or just summary info

    virtual void    Render();
    virtual void    MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    void            Update();                       ///< updates indicators with values of associated object.  Does not do layout and resizing.
    void            Refresh();                      ///< updates, redoes layout, resizes indicator

    /** Enables, or disables if \a enable is false, issuing orders via this ResourcePanel. */
    void            EnableOrderIssuing(bool enable = true);
    //@}

private:
    void            ExpandCollapseButtonPressed();      ///< toggles panel expanded or collapsed
    void            DoLayout();                         ///< resizes panel and positions widgets

    int                         m_rescenter_id;         ///< object id for the UniverseObject that is also a PopCenter which is being displayed in this panel

    StatisticIcon*              m_industry_stat;        ///< icon and number of industry production
    StatisticIcon*              m_research_stat;        ///< icon and number of research production
    StatisticIcon*              m_trade_stat;           ///< icon and number of trade production
    StatisticIcon*              m_construction_stat;    ///< icon and number of infrastructure

    MultiIconValueIndicator*    m_multi_icon_value_indicator;   ///< textually / numerically indicates resource production and construction meter
    MultiMeterStatusBar*        m_multi_meter_status_bar;       ///< graphically indicates meter values

    static std::map<int, bool>  s_expanded_map;                 ///< map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false)
};

#endif
