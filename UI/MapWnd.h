//MapWnd.h
#ifndef _MapWnd_h_
#define _MapWnd_h_

#include <vector>

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGTexture_h_
#include "GGTexture.h"
#endif

#define GMAP_NUM_BACKGROUNDS 3

class SystemIcon;
class Icon;
/**
    This class is a huge window that stores EVERYTHING in the whole universe graphically
*/
class MapWnd : public GG::Wnd
{
public:
    static const int MAX_SCALE_FACTOR;
    
    typedef std::vector<SystemIcon*> StarControlVec;
    typedef std::vector<Icon*> IconVec;
    
    enum     //!< enumeration for indices to the icon vector
    {
        FLEET_ICON          = 0,
        NUTRIENTS_ICON      = 1,
        SCIENCE_ICON        = 2,
        MINERALS_ICON       = 3,
        INDUSTRY_ICON       = 4,
        POPULATION_ICON     = 5
    };
    
public:

    //! \name Structors //!@{
    MapWnd();    //!< default ctor
    ~MapWnd();   //!< default dtor
    
    //!@}
    
    //! \name Accessors //!@{

    //!@}
    
    //! \name Mutators //!@{
    virtual int Render();    //!< custom rendering code
    void InitTurn();        //!< called at the start of each turn by
    virtual int LDrag (const GG::Pt &pt, const GG::Pt &move, Uint32 keys); //!< override this code so we don't drag too far
    //!@}

private:
    //! \name Private methods //!@{
    void RenderBackgrounds();    //!< this renders the backgrounds onto the screen
    void MoveBackgrounds(const GG::Pt& move);        //!< this scrolls the backgrounds at their respective rates
    
    //!@}
    
private:
    //! \name Background Data Storage //!@{
    // TODO: Convert this to a "Background" class
    GG::Texture*    m_background[GMAP_NUM_BACKGROUNDS];  //!< array, 3 layers of backgrounds
    float           m_bg_scroll_rate[GMAP_NUM_BACKGROUNDS]; //!< array, the rates at which each background scrolls
    float           m_bg_position_X[GMAP_NUM_BACKGROUNDS]; //!< array, the X position of the first full background image
    float           m_bg_position_Y[GMAP_NUM_BACKGROUNDS]; //!< array, the Y positions of the backgrounds
    
    //!@}
    StarControlVec  m_stars;          //! a vector of StarControls
    
    IconVec         m_icons;          //! a list of icons

friend class GalaxyMapScreen;    //this is basically a part of that screen anyway
};

#endif
