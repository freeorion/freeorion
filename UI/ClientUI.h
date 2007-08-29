// -*- C++ -*-
#ifndef _ClientUI_h_
#define _ClientUI_h_

#include "../universe/Enums.h"
#include "../util/Random.h"
#include "../util/SitRepEntry.h"

#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>

class ClientNetworking;
class Combat;
class Fleet;
class IntroScreen;
class MapWnd;
class SitRepEntry;
struct SaveGameUIData;
class System;
class Tech;
class TurnProgressWnd;
class BuildingType;
namespace GG {
    class Clr;
    class SubTexture;
    class Texture;
}
namespace log4cpp {
    class Category;
}

//! \brief ClientUI Main Module
//!This is the main class of the ClientUI module.
//!it contains objects of many other classes and controls the
//!display of all information onscreen.
class ClientUI
{
public:
    //! \name Structors //!@{
    ClientUI();     //!< construction (calls Initialize())
    ~ClientUI();    //!< destruction (calls Cleanup())
    //!@}
    
    //! \name Accessors //!@{
    MapWnd* GetMapWnd();                                //!< Returns the main map window (may be null).
    void GetSaveGameUIData(SaveGameUIData& data) const; //!< populates the relevant UI state that should be restored after a save-and-load cycle
    //!@}

    //! \name Mutators //!@{
    void InitTurn(int turn_number);      //!< resets all active controls to use the latest data when it has been changed at the beginning of a new turn
    void RestoreFromSaveData(const SaveGameUIData& elem); ///< restores the UI state that was saved in an earlier call to GetSaveGameUIData().

    void ShowMap();       //!< Makes the Map Screen visible

    bool ZoomToPlanet(int id);  //!< Zooms to a particular planet on the galaxy map and opens the planet screen for that planet
    bool ZoomToSystem(int id);  //!< Zooms to a particular system on the galaxy map
    bool ZoomToFleet(int id);   //!< Zooms to a particular fleet on the galaxy map and opens the fleet window
    bool ZoomToShip(int id);    //!< Zooms to a particular ship on the galaxy map and opens its fleet and/or ship window
    bool ZoomToTech(const std::string& tech_name); //!< Opens the technology screen and presents a description of the given technology
    bool ZoomToBuildingType(const std::string& building_type_name); //!< Opens the building type screen and presents a description of the given building type
    bool ZoomToEncyclopediaEntry(const std::string& str); //!< Opens the encyclodedia screen and presents the entry for the given item
    void ZoomToSystem(System* system); //!< Zooms to a particular system on the galaxy map
    void ZoomToFleet(Fleet* fleet);    //!< Zooms to a particular fleet on the galaxy map and opens the fleet window

    /** Loads a texture at random from the set of files starting with \a prefix in directory \a dir. */
    boost::shared_ptr<GG::Texture> GetRandomTexture(const boost::filesystem::path& dir, const std::string& prefix);

    /** Loads texture \a n % N from the set of files starting with \a prefix in directory \a dir, where N is the number
        of files found in \a dir with prefix \a prefix. */
    boost::shared_ptr<GG::Texture> GetModuloTexture(const boost::filesystem::path& dir, const std::string& prefix, int n);
    //!@}
    
    static ClientUI*    GetClientUI();   //!< returns a pointer to the singleton ClientUI class

    /** shows a message dialog box with the given message; if \a play_alert_sound is true, and UI sound effects are
        currently enabled, the default alert sound will be played as the message box opens */
    static void MessageBox(const std::string& message, bool play_alert_sound = false);

    static void GenerateSitRepText(SitRepEntry *sit_rep); ///< generates a SitRep string from \a sit_rep.

    /** Loads the requested texture from file \a name; mipmap textures are generated if \a mipmap is true; loads default
        missing.png if name isn't found. */
    static boost::shared_ptr<GG::Texture> GetTexture(const boost::filesystem::path& path, bool mipmap = false);

    //!@{
    static boost::filesystem::path ArtDir();   //!< directory holding artwork
    static boost::filesystem::path SoundDir(); //!< directory holding sound and music

    static std::string  Font();            //!< The default font to use
    static std::string  FontBold();        //!< The default bold font to use
    static std::string  FontItalic();      //!< The default italic font to use
    static std::string  FontBoldItalic();  //!< The default bold and italic font to use
    static int          Pts();             //!< default point size
    static std::string  TitleFont();       //!< The default font to use for the window title
    static int          TitlePts();        //!< default point size to use for window title
    
    static GG::Clr      TextColor();       //!< color of UI text
    
    // generic UI windows
    static GG::Clr      WndColor();            //!< color of a UI window
    static GG::Clr      WndBorderColor();      //!< color of window borders
    static GG::Clr      WndOuterBorderColor(); //!< color of the outermost border
    static GG::Clr      WndInnerBorderColor(); //!< color of the innermost border

    // controls
    static GG::Clr      CtrlColor();           //!< color of UI controls
    static GG::Clr      CtrlBorderColor();

    static GG::Clr      ButtonColor();

    static GG::Clr      StateButtonColor();

    static GG::Clr      ScrollTabColor();
    static  int         ScrollWidth();
    
    static GG::Clr      DropDownListIntColor();
    static GG::Clr      DropDownListArrowColor();

    static GG::Clr      EditHiliteColor();
    static GG::Clr      EditIntColor();
    static GG::Clr      MultieditIntColor();

    static GG::Clr      StatIncrColor();   //!< used to color increasing stats text (eg "+2")
    static GG::Clr      StatDecrColor();   //!< used to color decreasing stats text (eg "-3")

    static int          SystemIconSize();                //!< the width/height of a System/Icon at zoom = 1.0
    static double       FleetButtonSize();               //!< the width/height of a FleetButton, relative to the size of a SystemIcon
    static double       SystemSelectionIndicatorSize();  //!< the width/height of a System Selection Indicator, relative to the size of a SystemIcon

    // SidePanel
    static GG::Clr      SidePanelColor();

    // Content Texture Getters
    static boost::shared_ptr<GG::Texture>
                        ShipIcon(int design_id);
    static boost::shared_ptr<GG::Texture>
                        BuildingTexture(const std::string& building_type_name);
    static boost::shared_ptr<GG::Texture> 
                        CategoryIcon(const std::string& category_name);
    static boost::shared_ptr<GG::Texture>
                        TechTexture(const std::string& tech_name);
    static boost::shared_ptr<GG::Texture>
                        SpecialTexture(const std::string& special_name);
    static boost::shared_ptr<GG::Texture>
                        MeterIcon(MeterType meter_type);

    // research screen
    static GG::Clr      KnownTechFillColor();
    static GG::Clr      KnownTechTextAndBorderColor();
    static GG::Clr      ResearchableTechFillColor();
    static GG::Clr      ResearchableTechTextAndBorderColor();
    static GG::Clr      UnresearchableTechFillColor();
    static GG::Clr      UnresearchableTechTextAndBorderColor();
    static GG::Clr      TechWndProgressBarBackground();
    static GG::Clr      TechWndProgressBar();

    static GG::Clr      CategoryColor(const std::string& category_name);

    static std::map<StarType, std::string>& StarTypeFilePrefixes();
    static std::map<StarType, std::string>& HaloStarTypeFilePrefixes();
    //!@}

private:
    typedef std::pair<std::vector<boost::shared_ptr<GG::Texture> >, boost::shared_ptr<SmallIntDistType> > TexturesAndDist;
    typedef std::map<std::string, TexturesAndDist> PrefixedTextures;
    TexturesAndDist PrefixedTexturesAndDist(const boost::filesystem::path& dir, const std::string& prefix);

    MapWnd*           m_map_wnd;           //!< the galaxy map

    PrefixedTextures  m_prefixed_textures;

    static ClientUI*  s_the_UI;            //!< pointer to the one and only ClientUI object
};

/** This exists as a way of allowing UI colors to be specified on the command line with one option and "(r,g,b,a)", instead of one option per color
    channel.  GG::Clr is not streamable using the normal means, due to what appears to be a bug in MSVC 7.1. */
struct StreamableColor
{
    StreamableColor();
    StreamableColor(const GG::Clr& clr);
    GG::Clr ToClr() const;
    int r, g, b, a;
};
std::ostream& operator<<(std::ostream& os, const StreamableColor& clr);
std::istream& operator>>(std::istream& is, StreamableColor& clr);

/** Wraps boost::format such that it won't crash if passed the wrong number of arguments */
boost::format FlexibleFormat(const std::string &string_to_format);

extern const double SMALL_UI_DISPLAY_VALUE;
extern const double LARGE_UI_DISPLAY_VALUE;
extern const double UNKNOWN_UI_DISPLAY_VALUE;

int EffectiveSign(double val, bool integerize);     ///< returns sign of value, accounting for SMALL_UI_DISPLAY_VALUE: +1 for positive values and -1 for negative values if their absolute value is larger than SMALL VALUE, and returns 0 for zero values or values with absolute value less than SMALL_UI_DISPLAY_VALUE
std::string DoubleToString(double val, int digits, bool integerize, bool showsign); ///< converts double to string with \a digits significant figures.  Represents large or small numbers with SI prefixes.

#endif // _ClientUI_h_
