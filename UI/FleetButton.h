// -*- C++ -*-
#ifndef _FleetButton_h_
#define _FleetButton_h_

#include <GG/Button.h>
#include "CUIDrawUtil.h"

class Fleet;
class FleetWnd;
class UniverseObject;

/** represents a group of same-empire fleets at the same location.  Clicking a FleetButton brings up a fleet window 
    from which the user can view and/or give orders to the fleets shown. */
class FleetButton : public GG::Button
{
public:
    /** \name Structors */ //@{
    FleetButton(GG::Clr color, const std::vector<int>& fleet_IDs); ///< basic  ctor
    FleetButton(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Clr color, const std::vector<int>& fleet_IDs); ///< ctor to specify size and location
    //@}

    /** \name Accessors */ //@{
    virtual bool           InWindow(const GG::Pt& pt) const;

    const std::vector<Fleet*>& Fleets() const {return m_fleets;} ///< returns the fleets represented by this control

    /** returns the orientation of the fleet marker (will be one of SHAPE_LEFT ans SHAPE_RIGHT) */
    ShapeOrientation Orientation() const {return m_orientation;}
    //@}

    /** \name Mutators */ //@{
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);

    /** sets the orientation of the fleet marker (must be one of SHAPE_LEFT and SHAPE_RIGHT; 
        otherwise, SHAPE_LEFT will be used) */
    void SetOrientation(ShapeOrientation orientation) {m_orientation = orientation;}

protected:
    /** \name Mutators */ //@{
    virtual void   RenderUnpressed();
    virtual void   RenderPressed();
    virtual void   RenderRollover();
    //@}

private:
    std::vector<Fleet*> m_fleets;   ///< the fleets represented by this button
    ShapeOrientation m_orientation;
};

#endif // _FleetButton_h_
