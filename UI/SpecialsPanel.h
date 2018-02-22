#ifndef _SpecialsPanel_h_
#define _SpecialsPanel_h_

#include <GG/Wnd.h>

class StatisticIcon;

/** Displays a set of specials attached to an UniverseObject */
class SpecialsPanel : public GG::Wnd {
public:
    /** \name Structors */ //@{
    SpecialsPanel(GG::X w, int object_id);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    bool InWindow(const GG::Pt& pt) const override;
    int  ObjectID() const { return m_object_id; }
    //@}

    /** \name Mutators */ //@{
    void Render() override;
    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;
    void Update();          ///< regenerates indicators according specials on object
    //@}

private:
    void SpecialRightClicked(const std::string& name);

    int                                                     m_object_id;    ///< id for the Object whose specials this panel displays
    std::map<std::string, std::shared_ptr<StatisticIcon>>   m_icons;
};

#endif
