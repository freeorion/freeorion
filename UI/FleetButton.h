// -*- C++ -*-
//FleetButton.h
#ifndef _FleetButton_h_
#define _FleetButton_h_

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

#ifndef _CUIDrawUtil_h_
#include "CUIDrawUtil.h"
#endif

class FleetWnd;

/** represents a group of same-empire fleets at the same location.  Clicking a FleetButton brings up a fleet window 
    from which the user can view and/or give orders to the fleets shown. */
class FleetButton : public GG::Button
{
public:
    /** \name Structors */ //@{
    FleetButton(int x, int y, int w, int h, GG::Clr color, const std::vector<int>& fleet_IDs, ShapeOrientation orientation); ///< basic ctor
    FleetButton(const GG::XMLElement& elem); ///< ctor that constructs a FleetButton object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a FleetButton object
    //@}

    /** \name Accessors */ //@{
    virtual bool           InWindow(const GG::Pt& pt) const;
    virtual GG::XMLElement XMLEncode() const;

    /** returns the orientation of the fleet marker (will be one of SHAPE_LEFT ans SHAPE_RIGHT) */
    ShapeOrientation Orientation() const {return m_orientation;}
    //@}

    /** \name Mutators */ //@{
    /** sets the orientation of the fleet marker (must be one of SHAPE_LEFT and SHAPE_RIGHT; 
        otherwise, SHAPE_LEFT will be used) */
    void SetOrientation(ShapeOrientation orientation) {m_orientation = orientation;}
    //@}

protected:
    /** \name Mutators */ //@{
    virtual void   RenderUnpressed();
    virtual void   RenderPressed();
    virtual void   RenderRollover();
    //@}

private:
    void Clicked();

    std::vector<int> m_fleet_ids;   ///< the ID numbers of the fleets represented by this button
    ShapeOrientation m_orientation;

    static void FleetIsBeingExamined(int id);
    static void FleetIsNotBeingExamined(int id);

    static std::set<int> s_open_fleet_ids;
};

#endif // _FleetButton_h_
