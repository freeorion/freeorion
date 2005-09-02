// -*- C++ -*-
#ifndef _BuildDesignatorWnd_h_
#define _BuildDesignatorWnd_h_

#include "GGWnd.h"
#include "../universe/Enums.h"

class SidePanel;

class BuildDesignatorWnd : public GG::Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (BuildType, const std::string&, int, int)> AddBuildToQueueSignalType; ///< emitted when the indicated build is indicated by the user
    //@}

    /** \name Slot Types */ //@{
    typedef AddBuildToQueueSignalType::slot_type AddBuildToQueueSlotType; ///< type of functor(s) invoked on a BuildPickedSignalType
    //@}

    /** \name Structors */ //@{
    BuildDesignatorWnd(int w, int h);
    //@}

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;
    virtual bool InClient(const GG::Pt& pt) const;

    GG::Rect MapViewHole() const;
    //@}

    /** \name Mutators */ //@{
    void CenterOnBuild(/* TODO */);
    void SelectSystem(int system);
    void SelectPlanet(int planet);
    void Reset();
    void Clear();
    //@}

    mutable AddBuildToQueueSignalType AddBuildToQueueSignal;

private:
    class BuildDetailPanel;
    class BuildSelector;

    void BuildDesignatorWnd::BuildItemRequested(BuildType build_type, const std::string& item);

    BuildDetailPanel* m_build_detail_panel;
    BuildSelector*    m_build_selector;
    SidePanel*        m_side_panel;
    int               m_build_location;
    GG::Rect          m_map_view_hole;
};

inline std::pair<std::string, std::string> BuildDesignatorWndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _BuildDesignatorWnd_h_
