// -*- C++ -*-
#ifndef _FleetButton_h_
#define _FleetButton_h_

#include <GG/Button.h>
#include "CUIDrawUtil.h"

class Fleet;
class FleetWnd;
class UniverseObject;

/** represents one or more fleets of an empire at a location on the map. */
class FleetButton : public GG::Button
{
public:
    enum SizeType {
        FLEET_BUTTON_TINY,
        FLEET_BUTTON_SMALL,
        FLEET_BUTTON_LARGE
    };

    /** \name Structors */ //@{
    FleetButton(const std::vector<int>& fleet_IDs, SizeType size = FLEET_BUTTON_SMALL);
    FleetButton(int fleet_id, SizeType size = FLEET_BUTTON_SMALL);
    //@}

    /** \name Accessors */ //@{
    virtual bool                InWindow(const GG::Pt& pt) const;   ///< returns true if \a pt is within or over the button

    const std::vector<Fleet*>&  Fleets() const {return m_fleets;}   ///< returns the fleets represented by this control
    //@}

    /** \name Mutators */ //@{
    virtual void                MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    //@}

protected:
    /** \name Mutators */ //@{
    virtual void                RenderUnpressed();
    virtual void                RenderPressed();
    virtual void                RenderRollover();
    //@}

private:
    void Init(const std::vector<int>& fleet_IDs, SizeType size);

    std::vector<Fleet*>             m_fleets;       ///< the fleets represented by this button
    boost::shared_ptr<GG::Texture>  m_head_icon;    ///< icon texture representing type of fleet
    boost::shared_ptr<GG::Texture>  m_size_icon;    ///< icon texture representing number of ships in fleet
};

#endif // _FleetButton_h_
