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

class Fleet;
class FleetWnd;

/** represents a group of same-empire fleets at the same location.  Clicking a FleetButton brings up a fleet window 
    from which the user can view and/or give orders to the fleets shown. */
class FleetButton : public GG::Button
{
public:
    /** \name Structors */ //@{
    FleetButton(GG::Clr color, const std::vector<int>& fleet_IDs, double zoom); ///< ctor for moving fleets
    FleetButton(int x, int y, int w, int h, GG::Clr color, const std::vector<int>& fleet_IDs, ShapeOrientation orientation); ///< ctor for a fleets at a System
    FleetButton(const GG::XMLElement& elem); ///< ctor that constructs a FleetButton object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a FleetButton object
    //@}

    /** \name Accessors */ //@{
    virtual bool           InWindow(const GG::Pt& pt) const;
    virtual GG::XMLElement XMLEncode() const;

    const std::vector<Fleet*>& Fleets() const {return m_fleets;} ///< returns the ID numbers of the fleets represented by this control

    /** returns the orientation of the fleet marker (will be one of SHAPE_LEFT ans SHAPE_RIGHT) */
    ShapeOrientation Orientation() const {return m_orientation;}
    //@}

    /** \name Mutators */ //@{
    /** sets the orientation of the fleet marker (must be one of SHAPE_LEFT and SHAPE_RIGHT; 
        otherwise, SHAPE_LEFT will be used) */
    void SetOrientation(ShapeOrientation orientation) {m_orientation = orientation;}

    /** sets the FleetButton that represents moving fleets at the same system as this button (if this button represents stationary fleets at a system)
        or stationary fleets at the same system as this button (if this button represents moving fleets at a system) */
    void SetCompliment(FleetButton* compliment) {m_compliment = compliment;}
    //@}

protected:
    /** \name Mutators */ //@{
    virtual void   RenderUnpressed();
    virtual void   RenderPressed();
    virtual void   RenderRollover();
    //@}

private:
    void Clicked();

    std::vector<Fleet*> m_fleets;   ///< the fleets represented by this button
    ShapeOrientation m_orientation;

    /** the FleetButton that represents the other moving or stationary fleets at the same system as this button 
        (if this one is stationary or moving, respectively) */
    FleetButton* m_compliment;

    static void FleetIsBeingExamined(Fleet* fleet);
    static void FleetIsNotBeingExamined(Fleet* fleet);

    static std::set<Fleet*> s_open_fleets;
};

#endif // _FleetButton_h_
