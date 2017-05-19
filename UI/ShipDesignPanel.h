#ifndef _ShipDesignPanel_h_
#define _ShipDesignPanel_h_

#include <GG/GGFwd.h>
#include <GG/Control.h>

#include "DesignWnd.h"

/** Represents a ShipDesign */
class ShipDesignPanel : public GG::Control {
public:
    /** \name Structors */ //@{
    ShipDesignPanel(GG::X w, GG::Y h, int design_id);
    //@}

    /** \name Accessors */ //@{
    int                     DesignID() const {return m_design_id;}
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Render() override;

    void                    Update();
    void                    SetAvailability(const Availability::Enum type);

    //@}

private:
    int                     m_design_id;        ///< id for the ShipDesign this panel displays

protected:
    GG::StaticGraphic*      m_graphic;
    GG::Label*              m_name;
};

#endif
