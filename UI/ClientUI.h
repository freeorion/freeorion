#ifndef _ClientUI_h_
#define _ClientUI_h_


#include <GG/GGFwd.h>

#include <boost/filesystem/path.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/container/flat_map.hpp>

#include "SaveFileDialog.h"
#include "../universe/EnumsFwd.h"
#include "../util/Random.h"

#include <map>
#include <memory>


class Fleet;
class IntroScreen;
class MapWnd;
class MessageWnd;
class PlayerListWnd;
class MultiPlayerLobbyWnd;
class PasswordEnterWnd;
struct SaveGameUIData;
class System;
class ShipDesignManager;

//! \brief ClientUI Main Module
//!This is the main class of the ClientUI module.
//!it contains objects of many other classes and controls the
//!display of all information onscreen.
class ClientUI {
public:
    ClientUI();
    ~ClientUI();

    MapWnd*                                 GetMapWnd(bool construct);  //!< Returns the main map window. if \a is true, creates a MapWnd if one doesn't already exist. if \a is false, may return nullptr
    const MapWnd*                           GetMapWndConst() const noexcept { return m_map_wnd.get(); }
    std::shared_ptr<MapWnd>                 GetMapWndShared();          //!< Returns the main map window, constructing one if not already available
    std::shared_ptr<MessageWnd>             GetMessageWnd();            //!< Returns the chat / message window.
    std::shared_ptr<PlayerListWnd>          GetPlayerListWnd();         //!< Returns the players list window.
    std::shared_ptr<IntroScreen>            GetIntroScreen();           //!< Returns the intro screen / splash window.
    std::shared_ptr<MultiPlayerLobbyWnd>    GetMultiPlayerLobbyWnd();   //!< Returns the multiplayer lobby window.
    std::shared_ptr<PasswordEnterWnd>       GetPasswordEnterWnd();      //!< Returns the authentication window.
    std::shared_ptr<SaveFileDialog>         GetSaveFileDialog();        //!< Returns a perhaps nullptr to any existing SaveFileDialog
    ShipDesignManager*                      GetShipDesignManager() { return m_ship_designs.get(); };
    void                                    GetSaveGameUIData(SaveGameUIData& data); //!< populates the relevant UI state that should be restored after a save-and-load cycle

    /** Return the IntroScreen. Hides the MapWnd, MessageWnd and
      * PlayerListWnd if visible, but doesn't create them just to hide them. **/
    void ShowIntroScreen();
    void ShowMultiPlayerLobbyWnd();

    /** Creates a SaveFileDialog with \p purpose, Save or Load and \p type,
        SinglePlayer or MultiPlayer if one does not already exist. Returns the
        filename provided by the player, if any.*/
    std::string GetFilenameWithSaveFileDialog(const SaveFileDialog::Purpose purpose,
                                              const SaveFileDialog::SaveType type);

    void RestoreFromSaveData(const SaveGameUIData& elem);   //!< restores the UI state that was saved in an earlier call to GetSaveGameUIData().

    bool ZoomToObject(const std::string& name);
    bool ZoomToObject(int id);
    bool ZoomToPlanet(int id);      //!< Zooms to a particular planet on the galaxy map and opens the sidepanel to show it, or if production screen is open selects it
    bool ZoomToPlanetPedia(int id); //!< Opens the encyclodedia window and presents the entry for the given planet
    bool ZoomToSystem(int id);      //!< Zooms to a particular system on the galaxy map and opens the sidepanel to show it
    bool ZoomToFleet(int id);       //!< Zooms to a particular fleet on the galaxy map and opens the fleet window
    bool ZoomToShip(int id);        //!< Zooms to a particular ship on the galaxy map and opens its fleet and/or ship window
    bool ZoomToBuilding(int id);    //!< Zooms to a particular building on the galaxy map and opens the sidepanel to show it
    bool ZoomToField(int id);       //!< Zooms to a particular field on the map
    bool ZoomToCombatLog(int id);   //!< Opens combat log for indicated combat

    void ZoomToSystem(std::shared_ptr<const System> system);//!< Zooms to a system on the galaxy map
    void ZoomToFleet(std::shared_ptr<const Fleet> fleet);   //!< Zooms to a particular fleet on the galaxy map and opens the fleet window

    bool ZoomToContent(const std::string& name, bool reverse_lookup = false);
    bool ZoomToTech(std::string tech_name);                  //!< Opens the technology screen and presents a description of the given technology
    bool ZoomToPolicy(std::string policy_name);              //!< ???
    bool ZoomToBuildingType(std::string building_type_name); //!< Opens the production screen and presents a description of the given building type
    bool ZoomToSpecial(std::string special_name);            //!< Opens the ??? screen and presents a description of the given special
    bool ZoomToShipHull(std::string hull_name);              //!< Opens the design screen and presents a description of the given hull type
    bool ZoomToShipPart(std::string part_name);              //!< Opens the design screen and presents a description of the given part type
    bool ZoomToSpecies(std::string species_name);            //!< Opens the ??? screen and presents a description of the given species
    bool ZoomToFieldType(std::string field_type_name);       //!< Opens the ??? screen and presents a description of the given field type

    bool ZoomToShipDesign(int design_id);                    //!< Opens the design screen and presents a description of the given ship design
    bool ZoomToEmpire(int empire_id);                        //!< Opens the ??? screen and presents a description of the given empire
    bool ZoomToMeterTypeArticle(std::string meter_string);   //!< Opens the encyclopedia and presents the entry for MeterType @a meter_string
    bool ZoomToMeterTypeArticle(MeterType meter_type);       //!< Opens the encyclopedia and presents the entry for MeterType @a meter_type
    bool ZoomToEncyclopediaEntry(std::string str);           //!< Opens the encyclodedia window and presents the entry for the given term

    void DumpObject(int object_id);                                 //!< Displays debug info about specified object in messages window

    void InitializeWindows();

    /** Loads a texture at random from the set of files starting with \a prefix
        in directory \a dir. */
    std::shared_ptr<GG::Texture> GetRandomTexture(const boost::filesystem::path& dir,
                                                  std::string_view prefix,
                                                  bool mipmap = false);

    /** Loads texture \a n % N from the set of files starting with \a prefix in
        directory \a dir, where N is the number of files found in \a dir with
        prefix \a prefix. */
    std::shared_ptr<GG::Texture> GetModuloTexture(const boost::filesystem::path& dir,
                                                  std::string_view prefix, int n,
                                                  bool mipmap = false);

    /** Returns all textures in the set of files starting with \a prefix in
        directory \a dir. */
    const std::vector<std::shared_ptr<GG::Texture>>& GetPrefixedTextures(
        const boost::filesystem::path& dir, std::string_view prefix, bool mipmap = false);

    static ClientUI* GetClientUI();     //!< returns a pointer to the singleton ClientUI class

    /** Shows a message dialog box with the given message; if
      * \a play_alert_sound is true, and UI sound effects are currently enabled,
      * the default alert sound will be played as the message box opens */
    static void MessageBox(const std::string& message, bool play_alert_sound = false);

    /** Loads the requested texture from file \a name; mipmap textures are
      * generated if \a mipmap is true; loads default missing.png if name isn't
      * found. */
    static std::shared_ptr<GG::Texture> GetTexture(const boost::filesystem::path& path,
                                                   bool mipmap = false);

    /** Returns the default font in the specified point size. Uses "ui.font.path"
      * option setting as the font filename, and provides Unicode character sets
      * based on the contents of the stringtable in use. */
    static std::shared_ptr<GG::Font> GetFont(int pts = Pts());

    /** Returns the default font in the specified point size.  Uses
      * "ui.font.bold.path" option setting as the font filename, and provides
      * Unicode character sets based on the contents of the stringtable in use.
      * */
    static std::shared_ptr<GG::Font> GetBoldFont(int pts = Pts());

    /** Returns the default font in the specified point size.  Uses
      * "ui.font.title.path" option setting as the font filename, and provides
      * Unicode character sets based on the contents of the stringtable in use. */
    static std::shared_ptr<GG::Font> GetTitleFont(int pts = TitlePts());

    /** Returns formatted POSIX UTC-time in local timezone. */
    static std::string FormatTimestamp(boost::posix_time::ptime timestamp);

    //!@{
    static boost::filesystem::path ArtDir();    //!< directory holding artwork
    static boost::filesystem::path SoundDir();  //!< directory holding sound and music

    static int      Pts();                      //!< default point size
    static int      TitlePts();                 //!< default point size to use for window title
    static GG::Clr  TextColor();                //!< color of UI text
    static GG::Clr  DefaultLinkColor();         //!< default color of UI links
    static GG::Clr  RolloverLinkColor();        //!< rollover color of UI links
    static GG::Clr  DefaultTooltipColor();      //!< default color of UI text with hover tooltip
    static GG::Clr  RolloverTooltipColor();     //!< rollover color of UI text with hover tooltip
    static GG::Clr  WndColor();                 //!< background color of a UI window
    static GG::Clr  WndOuterBorderColor();      //!< color of the outermost border
    static GG::Clr  WndInnerBorderColor();      //!< color of the innermost border
    static GG::Clr  CtrlColor();                //!< background color of UI controls
    static GG::Clr  CtrlBorderColor();
    static GG::Clr  ButtonHiliteColor();
    static GG::Clr  ButtonHiliteBorderColor();
    static int      ScrollWidth();
    static GG::Clr  DropDownListArrowColor();
    static GG::Clr  EditHiliteColor();
    static GG::Clr  StatIncrColor();                    //!< used to color increasing stats text (eg "+2")
    static GG::Clr  StatDecrColor();                    //!< used to color decreasing stats text (eg "-3")
    static GG::Clr  StateButtonColor();                 //!< colour of selected state button markers
    static int      SystemIconSize();                   //!< the width/height of a System/Icon at zoom = 1.0
    static int      SystemTinyIconSizeThreshold();      //!< the width/height of a system icon below which the tiny system icons should be used
    static int      SystemCircleSize();                 //!< the width/height of the system-enclosing circle
    static int      SystemSelectionIndicatorSize();     //!< the width/height of a system selection indicator
    static int      SystemSelectionIndicatorRPM();      //!< revolutions per minute to rotate system selection indicator
    static GG::Clr  SystemNameTextColor();              //!< the colour of system names that aren't owned by any player (as far as this client knows)
    static double   TinyFleetButtonZoomThreshold();     //!< the minimum zoom level of the map at which to show tiny (any) fleet icons
    static double   SmallFleetButtonZoomThreshold();    //!< the minimum zoom level of the map at which to show small fleet icons
    static double   MediumFleetButtonZoomThreshold();   //!< the minimum zoom level of the map at which to show medium fleet icons
    static double   BigFleetButtonZoomThreshold();      //!< the minimum zoom level of the map at which to show big fleet icons
    static bool     DisplayTimestamp();                 //!< Will be timestamp shown in the chats.

    // Content Texture Getters
    static std::shared_ptr<GG::Texture> PlanetIcon(PlanetType planet_type);
    static std::shared_ptr<GG::Texture> PlanetSizeIcon(PlanetSize planet_size);
    static std::shared_ptr<GG::Texture> MeterIcon(MeterType meter_type);
    static std::shared_ptr<GG::Texture> BuildingIcon(std::string_view building_type_name);
    static std::shared_ptr<GG::Texture> CategoryIcon(std::string_view category_name);
    static std::shared_ptr<GG::Texture> TechIcon(std::string_view tech_name);
    static std::shared_ptr<GG::Texture> PolicyIcon(std::string_view policy_name);
    static std::shared_ptr<GG::Texture> SpecialIcon(std::string_view special_name);
    static std::shared_ptr<GG::Texture> SpeciesIcon(std::string_view species_name);
    static std::shared_ptr<GG::Texture> FieldTexture(std::string_view field_type_name);
    static std::shared_ptr<GG::Texture> PartIcon(std::string_view part_name);
    static std::shared_ptr<GG::Texture> HullTexture(std::string_view hull_name);
    static std::shared_ptr<GG::Texture> HullIcon(std::string_view hull_name);
    static std::shared_ptr<GG::Texture> ShipDesignIcon(int design_id);

    // research screen
    static GG::Clr  KnownTechFillColor();
    static GG::Clr  KnownTechTextAndBorderColor();
    static GG::Clr  ResearchableTechFillColor();
    static GG::Clr  ResearchableTechTextAndBorderColor();
    static GG::Clr  UnresearchableTechFillColor();
    static GG::Clr  UnresearchableTechTextAndBorderColor();
    static GG::Clr  TechWndProgressBarBackgroundColor();
    static GG::Clr  TechWndProgressBarColor();
    static GG::Clr  CategoryColor(std::string_view category_name);

    static std::string_view PlanetTypeFilePrefix(PlanetType planet_type) noexcept;
    static std::string_view StarTypeFilePrefix(StarType star_type) noexcept;
    static std::string_view HaloStarTypeFilePrefix(StarType star_type) noexcept;
    //!@}

private:
    void HandleSizeChange(bool fullscreen) const;
    void HandleFullscreenSwitch() const;

    mutable std::shared_ptr<MapWnd>         m_map_wnd;              //!< the galaxy map
    std::shared_ptr<MessageWnd>             m_message_wnd;          //!< the messages / chat display
    std::shared_ptr<PlayerListWnd>          m_player_list_wnd;      //!< the players list
    std::shared_ptr<IntroScreen>            m_intro_screen;         //!< splash screen / main menu when starting program
    std::shared_ptr<MultiPlayerLobbyWnd>    m_multiplayer_lobby_wnd;//!< the multiplayer lobby
    std::shared_ptr<SaveFileDialog>         m_savefile_dialog;
    std::shared_ptr<PasswordEnterWnd>       m_password_enter_wnd;   //!< the authentication window

    //!< map key represents a directory and first part of a texture filename.
    //!< when textures are looked up with GetPrefixedTextures, the specified
    //!< dir is searched for filenames that start with the prefix. pointers
    //!< to the Texture objects for these files are stored as the mapped value.
    boost::container::flat_map<std::string, std::vector<std::shared_ptr<GG::Texture>>, std::less<>>
                                            m_prefixed_textures;

    std::unique_ptr<ShipDesignManager>      m_ship_designs;         //!< ship designs the client knows about, and their ordering in the UI

    static constinit ClientUI*              s_the_UI;               //!< the singleton ClientUI object
};

namespace GG {
    std::istream& operator>>(std::istream& is, Clr& clr);
}

/** Increases the given value when font size is larger than 12 */
int FontBasedUpscale(int x);


#endif
