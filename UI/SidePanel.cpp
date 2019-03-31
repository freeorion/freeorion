#include "SidePanel.h"

#include "CUIWnd.h"
#include "CUIControls.h"
#include "MultiIconValueIndicator.h"
#include "SystemIcon.h"
#include "Sound.h"
#include "FleetWnd.h"
#include "BuildingsPanel.h"
#include "MilitaryPanel.h"
#include "PopulationPanel.h"
#include "ResourcePanel.h"
#include "MapWnd.h"
#include "ShaderProgram.h"
#include "SpecialsPanel.h"
#include "SystemResourceSummaryBrowseWnd.h"
#include "TextBrowseWnd.h"
#include "../universe/Predicates.h"
#include "../universe/ShipDesign.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Building.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Enums.h"
#include "../Empire/Empire.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/XMLDoc.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../client/human/HumanClientApp.h"

#include <GG/DrawUtil.h>
#include <GG/DynamicGraphic.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/Scroll.h>
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>

class RotatingPlanetControl;

namespace {
    const int       EDGE_PAD(3);
    std::map<std::pair<int,int>,float>          colony_projections;
    std::map<std::pair<std::string,int>,float>  species_colony_projections;

    /** @content_tag{CTRL_ALWAYS_BOMBARD} Select this ship during automatic ship selection for bombard, regardless of any tags **/
    const std::string TAG_BOMBARD_ALWAYS = "CTRL_ALWAYS_BOMBARD";
    /** @content_tag{CTRL_BOMBARD_} Prefix tag allowing automatic ship selection for bombard, must post-fix a valid planet tag **/
    const std::string TAG_BOMBARD_PREFIX = "CTRL_BOMBARD_";

    void        PlaySidePanelOpenSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.map.sidepanel.open.sound.path"), true); }

    struct RotatingPlanetData {
        RotatingPlanetData(const XMLElement& elem) {
            if (elem.Tag() != "RotatingPlanetData")
                throw std::invalid_argument("Attempted to construct a RotatingPlanetData from an XMLElement that had a tag other than \"RotatingPlanetData\"");

            planet_type = boost::lexical_cast<PlanetType>(elem.attributes.at("planet_type"));
            filename = elem.attributes.at("filename");
            shininess = boost::lexical_cast<double>(elem.attributes.at("shininess"));

            // ensure proper bounds
            shininess = std::max(0.0, std::min(shininess, 128.0));
        }

        PlanetType  planet_type;    ///< the type of planet for which this data may be used
        std::string filename;       ///< the filename of the image used to texture a rotating image
        double      shininess;      ///< the exponent of specular (shiny) reflection off of the planet; must be in [0.0, 128.0]
    };

    struct PlanetAtmosphereData {
        struct Atmosphere {
            Atmosphere() {}
            Atmosphere(const XMLElement& elem) {
                if (elem.Tag() != "Atmosphere")
                    throw std::invalid_argument("Attempted to construct an Atmosphere from an XMLElement that had a tag other than \"Atmosphere\"");
                filename = elem.attributes.at("filename");
                alpha = boost::lexical_cast<int>(elem.attributes.at("alpha"));
                alpha = std::max(0, std::min(alpha, 255));
            }

            std::string filename;
            int         alpha;
        };

        PlanetAtmosphereData() {}
        PlanetAtmosphereData(const XMLElement& elem) {
            if (elem.Tag() != "PlanetAtmosphereData")
                throw std::invalid_argument("Attempted to construct a PlanetAtmosphereData from an XMLElement that had a tag other than \"PlanetAtmosphereData\"");
            planet_filename = elem.attributes.at("planet_filename");
            for (const XMLElement& atmosphere : elem.Child("atmospheres").children) {
                atmospheres.push_back(Atmosphere(atmosphere));
            }
        }

        std::string             planet_filename; ///< the filename of the planet image that this atmosphere image data goes with
        std::vector<Atmosphere> atmospheres;     ///< the filenames of the atmosphere images suitable for use with this planet image
    };

    const std::map<PlanetType, std::vector<RotatingPlanetData>>&    GetRotatingPlanetData() {
        ScopedTimer timer("GetRotatingPlanetData", true);
        static std::map<PlanetType, std::vector<RotatingPlanetData>> data;
        if (data.empty()) {
            XMLDoc doc;
            try {
                boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
                doc.ReadDoc(ifs);
                ifs.close();
            } catch (const std::exception& e) {
                ErrorLogger() << "GetRotatingPlanetData: error reading artdir/planets/planets.xml: " << e.what();
            }

            if (doc.root_node.ContainsChild("GLPlanets")) {
                const XMLElement& elem = doc.root_node.Child("GLPlanets");
                for (const XMLElement& planet_definition : elem.children) {
                    if (planet_definition.Tag() == "RotatingPlanetData") {
                        try {
                            RotatingPlanetData current_data(planet_definition);
                            data[current_data.planet_type].push_back(current_data);
                        } catch(const std::exception& e) {
                            ErrorLogger() << "GetRotatingPlanetData: unable to load entry: " << e.what();
                        }
                    }
                }
            }
        }
        return data;
    }

    const std::map<std::string, PlanetAtmosphereData>&              GetPlanetAtmosphereData() {
        static std::map<std::string, PlanetAtmosphereData> data;
        if (data.empty()) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "atmospheres.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            for (const XMLElement& atmosphere_definition : doc.root_node.children) {
                if (atmosphere_definition.Tag() == "PlanetAtmosphereData") {
                    try {
                        PlanetAtmosphereData current_data(atmosphere_definition);
                        data[current_data.planet_filename] = current_data;
                    } catch (const std::exception& e) {
                        ErrorLogger() << "GetPlanetAtmosphereData: " << e.what();
                    }
                }
            }
        }
        return data;
    }

    double      GetAsteroidsFPS() {
        static double retval = -1.0;
        if (retval == -1.0) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("asteroids_fps"))
                retval = boost::lexical_cast<double>(doc.root_node.Child("asteroids_fps").Text());
            else
                retval = 15.0;

            retval = std::max(0.0, std::min(retval, 60.0));
        }
        return retval;
    }

    double      GetRotatingPlanetAmbientIntensity() {
        static double retval = -1.0;

        if (retval == -1.0) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLPlanets") && doc.root_node.Child("GLPlanets").ContainsChild("ambient_intensity"))
                retval = boost::lexical_cast<double>(doc.root_node.Child("GLPlanets").Child("ambient_intensity").Text());
            else
                retval = 0.5;

            retval = std::max(0.0, std::min(retval, 1.0));
        }

        return retval;
    }

    double      GetRotatingPlanetDiffuseIntensity() {
        static double retval = -1.0;

        if (retval == -1.0) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLPlanets") && doc.root_node.Child("GLPlanets").ContainsChild("diffuse_intensity"))
                retval = boost::lexical_cast<double>(doc.root_node.Child("GLPlanets").Child("diffuse_intensity").Text());
            else
                retval = 0.5;

            retval = std::max(0.0, std::min(retval, 1.0));
        }

        return retval;
    }

    void        RenderSphere(double r, const GG::Clr& ambient, const GG::Clr& diffuse,
                             const GG::Clr& spec, double shine,
                             std::shared_ptr<GG::Texture> texture)
    {
        static GLUquadric* quad = gluNewQuadric();
        if (!quad)
            return;

        if (texture) {
            glBindTexture(GL_TEXTURE_2D, texture->OpenGLId());
        }

        // commented out shininess rendering because it wasn't working properly.
        // it just appeared as a white blob, seemingly at the poles of the planet (but possibly not?)
        // regardless, IMO it didn't look good. -Geoff
        //if (shine) {
        //    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, static_cast<float>(shine));
        //    GLfloat spec_v[] = {spec.r / 255.0f, spec.g / 255.0f, spec.b / 255.0f, spec.a / 255.0f};
        //    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_v);
        //}

        GLfloat ambient_v[] = {ambient.r / 255.0f, ambient.g / 255.0f, ambient.b / 255.0f, ambient.a / 255.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_v);
        GLfloat diffuse_v[] = {diffuse.r / 255.0f, diffuse.g / 255.0f, diffuse.b / 255.0f, diffuse.a / 255.0f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_v);

        gluQuadricTexture(quad,     texture ? GL_TRUE : GL_FALSE);
        gluQuadricNormals(quad,     GLU_SMOOTH);
        gluQuadricOrientation(quad, GLU_OUTSIDE);
        gluQuadricDrawStyle(quad,   GLU_FILL);

        glColor(GG::CLR_WHITE);

        gluSphere(quad, r, 30, 30);
    }

    GLfloat*    GetLightPosition() {
        static GLfloat retval[] = {0.0, 0.0, 0.0, 0.0};

        if (retval[0] == 0.0 && retval[1] == 0.0 && retval[2] == 0.0) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            retval[0] = boost::lexical_cast<GLfloat>(doc.root_node.Child("GLPlanets").Child("light_pos").Child("x").Text());
            retval[1] = boost::lexical_cast<GLfloat>(doc.root_node.Child("GLPlanets").Child("light_pos").Child("y").Text());
            retval[2] = boost::lexical_cast<GLfloat>(doc.root_node.Child("GLPlanets").Child("light_pos").Child("z").Text());
        }

        return retval;
    }

    const std::map<StarType, std::vector<float>>& GetStarLightColors() {
        static std::map<StarType, std::vector<float>> light_colors;

        if (light_colors.empty()) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLStars") && 0 < doc.root_node.Child("GLStars").children.size()) {
                for (const XMLElement& star_definition : doc.root_node.Child("GLStars").children) {
                    try {
                        std::string hex_colour("#");
                        hex_colour.append(star_definition.attributes.at("color"));
                        std::vector<float>& color_vec = light_colors[boost::lexical_cast<StarType>(star_definition.attributes.at("star_type"))];
                        GG::Clr color = GG::HexClr(hex_colour);

                        color_vec.push_back(color.r / 255.0f);
                        color_vec.push_back(color.g / 255.0f);
                        color_vec.push_back(color.b / 255.0f);
                        color_vec.push_back(color.a / 255.0f);
                    } catch(const std::exception& e) {
                        std::cerr << "planets.xml: " << e.what() << std::endl;
                    }
                }
            } else {
                for (int i = STAR_BLUE; i < NUM_STAR_TYPES; ++i) {
                    light_colors[StarType(i)].resize(4, 1.0);
                }
            }
        }

        return light_colors;
    }

    const std::vector<float>& StarLightColour(StarType star_type) {
        static std::vector<float> white(4, 0.0f);
        const auto& colour_map = GetStarLightColors();
        auto it = colour_map.find(star_type);
        if (it != colour_map.end())
            return it->second;
        return white;
    }

    void        RenderPlanet(const GG::Pt& center, int diameter, std::shared_ptr<GG::Texture> texture,
                             std::shared_ptr<GG::Texture> overlay_texture,
                             double initial_rotation, double RPM, double axial_tilt, double shininess,
                             StarType star_type)
    {
        glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT | GL_SCISSOR_BIT);
        HumanClientApp::GetApp()->Exit2DMode();

        // slide the texture coords to simulate a rotating axis
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(initial_rotation - GG::GUI::GetGUI()->Ticks() / 1000.0 * RPM / 60.0, 0.0, 0.0);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, Value(HumanClientApp::GetApp()->AppWidth()),
                Value(HumanClientApp::GetApp()->AppHeight()), 0.0,
                0.0, Value(HumanClientApp::GetApp()->AppWidth()));

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);
        GLfloat* light_position = GetLightPosition();
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        const std::vector<float>& colour = StarLightColour(star_type);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, &colour[0]);
        glLightfv(GL_LIGHT0, GL_SPECULAR, &colour[0]);
        glEnable(GL_TEXTURE_2D);

        glTranslated(Value(center.x), Value(center.y), -(diameter / 2 + 1));// relocate to locatin on screen where planet is to be rendered
        glRotated(95.0, -1.0, 0.0, 0.0);                                    // make the poles upright, instead of head-on (we go a bit more than 90 degrees, to avoid some artifacting caused by the GLU-supplied texture coords)
        glRotated(axial_tilt, 0.0, 1.0, 0.0);                               // axial tilt

        float intensity = static_cast<float>(GetRotatingPlanetAmbientIntensity());
        GG::Clr ambient = GG::FloatClr(intensity, intensity, intensity, 1.0f);
        intensity = static_cast<float>(GetRotatingPlanetDiffuseIntensity());
        GG::Clr diffuse = GG::FloatClr(intensity, intensity, intensity, 1.0f);

        RenderSphere(diameter / 2, ambient, diffuse, GG::CLR_WHITE, shininess, texture);

        if (overlay_texture) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glCullFace(GL_FRONT);
            glEnable(GL_CULL_FACE);
            RenderSphere(diameter / 2 + 0.1, ambient, diffuse, GG::CLR_WHITE, 0.0, overlay_texture);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
        }

        glPopAttrib();

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        HumanClientApp::GetApp()->Enter2DMode();
        glPopAttrib();
    }

    int         MaxPlanetDiameter()
    { return GetOptionsDB().Get<int>("ui.map.sidepanel.planet.diameter.max"); }

    int         PlanetDiameter(PlanetSize size) {
        double scale = 0.0;
        switch (size) {
        case SZ_TINY      : scale = 1.0/7.0; break;
        case SZ_SMALL     : scale = 2.0/7.0; break;
        case SZ_MEDIUM    : scale = 3.0/7.0; break;
        case SZ_LARGE     : scale = 4.0/7.0; break;
        case SZ_HUGE      : scale = 5.0/7.0; break;
        case SZ_GASGIANT  : scale = 7.0/7.0; break;
        case SZ_ASTEROIDS : scale = 7.0/7.0; break;
        default           : scale = 3.0/7.0; break;
        }

        int MIN_PLANET_DIAMETER = GetOptionsDB().Get<int>("ui.map.sidepanel.planet.diameter.min");
        // sanity check
        if (MIN_PLANET_DIAMETER > MaxPlanetDiameter())
            MIN_PLANET_DIAMETER = MaxPlanetDiameter();

        return static_cast<int>(MIN_PLANET_DIAMETER + (MaxPlanetDiameter() - MIN_PLANET_DIAMETER) * scale) - 2 * EDGE_PAD;
    }

    /** Adds options related to sidepanel to Options DB. */
    void        AddOptions(OptionsDB& db) {
        db.Add("ui.map.sidepanel.planet.diameter.max",      UserStringNop("OPTIONS_DB_UI_SIDEPANEL_PLANET_MAX_DIAMETER"),
               128,                         RangedValidator<int>(16, 512));
        db.Add("ui.map.sidepanel.planet.diameter.min",      UserStringNop("OPTIONS_DB_UI_SIDEPANEL_PLANET_MIN_DIAMETER"),
               24,                          RangedValidator<int>(8,  128));
        db.Add("ui.map.sidepanel.planet.shown",             UserStringNop("OPTIONS_DB_UI_SIDEPANEL_PLANET_SHOWN"),
               true,                        Validator<bool>());
        db.Add("ui.map.sidepanel.planet.scanlane.color",    UserStringNop("OPTIONS_DB_UI_PLANET_FOG_CLR"),
               GG::Clr(0, 0, 0, 128),       Validator<GG::Clr>());
        db.Add("UI.sidepanel-planet-status-icon-size",       UserStringNop("OPTIONS_DB_UI_PLANET_STATUS_ICON_SIZE"),
               32,                          RangedValidator<int>(8, 128));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /** Returns map from planet ID to issued colonize orders affecting it. There
      * should be only one ship colonzing each planet for this client. */
    std::map<int, int> PendingColonizationOrders() {
        std::map<int, int> retval;
        const ClientApp* app = ClientApp::GetApp();
        if (!app)
            return retval;
        for (const auto& id_and_order : app->Orders()) {
            if (auto order = std::dynamic_pointer_cast<ColonizeOrder>(id_and_order.second)) {
                retval[order->PlanetID()] = id_and_order.first;
            }
        }
        return retval;
    }

    /** Returns map from planet ID to issued invasion orders affecting it. There
      * may be multiple ships invading a single planet. */
    std::map<int, std::set<int>> PendingInvadeOrders() {
        std::map<int, std::set<int>> retval;
        const ClientApp* app = ClientApp::GetApp();
        if (!app)
            return retval;
        for (const auto& id_and_order : app->Orders()) {
            if (auto order = std::dynamic_pointer_cast<InvadeOrder>(id_and_order.second)) {
                retval[order->PlanetID()].insert(id_and_order.first);
            }
        }
        return retval;
    }

    /** Returns map from planet ID to issued bombard orders affecting it. There
      * may be multiple ships bombarding a single planet. */
    std::map<int, std::set<int>> PendingBombardOrders() {
        std::map<int, std::set<int>> retval;
        const ClientApp* app = ClientApp::GetApp();
        if (!app)
            return retval;
        for (const auto& id_and_order : app->Orders()) {
            if (auto order = std::dynamic_pointer_cast<BombardOrder>(id_and_order.second)) {
                retval[order->PlanetID()].insert(id_and_order.first);
            }
        }
        return retval;
    }

    bool ClientPlayerIsModerator()
    { return HumanClientApp::GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR; }
}


/** A single planet's info and controls; several of these may appear at any
  * one time in a SidePanel */
class SidePanel::PlanetPanel : public GG::Control {
public:
    /** \name Structors */ //@{
    PlanetPanel(GG::X w, int planet_id, StarType star_type);

    ~PlanetPanel();
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    bool InWindow(const GG::Pt& pt) const override;

    int PlanetID() const { return m_planet_id; }
    //@}

    /** \name Mutators */ //@{
    void PreRender() override;

    void Render() override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Select(bool selected);

    void Refresh(); ///< updates panels, shows / hides colonize button, redoes layout of infopanels

    /** Enables, or disables if \a enable is false, issuing orders via this PlanetPanel. */
    void EnableOrderIssuing(bool enable = true);
    //@}

    /** emitted when the planet panel is left clicked by the user.
      * returns the id of the clicked planet */
    mutable boost::signals2::signal<void (int)> LeftClickedSignal;

    /** emitted when the planet is left double clicked by the user.
      * returns id of the clicked planet */
    mutable boost::signals2::signal<void (int)> LeftDoubleClickedSignal;

    /** emitted when the planet panel is right clicked by the user.
      * returns the id of the clicked planet */
    mutable boost::signals2::signal<void (int)> RightClickedSignal;

    /** emitted when resized, so external container can redo
      * layout */
    mutable boost::signals2::signal<void ()> ResizedSignal;

    /** emitted when focus is changed */
    mutable boost::signals2::signal<void (const std::string&)> FocusChangedSignal;

    mutable boost::signals2::signal<void (int)> BuildingRightClickedSignal;

    /** Emitted when an order button changes state, used to update controls
     *  of panels for other planets in the same system */
    mutable boost::signals2::signal<void (int)> OrderButtonChangedSignal;

private:
    void DoLayout();
    void RefreshPlanetGraphic();
    void SetFocus(const std::string& focus); ///< set the focus of the planet to \a focus
    void ClickColonize();                    ///< called if colonize button is pressed
    void ClickInvade();                      ///< called if invade button is pressed
    void ClickBombard();                     ///< called if bombard button is pressed

    void FocusDropListSelectionChangedSlot(GG::DropDownList::iterator selected); ///< called when droplist selection changes, emits FocusChangedSignal

    int                                     m_planet_id;                ///< id for the planet with is represented by this planet panel
    std::shared_ptr<GG::TextControl>        m_planet_name;              ///< planet name;
    std::shared_ptr<GG::Label>              m_env_size;                 ///< indicates size and planet environment rating uncolonized planets;
    std::shared_ptr<GG::Button>             m_colonize_button;          ///< btn which can be pressed to colonize this planet;
    std::shared_ptr<GG::Button>             m_invade_button;            ///< btn which can be pressed to invade this planet;
    std::shared_ptr<GG::Button>             m_bombard_button;           ///< btn which can be pressed to bombard this planet;
    std::shared_ptr<GG::DynamicGraphic>     m_planet_graphic;           ///< image of the planet (can be a frameset); this is now used only for asteroids;
    std::shared_ptr<GG::StaticGraphic>      m_planet_status_graphic;    ///< gives information about the planet status, like supply disconnection
    std::shared_ptr<RotatingPlanetControl>  m_rotating_planet_graphic;  ///< a realtime-rendered planet that rotates, with a textured surface mapped onto it
    bool                                    m_selected;                 ///< is this planet panel selected
    bool                                    m_order_issuing_enabled;    ///< can orders be issues via this planet panel?
    GG::Clr                                 m_empire_colour;            ///< colour to use for empire-specific highlighting.  set based on ownership of planet.
    std::shared_ptr<GG::DropDownList>       m_focus_drop;               ///< displays and allows selection of planetary focus;
    std::shared_ptr<PopulationPanel>        m_population_panel;         ///< contains info about population and health
    std::shared_ptr<ResourcePanel>          m_resource_panel;           ///< contains info about resources production and focus selection UI
    std::shared_ptr<MilitaryPanel>          m_military_panel;           ///< contains icons representing military-related meters
    std::shared_ptr<BuildingsPanel>         m_buildings_panel;          ///< contains icons representing buildings
    std::shared_ptr<SpecialsPanel>          m_specials_panel;           ///< contains icons representing specials
    StarType                                m_star_type;

    boost::signals2::connection             m_planet_connection;
};

/** Container class that holds PlanetPanels.  Creates and destroys PlanetPanel
  * as necessary, and does layout of them after creation and in response to
  * scrolling through them by the user. */
class SidePanel::PlanetPanelContainer : public GG::Wnd {
public:
    /** \name Structors */ //@{
    PlanetPanelContainer();
    ~PlanetPanelContainer();
    //@}

    /** \name Accessors */ //@{
    bool InWindow(const GG::Pt& pt) const override;

    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    int                     SelectedPlanetID() const    {return m_selected_planet_id;}
    const std::set<int>&    SelectionCandidates() const {return m_candidate_ids;}
    int                     ScrollPosition() const;
    //@}

    /** \name Mutators */ //@{
    void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void PreRender() override;

    void Clear();
    void SetPlanets(const std::vector<int>& planet_ids, StarType star_type);
    void SelectPlanet(int planet_id);        //!< programatically selects a planet with id \a planet_id
    void SetValidSelectionPredicate(const std::shared_ptr<UniverseObjectVisitor> &visitor);
    void ScrollTo(int pos);

    /** Updates data displayed in info panels and redoes layout
     *  @param[in] excluded_planet_id Excludes panels with this planet id
     *  @param[in] require_prerender Set panels to RequirePreRender */
    void RefreshAllPlanetPanels(int excluded_planet_id = INVALID_OBJECT_ID,
                                bool require_prerender = false);

    virtual void    ShowScrollbar();
    virtual void    HideScrollbar();

    /** Enables, or disables if \a enable is false, issuing orders via the
      * PlanetPanels in this PlanetPanelContainer. */
    void EnableOrderIssuing(bool enable = true);
    //@}

    /** emitted when an enabled planet panel is clicked by the user */
    mutable boost::signals2::signal<void (int)> PlanetClickedSignal;

    /** emitted when a planet panel is left-double-clicked*/
    mutable boost::signals2::signal<void (int)> PlanetLeftDoubleClickedSignal;

    /** emitted when a planet panel is right-clicked */
    mutable boost::signals2::signal<void (int)> PlanetRightClickedSignal;

    mutable boost::signals2::signal<void (int)> BuildingRightClickedSignal;

private:
    void DisableNonSelectionCandidates();    //!< disables planet panels that aren't selection candidates

    void DoPanelsLayout();                   //!< repositions PlanetPanels, without moving top panel.  Panels below may shift if ones above them have resized.

    /** Resize the panels and \p use_vscroll if requested.  Return the total height.*/
    GG::Y ResizePanelsForVScroll(bool use_vscroll);
    void DoLayout();

    void VScroll(int pos_top, int pos_bottom, int range_min, int range_max); //!< responds to user scrolling of planet panels list.  all but first parameter ignored

    std::vector<std::shared_ptr<PlanetPanel>>   m_planet_panels;
    int                                         m_selected_planet_id;
    std::set<int>                               m_candidate_ids;
    std::shared_ptr<UniverseObjectVisitor>      m_valid_selection_predicate;
    std::shared_ptr<GG::Scroll>                 m_vscroll; ///< the vertical scroll (for viewing all the planet panes);
    bool                                        m_ignore_recursive_resize;
};

class RotatingPlanetControl : public GG::Control {
public:
    RotatingPlanetControl(GG::X x, GG::Y y, int planet_id, StarType star_type) :
        GG::Control(x, y, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
        m_planet_id(planet_id),
        m_initial_rotation(fmod(planet_id / 7.352535, 1.0)),    // arbitrary scale number applied to id to give consistent by varied angles
        m_star_type(star_type)
    {
        Refresh();
    }

    void Render() override {
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        // render rotating base planet texture
        RenderPlanet(ul + GG::Pt(Width() / 2, Height() / 2), Value(Width()), m_surface_texture, m_overlay_texture,
                     m_initial_rotation, m_rpm, m_axial_tilt, m_shininess, m_star_type);

        // overlay atmosphere texture (non-animated)
        if (m_atmosphere_texture) {
            double texture_w = Value(m_atmosphere_texture->DefaultWidth());
            double texture_h = Value(m_atmosphere_texture->DefaultHeight());
            double x_scale = m_diameter / texture_w;
            double y_scale = m_diameter / texture_h;
            glColor4ub(255, 255, 255, m_atmosphere_alpha);
            m_atmosphere_texture->OrthoBlit(GG::Pt(static_cast<GG::X>(ul.x - m_atmosphere_planet_rect.ul.x * x_scale),
                                                   static_cast<GG::Y>(ul.y - m_atmosphere_planet_rect.ul.y * y_scale)),
                                            GG::Pt(static_cast<GG::X>(lr.x + (texture_w - m_atmosphere_planet_rect.lr.x) * x_scale),
                                                   static_cast<GG::Y>(lr.y + (texture_h - m_atmosphere_planet_rect.lr.y) * y_scale)));
        }

        // render fog of war over planet if it's not visible to this client's player
        if ((m_visibility <= VIS_BASIC_VISIBILITY) && GetOptionsDB().Get<bool>("ui.map.scanlines.shown")) {
            s_scanline_shader.SetColor(GetOptionsDB().Get<GG::Clr>("ui.map.sidepanel.planet.scanlane.color"));
            s_scanline_shader.RenderCircle(ul, lr);
        }
    }

    void Refresh() {
        ScopedTimer timer("RotatingPlanetControl::Refresh", true);
        auto planet = GetPlanet(m_planet_id);
        if (!planet) return;

        // these values ensure that wierd GLUT-sphere artifacts do not show themselves
        double period = static_cast<double>(planet->RotationalPeriod());// gives about one rpm for a 1 "Day" rotational period
        if (std::abs(period) <  0.1)    // prevent divide by zero or extremely fast rotations
            period = 0.1;
        m_rpm = 1.0 / period;
        m_diameter = PlanetDiameter(planet->Size());
        m_axial_tilt = std::max(-30.0, std::min(static_cast<double>(planet->AxialTilt()), 60.0));
        m_visibility = GetUniverse().GetObjectVisibilityByEmpire(m_planet_id, HumanClientApp::GetApp()->EmpireID());

        const std::string texture_filename;
        const auto& planet_data = GetRotatingPlanetData();

        auto planet_data_it = planet_data.find(planet->Type());
        int num_planets_of_type;
        if (planet_data_it != planet_data.end() && (num_planets_of_type = planet_data.find(planet->Type())->second.size())) {
            unsigned int hash_value = static_cast<int>(m_planet_id);
            const RotatingPlanetData& rpd = planet_data_it->second[hash_value % num_planets_of_type];
            m_surface_texture = ClientUI::GetTexture(ClientUI::ArtDir() / rpd.filename, true);
            m_shininess = rpd.shininess;

            const auto& atmosphere_data = GetPlanetAtmosphereData();
            auto it = atmosphere_data.find(rpd.filename);
            if (it != atmosphere_data.end()) {
                const auto& atmosphere = it->second.atmospheres[RandSmallInt(0, it->second.atmospheres.size() - 1)];
                m_atmosphere_texture = ClientUI::GetTexture(ClientUI::ArtDir() / atmosphere.filename, true);
                m_atmosphere_alpha = atmosphere.alpha;
                m_atmosphere_planet_rect = GG::Rect(GG::X1, GG::Y1, m_atmosphere_texture->DefaultWidth() - 4, m_atmosphere_texture->DefaultHeight() - 4);
            }
        }

        if (!planet->SurfaceTexture().empty())
            m_overlay_texture = ClientUI::GetTexture(ClientUI::ArtDir() / planet->SurfaceTexture(), true);

        Resize(GG::Pt(GG::X(PlanetDiameter(planet->Size())), GG::Y(PlanetDiameter(planet->Size()))));
    }

private:
    int                             m_planet_id;
    double                          m_rpm;
    int                             m_diameter;
    double                          m_axial_tilt;
    Visibility                      m_visibility;
    std::shared_ptr<GG::Texture>    m_surface_texture;
    double                          m_shininess;
    std::shared_ptr<GG::Texture>    m_overlay_texture;
    std::shared_ptr<GG::Texture>    m_atmosphere_texture;
    int                             m_atmosphere_alpha;
    GG::Rect                        m_atmosphere_planet_rect;
    double                          m_initial_rotation;
    StarType                        m_star_type;

    static ScanlineRenderer         s_scanline_shader;
};

ScanlineRenderer RotatingPlanetControl::s_scanline_shader;


namespace {
    int SystemNameFontSize() {
        return ClientUI::Pts()*1.5;
    }

    class SystemRow : public GG::ListBox::Row {
    public:
        SystemRow(int system_id, GG::Y h) :
            GG::ListBox::Row(GG::X1, h, "SystemRow"),
            m_system_id(system_id)
        {
            SetName("SystemRow");
            RequirePreRender();
        }

        void Init() {
            auto name(GG::Wnd::Create<OwnerColoredSystemName>(m_system_id, SystemNameFontSize(), false));
            push_back(name);
            SetColAlignment(0, GG::ALIGN_CENTER);
            GetLayout()->PreRender();
        }

        void PreRender() override {
            // If there is no control add it.
            if (GetLayout()->Children().empty())
                Init();

            GG::ListBox::Row::PreRender();
        }

        int SystemID() const { return m_system_id; }

        SortKeyType SortKey(std::size_t column) const override
        { return GetSystem(m_system_id)->Name() + std::to_string(m_system_id); }

    private:
        int m_system_id;
    };
}
/** A class to display all of the system names*/
class SidePanel::SystemNameDropDownList : public CUIDropDownList {
    public:
    SystemNameDropDownList(size_t num_shown_elements) :
        CUIDropDownList(num_shown_elements),
        m_order_issuing_enabled(true)
    { }

    /** Enable/disable the ability to give orders that modify the system name.*/
    void EnableOrderIssuing(bool enable = true)
    { m_order_issuing_enabled = enable; }

    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override {
        if (CurrentItem() == end())
            return;

        SystemRow* system_row(dynamic_cast<SystemRow*>(CurrentItem()->get()));
        if (!system_row)
            return;

        auto system = GetSystem(system_row->SystemID());
        if (!system)
            return;

        auto popup = GG::Wnd::Create<GG::PopupMenu>(
            pt.x, pt.y, ClientUI::GetFont(), ClientUI::TextColor(),
            ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

        auto rename_action = [this, system]() {
            auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(GG::X(350), UserString("SP_ENTER_NEW_SYSTEM_NAME"), system->Name());
            edit_wnd->Run();

            if (!m_order_issuing_enabled)
                return;

            if (!RenameOrder::Check(HumanClientApp::GetApp()->EmpireID(), system->ID(), edit_wnd->Result()))
                return;

            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<RenameOrder>(HumanClientApp::GetApp()->EmpireID(), system->ID(), edit_wnd->Result()));
            if (SidePanel* side_panel = dynamic_cast<SidePanel*>(Parent().get()))
                side_panel->Refresh();
        };

        if (m_order_issuing_enabled && system->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
            popup->AddMenuItem(GG::MenuItem(UserString("SP_RENAME_SYSTEM"), false, false, rename_action));

        popup->Run();
    }

    bool m_order_issuing_enabled;
};

namespace {
    const std::vector<std::shared_ptr<GG::Texture>>& GetAsteroidTextures() {
        static std::vector<std::shared_ptr<GG::Texture>> retval;
        if (retval.empty()) {
            retval = ClientUI::GetClientUI()->GetPrefixedTextures(
                ClientUI::ArtDir() / "planets" / "asteroids", "asteroids1_", false);
        }
        return retval;
    }

    const std::string EMPTY_STRING;

    const std::string& GetPlanetSizeName(std::shared_ptr<const Planet> planet) {
        if (planet->Size() == SZ_ASTEROIDS || planet->Size() == SZ_GASGIANT)
            return EMPTY_STRING;
        return UserString(boost::lexical_cast<std::string>(planet->Size()));
    }

    const std::string& GetPlanetTypeName(std::shared_ptr<const Planet> planet)
    { return UserString(boost::lexical_cast<std::string>(planet->Type())); }

    const std::string& GetPlanetEnvironmentName(std::shared_ptr<const Planet> planet, const std::string& species_name)
    { return UserString(boost::lexical_cast<std::string>(planet->EnvironmentForSpecies(species_name))); }

    const std::string& GetStarTypeName(std::shared_ptr<const System> system) {
        if (system->GetStarType() == INVALID_STAR_TYPE)
            return EMPTY_STRING;
        return UserString(boost::lexical_cast<std::string>(system->GetStarType()));
    }

    const GG::Y PLANET_PANEL_TOP = GG::Y(140);
}

////////////////////////////////////////////////
// SidePanel::PlanetPanel
////////////////////////////////////////////////
namespace {
    static const bool SHOW_ALL_PLANET_PANELS = false;   //!< toggles whether to show population, resource, military and building info panels on planet panels that this player doesn't control

    /** How big we want meter icons with respect to the current UI font size.
      * Meters should scale along font size, but not below the size for the
      * default 12 points font. */
    GG::Pt MeterIconSize() {
        const int icon_size = std::max(ClientUI::Pts(), 12) * 4/3;
        return GG::Pt(GG::X(icon_size), GG::Y(icon_size));
    }
}

SidePanel::PlanetPanel::PlanetPanel(GG::X w, int planet_id, StarType star_type) :
    GG::Control(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_planet_id(planet_id),
    m_planet_name(nullptr),
    m_env_size(nullptr),
    m_colonize_button(nullptr),
    m_invade_button(nullptr),
    m_bombard_button(nullptr),
    m_planet_graphic(nullptr),
    m_planet_status_graphic(nullptr),
    m_rotating_planet_graphic(nullptr),
    m_selected(false),
    m_order_issuing_enabled(true),
    m_empire_colour(GG::CLR_ZERO),
    m_focus_drop(nullptr),
    m_population_panel(nullptr),
    m_resource_panel(nullptr),
    m_military_panel(nullptr),
    m_buildings_panel(nullptr),
    m_specials_panel(nullptr),
    m_star_type(star_type)
{}

void SidePanel::PlanetPanel::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    SetName(UserString("PLANET_PANEL"));

    auto planet = GetPlanet(m_planet_id);
    if (!planet) {
        ErrorLogger() << "SidePanel::PlanetPanel::PlanetPanel couldn't get latest known planet with ID " << m_planet_id;
        return;
    }

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    // create planet name text

    // apply formatting tags around planet name to indicate:
    //    Bold for capital(s)
    bool capital = false;

    // need to check all empires for capitals
    for (const auto& entry : Empires()) {
        const Empire* empire = entry.second;
        if (!empire) {
            ErrorLogger() << "PlanetPanel::PlanetPanel got null empire pointer for id " << entry.first;
            continue;
        }
        if (empire->CapitalID() == m_planet_id) {
            capital = true;
            break;
        }
    }

    // determine font based on whether planet is a capital...
    std::shared_ptr<GG::Font> font;
    if (capital)
        font = ClientUI::GetBoldFont(ClientUI::Pts()*4/3);
    else
        font = ClientUI::GetFont(ClientUI::Pts()*4/3);

    GG::X panel_width = Width() - MaxPlanetDiameter() - 2*EDGE_PAD;

    // create planet name control
    m_planet_name = GG::Wnd::Create<GG::TextControl>(
        GG::X0, GG::Y0, GG::X1, GG::Y1, " ", font, ClientUI::TextColor());
    m_planet_name->MoveTo(GG::Pt(GG::X(MaxPlanetDiameter() + EDGE_PAD), GG::Y0));
    m_planet_name->Resize(m_planet_name->MinUsableSize());
    AttachChild(m_planet_name);


    // focus-selection droplist
    m_focus_drop = GG::Wnd::Create<CUIDropDownList>(6);
    AttachChild(m_focus_drop);
    m_focus_drop->SelChangedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::FocusDropListSelectionChangedSlot, this, _1));
    this->FocusChangedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::SetFocus, this, _1));
    m_focus_drop->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_focus_drop->SetStyle(GG::LIST_NOSORT | GG::LIST_SINGLESEL);
    m_focus_drop->ManuallyManageColProps();
    m_focus_drop->SetNumCols(2);
    m_focus_drop->SetColWidth(0, m_focus_drop->DisplayedRowWidth());
    m_focus_drop->SetColStretch(0, 0.0);
    m_focus_drop->SetColStretch(1, 1.0);
    m_focus_drop->SetOnlyMouseScrollWhenDropped(true);


    // meter panels
    m_population_panel = GG::Wnd::Create<PopulationPanel>(panel_width, m_planet_id);
    AttachChild(m_population_panel);
    m_population_panel->ExpandCollapseSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::RequirePreRender, this));

    m_resource_panel = GG::Wnd::Create<ResourcePanel>(panel_width, m_planet_id);
    AttachChild(m_resource_panel);
    m_resource_panel->ExpandCollapseSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::RequirePreRender, this));

    m_military_panel = GG::Wnd::Create<MilitaryPanel>(panel_width, m_planet_id);
    AttachChild(m_military_panel);
    m_military_panel->ExpandCollapseSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::RequirePreRender, this));

    m_buildings_panel = GG::Wnd::Create<BuildingsPanel>(panel_width, 4, m_planet_id);
    AttachChild(m_buildings_panel);
    m_buildings_panel->ExpandCollapseSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::RequirePreRender, this));
    m_buildings_panel->BuildingRightClickedSignal.connect(
        BuildingRightClickedSignal);

    m_specials_panel = GG::Wnd::Create<SpecialsPanel>(panel_width, m_planet_id);
    AttachChild(m_specials_panel);

    m_env_size = GG::Wnd::Create<CUILabel>("", GG::FORMAT_NOWRAP);
    m_env_size->MoveTo(GG::Pt(GG::X(MaxPlanetDiameter()), GG::Y0));
    AttachChild(m_env_size);


    m_colonize_button = Wnd::Create<CUIButton>(UserString("PL_COLONIZE"));
    m_colonize_button->LeftClickedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::ClickColonize, this));

    m_invade_button   = Wnd::Create<CUIButton>(UserString("PL_INVADE"));
    m_invade_button->LeftClickedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::ClickInvade, this));

    m_bombard_button  = Wnd::Create<CUIButton>(UserString("PL_BOMBARD"));
    m_bombard_button->LeftClickedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::ClickBombard, this));

    SetChildClippingMode(ClipToWindow);

    Refresh();

    RequirePreRender();
}

SidePanel::PlanetPanel::~PlanetPanel()
{}

void SidePanel::PlanetPanel::DoLayout() {
    GG::X left = GG::X0 + MaxPlanetDiameter() + EDGE_PAD;
    GG::X right = left + Width() - MaxPlanetDiameter() - 2*EDGE_PAD;
    GG::Y y = GG::Y0;

    if (m_planet_name) {
        m_planet_name->MoveTo(GG::Pt(left, y));
        y += m_planet_name->Height();                           // no interpanel space needed here, I declare arbitrarily
    }

    if (m_specials_panel) {
        m_specials_panel->SizeMove(GG::Pt(left, y), GG::Pt(right, y + m_specials_panel->Height())); // assumed to always be this Wnd's child
        y += m_specials_panel->Height() + EDGE_PAD;
    }

    if (m_env_size && m_env_size->Parent().get() == this) {
        m_env_size->MoveTo(GG::Pt(left, y));
        y += m_env_size->Height() + EDGE_PAD;
    }

    if (m_colonize_button && m_colonize_button->Parent().get() == this) {
        m_colonize_button->MoveTo(GG::Pt(left, y));
        m_colonize_button->Resize(GG::Pt(GG::X(ClientUI::Pts()*15), m_colonize_button->MinUsableSize().y));
        y += m_colonize_button->Height() + EDGE_PAD;
    }
    if (m_invade_button && m_invade_button->Parent().get() == this) {
        m_invade_button->MoveTo(GG::Pt(left, y));
        m_invade_button->Resize(GG::Pt(GG::X(ClientUI::Pts()*15), m_invade_button->MinUsableSize().y));
        y += m_invade_button->Height() + EDGE_PAD;
    }
    if (m_bombard_button && m_bombard_button->Parent().get() == this) {
        m_bombard_button->MoveTo(GG::Pt(left, y));
        m_bombard_button->Resize(GG::Pt(GG::X(ClientUI::Pts()*15), m_bombard_button->MinUsableSize().y));
        y += m_bombard_button->Height() + EDGE_PAD;
    }

    if (m_focus_drop && m_focus_drop->Parent().get() == this) {
        m_focus_drop->MoveTo(GG::Pt(left, y));
        m_focus_drop->Resize(GG::Pt(MeterIconSize().x*4, MeterIconSize().y*3/2 + 4));
        y += m_focus_drop->Height() + EDGE_PAD;
    }

    if (m_population_panel && m_population_panel->Parent().get() == this) {
        m_population_panel->SizeMove(GG::Pt(left, y), GG::Pt(right, y + m_population_panel->Height()));
        y += m_population_panel->Height() + EDGE_PAD;
    }

    if (m_resource_panel && m_resource_panel->Parent().get() == this) {
        m_resource_panel->SizeMove(GG::Pt(left, y), GG::Pt(right, y + m_resource_panel->Height()));
        y += m_resource_panel->Height() + EDGE_PAD;
    }

    if (m_military_panel && m_military_panel->Parent().get() == this) {
        m_military_panel->SizeMove(GG::Pt(left, y), GG::Pt(right, y + m_military_panel->Height()));
        y += m_military_panel->Height() + EDGE_PAD;
    }

    if (m_buildings_panel && m_buildings_panel->Parent().get() == this) {
        m_buildings_panel->SizeMove(GG::Pt(left, y), GG::Pt(right, y + m_buildings_panel->Height()));
        y += m_buildings_panel->Height() + EDGE_PAD;
    }

    GG::Y min_height(MaxPlanetDiameter());

    RefreshPlanetGraphic();
    if (m_planet_graphic)
        min_height = m_planet_graphic->Height();
    // TODO: get following to resize panel properly...
    //else if (m_rotating_planet_graphic)
    //    min_height = m_rotating_planet_graphic->Height();

    Resize(GG::Pt(Width(), std::max(y, min_height)));

    // DoLayout() is only called during prerender so prerender the specials panel
    // in case it has pending changes.
    if (m_specials_panel)
        GG::GUI::PreRenderWindow(m_specials_panel);

    ResizedSignal();
}

void SidePanel::PlanetPanel::RefreshPlanetGraphic() {
    auto planet = GetPlanet(m_planet_id);
    if (!planet || !GetOptionsDB().Get<bool>("ui.map.sidepanel.planet.shown"))
        return;

    DetachChildAndReset(m_planet_graphic);
    DetachChildAndReset(m_rotating_planet_graphic);

    if (planet->Type() == PT_ASTEROIDS) {
        const auto& textures = GetAsteroidTextures();
        if (textures.empty())
            return;
        GG::X texture_width = textures[0]->DefaultWidth();
        GG::Y texture_height = textures[0]->DefaultHeight();
        GG::Pt planet_image_pos(GG::X(MaxPlanetDiameter() / 2 - texture_width / 2 + 3), GG::Y0);

        m_planet_graphic = GG::Wnd::Create<GG::DynamicGraphic>(planet_image_pos.x, planet_image_pos.y,
                                                               texture_width, texture_height, true,
                                                               texture_width, texture_height, 0, textures,
                                                               GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        m_planet_graphic->SetFPS(GetAsteroidsFPS());
        m_planet_graphic->SetFrameIndex(RandSmallInt(0, textures.size() - 1));
        AttachChild(m_planet_graphic);
        m_planet_graphic->Play();
    } else if (planet->Type() < NUM_PLANET_TYPES) {
        int planet_image_sz = PlanetDiameter(planet->Size());
        GG::Pt planet_image_pos(GG::X(MaxPlanetDiameter() / 2 - planet_image_sz / 2 + 3),
                                GG::Y(MaxPlanetDiameter() / 2 - planet_image_sz / 2));
        m_rotating_planet_graphic = GG::Wnd::Create<RotatingPlanetControl>(planet_image_pos.x, planet_image_pos.y,
                                                                           m_planet_id, m_star_type);
        AttachChild(m_rotating_planet_graphic);
    }
}

namespace {
    bool IsAvailable(std::shared_ptr<const Ship> ship, int system_id, int empire_id) {
        if (!ship)
            return false;
        auto fleet = GetFleet(ship->FleetID());
        if (!fleet)
            return false;
        if (ship->SystemID() == system_id &&
            ship->OwnedBy(empire_id) &&
            ship->GetVisibility(empire_id) >= VIS_PARTIAL_VISIBILITY &&
            ship->OrderedScrapped() == false &&
            fleet->FinalDestinationID() == INVALID_OBJECT_ID)
        { return true; }
        return false;
    }

    bool AvailableToColonize(std::shared_ptr<const Ship> ship, int system_id, int empire_id) {
        if (!ship)
            return false;
        auto fleet = GetFleet(ship->FleetID());
        if (!fleet)
            return false;
        if (IsAvailable(ship, system_id, empire_id) &&
            ship->CanColonize() &&
            ship->OrderedColonizePlanet() == INVALID_OBJECT_ID)
        { return true; }
        return false;
    };

    bool AvailableToInvade(std::shared_ptr<const Ship> ship, int system_id, int empire_id) {
        if (!ship)
            return false;
        auto fleet = GetFleet(ship->FleetID());
        if (!fleet)
            return false;
        if (IsAvailable(ship, system_id, empire_id) &&
            ship->HasTroops() &&
            ship->OrderedInvadePlanet() == INVALID_OBJECT_ID)
        { return true; }
        return false;
    };

    bool AvailableToBombard(std::shared_ptr<const Ship> ship, int system_id, int empire_id) {
        if (!ship)
            return false;
        auto fleet = GetFleet(ship->FleetID());
        if (!fleet)
            return false;
        if (IsAvailable(ship, system_id, empire_id) &&
            ship->CanBombard() &&
            ship->OrderedBombardPlanet() == INVALID_OBJECT_ID)
        { return true; }
        return false;
    };

    /** Content tags that note if a Ship should be auto-selected for bombarding a Planet.
     *  These tags are determined from the TAG_BOMBARD_PREFIX tags of @a ship and potentially match those of a Planet.
     *  If the Ship contains the content tag defined in TAG_BOMBARD_ALWAYS, only that tag will be returned.
     */
    std::vector<std::string> BombardTagsForShip(std::shared_ptr<const Ship> ship) {
        std::vector<std::string> retval;
        if (!ship)
            return retval;
        for (const std::string& tag : ship->Tags()) {
            if (tag == TAG_BOMBARD_ALWAYS) {
                retval.clear();
                retval.push_back(tag);
                break;
            } else if ((tag.length() > TAG_BOMBARD_PREFIX.length()) &&
                       (tag.substr(0, TAG_BOMBARD_PREFIX.length()) == TAG_BOMBARD_PREFIX))
            { retval.push_back(tag.substr(TAG_BOMBARD_PREFIX.length())); }
        }
        return retval;
    }

    bool CanColonizePlanetType(std::shared_ptr<const Ship> ship, PlanetType planet_type) {
        if (!ship || planet_type == INVALID_PLANET_TYPE)
            return false;

        float colony_ship_capacity = 0.0f;

        const auto design = ship->Design();
        if (design) {
            colony_ship_capacity = design->ColonyCapacity();
            if (ship->CanColonize() && colony_ship_capacity == 0.0f)
                return true;    // outpost ship; planet type doesn't matter as there is no species to check against
        }

        if (const Species* colony_ship_species = GetSpecies(ship->SpeciesName())) {
            PlanetEnvironment planet_env_for_colony_species = colony_ship_species->GetPlanetEnvironment(planet_type);

            // One-Click Colonize planets that are colonizable (even if they are
            // not hospitable), and One-Click Outpost planets that are not
            // colonizable.
            if (colony_ship_capacity > 0.0f) {
                return planet_env_for_colony_species >= PE_HOSTILE && planet_env_for_colony_species <= PE_GOOD;
            } else {
                return planet_env_for_colony_species < PE_HOSTILE || planet_env_for_colony_species > PE_GOOD;
            }
        }
        return false;
    }

    std::set<std::shared_ptr<const Ship>> ValidSelectedInvasionShips(int system_id) {
        std::set<std::shared_ptr<const Ship>> retval;

        // if not looking in a valid system, no valid invasion ship can be available
        if (system_id == INVALID_OBJECT_ID)
            return retval;

        // is there a valid single selected ship in the active FleetWnd?
        for (int ship_id : FleetUIManager::GetFleetUIManager().SelectedShipIDs())
            if (auto ship = GetUniverse().Objects().Object<Ship>(ship_id))
                if (ship->SystemID() == system_id && ship->HasTroops() && ship->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
                    retval.insert(ship);

        return retval;
    }

    std::set<std::shared_ptr<const Ship>> ValidSelectedBombardShips(int system_id) {
        std::set<std::shared_ptr<const Ship>> retval;

        // if not looking in a valid system, no valid bombard ship can be available
        if (system_id == INVALID_OBJECT_ID)
            return retval;

        // is there a valid single selected ship in the active FleetWnd?
        for (int ship_id : FleetUIManager::GetFleetUIManager().SelectedShipIDs()) {
            auto ship = GetShip(ship_id);
            if (!ship || ship->SystemID() != system_id)
                continue;
            if (!ship->CanBombard() || !ship->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
                continue;
            retval.insert(ship);
        }

        return retval;
    }
}

std::shared_ptr<const Ship> ValidSelectedColonyShip(int system_id) {
    // if not looking in a valid system, no valid colony ship can be available
    if (system_id == INVALID_OBJECT_ID)
        return nullptr;

    // is there a valid selected ship in the active FleetWnd?
    for (int ship_id : FleetUIManager::GetFleetUIManager().SelectedShipIDs())
        if (auto ship = GetShip(ship_id))
            if (ship->SystemID() == system_id && ship->CanColonize() && ship->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
                return ship;
    return nullptr;
}

int AutomaticallyChosenColonyShip(int target_planet_id) {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES)
        return INVALID_OBJECT_ID;
    if (GetUniverse().GetObjectVisibilityByEmpire(target_planet_id, empire_id) < VIS_PARTIAL_VISIBILITY)
        return INVALID_OBJECT_ID;
    auto target_planet = GetPlanet(target_planet_id);
    if (!target_planet)
        return INVALID_OBJECT_ID;
    int system_id = target_planet->SystemID();
    auto system = GetSystem(system_id);
    if (!system)
        return INVALID_OBJECT_ID;
    // is planet a valid colonization target?
    if (target_planet->InitialMeterValue(METER_POPULATION) > 0.0 ||
        (!target_planet->Unowned() && !target_planet->OwnedBy(empire_id)))
    { return INVALID_OBJECT_ID; }

    PlanetType target_planet_type = target_planet->Type();

    // todo: return vector of ships from system ids using new Objects().FindObjects<Ship>(system->FindObjectIDs())
    auto ships = Objects().FindObjects<const Ship>(system->ShipIDs());
    std::vector<std::shared_ptr<const Ship>> capable_and_available_colony_ships;
    capable_and_available_colony_ships.reserve(ships.size());

    // get all ships that can colonize and that are free to do so in the
    // specified planet'ssystem and that can colonize the requested planet
    for (auto& ship : ships) {
        if (!AvailableToColonize(ship, system_id, empire_id))
            continue;
        if (!CanColonizePlanetType(ship, target_planet_type))
            continue;
        capable_and_available_colony_ships.push_back(ship);
    }

    // simple case early exits: no ships, or just one capable ship
    if (capable_and_available_colony_ships.empty())
        return INVALID_OBJECT_ID;
    if (capable_and_available_colony_ships.size() == 1)
        return (*capable_and_available_colony_ships.begin())->ID();

    // have more than one ship capable and available to colonize.
    // pick the "best" one.
    std::string orig_species = target_planet->SpeciesName(); //should be just ""
    int orig_owner = target_planet->Owner();
    float orig_initial_target_pop = target_planet->GetMeter(METER_TARGET_POPULATION)->Initial();
    int best_ship = INVALID_OBJECT_ID;
    float best_capacity = -999;
    bool changed_planet = false;

    GetUniverse().InhibitUniverseObjectSignals(true);
    for (auto& ship : capable_and_available_colony_ships) {
        if (!ship)
            continue;
        int ship_id = ship->ID();
        float planet_capacity = -999.9f;
        auto this_pair = std::make_pair(ship_id, target_planet_id);
        auto pair_it = colony_projections.find(this_pair);
        if (pair_it != colony_projections.end()) {
            planet_capacity = pair_it->second;
        } else {
            float colony_ship_capacity = 0.0f;
            std::string ship_species_name;
            const ShipDesign* design = ship->Design();
            if (!design)
                continue;
            colony_ship_capacity = design->ColonyCapacity();
            if (colony_ship_capacity > 0.0f ) {
                ship_species_name = ship->SpeciesName();
                auto spec_pair = std::make_pair(ship_species_name, target_planet_id);
                auto spec_pair_it = species_colony_projections.find(spec_pair);
                if (spec_pair_it != species_colony_projections.end()) {
                    planet_capacity = spec_pair_it->second;
                } else {
                    const Species* species = GetSpecies(ship_species_name);
                    PlanetEnvironment planet_environment = PE_UNINHABITABLE;
                    if (species)
                        planet_environment = species->GetPlanetEnvironment(target_planet->Type());
                    if (planet_environment != PE_UNINHABITABLE) {
                        changed_planet = true;
                        target_planet->SetOwner(empire_id);
                        target_planet->SetSpecies(ship_species_name);
                        target_planet->GetMeter(METER_TARGET_POPULATION)->Reset();

                        // temporary meter update with currently set species
                        GetUniverse().UpdateMeterEstimates(target_planet_id);
                        planet_capacity = target_planet->CurrentMeterValue(METER_TARGET_POPULATION);    // want value after meter update, so check current, not initial value
                    }
                    species_colony_projections[spec_pair] = planet_capacity;
                }
            } else {
                planet_capacity = 0.0f;
            }
            colony_projections[this_pair] = planet_capacity;
        }
        if (planet_capacity > best_capacity) {
            best_capacity = planet_capacity;
            best_ship = ship_id;
        }
    }
    if (changed_planet) {
        target_planet->SetOwner(orig_owner);
        target_planet->SetSpecies(orig_species);
        target_planet->GetMeter(METER_TARGET_POPULATION)->Set(orig_initial_target_pop, orig_initial_target_pop);
        GetUniverse().UpdateMeterEstimates(target_planet_id);
    }
    GetUniverse().InhibitUniverseObjectSignals(false);

    return best_ship;
}

std::set<std::shared_ptr<const Ship>> AutomaticallyChosenInvasionShips(int target_planet_id) {
    std::set<std::shared_ptr<const Ship>> retval;

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES)
        return retval;

    auto target_planet = GetPlanet(target_planet_id);
    if (!target_planet)
        return retval;
    int system_id = target_planet->SystemID();
    auto system = GetSystem(system_id);
    if (!system)
        return retval;

    //Can't invade owned-by-self planets; early exit
    if (target_planet->OwnedBy(empire_id))
        return retval;


    // get "just enough" ships that can invade and that are free to do so
    double defending_troops = target_planet->InitialMeterValue(METER_TROOPS);

    double invasion_troops = 0;
    for (auto& ship : Objects().FindObjects<Ship>()) {
        if (!AvailableToInvade(ship, system_id, empire_id))
            continue;

        invasion_troops += ship->TroopCapacity();

        retval.insert(ship);

        if (invasion_troops > defending_troops)
            break;
    }

    return retval;
}

/** Returns valid Ship%s capable of bombarding a given Planet.
 * @param target_planet_id ID of Planet to potentially bombard
 */
std::set<std::shared_ptr<const Ship>> AutomaticallyChosenBombardShips(int target_planet_id) {
    std::set<std::shared_ptr<const Ship>> retval;

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES)
        return retval;

    auto target_planet = GetPlanet(target_planet_id);
    if (!target_planet)
        return retval;
    int system_id = target_planet->SystemID();
    auto system = GetSystem(system_id);
    if (!system)
        return retval;

    // Can't bombard owned-by-self planets; early exit
    if (target_planet->OwnedBy(empire_id))
        return retval;

    for (auto& ship : Objects().FindObjects<Ship>()) {
        // owned ship is capable of bombarding a planet in this system
        if (!AvailableToBombard(ship, system_id, empire_id))
            continue;

        // Select ship if the planet contains a content tag specified by the ship, or ship is tagged to always be selected
        for (const std::string& tag : BombardTagsForShip(ship)) {
            if ((tag == TAG_BOMBARD_ALWAYS) || (target_planet->HasTag(tag))) {
                retval.insert(ship);
                break;
            }
        }
    }

    return retval;
}

void SidePanel::PlanetPanel::Refresh() {
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    m_planet_connection.disconnect();

    auto planet = GetPlanet(m_planet_id);
    if (!planet) {
        DebugLogger() << "PlanetPanel::Refresh couldn't get planet!";
        // clear / hide everything...
        DetachChildAndReset(m_planet_name);
        DetachChildAndReset(m_env_size);
        DetachChildAndReset(m_focus_drop);
        DetachChildAndReset(m_population_panel);
        DetachChildAndReset(m_resource_panel);
        DetachChildAndReset(m_military_panel);
        DetachChildAndReset(m_buildings_panel);
        DetachChildAndReset(m_colonize_button);
        DetachChildAndReset(m_invade_button);
        DetachChildAndReset(m_bombard_button);
        DetachChildAndReset(m_specials_panel);
        DetachChildAndReset(m_planet_status_graphic);

        RequirePreRender();
        return;
    }


    // set planet name, formatted to indicate presense of shipyards / homeworlds

    // apply formatting tags around planet name to indicate:
    //    Italic for homeworlds
    //    Underline for shipyard(s), and
    bool homeworld = false, has_shipyard = false;

    // need to check all species for homeworlds
    for (const auto& entry : GetSpeciesManager()) {
        if (const auto& species = entry.second) {
            const auto& homeworld_ids = species->Homeworlds();
            if (homeworld_ids.count(m_planet_id)) {
                homeworld = true;
                break;
            }
        }
    }

    // check for shipyard
    const auto& known_destroyed_object_ids =
        GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id);
    for (int building_id : planet->BuildingIDs()) {
        auto building = GetBuilding(building_id);
        if (!building)
            continue;
        if (known_destroyed_object_ids.count(building_id))
            continue;
        if (building->HasTag(TAG_SHIPYARD)) {
            has_shipyard = true;
            break;
        }
    }

    // wrap with formatting tags
    std::string wrapped_planet_name = planet->Name();
    if (homeworld)
        wrapped_planet_name = "<i>" + wrapped_planet_name + "</i>";
    if (has_shipyard)
        wrapped_planet_name = "<u>" + wrapped_planet_name + "</u>";
    if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
        wrapped_planet_name = wrapped_planet_name + " (" + std::to_string(m_planet_id) + ")";
    }

    // set name
    m_planet_name->SetText("<s>" + wrapped_planet_name + "</s>");
    m_planet_name->MoveTo(GG::Pt(GG::X(MaxPlanetDiameter() + EDGE_PAD), GG::Y0));
    m_planet_name->Resize(m_planet_name->MinUsableSize());


    // colour planet name with owner's empire colour
    m_empire_colour = GG::CLR_ZERO;
    if (!planet->Unowned() && m_planet_name) {
        if (Empire* planet_empire = GetEmpire(planet->Owner())) {
            m_empire_colour = planet_empire->Color();
            m_planet_name->SetTextColor(planet_empire->Color());
        } else {
            m_planet_name->SetTextColor(ClientUI::TextColor());
        }
    }

    auto selected_colony_ship = ValidSelectedColonyShip(SidePanel::SystemID());
    if (!selected_colony_ship && FleetUIManager::GetFleetUIManager().SelectedShipIDs().empty())
        selected_colony_ship = GetShip(AutomaticallyChosenColonyShip(m_planet_id));

    auto invasion_ships = ValidSelectedInvasionShips(SidePanel::SystemID());
    if (invasion_ships.empty()) {
        auto autoselected_invasion_ships = AutomaticallyChosenInvasionShips(m_planet_id);
        invasion_ships.insert(autoselected_invasion_ships.begin(), autoselected_invasion_ships.end());
    }

    auto bombard_ships = ValidSelectedBombardShips(SidePanel::SystemID());
    if (bombard_ships.empty()) {
        auto autoselected_bombard_ships = AutomaticallyChosenBombardShips(m_planet_id);
        bombard_ships.insert(autoselected_bombard_ships.begin(), autoselected_bombard_ships.end());
    }

    std::string colony_ship_species_name;
    float colony_ship_capacity = 0.0f;
    if (selected_colony_ship) {
        colony_ship_species_name = selected_colony_ship->SpeciesName();
        colony_ship_capacity = selected_colony_ship->ColonyCapacity();
    }
    const Species* colony_ship_species = GetSpecies(colony_ship_species_name);
    PlanetEnvironment planet_env_for_colony_species = PE_UNINHABITABLE;
    if (colony_ship_species)
        planet_env_for_colony_species = colony_ship_species->GetPlanetEnvironment(planet->Type());


    // calculate truth tables for planet colonization and invasion
    bool has_owner =        !planet->Unowned();
    bool mine =             planet->OwnedBy(client_empire_id);
    bool populated =        planet->InitialMeterValue(METER_POPULATION) > 0.0f;
    bool habitable =        planet_env_for_colony_species >= PE_HOSTILE && planet_env_for_colony_species <= PE_GOOD;
    bool visible =          GetUniverse().GetObjectVisibilityByEmpire(m_planet_id, client_empire_id) >= VIS_PARTIAL_VISIBILITY;
    bool shielded =         planet->InitialMeterValue(METER_SHIELD) > 0.0f;
    bool has_defenses =     planet->InitialMeterValue(METER_MAX_SHIELD) > 0.0f ||
                            planet->InitialMeterValue(METER_MAX_DEFENSE) > 0.0f ||
                            planet->InitialMeterValue(METER_MAX_TROOPS) > 0.0f;
    bool being_colonized =  planet->IsAboutToBeColonized();
    bool outpostable =                   !populated && (  !has_owner /*&& !shielded*/         ) && visible && !being_colonized;
    bool colonizable =      habitable && !populated && ( (!has_owner /*&& !shielded*/) || mine) && visible && !being_colonized;
    bool can_colonize =     selected_colony_ship && (   (colonizable  && (colony_ship_capacity > 0.0f))
                                                     || (outpostable && (colony_ship_capacity == 0.0f)));

    bool at_war_with_me =   !mine && (populated || (has_owner && Empires().GetDiplomaticStatus(client_empire_id, planet->Owner()) == DIPLO_WAR));

    bool being_invaded =    planet->IsAboutToBeInvaded();
    bool invadable =        at_war_with_me && !shielded && visible && !being_invaded && !invasion_ships.empty();

    bool being_bombarded =  planet->IsAboutToBeBombarded();
    bool bombardable =      at_war_with_me && visible && !being_bombarded && !bombard_ships.empty();

    if (populated || SHOW_ALL_PLANET_PANELS) {
        AttachChild(m_population_panel);
        if (m_population_panel)
            m_population_panel->Refresh();
    } else {
        DetachChild(m_population_panel);
    }

    if (populated || has_owner || SHOW_ALL_PLANET_PANELS) {
        AttachChild(m_resource_panel);
        if (m_resource_panel)
            m_resource_panel->Refresh();
    } else {
        DetachChild(m_resource_panel);
    }

    if (populated || has_owner || has_defenses || SHOW_ALL_PLANET_PANELS) {
        AttachChild(m_military_panel);
        if (m_military_panel)
            m_military_panel->Refresh();
    } else {
        DetachChild(m_military_panel);
    }


    DetachChild(m_invade_button);
    DetachChild(m_colonize_button);
    DetachChild(m_bombard_button);
    DetachChild(m_focus_drop);

    std::string species_name;
    if (!planet->SpeciesName().empty())
        species_name = planet->SpeciesName();
    else if (!colony_ship_species_name.empty())
        species_name = colony_ship_species_name;

    std::string env_size_text;

    if (species_name.empty())
        env_size_text = boost::io::str(FlexibleFormat(UserString("PL_TYPE_SIZE"))
                                       % GetPlanetSizeName(planet)
                                       % GetPlanetTypeName(planet));
    else
        env_size_text = boost::io::str(FlexibleFormat(UserString("PL_TYPE_SIZE_ENV"))
                                       % GetPlanetSizeName(planet)
                                       % GetPlanetTypeName(planet)
                                       % GetPlanetEnvironmentName(planet, species_name));


    if (Disabled() || !(can_colonize || being_colonized || invadable || being_invaded)) {
        // hide everything
    }

    if (can_colonize) {
        // show colonize button; in case the chosen colony ship is not actually
        // selected, but has been chosen by AutomaticallyChosenColonyShip,
        // determine what population capacity to put on the conolnize buttone by
        // temporarily setting ownership (for tech) and species of the planet,
        // reading the target population, then setting the planet back as it was.
        // The results are cached for the duration of the turn in the
        // colony_projections map.
        AttachChild(m_colonize_button);
        double planet_capacity;
        auto this_pair = std::make_pair(selected_colony_ship->ID(), m_planet_id);
        auto pair_it = colony_projections.find(this_pair);
        if (pair_it != colony_projections.end()) {
            planet_capacity = pair_it->second;
        } else if (colony_ship_capacity == 0.0f) {
            planet_capacity = 0.0f;
            colony_projections[this_pair] = planet_capacity;
        } else {
            GetUniverse().InhibitUniverseObjectSignals(true);
            std::string orig_species = planet->SpeciesName(); //should be just ""
            int orig_owner = planet->Owner();
            float orig_initial_target_pop = planet->GetMeter(METER_TARGET_POPULATION)->Initial();
            planet->SetOwner(client_empire_id);
            planet->SetSpecies(colony_ship_species_name);
            planet->GetMeter(METER_TARGET_POPULATION)->Reset();

            // temporary meter updates for curently set species
            GetUniverse().UpdateMeterEstimates(m_planet_id);
            planet_capacity = ((planet_env_for_colony_species == PE_UNINHABITABLE) ? 0 : planet->CurrentMeterValue(METER_TARGET_POPULATION));   // want target pop after meter update, so check current value of meter
            planet->SetOwner(orig_owner);
            planet->SetSpecies(orig_species);
            planet->GetMeter(METER_TARGET_POPULATION)->Set(orig_initial_target_pop, orig_initial_target_pop);
            GetUniverse().UpdateMeterEstimates(m_planet_id);

            colony_projections[this_pair] = planet_capacity;
            GetUniverse().InhibitUniverseObjectSignals(false);
        }

        std::string colonize_text;
        if (colony_ship_capacity > 0.0f) {
            std::string initial_pop = DoubleToString(colony_ship_capacity, 2, false);

            std::string clr_tag;
            if (planet_capacity < colony_ship_capacity && colony_ship_capacity > 0.0f)
                clr_tag = GG::RgbaTag(ClientUI::StatDecrColor());
            else if (planet_capacity > colony_ship_capacity && colony_ship_capacity > 0.0f)
                clr_tag = GG::RgbaTag(ClientUI::StatIncrColor());
            std::string clr_tag_close = (clr_tag.empty() ? "" : "</rgba>");
            std::string target_pop = clr_tag + DoubleToString(planet_capacity, 2, false) + clr_tag_close;

            colonize_text = boost::io::str(FlexibleFormat(UserString("PL_COLONIZE")) % initial_pop % target_pop);
        } else {
            colonize_text = UserString("PL_OUTPOST");
        }
        if (m_colonize_button)
            m_colonize_button->SetText(colonize_text);

    } else if (being_colonized) {
        // shown colonize cancel button
        AttachChild(m_colonize_button);
        if (m_colonize_button)
            m_colonize_button->SetText(UserString("PL_CANCEL_COLONIZE"));
    }

    if (invadable) {
        // show invade button
        AttachChild(m_invade_button);
        float invasion_troops = 0.0f;
        for (auto& invasion_ship : invasion_ships) {
            invasion_troops += invasion_ship->TroopCapacity();
        }
        std::string invasion_troops_text = DoubleToString(invasion_troops, 3, false);

        // adjust defending troops number before passing into DoubleToString to ensure
        // rounding up, as it's better to slightly overestimate defending troops than
        // underestimate, since one needs to drop more droops than there are defenders
        // to capture a planet
        float defending_troops = planet->InitialMeterValue(METER_TROOPS);
        //std::cout << "def troops: " << defending_troops << std::endl;
        float log10_df = floor(std::log10(defending_troops));
        //std::cout << "def troops log10: " << log10_df << std::endl;
        float rounding_adjustment = std::pow(10.0f, log10_df - 2.0f);
        //std::cout << "adjustment: " << rounding_adjustment << std::endl;
        defending_troops += rounding_adjustment;
        //std::cout << "adjusted def troops: " << defending_troops<< std::endl;

        std::string defending_troops_text = DoubleToString(defending_troops, 3, false);
        std::string invasion_text = boost::io::str(FlexibleFormat(UserString("PL_INVADE"))
                                                   % invasion_troops_text % defending_troops_text);
        // todo: tooltip breaking down which or how many ships are being used to invade, their troop composition / contribution
        if (m_invade_button)
            m_invade_button->SetText(invasion_text);

    } else if (being_invaded) {
        // show invade cancel button
        AttachChild(m_invade_button);
        if (m_invade_button)
            m_invade_button->SetText(UserString("PL_CANCEL_INVADE"));
    }

    if (bombardable) {
        // show bombard button
        AttachChild(m_bombard_button);
        if (m_bombard_button)
            m_bombard_button->SetText(UserString("PL_BOMBARD"));

    } else if (being_bombarded) {
        // show bombard cancel button
        AttachChild(m_bombard_button);
        if (m_bombard_button)
            m_bombard_button->SetText(UserString("PL_CANCEL_BOMBARD"));
    }

    m_env_size->SetText(env_size_text);

    if (!planet->SpeciesName().empty()) {
        AttachChild(m_focus_drop);

        auto available_foci = planet->AvailableFoci();

        // refresh items in list
        m_focus_drop->Clear();
        std::vector<std::shared_ptr<GG::DropDownList::Row>> rows;
        rows.reserve(available_foci.size());
        for (const std::string& focus_name : available_foci) {
            auto texture = ClientUI::GetTexture(
                ClientUI::ArtDir() / planet->FocusIcon(focus_name), true);
            auto graphic = GG::Wnd::Create<GG::StaticGraphic>(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            graphic->Resize(GG::Pt(MeterIconSize().x*3/2, MeterIconSize().y*3/2));
            auto row = GG::Wnd::Create<GG::DropDownList::Row>(graphic->Width(), graphic->Height(), "FOCUS");

            row->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
            row->SetBrowseText(
                boost::io::str(FlexibleFormat(UserString("RP_FOCUS_TOOLTIP"))
                               % UserString(focus_name)));

            row->push_back(graphic);
            rows.push_back(row);
        }
        m_focus_drop->Insert(rows);

        // set browse text and select appropriate focus in droplist
        std::string focus_text;
        if (!planet->Focus().empty()) {
            for (unsigned int i = 0; i < available_foci.size(); ++i) {
                if (available_foci[i] == planet->Focus()) {
                    m_focus_drop->Select(i);
                    focus_text = boost::io::str(FlexibleFormat(UserString("RP_FOCUS_TOOLTIP"))
                                                % UserString(planet->Focus()));
                }
            }
        } else {
            m_focus_drop->Select(m_focus_drop->end());
        }
        m_focus_drop->SetBrowseText(focus_text);

        // prevent manipulation for unowned planets
        if (!planet->OwnedBy(client_empire_id))
            m_focus_drop->Disable();
    }


    // other panels...
    if (m_buildings_panel)
        m_buildings_panel->Refresh();
    if (m_specials_panel)
        m_specials_panel->Update();

    // create planet status marker
    if (planet->OwnedBy(client_empire_id)) {
        DetachChild(m_planet_status_graphic);
        std::vector<std::string> planet_status_messages;
        std::shared_ptr<GG::Texture> planet_status_texture;

        // status: no supply
        if (!GetSupplyManager().SystemHasFleetSupply(planet->SystemID(), planet->Owner())) {
            planet_status_messages.emplace_back(boost::io::str(FlexibleFormat(
                                                UserString("OPTIONS_DB_UI_PLANET_STATUS_NO_SUPPLY")) % planet->Name()));
            planet_status_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_supply.png", true);
        }

        // status: attacked on previous turn
        if (planet->LastTurnAttackedByShip() == CurrentTurn() - 1) {
            planet_status_messages.emplace_back(boost::io::str(FlexibleFormat(
                                                UserString("OPTIONS_DB_UI_PLANET_STATUS_ATTACKED")) % planet->Name()));
            planet_status_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_attacked.png", true);
        }

        // status: conquered on previous turn
        if (planet->LastTurnConquered() == CurrentTurn() - 1) {
            planet_status_messages.emplace_back(boost::io::str(FlexibleFormat(
                                                UserString("OPTIONS_DB_UI_PLANET_STATUS_CONQUERED")) % planet->Name()));
            planet_status_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_conquered.png", true);
        }

        // status: very unhappy
        if ((planet->GetMeter(METER_HAPPINESS)->Current() <= 5) && (planet->GetMeter(METER_POPULATION)->Current() > 0)) {
            planet_status_messages.emplace_back(boost::io::str(FlexibleFormat(
                UserString("OPTIONS_DB_UI_PLANET_STATUS_UNHAPPY")) % planet->Name()));
            planet_status_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_unhappy.png", true);
        }

        // status: bombarded (TBD)

        // status: several
        if (planet_status_messages.size() > 1) {
            planet_status_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_warning.png", true);
        }

        if (planet_status_messages.size() > 0 && planet_status_texture) {
            int texture_size = GetOptionsDB().Get<int>("UI.sidepanel-planet-status-icon-size");
            m_planet_status_graphic = Wnd::Create<GG::StaticGraphic>(GG::SubTexture(planet_status_texture),
                GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE);
            m_planet_status_graphic->SizeMove(GG::Pt(GG::X(4), GG::Y(4)), GG::Pt(GG::X(texture_size + 4), GG::Y(texture_size + 4)));

            std::string tooltip_message;
            for (std::string message : planet_status_messages)
                { tooltip_message += message + "\n\n"; }
            m_planet_status_graphic->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString("OPTIONS_DB_UI_PLANET_STATUS_ICON_TITLE"),
                                                  tooltip_message.substr(0, tooltip_message.size()-4)));
            m_planet_status_graphic->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
            AttachChild(m_planet_status_graphic);
        }
    }

    // set planetpanel stealth browse text
    ClearBrowseInfoWnd();

    if (client_empire_id != ALL_EMPIRES) {
        Empire* client_empire = GetEmpire(client_empire_id);
        Visibility visibility = GetUniverse().GetObjectVisibilityByEmpire(m_planet_id, client_empire_id);
        auto visibility_turn_map = GetUniverse().GetObjectVisibilityTurnMapByEmpire(m_planet_id, client_empire_id);
        float client_empire_detection_strength = client_empire->GetMeter("METER_DETECTION_STRENGTH")->Current();
        float apparent_stealth = planet->InitialMeterValue(METER_STEALTH);

        std::string visibility_info;
        std::string detection_info;

        if (visibility == VIS_NO_VISIBILITY) {
            visibility_info = UserString("PL_NO_VISIBILITY");

            auto last_turn_visible_it = visibility_turn_map.find(VIS_BASIC_VISIBILITY);
            if (last_turn_visible_it != visibility_turn_map.end() && last_turn_visible_it->second > 0) {
                visibility_info += "  " + boost::io::str(FlexibleFormat(UserString("PL_LAST_TURN_SEEN")) %
                                                                        std::to_string(last_turn_visible_it->second));
            }
            else {
                visibility_info += "  " + UserString("PL_NEVER_SEEN");
                ErrorLogger() << "Empire " << client_empire_id << " knows about planet " << planet->Name() <<
                                 " (id: " << m_planet_id << ") without having seen it before!";
            }

            auto system = GetSystem(planet->SystemID());
            if (system && system->GetVisibility(client_empire_id) <= VIS_BASIC_VISIBILITY) { // HACK: system is basically visible or less, so we must not be in detection range of the planet.
                detection_info = UserString("PL_NOT_IN_RANGE");
            }
            else if (apparent_stealth > client_empire_detection_strength) {
                detection_info = boost::io::str(FlexibleFormat(UserString("PL_APPARENT_STEALTH_EXCEEDS_DETECTION")) %
                                                std::to_string(apparent_stealth) %
                                                std::to_string(client_empire_detection_strength));
            }
            else {
                detection_info = boost::io::str(FlexibleFormat(UserString("PL_APPARENT_STEALTH_DOES_NOT_EXCEED_DETECTION")) %
                                                std::to_string(client_empire_detection_strength));
            }

            std::string info = visibility_info + "\n\n" + detection_info;
            SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString("METER_STEALTH"), info));
        }
        else if (visibility == VIS_BASIC_VISIBILITY) {
            visibility_info = UserString("PL_BASIC_VISIBILITY");

            auto last_turn_visible_it = visibility_turn_map.find(VIS_PARTIAL_VISIBILITY);
            if (last_turn_visible_it != visibility_turn_map.end() && last_turn_visible_it->second > 0) {
                visibility_info += "  " + boost::io::str(FlexibleFormat(UserString("PL_LAST_TURN_SCANNED")) %
                                                                        std::to_string(last_turn_visible_it->second));
            }
            else {
                visibility_info += "  " + UserString("PL_NEVER_SCANNED");
            }

            if (apparent_stealth > client_empire_detection_strength) {
                detection_info = boost::io::str(FlexibleFormat(UserString("PL_APPARENT_STEALTH_EXCEEDS_DETECTION")) %
                                                std::to_string(apparent_stealth)                  %
                                                std::to_string(client_empire_detection_strength));
            }
            else {
                detection_info = boost::io::str(FlexibleFormat(UserString("PL_APPARENT_STEALTH_DOES_NOT_EXCEED_DETECTION")) %
                                                std::to_string(client_empire_detection_strength));
            }

            std::string info = visibility_info + "\n\n" + detection_info;
            SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString("METER_STEALTH"), info));
        }
    }


    // BuildingsPanel::Refresh (and other panels) emit ExpandCollapseSignal,
    // which should be connected to SidePanel::PlanetPanel::DoLayout

    m_planet_connection = planet->StateChangedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::Refresh, this), boost::signals2::at_front);
}

void SidePanel::PlanetPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void SidePanel::PlanetPanel::SetFocus(const std::string& focus) {
    auto planet = GetPlanet(m_planet_id);
    if (!planet || !planet->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
        return;
    // todo: if focus is already equal to planet's focus, return early.
    colony_projections.clear();// in case new or old focus was Growth (important that be cleared BEFORE Order is issued)
    species_colony_projections.clear();
    HumanClientApp::GetApp()->Orders().IssueOrder(
        std::make_shared<ChangeFocusOrder>(HumanClientApp::GetApp()->EmpireID(), planet->ID(), focus));
}

bool SidePanel::PlanetPanel::InWindow(const GG::Pt& pt) const {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (!(ul <= pt && pt < lr))
        return false;

    GG::Pt planet_box_lr = ul + GG::Pt(GG::X(MaxPlanetDiameter()), GG::Y(MaxPlanetDiameter()));

    // if pt is to the right of the space where the planet render could be,
    // it doesn't matter whether the render is being shown
    if (pt.x >= planet_box_lr.x)
        return true;

    // if pt is in the horizontal space of the planet render, it matters
    // whether the render is being shown
    if (m_rotating_planet_graphic) {
        // showing full sized graphic.  size defaulted to above is accurate
    } else if (m_planet_graphic) {
        planet_box_lr = m_planet_graphic->LowerRight(); // smaller sized image being shown.  use its size.
    } else {
        return false;   // not showing a render, and pt is outside the non-render space, so is not over panel
    }

    // if pt is below planet render space, it can't be over the panel
    if (pt.y > planet_box_lr.y)
        return false;

    // TODO: consider corners

    // otherwise, pt is over render or graphic
    return true;
}

void SidePanel::PlanetPanel::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    //std::cout << "SidePanel::PlanetPanel::LClick m_planet_id: " << m_planet_id << std::endl;
    if (!Disabled())
        LeftClickedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        LeftDoubleClickedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    auto planet = GetPlanet(m_planet_id);
    if (!planet)
        return;

    auto system = GetSystem(planet->SystemID());

    // determine which other empires are at peace with client empire and have
    // an owned object in this fleet's system
    std::set<int> peaceful_empires_in_system;
    if (system) {
        auto system_objects = Objects().FindObjects<const UniverseObject>(system->ObjectIDs());
        for (auto& obj : system_objects) {
            if (obj->GetVisibility(client_empire_id) < VIS_PARTIAL_VISIBILITY)
                continue;
            if (obj->Owner() == client_empire_id || obj->Unowned())
                continue;
            if (peaceful_empires_in_system.count(obj->Owner()))
                continue;
            if (Empires().GetDiplomaticStatus(client_empire_id, obj->Owner()) != DIPLO_PEACE)
                continue;
            peaceful_empires_in_system.insert(obj->Owner());
        }
    }


    const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (ClientPlayerIsModerator() && map_wnd->GetModeratorActionSetting() != MAS_NoAction) {
        RightClickedSignal(planet->ID());  // response handled in MapWnd
        return;
    }

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    if (planet->OwnedBy(HumanClientApp::GetApp()->EmpireID()) && m_order_issuing_enabled
        && m_planet_name->InClient(pt))
    {
        auto rename_action = [this, planet]() { // rename planet
            auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(GG::X(350), UserString("SP_ENTER_NEW_PLANET_NAME"), planet->Name());
            edit_wnd->Run();

            if (!m_order_issuing_enabled)
                return;

            if (!RenameOrder::Check(HumanClientApp::GetApp()->EmpireID(), planet->ID(), edit_wnd->Result()))
                return;

            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<RenameOrder>(HumanClientApp::GetApp()->EmpireID(), planet->ID(), edit_wnd->Result()));
            Refresh();
        };
        popup->AddMenuItem(GG::MenuItem(UserString("SP_RENAME_PLANET"), false, false, rename_action));
    }

    auto pedia_to_planet_action = [this]() { ClientUI::GetClientUI()->ZoomToPlanetPedia(m_planet_id); };

    popup->AddMenuItem(GG::MenuItem(UserString("SP_PLANET_SUITABILITY"), false, false, pedia_to_planet_action));

    if (planet->OwnedBy(client_empire_id)
        && !peaceful_empires_in_system.empty()
        && !ClientPlayerIsModerator())
    {
        popup->AddMenuItem(GG::MenuItem(true));

        // submenus for each available recipient empire
        GG::MenuItem give_away_menu(UserString("ORDER_GIVE_PLANET_TO_EMPIRE"), false, false);
        for (auto& entry : Empires()) {
            int recipient_empire_id = entry.first;
            auto gift_action = [recipient_empire_id, client_empire_id, planet]() {
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    std::make_shared<GiveObjectToEmpireOrder>(
                        client_empire_id, planet->ID(), recipient_empire_id));
            };
            if (!peaceful_empires_in_system.count(recipient_empire_id))
                continue;
            give_away_menu.next_level.push_back(GG::MenuItem(entry.second->Name(), false, false, gift_action));
        }
        popup->AddMenuItem(std::move(give_away_menu));

        if (planet->OrderedGivenToEmpire() != ALL_EMPIRES) {
            auto ungift_action = [planet]() { // cancel give away order for this fleet
                const OrderSet orders = HumanClientApp::GetApp()->Orders();
                for (const auto& id_and_order : orders) {
                    if (auto order = std::dynamic_pointer_cast<
                        GiveObjectToEmpireOrder>(id_and_order.second))
                    {
                        if (order->ObjectID() == planet->ID()) {
                            HumanClientApp::GetApp()->Orders().RescindOrder(id_and_order.first);
                            // could break here, but won't to ensure there are no problems with doubled orders
                        }
                    }
                }

            };
            GG::MenuItem cancel_give_away_menu(UserString("ORDER_CANCEL_GIVE_PLANET"), false, false, ungift_action);
            popup->AddMenuItem(std::move(cancel_give_away_menu));
        }
    }

    popup->Run();
}

void SidePanel::PlanetPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void SidePanel::PlanetPanel::PreRender() {
    GG::Control::PreRender();
    DoLayout();
}

void SidePanel::PlanetPanel::Render() {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt name_ul = m_planet_name->UpperLeft() - GG::Pt(GG::X(EDGE_PAD), GG::Y0);
    GG::Pt name_lr = GG::Pt(lr.x, m_planet_name->Bottom());
    GG::Pt planet_box_lr = ul + GG::Pt(GG::X(MaxPlanetDiameter()), GG::Y(MaxPlanetDiameter()));
    bool show_planet_box = true;

    if (m_rotating_planet_graphic) {
        // default OK
    } else if (m_planet_graphic) {
        planet_box_lr = m_planet_graphic->LowerRight();
    } else {
        show_planet_box = false;    // no planet render to put box behind
    }


    GG::Clr background_colour = ClientUI::CtrlColor();
    GG::Clr title_background_colour = ClientUI::WndOuterBorderColor();
    GG::Clr border_colour = (m_selected ? m_empire_colour : ClientUI::WndOuterBorderColor());


    static const int OFFSET = 15;   // size of corners cut off sticky-out bit of background around planet render

    GG::GL2DVertexBuffer verts;
    verts.reserve(12);

    // title box background
    verts.store(name_lr.x,              name_ul.y);
    verts.store(name_ul.x,              name_ul.y);
    verts.store(name_ul.x,              name_lr.y);
    verts.store(name_lr.x,              name_lr.y);

    // main border / background
    verts.store(lr.x,                   ul.y);                      // top right corner
    if (show_planet_box) {
        verts.store(ul.x + OFFSET,      ul.y);                      // top left, offset right to cut off corner
        verts.store(ul.x,               ul.y + OFFSET);             // top left, offset down to cut off corner
        verts.store(ul.x,               planet_box_lr.y - OFFSET);  // bottom left, offset up to cut off corner
        verts.store(ul.x + OFFSET,      planet_box_lr.y);           // bottom left, offset right to cut off corner
        verts.store(planet_box_lr.x,    planet_box_lr.y);           // inner corner between planet box and rest of panel
    } else {
        verts.store(planet_box_lr.x,    ul.y);                      // top left of main panel, excluding planet box
    }
    verts.store(planet_box_lr.x,        lr.y);                      // bottom left of main panel
    verts.store(lr.x,                   lr.y);                      // bottom right

    verts.activate();

    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);

    // standard WndColor background for whole panel
    glColor(background_colour);
    glDrawArrays(GL_TRIANGLE_FAN, 4, verts.size() - 4);

    // title background box
    glColor(title_background_colour);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // border
    glColor(border_colour);
    glLineWidth(1.5f);
    glDrawArrays(GL_LINE_LOOP, 4, verts.size() - 4);
    glLineWidth(1.0f);

    // disable greyover
    static const GG::Clr HALF_GREY(128, 128, 128, 128);
    if (Disabled()) {
        glColor(HALF_GREY);
        glDrawArrays(GL_TRIANGLE_FAN, 4, verts.size() - 4);
    }

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}

void SidePanel::PlanetPanel::Select(bool selected) {
    if (m_selected != selected) {
        m_selected = selected;
        // TODO: consider setting text box colours?
    }
}

namespace {
    void CancelColonizeInvadeBombardScrapShipOrders(std::shared_ptr<const Ship> ship) {
        if (!ship)
            return;

        const ClientApp* app = ClientApp::GetApp();
        if (!app)
            return;
        const OrderSet orders = app->Orders();

        // is selected ship already ordered to colonize?  If so, recind that order.
        if (ship->OrderedColonizePlanet() != INVALID_OBJECT_ID) {
            for (const auto& id_and_order : orders) {
                if (auto order = std::dynamic_pointer_cast<ColonizeOrder>(id_and_order.second)) {
                    if (order->ShipID() == ship->ID()) {
                        HumanClientApp::GetApp()->Orders().RescindOrder(id_and_order.first);
                        // could break here, but won't to ensure there are no problems with doubled orders
                    }
                }
            }
        }

        // is selected ship ordered to invade?  If so, recind that order
        if (ship->OrderedInvadePlanet() != INVALID_OBJECT_ID) {
            for (const auto& id_and_order : orders) {
               if (auto order = std::dynamic_pointer_cast<InvadeOrder>(id_and_order.second)) {
                    if (order->ShipID() == ship->ID()) {
                        HumanClientApp::GetApp()->Orders().RescindOrder(id_and_order.first);
                        // could break here, but won't to ensure there are no problems with doubled orders
                    }
                }
            }
        }

        // is selected ship ordered scrapped?  If so, recind that order
        if (ship->OrderedScrapped()) {
            for (const auto& id_and_order : orders) {
                if (auto order = std::dynamic_pointer_cast<ScrapOrder>(id_and_order.second)) {
                    if (order->ObjectID() == ship->ID()) {
                        HumanClientApp::GetApp()->Orders().RescindOrder(id_and_order.first);
                        // could break here, but won't to ensure there are no problems with doubled orders
                    }
                }
            }
        }

        // is selected ship order to bombard?  If so, recind that order
        if (ship->OrderedBombardPlanet() != INVALID_OBJECT_ID) {
            for (const auto& id_and_order : orders) {
               if (auto order = std::dynamic_pointer_cast<BombardOrder>(id_and_order.second)) {
                    if (order->ShipID() == ship->ID()) {
                        HumanClientApp::GetApp()->Orders().RescindOrder(id_and_order.first);
                        // could break here, but won't to ensure there are no problems with doubled orders
                    }
                }
            }
        }
    }
}

void SidePanel::PlanetPanel::ClickColonize() {
    // order or cancel colonization, depending on whether it has previously
    // been ordered

    auto planet = GetPlanet(m_planet_id);
    if (!planet || planet->InitialMeterValue(METER_POPULATION) != 0.0 || !m_order_issuing_enabled)
        return;

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES)
        return;

    auto pending_colonization_orders = PendingColonizationOrders();
    auto it = pending_colonization_orders.find(m_planet_id);

    if (it != pending_colonization_orders.end()) {
        // cancel previous colonization order for planet
        HumanClientApp::GetApp()->Orders().RescindOrder(it->second);

    } else {
        // find colony ship and order it to colonize
        auto ship = ValidSelectedColonyShip(SidePanel::SystemID());
        if (!ship)
            ship = GetShip(AutomaticallyChosenColonyShip(m_planet_id));

        if (!ship) {
            ErrorLogger() << "SidePanel::PlanetPanel::ClickColonize valid colony not found!";
            return;
        }
        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "SidePanel::PlanetPanel::ClickColonize selected colony ship not owned by this client's empire.";
            return;
        }

        CancelColonizeInvadeBombardScrapShipOrders(ship);

        HumanClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ColonizeOrder>(empire_id, ship->ID(), m_planet_id));
    }
    OrderButtonChangedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::ClickInvade() {
    // order or cancel invasion, depending on whether it has previously
    // been ordered

    auto planet = GetPlanet(m_planet_id);
    if (!planet ||
        !m_order_issuing_enabled ||
        (planet->InitialMeterValue(METER_POPULATION) <= 0.0 && planet->Unowned()))
    { return; }

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES)
        return;

    auto pending_invade_orders = PendingInvadeOrders();
    auto it = pending_invade_orders.find(m_planet_id);

    if (it != pending_invade_orders.end()) {
        auto& planet_invade_orders = it->second;
        // cancel previous invasion orders for this planet
        for (int order_id : planet_invade_orders)
        { HumanClientApp::GetApp()->Orders().RescindOrder(order_id); }

    } else {
        // order selected invasion ships to invade planet
        auto invasion_ships = ValidSelectedInvasionShips(planet->SystemID());

        if (invasion_ships.empty()) {
            auto autoselected_invasion_ships = AutomaticallyChosenInvasionShips(m_planet_id);
            invasion_ships.insert(autoselected_invasion_ships.begin(), autoselected_invasion_ships.end());
        }

        for (auto& ship : invasion_ships) {
            if (!ship)
                continue;

            CancelColonizeInvadeBombardScrapShipOrders(ship);

            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<InvadeOrder>(empire_id, ship->ID(), m_planet_id));
        }
    }
    OrderButtonChangedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::ClickBombard() {
    // order or cancel bombard, depending on whether it has previously
    // been ordered

    auto planet = GetPlanet(m_planet_id);
    if (!planet ||
        !m_order_issuing_enabled ||
        (planet->InitialMeterValue(METER_POPULATION) <= 0.0 && planet->Unowned()))
    { return; }

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES)
        return;

    auto pending_bombard_orders = PendingBombardOrders();
    auto it = pending_bombard_orders.find(m_planet_id);

    if (it != pending_bombard_orders.end()) {
        auto& planet_bombard_orders = it->second;
        // cancel previous bombard orders for this planet
        for (int order_id : planet_bombard_orders)
        { HumanClientApp::GetApp()->Orders().RescindOrder(order_id); }

    } else {
        // order selected bombard ships to bombard planet
        auto bombard_ships = ValidSelectedBombardShips(planet->SystemID());

        if (bombard_ships.empty()) {
            auto autoselected_bombard_ships = AutomaticallyChosenBombardShips(m_planet_id);
            bombard_ships.insert(autoselected_bombard_ships.begin(), autoselected_bombard_ships.end());
        }

        for (auto& ship : bombard_ships) {
            if (!ship)
                continue;

            CancelColonizeInvadeBombardScrapShipOrders(ship);

            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<BombardOrder>(empire_id, ship->ID(), m_planet_id));
        }
    }
    OrderButtonChangedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::FocusDropListSelectionChangedSlot(GG::DropDownList::iterator selected) {
    // all this funciton needs to do is emit FocusChangedSignal.  The code
    // preceeding that determines which focus was selected from the iterator
    // parameter, does some safety checks, and disables UI sounds
    DebugLogger() << "SidePanel::PlanetPanel::FocusDropListSelectionChanged";
    if (m_focus_drop->CurrentItem() == m_focus_drop->end()) {
        ErrorLogger() << "PlanetPanel::FocusDropListSelectionChanged passed end / invalid interator";
        return;
    }

    auto res = GetResourceCenter(m_planet_id);
    if (!res) {
        ErrorLogger() << "PlanetPanel::FocusDropListSelectionChanged couldn't convert object with id " << m_planet_id << " to a ResourceCenter";
        return;
    }

    std::size_t i = m_focus_drop->IteratorToIndex(selected);
    if (i >= res->AvailableFoci().size()) {
        ErrorLogger() << "PlanetPanel::FocusDropListSelectionChanged got invalid focus selected index: " << i;
        return;
    }

    Sound::TempUISoundDisabler sound_disabler;
    DebugLogger() << "About to send focus-changed signal.";
    FocusChangedSignal(res->AvailableFoci().at(i));
    DebugLogger() << "Returned from sending focus-changed signal.";
}

void SidePanel::PlanetPanel::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;

    m_colonize_button->Disable(!enable);
    m_invade_button->Disable(!enable);
    m_bombard_button->Disable(!enable);

    m_buildings_panel->EnableOrderIssuing(enable);

    auto obj = GetUniverseObject(m_planet_id);
    if (!enable || !obj || !obj->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
        m_focus_drop->Disable();
    else
        m_focus_drop->Disable(false);
}

////////////////////////////////////////////////
// SidePanel::PlanetPanelContainer
////////////////////////////////////////////////
SidePanel::PlanetPanelContainer::PlanetPanelContainer() :
    Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
    m_planet_panels(),
    m_selected_planet_id(INVALID_OBJECT_ID),
    m_vscroll(GG::Wnd::Create<CUIScroll>(GG::VERTICAL)),
    m_ignore_recursive_resize(false)
{
    SetName("PlanetPanelContainer");
    SetChildClippingMode(ClipToClient);
    m_vscroll->ScrolledSignal.connect(
        boost::bind(&SidePanel::PlanetPanelContainer::VScroll, this, _1, _2, _3, _4));
    RequirePreRender();
}

SidePanel::PlanetPanelContainer::~PlanetPanelContainer()
{}

bool SidePanel::PlanetPanelContainer::InWindow(const GG::Pt& pt) const {
    // ensure pt is below top of container
    if (pt.y < Top())
        return false;

    // allow point to be within any planet panel that is below top of container
    for (auto& panel : m_planet_panels) {
        if (panel->InWindow(pt))
            return true;
    }

    // disallow point between containers that is left of solid portion of sidepanel.
    // this done by restricting InWindow to discard space between left of container
    // and left of solid portion that wasn't already caught above as being part
    // of a planet panel.

    return UpperLeft() + GG::Pt(GG::X(MaxPlanetDiameter()), GG::Y0) <= pt && pt < LowerRight();
}

void SidePanel::PlanetPanelContainer::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) {
    if (m_vscroll && m_vscroll->Parent().get() == this) {
        const std::pair<int, int> initial_pos = m_vscroll->PosnRange();
        if (move < 0)
            m_vscroll->ScrollLineIncr();
        else
            m_vscroll->ScrollLineDecr();
        if (initial_pos != m_vscroll->PosnRange())
            GG::SignalScroll(*m_vscroll, true);
    }
}

void SidePanel::PlanetPanelContainer::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

int SidePanel::PlanetPanelContainer::ScrollPosition() const {
    if (m_vscroll && m_vscroll->Parent().get() == this)
        return m_vscroll->PosnRange().first;
    else
        return 0;
}

void SidePanel::PlanetPanelContainer::ScrollTo(int pos) {
    if (m_vscroll && m_vscroll->Parent().get() == this) {
        auto initial_pos = m_vscroll->PosnRange();
        m_vscroll->ScrollTo(pos);
        if (initial_pos != m_vscroll->PosnRange())
            GG::SignalScroll(*m_vscroll, true);
    }
}

void SidePanel::PlanetPanelContainer::Clear() {
    m_planet_panels.clear();
    m_selected_planet_id = INVALID_OBJECT_ID;
    DetachChildren();
    AttachChild(m_vscroll);
}

void SidePanel::PlanetPanelContainer::SetPlanets(const std::vector<int>& planet_ids, StarType star_type) {
    //std::cout << "SidePanel::PlanetPanelContainer::SetPlanets( size: " << planet_ids.size() << " )" << std::endl;

    int initial_selected_planet_panel = m_selected_planet_id;

    // remove old panels
    Clear();

    std::multimap<int, int> orbits_planets;
    for (int planet_id : planet_ids) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetPanelContainer::SetPlanets couldn't find planet with id " << planet_id;
            continue;
        }
        int system_id = planet->SystemID();
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "PlanetPanelContainer::SetPlanets couldn't find system of planet" << planet->Name();
            continue;
        }
        orbits_planets.insert({system->OrbitOfPlanet(planet_id), planet_id});
    }

    // create new panels and connect their signals
    for (auto& orbit_planet : orbits_planets) {
        auto planet_panel = GG::Wnd::Create<PlanetPanel>(Width() - m_vscroll->Width(),
                                                         orbit_planet.second, star_type);
        AttachChild(planet_panel);
        m_planet_panels.push_back(planet_panel);
        m_planet_panels.back()->LeftClickedSignal.connect(
            PlanetClickedSignal);
        m_planet_panels.back()->LeftDoubleClickedSignal.connect(
            PlanetLeftDoubleClickedSignal);
        m_planet_panels.back()->RightClickedSignal.connect(
            PlanetRightClickedSignal);
        m_planet_panels.back()->BuildingRightClickedSignal.connect(
            BuildingRightClickedSignal);
        m_planet_panels.back()->ResizedSignal.connect(
            boost::bind(&SidePanel::PlanetPanelContainer::RequirePreRender, this));
        m_planet_panels.back()->OrderButtonChangedSignal.connect(
            [this](int excluded_planet_id) {
                RefreshAllPlanetPanels(excluded_planet_id, true);
            });
    }

    // disable non-selectable planet panels
    DisableNonSelectionCandidates();

    // redo contents and layout of panels, after enabling or disabling, so
    // they take this into account when doing contents
    RefreshAllPlanetPanels();

    SelectPlanet(initial_selected_planet_panel);

    RequirePreRender();
}

void SidePanel::PlanetPanelContainer::DoPanelsLayout() {
    if (m_ignore_recursive_resize)
        return;

    GG::ScopedAssign<bool> prevent_inifinite_recursion(m_ignore_recursive_resize, true);

    // Determine if scroll bar is required for height or expected because it is already present.
    GG::Y available_height = Height();
    GG::Y initially_used_height = GG::Y0;
    for (auto& panel : m_planet_panels) {
        initially_used_height += panel->Height() + EDGE_PAD;
    }

    bool vscroll_expected((initially_used_height > available_height)
                          || ((m_vscroll->Parent().get() == this)
                              && (m_vscroll->PosnRange().first > 0)) );

    // Size panels accounting for the expected vscroll
    GG::Y actual_used_height = ResizePanelsForVScroll(vscroll_expected);

    // Check that the expected height and the actual height are consistent with
    // the choice of vscroll.  Some planet panels can result in an unstable
    // situation where narrowing the panel for the vscroll results in a
    // **shorter** panel, which would not require a vscroll.  In those
    // situations always use the vscroll.
    bool vscroll_required(vscroll_expected);
    if (!vscroll_expected && (actual_used_height > available_height)) {
        vscroll_required = true;

        // Size panels accounting for the required vscroll
        ResizePanelsForVScroll(vscroll_required);
    }

    GG::Y y = GG::Y0;

    // if scrollbar present, start placing panels above the top of this
    // container by a distances determined by the scrollbar position
    if (vscroll_required && m_vscroll->Parent().get() == this)
        y = GG::Y(-m_vscroll->PosnRange().first);

    // place panels in sequence from the top, each below the previous
    for (auto& panel : m_planet_panels) {
        panel->MoveTo(GG::Pt(GG::X0, y));
        y += panel->Height() + EDGE_PAD;
    }

    // Hide the scroll bar if not required.
    if (!vscroll_required) {
        DetachChild(m_vscroll);
        return;
    }

    // show and adjust scrollbar size to represent space needed for panels
    AttachChild(m_vscroll);
    m_vscroll->Show();

    unsigned int line_size = MaxPlanetDiameter();
    unsigned int page_size = Value(available_height);
    int scroll_max = Value(actual_used_height);
    m_vscroll->SizeScroll(0, scroll_max, line_size, page_size);
}

GG::Y SidePanel::PlanetPanelContainer::ResizePanelsForVScroll(bool use_vscroll) {
    // Size panels accounting for the expected vscroll
    GG::X expected_width(Width() - (use_vscroll ? m_vscroll->Width() : GG::X0));
    GG::Y used_height(GG::Y0);
    for (auto& panel : m_planet_panels) {
        panel->Resize(GG::Pt(expected_width, panel->Height()));

        // Force planet panel container to render.  Since planet panel rendering
        // is slow its absence is visible as a glitch.
        GG::GUI::PreRenderWindow(panel);
        used_height += panel->Height() + EDGE_PAD;
    }
    return used_height;
}

void SidePanel::PlanetPanelContainer::PreRender() {
    GG::Wnd::PreRender();
    DoLayout();
}

void SidePanel::PlanetPanelContainer::DoLayout() {
    GG::Pt scroll_ul(Width() - ClientUI::ScrollWidth(), GG::Y0);
    GG::Pt scroll_lr = scroll_ul + GG::Pt(GG::X(ClientUI::ScrollWidth()), Height());
    m_vscroll->SizeMove(scroll_ul, scroll_lr);

    DoPanelsLayout();
}

void SidePanel::PlanetPanelContainer::SelectPlanet(int planet_id) {
    //std::cout << "SidePanel::PlanetPanelContainer::SelectPlanet(" << planet_id << ")" << std::endl;
    if (planet_id != m_selected_planet_id && m_candidate_ids.count(planet_id)) {
        m_selected_planet_id = planet_id;
        bool planet_id_match_found = false;

        // scan through panels in container, marking the selected one and
        // unmarking the rest, and remembering if any are marked
        for (auto& panel : m_planet_panels) {
            if (panel->PlanetID() == m_selected_planet_id) {
                panel->Select(true);
                planet_id_match_found = true;
                //std::cout << " ... selecting planet with id " << panel->PlanetID() << std::endl;
            } else {
                panel->Select(false);
            }
        }

        // if a panel was marked, signal this fact
        if (!planet_id_match_found) {
            m_selected_planet_id = INVALID_OBJECT_ID;
            //std::cout << " ... no planet with requested ID found" << std::endl;
        }

        // send order changes could be made on other planets
        HumanClientApp::GetApp()->SendPartialOrders();
    }
}

void SidePanel::PlanetPanelContainer::SetValidSelectionPredicate(const std::shared_ptr<UniverseObjectVisitor>& visitor)
{ m_valid_selection_predicate = visitor; }

void SidePanel::PlanetPanelContainer::DisableNonSelectionCandidates() {
    //std::cout << "SidePanel::PlanetPanelContainer::DisableNonSelectionCandidates" << std::endl;
    m_candidate_ids.clear();
    std::set<std::shared_ptr<PlanetPanel>> disabled_panels;

    if (m_valid_selection_predicate) {
        // if there is a selection predicate, which determines which planet panels
        // can be selected, refresh the candidiates and disable the non-selectables

        // find selectables
        for (auto& panel : m_planet_panels) {
            int planet_id = panel->PlanetID();
            auto planet = GetPlanet(planet_id);

            if (planet && planet->Accept(*m_valid_selection_predicate)) {
                m_candidate_ids.insert(planet_id);
                //std::cout << " ... planet " << planet->ID() << " is a selection candidate" << std::endl;
            } else {
                disabled_panels.insert(panel);
            }
        }
    }

    // disable and enabled appropriate panels
    for (auto& panel : m_planet_panels) {
        if (disabled_panels.count(panel)) {
            panel->Disable(true);
            //std::cout << " ... DISABLING PlanetPanel for planet " << panel->PlanetID() << std::endl;
        } else {
            panel->Disable(false);
        }
    }
}

void SidePanel::PlanetPanelContainer::VScroll(int pos_top, int pos_bottom, int range_min, int range_max) {
    if (pos_bottom > range_max) {
        // prevent scrolling beyond allowed max
        int extra = pos_bottom - range_max;
        pos_top -= extra;
    }

    RequirePreRender();
}

void SidePanel::PlanetPanelContainer::RefreshAllPlanetPanels(
    int excluded_planet_id, bool require_prerender)
{
    for (auto& panel : m_planet_panels) {
        if (excluded_planet_id > 0 && panel->PlanetID() == INVALID_OBJECT_ID)
            continue;
        panel->Refresh();
        if (require_prerender)
            panel->RequirePreRender();
    }
}

void SidePanel::PlanetPanelContainer::ShowScrollbar()
{ RequirePreRender(); }

void SidePanel::PlanetPanelContainer::HideScrollbar()
{ DetachChild(m_vscroll); }

void SidePanel::PlanetPanelContainer::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void SidePanel::PlanetPanelContainer::EnableOrderIssuing(bool enable/* = true*/) {
    for (auto& panel : m_planet_panels) {
        panel->EnableOrderIssuing(enable);
    }
}

namespace {
    GG::X LabelWidth()
    { return GG::X(ClientUI::Pts() * 9); }

    GG::X ValueWidth()
    { return GG::X(ClientUI::Pts() * 4); }

    GG::Y RowHeight()
    { return GG::Y(ClientUI::Pts() * 3 / 2); }

    class SystemMeterBrowseWnd : public GG::BrowseInfoWnd {
    public:
        SystemMeterBrowseWnd(MeterType meter_type, int system_id) :
            GG::BrowseInfoWnd(GG::X0, GG::Y0, LabelWidth() + ValueWidth(), GG::Y1),
            m_meter_type(meter_type),
            m_system_id(system_id)
        {}

        bool WndHasBrowseInfo(const GG::Wnd* wnd, std::size_t mode) const override {
            assert(mode <= wnd->BrowseModes().size());
            return true;
        }

        void Render() override {
            const GG::Pt ul(UpperLeft());
            const GG::Pt lr(LowerRight());
            // main background
            GG::FlatRectangle(ul, lr, OpaqueColor(ClientUI::WndColor()), ClientUI::WndOuterBorderColor(), 1);
            // title bar background
            GG::FlatRectangle(ul, GG::Pt(lr.x, ul.y + RowHeight()),
                ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);
        }

        void UpdateImpl(std::size_t mode, const GG::Wnd* target) override {
            const GG::Y row_height(RowHeight());
            for (const auto& label_pair : m_labels_and_amounts) {
                DetachChild(label_pair.first);
                DetachChild(label_pair.second);
            }
            m_labels_and_amounts.clear();

            auto system = GetSystem(m_system_id);
            if (!system)
                return;

            auto total_meter_value = 0.0f;

            auto title_label = GG::Wnd::Create<CUILabel>(UserString(boost::lexical_cast<std::string>(m_meter_type)), GG::FORMAT_RIGHT);
            title_label->MoveTo(GG::Pt(GG::X0, GG::Y0));
            title_label->Resize(GG::Pt(LabelWidth(), row_height));
            title_label->SetFont(ClientUI::GetBoldFont());
            AttachChild(title_label);

            GG::Y top = row_height;
            // add label-value pair for each resource-producing object in system to indicate amount of resource produced
            auto objects = Objects().FindObjects<const Planet>(system->ContainedObjectIDs());
            for (const auto& planet : objects) {
                // Ignore empty planets
                if (planet->Unowned() && planet->SpeciesName().empty())
                    continue;

                const auto name = planet->Name();
                auto label = GG::Wnd::Create<CUILabel>(name, GG::FORMAT_RIGHT);
                label->MoveTo(GG::Pt(GG::X0, top));
                label->Resize(GG::Pt(LabelWidth(), row_height));
                AttachChild(label);

                const auto meter_value = planet->InitialMeterValue(m_meter_type);
                if (m_meter_type == METER_SUPPLY) {
                    total_meter_value = std::max(total_meter_value, meter_value);
                } else {
                    total_meter_value += meter_value;
                }
                auto value = GG::Wnd::Create<CUILabel>(DoubleToString(meter_value, 3, false));
                value->MoveTo(GG::Pt(LabelWidth(), top));
                value->Resize(GG::Pt(ValueWidth(), row_height));
                AttachChild(value);

                m_labels_and_amounts.emplace_back(label, value);

                top += row_height;
            }

            auto title_value = GG::Wnd::Create<CUILabel>(DoubleToString(total_meter_value, 3, false));
            title_value->MoveTo(GG::Pt(LabelWidth(), GG::Y0));
            title_value->Resize(GG::Pt(ValueWidth(), row_height));
            title_value->SetFont(ClientUI::GetBoldFont());
            AttachChild(title_value);
            m_labels_and_amounts.emplace_back(title_label, title_value);

            Resize(GG::Pt(LabelWidth() + ValueWidth(), top));
        }
    private:
        MeterType m_meter_type;
        int m_system_id;
        std::vector<std::pair<std::shared_ptr<GG::Label>, std::shared_ptr<GG::Label>>> m_labels_and_amounts;
    };
}

////////////////////////////////////////////////
// SidePanel
////////////////////////////////////////////////
// static(s)
int                                        SidePanel::s_system_id = INVALID_OBJECT_ID;
int                                        SidePanel::s_planet_id = INVALID_OBJECT_ID;
bool                                       SidePanel::s_needs_update = false;
bool                                       SidePanel::s_needs_refresh = false;
std::set<std::weak_ptr<SidePanel>, std::owner_less<std::weak_ptr<SidePanel>>> SidePanel::s_side_panels;
std::set<boost::signals2::connection>      SidePanel::s_system_connections;
std::map<int, boost::signals2::connection> SidePanel::s_fleet_state_change_signals;
boost::signals2::signal<void ()>           SidePanel::ResourceCenterChangedSignal;
boost::signals2::signal<void (int)>        SidePanel::PlanetSelectedSignal;
boost::signals2::signal<void (int)>        SidePanel::PlanetRightClickedSignal;
boost::signals2::signal<void (int)>        SidePanel::PlanetDoubleClickedSignal;
boost::signals2::signal<void (int)>        SidePanel::BuildingRightClickedSignal;
boost::signals2::signal<void (int)>        SidePanel::SystemSelectedSignal;

SidePanel::SidePanel(const std::string& config_name) :
    CUIWnd("", GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP, config_name)
{}

void SidePanel::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    m_planet_panel_container = GG::Wnd::Create<PlanetPanelContainer>();
    AttachChild(m_planet_panel_container);

    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    m_button_prev = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrowmouseover.png")));

    m_button_next = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrowmouseover.png")));

    Sound::TempUISoundDisabler sound_disabler;

    m_system_name = GG::Wnd::Create<SystemNameDropDownList>(6);
    m_system_name->SetStyle(GG::LIST_NOSORT | GG::LIST_SINGLESEL);
    m_system_name->DisableDropArrow();
    m_system_name->SetInteriorColor(GG::Clr(0, 0, 0, 200));
    m_system_name->ManuallyManageColProps();
    m_system_name->NormalizeRowsOnInsert(false);
    m_system_name->SetNumCols(1);
    AttachChild(m_system_name);

    m_star_type_text = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(), ClientUI::TextColor());
    AttachChild(m_star_type_text);
    AttachChild(m_button_prev);
    AttachChild(m_button_next);

    m_system_resource_summary = GG::Wnd::Create<MultiIconValueIndicator>(Width() - EDGE_PAD*2);
    AttachChild(m_system_resource_summary);

    m_system_name->DropDownOpenedSignal.connect(
        boost::bind(&SidePanel::SystemNameDropListOpenedSlot, this, _1));
    m_system_name->SelChangedSignal.connect(
        boost::bind(&SidePanel::SystemSelectionChangedSlot, this, _1));
    m_system_name->SelChangedWhileDroppedSignal.connect(
        boost::bind(&SidePanel::SystemSelectionChangedSlot, this, _1));
    m_button_prev->LeftClickedSignal.connect(
        boost::bind(&SidePanel::PrevButtonClicked, this));
    m_button_next->LeftClickedSignal.connect(
        boost::bind(&SidePanel::NextButtonClicked, this));
    m_planet_panel_container->PlanetClickedSignal.connect(
        boost::bind(&SidePanel::PlanetClickedSlot, this, _1));
    m_planet_panel_container->PlanetLeftDoubleClickedSignal.connect(
        PlanetDoubleClickedSignal);
    m_planet_panel_container->PlanetRightClickedSignal.connect(
        PlanetRightClickedSignal);
    m_planet_panel_container->BuildingRightClickedSignal.connect(
        BuildingRightClickedSignal);

    SetMinSize(GG::Pt(GG::X(MaxPlanetDiameter() + BORDER_LEFT + BORDER_RIGHT + 120),
                      PLANET_PANEL_TOP + GG::Y(MaxPlanetDiameter())));

    s_needs_refresh = true;
    s_needs_update  = true;
    RequirePreRender();
    Hide();

    s_side_panels.insert(std::weak_ptr<SidePanel>(std::dynamic_pointer_cast<SidePanel>(shared_from_this())));
}

SidePanel::~SidePanel() {
    // disconnect any existing stored signals
    while (!s_system_connections.empty()) {
        s_system_connections.begin()->disconnect();
        s_system_connections.erase(s_system_connections.begin());
    }
    while (!s_fleet_state_change_signals.empty()) {
        s_fleet_state_change_signals.begin()->second.disconnect();
        s_fleet_state_change_signals.erase(s_fleet_state_change_signals.begin());
    }
}

bool SidePanel::InWindow(const GG::Pt& pt) const {
    return (UpperLeft() + GG::Pt(GG::X(MaxPlanetDiameter()), GG::Y0) <= pt && pt < LowerRight())
           || (m_planet_panel_container && m_planet_panel_container->InWindow(pt))
           || (m_system_resource_summary && m_system_resource_summary->Parent().get() == this && m_system_resource_summary->InWindow(pt));
}

GG::Pt SidePanel::ClientUpperLeft() const
{ return GG::Wnd::UpperLeft() + GG::Pt(BORDER_LEFT, BORDER_BOTTOM); }

void SidePanel::InitBuffers() {
    m_vertex_buffer.clear();
    m_vertex_buffer.reserve(19);
    m_buffer_indices.resize(4);
    std::size_t previous_buffer_size = m_vertex_buffer.size();

    GG::Pt ul = UpperLeft() + GG::Pt(GG::X(MaxPlanetDiameter() + 2), GG::Y0);
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft() + GG::Pt(GG::X(MaxPlanetDiameter() + 2), PLANET_PANEL_TOP);
    GG::Pt cl_lr = lr - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);


    // within m_vertex_buffer:
    // [0] is the start and range for minimized background triangle fan and minimized border line loop (probably not actually used for sidepanel, but to be consistent will leave in)
    // [1] is ... the background fan / outer border line loop
    // [2] is ... the inner border line loop
    // [3] is ... the resize tab line list

    // minimized background fan and border line loop
    m_vertex_buffer.store(Value(ul.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(lr.y));
    m_vertex_buffer.store(Value(ul.x),  Value(lr.y));
    m_buffer_indices[0].first = previous_buffer_size;
    m_buffer_indices[0].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // outer border, with optional corner cutout
    m_vertex_buffer.store(Value(ul.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(ul.y));
    if (!m_resizable) {
        m_vertex_buffer.store(Value(lr.x),                            Value(lr.y) - OUTER_EDGE_ANGLE_OFFSET);
        m_vertex_buffer.store(Value(lr.x) - OUTER_EDGE_ANGLE_OFFSET,  Value(lr.y));
    } else {
        m_vertex_buffer.store(Value(lr.x),  Value(lr.y));
    }
    m_vertex_buffer.store(Value(ul.x),      Value(lr.y));
    m_buffer_indices[1].first = previous_buffer_size;
    m_buffer_indices[1].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // inner border, with optional corner cutout
    m_vertex_buffer.store(Value(cl_ul.x),       Value(cl_ul.y));
    m_vertex_buffer.store(Value(cl_lr.x),       Value(cl_ul.y));
    if (m_resizable) {
        m_vertex_buffer.store(Value(cl_lr.x),                             Value(cl_lr.y) - INNER_BORDER_ANGLE_OFFSET);
        m_vertex_buffer.store(Value(cl_lr.x) - INNER_BORDER_ANGLE_OFFSET, Value(cl_lr.y));
    } else {
        m_vertex_buffer.store(Value(cl_lr.x),   Value(cl_lr.y));
    }
    m_vertex_buffer.store(Value(cl_ul.x),       Value(cl_lr.y));
    m_buffer_indices[2].first = previous_buffer_size;
    m_buffer_indices[2].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // resize hash marks
    m_vertex_buffer.store(Value(cl_lr.x),                           Value(cl_lr.y) - RESIZE_HASHMARK1_OFFSET);
    m_vertex_buffer.store(Value(cl_lr.x) - RESIZE_HASHMARK1_OFFSET, Value(cl_lr.y));
    m_vertex_buffer.store(Value(cl_lr.x),                           Value(cl_lr.y) - RESIZE_HASHMARK2_OFFSET);
    m_vertex_buffer.store(Value(cl_lr.x) - RESIZE_HASHMARK2_OFFSET, Value(cl_lr.y));
    m_buffer_indices[3].first = previous_buffer_size;
    m_buffer_indices[3].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    m_vertex_buffer.createServerBuffer();
}

void SidePanel::PreRender() {
    CUIWnd::PreRender();

    // save initial scroll position so it can be restored after repopulating the planet panel container
    const int initial_scroll_pos = m_planet_panel_container->ScrollPosition();

    // Needs refresh updates all data related to all SizePanels, including system list etc.
    if (s_needs_refresh)
        RefreshInPreRender();

    // Update updates the data for each planet tab in all SidePanels
    if (s_needs_update) {
        for (auto& weak_panel : s_side_panels)
            if (auto panel = weak_panel.lock())
                panel->UpdateImpl();
    }

    // On a resize only DoLayout should be called.
    DoLayout();

    // restore planet panel container scroll position from before clearing
    if (s_needs_refresh || s_needs_update)
        m_planet_panel_container->ScrollTo(initial_scroll_pos);

    // restore planet selection
    m_planet_panel_container->SelectPlanet(s_planet_id);

    s_needs_refresh = false;
    s_needs_update  = false;
}

void SidePanel::Update() {
    s_needs_update = true;
    for (auto& weak_panel : s_side_panels)
        if (auto panel = weak_panel.lock())
            panel->RequirePreRender();
}

void SidePanel::UpdateImpl() {
    //std::cout << "SidePanel::UpdateImpl" << std::endl;
    if (m_system_resource_summary)
        m_system_resource_summary->Update();
    // update individual PlanetPanels in PlanetPanelContainer, then redo layout of panel container
    m_planet_panel_container->RefreshAllPlanetPanels();
}

void SidePanel::Refresh() {
    s_needs_refresh = true;
    for (auto& weak_panel : s_side_panels)
        if (auto panel = weak_panel.lock())
            panel->RequirePreRender();
}

void SidePanel::RefreshInPreRender() {
    // disconnect any existing system and fleet signals
    for (const auto& con : s_system_connections)
        con.disconnect();
    s_system_connections.clear();

    for (auto& entry : s_fleet_state_change_signals)
        entry.second.disconnect();
    s_fleet_state_change_signals.clear();

    // clear any previous colony projections
    colony_projections.clear();
    species_colony_projections.clear();


    // refresh individual panels' contents
    for (auto& weak_panel : s_side_panels)
        if (auto panel = weak_panel.lock())
            panel->RefreshImpl();


    // early exit if no valid system object to get or connect signals to
    if (s_system_id == INVALID_OBJECT_ID)
        return;


    // connect state changed and insertion signals for planets and fleets in system
    auto system = GetSystem(s_system_id);
    if (!system) {
        ErrorLogger() << "SidePanel::Refresh couldn't get system with id " << s_system_id;
        return;
    }

    for (auto& planet : Objects().FindObjects<Planet>(system->PlanetIDs())) {
        s_system_connections.insert(planet->ResourceCenterChangedSignal.connect(
                                        SidePanel::ResourceCenterChangedSignal));
    }

    for (auto& fleet : Objects().FindObjects<Fleet>(system->FleetIDs())) {
        s_fleet_state_change_signals[fleet->ID()] = fleet->StateChangedSignal.connect(
                                                        &SidePanel::Update);
    }

    //s_system_connections.insert(s_system->StateChangedSignal.connect(&SidePanel::Update));
    s_system_connections.insert(system->FleetsInsertedSignal.connect(
        &SidePanel::FleetsInserted));
    s_system_connections.insert(system->FleetsRemovedSignal.connect(
        &SidePanel::FleetsRemoved));
}

void SidePanel::RefreshSystemNames() {
    //auto system = GetSystem(s_system_id);
    //if (!system)
    //    return;

    // Repopulate the system with all of the names of known systems, if it is closed.
    // If it is open do not change the system names because it runs in a seperate ModalEventPump
    // from the main UI.
    if (m_system_name->Dropped())
        return;

    m_system_name->Clear();

    //Sort the names

    // The system names are manually sorted here and not automatically in
    // the vectorized insert because the ListBox is currently never
    // resorted, inserted or deleted so this was faster when profiled.  If
    // the performance of the std::stable_sort used in ListBox improves to
    // N logN when switching to C++11 then this should simply insert the
    // entire vector into the ListBox.  If the approach switches to
    // maintaing the list by incrementally inserting/deleting system
    // names, then this approach should also be dropped.
    std::set<std::pair<std::string, int>> sorted_systems;
    for (auto& system : Objects().FindObjects<System>()) {
        // Skip rows for systems that aren't known to this client, except the selected system
        if (!system->Name().empty() || system->ID() == s_system_id)
            sorted_systems.insert({system->Name(), system->ID()});
    }

    auto system_name_font(ClientUI::GetBoldFont(SystemNameFontSize()));
    GG::Y system_name_height(system_name_font->Lineskip() + 4);

    // Make a vector of sorted rows and insert them in a single operation.
    std::vector<std::shared_ptr<GG::DropDownList::Row>> rows;
    rows.reserve(sorted_systems.size());
    for (const auto& entry : sorted_systems) {
        int sys_id = entry.second;
        rows.push_back(GG::Wnd::Create<SystemRow>(sys_id, system_name_height));
    }
    m_system_name->Insert(rows);

    // Select in the ListBox the currently-selected system.
    for (auto it = m_system_name->begin(); it != m_system_name->end(); ++it) {
        if (auto row = dynamic_cast<const SystemRow*>(it->get())) {
            if (s_system_id == row->SystemID()) {
                m_system_name->Select(it);
                break;
            }
        }
    }
}

void SidePanel::RefreshImpl() {
    ScopedTimer sidepanel_refresh_impl_timer("SidePanel::RefreshImpl", true);
    Sound::TempUISoundDisabler sound_disabler;

    // clear out current contents
    m_planet_panel_container->Clear();
    m_star_type_text->SetText("");
    DetachChildAndReset(m_star_graphic);
    DetachChildAndReset(m_system_resource_summary);


    RefreshSystemNames();

    auto system = GetSystem(s_system_id);
    // if no system object, there is nothing to populate with.  early abort.
    if (!system)
        return;

    // (re)create top right star graphic
    auto graphic = ClientUI::GetClientUI()->GetModuloTexture(
        ClientUI::ArtDir() / "stars_sidepanel",
        ClientUI::StarTypeFilePrefixes()[system->GetStarType()],
        s_system_id);
    std::vector<std::shared_ptr<GG::Texture>> textures;
    textures.push_back(graphic);

    int graphic_width = Value(Width()) - MaxPlanetDiameter();
    DetachChild(m_star_graphic);
    m_star_graphic = GG::Wnd::Create<GG::DynamicGraphic>(
        GG::X(MaxPlanetDiameter()), GG::Y0, GG::X(graphic_width), GG::Y(graphic_width),
        true, textures[0]->DefaultWidth(), textures[0]->DefaultHeight(),
        0, textures, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    AttachChild(m_star_graphic);
    MoveChildDown(m_star_graphic);


    // star type
    m_star_type_text->SetText("<s>" + GetStarTypeName(system) + "</s>");


    // configure selection of planet panels in panel container
    std::shared_ptr<UniverseObjectVisitor> vistor;
    if (m_selection_enabled) {
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        if (empire_id != ALL_EMPIRES)
            vistor = std::make_shared<OwnedVisitor<Planet>>(empire_id);
    }
    m_planet_panel_container->SetValidSelectionPredicate(vistor);


    // update planet panel container contents (applying just-set selection predicate)
    //std::cout << " ... setting planet panel container planets" << std::endl;
    const auto& planet_ids = system->PlanetIDs();
    std::vector<int> planet_ids_vec(planet_ids.begin(), planet_ids.end());
    m_planet_panel_container->SetPlanets(planet_ids_vec, system->GetStarType());


    // populate system resource summary

    // for getting just the planets owned by player's empire
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    // If all planets are owned by the same empire, then we show the Shields/Defense/Troops/Supply;
    // regardless, if there are any planets owned by the player in the system, we show
    // Production/Research/Trade.
    int all_owner_id = ALL_EMPIRES;
    bool all_planets_share_owner = true;
    std::vector<int> all_planets, player_planets;
    for (auto& planet : Objects().FindObjects<const Planet>(planet_ids)) {
        // If it is neither owned nor populated with natives, it can be ignored.
        if (planet->Unowned() && planet->SpeciesName().empty())
            continue;

        int owner = planet->Owner();
        // If all planets have the same owner as each other, then they must have the same owner
        // as the first planet, so store its owner here when finding the first planet.
        if (all_planets.empty())
            all_owner_id = owner;
        if (owner != all_owner_id)
            all_planets_share_owner = false;

        all_planets.push_back(planet->ID());
        if (owner == empire_id)
            player_planets.push_back(planet->ID());
    }

    // Resource meters; show only for player planets
    const std::vector<std::pair<MeterType, MeterType>> resource_meters =
       {{METER_INDUSTRY, METER_TARGET_INDUSTRY},
        {METER_RESEARCH, METER_TARGET_RESEARCH},
        //{METER_TRADE,    METER_TARGET_TRADE},
        {METER_STOCKPILE,METER_MAX_STOCKPILE}};
    // general meters; show only if all planets are owned by same empire
    const std::vector<std::pair<MeterType, MeterType>> general_meters =
       {{METER_SHIELD,  METER_MAX_SHIELD},
        {METER_DEFENSE, METER_MAX_DEFENSE},
        {METER_TROOPS,  METER_MAX_TROOPS},
        {METER_SUPPLY,  METER_MAX_SUPPLY}};
    std::vector<std::pair<MeterType, MeterType>> meter_types;
    if (!player_planets.empty()) {
        meter_types.insert(meter_types.end(), resource_meters.begin(), resource_meters.end());
    }
    if (all_planets_share_owner) {
        meter_types.insert(meter_types.end(), general_meters.begin(), general_meters.end());
    }

    // refresh the system resource summary.
    m_system_resource_summary = GG::Wnd::Create<MultiIconValueIndicator>(
        Width() - MaxPlanetDiameter() - 8,
        all_planets_share_owner ? all_planets : player_planets,
        meter_types);
    m_system_resource_summary->MoveTo(GG::Pt(GG::X(MaxPlanetDiameter() + 4),
                                             140 - m_system_resource_summary->Height()));
    AttachChild(m_system_resource_summary);


    // add tooltips and show system resource summary if it is not empty
    if (m_system_resource_summary->Empty()) {
        DetachChild(m_system_resource_summary);
    } else {
        // add tooltips to the system resource summary
        for (const auto& entry : resource_meters) {
            MeterType type = entry.first;
            m_system_resource_summary->SetToolTip(type,
                GG::Wnd::Create<SystemResourceSummaryBrowseWnd>(
                    MeterToResource(type), s_system_id, empire_id));
        }
        // and the other meters
        for (const auto& entry : general_meters) {
            MeterType type = entry.first;
            m_system_resource_summary->SetToolTip(type,
                GG::Wnd::Create<SystemMeterBrowseWnd>(type, s_system_id));
        }

        AttachChild(m_system_resource_summary);
        m_system_resource_summary->Update();
    }
}

void SidePanel::DoLayout() {
    if (m_system_name->CurrentItem() == m_system_name->end()) // no system to render
        return;

    const GG::Y name_height((*m_system_name->CurrentItem())->Height());
    const GG::X button_width(Value(name_height));

    // left button
    GG::Pt ul(GG::X(MaxPlanetDiameter()) + 2*EDGE_PAD, GG::Y0);
    GG::Pt lr(ul + GG::Pt(button_width, name_height));
    m_button_prev->SizeMove(ul, lr);

    // right button
    ul = GG::Pt(ClientWidth() - button_width - 2*EDGE_PAD, GG::Y0);
    lr = ul + GG::Pt(button_width, name_height);
    m_button_next->SizeMove(ul, lr);

    // system name / droplist
    ul = GG::Pt(GG::X(MaxPlanetDiameter()), GG::Y0);
    auto system_name_width = ClientWidth() - GG::X(MaxPlanetDiameter());
    lr = ul + GG::Pt(system_name_width, name_height);
    m_system_name->SetColWidth(0, system_name_width - 4 * GG::X(GG::ListBox::BORDER_THICK));
    m_system_name->SizeMove(ul, lr);

    // star type text
    ul = GG::Pt(GG::X(MaxPlanetDiameter()) + 2*EDGE_PAD, name_height + EDGE_PAD*4);
    lr = GG::Pt(ClientWidth() - 1, ul.y + m_star_type_text->Height());
    m_star_type_text->SizeMove(ul, lr);

    // resize planet panel container
    ul = GG::Pt(BORDER_LEFT, PLANET_PANEL_TOP);
    lr = GG::Pt(ClientWidth() - 1, ClientHeight() - GG::Y(INNER_BORDER_ANGLE_OFFSET));
    m_planet_panel_container->SizeMove(ul, lr);

    // Force system name, system summary and planet panel container to prerender
    // immediately.  All three may have had data that affects layout, change
    // after the PreRender() phase started, so they will not have been pre-rendered.  The
    // SidePanel layout and rendering is slow enough that this appears as a visible glitch.
    GG::GUI::PreRenderWindow(m_system_name);
    GG::GUI::PreRenderWindow(m_system_resource_summary);
    GG::GUI::PreRenderWindow(m_planet_panel_container);

    // hide scrollbar if there is no planets in the system
    auto system = GetSystem(s_system_id);
    if (system) {
        if (system->PlanetIDs().empty())
            m_planet_panel_container->HideScrollbar();
        else
            m_planet_panel_container->ShowScrollbar();
    }

    // resize system resource summary
    if (m_system_resource_summary) {
        ul = GG::Pt(GG::X(EDGE_PAD + 1), PLANET_PANEL_TOP - m_system_resource_summary->Height());
        lr = ul + GG::Pt(ClientWidth() - EDGE_PAD - 1, m_system_resource_summary->Height());
        m_system_resource_summary->SizeMove(ul, lr);
    }
}

void SidePanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void SidePanel::SystemNameDropListOpenedSlot(bool is_open) {
    // Refresh the system names when the drop list closes.
    if (is_open)
        return;
    RefreshSystemNames();
}

void SidePanel::SystemSelectionChangedSlot(GG::DropDownList::iterator it) {
    /** This handles cases when the list is dropped and not dropped in the
        same way. Refresh should not update the list of systems if the list
        is open. */
    int system_id = INVALID_OBJECT_ID;
    if (it != m_system_name->end())
        system_id = boost::polymorphic_downcast<const SystemRow*>(it->get())->SystemID();
    if (SystemID() != system_id)
        SystemSelectedSignal(system_id);
}

void SidePanel::PrevButtonClicked() {
    assert(!m_system_name->Empty());
    auto selected = m_system_name->CurrentItem();
    if (selected == m_system_name->begin())
        selected = m_system_name->end();
    m_system_name->Select(--selected);
    SystemSelectionChangedSlot(m_system_name->CurrentItem());
}

void SidePanel::NextButtonClicked() {
    assert(!m_system_name->Empty());
    auto selected = m_system_name->CurrentItem();
    if (++selected == m_system_name->end())
        selected = m_system_name->begin();
    m_system_name->Select(selected);
    SystemSelectionChangedSlot(m_system_name->CurrentItem());
}

void SidePanel::PlanetClickedSlot(int planet_id) {
    if (m_selection_enabled)
        SelectPlanet(planet_id);
}

void SidePanel::FleetsInserted(const std::vector<std::shared_ptr<Fleet>>& fleets) {
    for (auto& fleet : fleets) {
        s_fleet_state_change_signals[fleet->ID()].disconnect();  // in case already present
        s_fleet_state_change_signals[fleet->ID()] =
            fleet->StateChangedSignal.connect(&SidePanel::Update);
    }
    SidePanel::Update();
}

void SidePanel::FleetsRemoved(const std::vector<std::shared_ptr<Fleet>>& fleets) {
    for (auto& fleet : fleets) {
        auto signal_it = s_fleet_state_change_signals.find(fleet->ID());
        if (signal_it != s_fleet_state_change_signals.end()) {
            signal_it->second.disconnect();
            s_fleet_state_change_signals.erase(signal_it);
        }
    }
    SidePanel::Update();
}

int SidePanel::SystemID()
{ return s_system_id; }

int SidePanel::SelectedPlanetID() const
{ return (m_selection_enabled ? s_planet_id : INVALID_OBJECT_ID); }

bool SidePanel::PlanetSelectable(int planet_id) const {
    if (!m_selection_enabled)
        return false;

    auto system = GetSystem(s_system_id);
    if (!system)
        return false;

    const auto& planet_ids = system->PlanetIDs();
    if (planet_ids.count(planet_id) == 0)
        return false;

    auto planet = GetPlanet(planet_id);
    if (!planet)
        return false;

    // Find a selection visitor and apply it to planet
    std::shared_ptr<UniverseObjectVisitor> selectable_visitor;
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id != ALL_EMPIRES)
        selectable_visitor = std::make_shared<OwnedVisitor<Planet>>(empire_id);

    if (!selectable_visitor)
        return true;

    return planet->Accept(*selectable_visitor).use_count();
}

void SidePanel::SelectPlanet(int planet_id) {
    if (s_planet_id == planet_id)
        return;

    // Use the first sidepanel with selection enabled to determine if planet is selectable.
    bool planet_selectable(false);
    for (auto& weak_panel : s_side_panels) {
        if (auto panel = weak_panel.lock())
            if (panel->m_selection_enabled) {
                planet_selectable = panel->PlanetSelectable(planet_id);
                break;
            }
    }

    s_planet_id = INVALID_OBJECT_ID;

    if (!planet_selectable)
        return;

    s_planet_id = planet_id;

    for (auto& weak_panel : s_side_panels)
        if (auto panel = weak_panel.lock())
            panel->RequirePreRender();

    PlanetSelectedSignal(s_planet_id);
}

void SidePanel::SetSystem(int system_id) {
    if (s_system_id == system_id)
        return;

    auto system = GetSystem(system_id);
    if (!system) {
        s_system_id = INVALID_OBJECT_ID;
        return;
    }

    s_system_id = system_id;

    if (GetSystem(s_system_id))
        PlaySidePanelOpenSound();

    // refresh sidepanels
    Refresh();
}

void SidePanel::EnableSelection(bool enable/* = true*/)
{ m_selection_enabled = enable; }

void SidePanel::EnableOrderIssuing(bool enable/* = true*/) {
    m_system_name->EnableOrderIssuing(enable);
    m_planet_panel_container->EnableOrderIssuing(enable);
}
