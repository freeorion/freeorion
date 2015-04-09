// -*- C++ -*-
#ifndef _SpecialsPanel_h_
#define _SpecialsPanel_h_

#include <GG/Wnd.h>

class StatisticIcon;

/** Displays a set of specials attached to an UniverseObject */
class SpecialsPanel : public GG::Wnd {
public:
    /** \name Structors */ //@{
    SpecialsPanel(GG::X w, int object_id);   ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    bool                    InWindow(const GG::Pt& pt) const;
    int                     ObjectID() const { return m_object_id; }
    //@}

    /** \name Mutators */ //@{
    virtual void            Render();
    virtual void            MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void            SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual bool            EventFilter(GG::Wnd* w, const GG::WndEvent& event);

    void                    Update();          ///< regenerates indicators according specials on object
    //@}

private:
    void                    SpecialRightClicked(const std::string& name);

    int                                     m_object_id;        ///< id for the Object whose specials this panel displays
    std::map<std::string, StatisticIcon*>   m_icons;
};

#endif
