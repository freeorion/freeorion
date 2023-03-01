#ifndef _SpecialsPanel_h_
#define _SpecialsPanel_h_

#include <GG/Wnd.h>

class StatisticIcon;

/** Displays a set of specials attached to an UniverseObject */
class SpecialsPanel : public GG::Wnd {
public:
    SpecialsPanel(GG::X w, int object_id);
    void CompleteConstruction() override;

    bool InWindow(GG::Pt pt) const override;
    int  ObjectID() const { return m_object_id; }

    void Render() noexcept override {}
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void Update();          ///< regenerates indicators according specials on object

private:
    const int m_object_id; ///< id for the Object whose specials this panel displays
    std::vector<std::shared_ptr<StatisticIcon>> m_icons;
};

#endif
