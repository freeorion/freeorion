#include "SidePanel.h"

#include "CUIWnd.h"
#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Predicates.h"
#include "../universe/ShipDesign.h"
#include "SystemIcon.h"
#include "../util/Random.h"
#include "FleetWnd.h"
#include "InfoPanels.h"
#include "MapWnd.h"
#include "../util/XMLDoc.h"
#include "../Empire/Empire.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../util/OptionsDB.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/DynamicGraphic.h>
#include <GG/Scroll.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>

#include <fstream>

using boost::lexical_cast;

class PopulationPanel;
class ResourcePanel;
class BuildingsPanel;
class SpecialsPanel;

namespace {
    bool PlaySounds() {return GetOptionsDB().Get<bool>("UI.sound.enabled");}
    void PlaySidePanelOpenSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.sidepanel-open"));}
    void PlayFarmingFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.farming-focus"));}
    void PlayIndustryFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.industry-focus"));}
    void PlayResearchFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.research-focus"));}
    void PlayMiningFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.mining-focus"));}
    void PlayTradeFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.trade-focus"));}
    void PlayBalancedFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.balanced-focus"));}

    int CircleXFromY(double y, double r) {return static_cast<int>(std::sqrt(r * r - y * y) + 0.5);}

    struct RotatingPlanetData
    {
        RotatingPlanetData(const XMLElement& elem)
        {
            if (elem.Tag() != "RotatingPlanetData")
                throw std::invalid_argument("Attempted to construct a RotatingPlanetData from an XMLElement that had a tag other than \"RotatingPlanetData\"");

            planet_type = lexical_cast<PlanetType>(elem.Child("planet_type").Text());
            filename = elem.Child("filename").Text();
            shininess = lexical_cast<double>(elem.Child("shininess").Text());

            // ensure proper bounds
            shininess = std::max(0.0, std::min(shininess, 128.0));
        }

        XMLElement XMLEncode() const
        {
            XMLElement retval("RotatingPlanetData");
            retval.AppendChild(XMLElement("planet_type", lexical_cast<std::string>(planet_type)));
            retval.AppendChild(XMLElement("filename", filename));
            retval.AppendChild(XMLElement("shininess", lexical_cast<std::string>(shininess)));
            return retval;
        }

        PlanetType planet_type; ///< the type of planet for which this data may be used
        std::string filename;   ///< the filename of the image used to texture a rotating image
        double shininess;       ///< the exponent of specular (shiny) reflection off of the planet; must be in [0.0, 128.0]
    };

    struct PlanetAtmosphereData
    {
        struct Atmosphere
        {
            Atmosphere() {}
            Atmosphere(const XMLElement& elem)
            {
                if (elem.Tag() != "Atmosphere")
                    throw std::invalid_argument("Attempted to construct an Atmosphere from an XMLElement that had a tag other than \"Atmosphere\"");
                filename = elem.Child("filename").Text();
                alpha = lexical_cast<int>(elem.Child("alpha").Text());
                alpha = std::max(0, std::min(alpha, 255));
            }

            std::string filename;
            int         alpha;
        };

        PlanetAtmosphereData() {}
        PlanetAtmosphereData(const XMLElement& elem)
        {
            if (elem.Tag() != "PlanetAtmosphereData")
                throw std::invalid_argument("Attempted to construct a PlanetAtmosphereData from an XMLElement that had a tag other than \"PlanetAtmosphereData\"");
            planet_filename = elem.Child("planet_filename").Text();
            const XMLElement& atmospheres_elem = elem.Child("atmospheres");
            for (XMLElement::const_child_iterator it = atmospheres_elem.child_begin(); it != atmospheres_elem.child_end(); ++it) {
                atmospheres.push_back(Atmosphere(*it));
            }
        }

        std::string             planet_filename; ///< the filename of the planet image that this atmosphere image data goes with
        std::vector<Atmosphere> atmospheres;     ///< the filenames of the atmosphere images suitable for use with this planet image
    };

    const std::map<PlanetType, std::vector<RotatingPlanetData> >& GetRotatingPlanetData()
    {
        static std::map<PlanetType, std::vector<RotatingPlanetData> > data;
        if (data.empty()) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLPlanets")) {
                const XMLElement& elem = doc.root_node.Child("GLPlanets");
                for (XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
                    if (it->Tag() == "RotatingPlanetData") {
                        RotatingPlanetData current_data(*it);
                        data[current_data.planet_type].push_back(current_data);
                    }
                }
            }
        }
        return data;
    }

    const std::map<std::string, PlanetAtmosphereData>& GetPlanetAtmosphereData()
    {
        static std::map<std::string, PlanetAtmosphereData> data;
        if (data.empty()) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "atmospheres.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            for (XMLElement::const_child_iterator it = doc.root_node.child_begin(); it != doc.root_node.child_end(); ++it) {
                if (it->Tag() == "PlanetAtmosphereData") {
                    PlanetAtmosphereData current_data(*it);
                    data[current_data.planet_filename] = current_data;
                }
            }
        }
        return data;
    }

    double GetAsteroidsFPS()
    {
        static double retval = -1.0;
        if (retval == -1.0) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("asteroids_fps"))
                retval = lexical_cast<double>(doc.root_node.Child("asteroids_fps").Text());
            else
                retval = 15.0;

            retval = std::max(0.0, std::min(retval, 60.0));
        }
        return retval;
    }

    double GetRotatingPlanetAmbientIntensity()
    {
        static double retval = -1.0;

        if (retval == -1.0) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLPlanets") && doc.root_node.Child("GLPlanets").ContainsChild("ambient_intensity"))
                retval = lexical_cast<double>(doc.root_node.Child("GLPlanets").Child("ambient_intensity").Text());
            else
                retval = 0.5;

            retval = std::max(0.0, std::min(retval, 1.0));
        }

        return retval;
    }

    double GetRotatingPlanetDiffuseIntensity()
    {
        static double retval = -1.0;

        if (retval == -1.0) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLPlanets") && doc.root_node.Child("GLPlanets").ContainsChild("diffuse_intensity"))
                retval = lexical_cast<double>(doc.root_node.Child("GLPlanets").Child("diffuse_intensity").Text());
            else
                retval = 0.5;

            retval = std::max(0.0, std::min(retval, 1.0));
        }

        return retval;
    }

    void RenderSphere(double r, const GG::Clr& ambient, const GG::Clr& diffuse, const GG::Clr& spec, double shine, 
                      boost::shared_ptr<GG::Texture> texture)
    {
        static GLUquadric* quad = gluNewQuadric();

        if (quad) {
            if (texture) {
                glBindTexture(GL_TEXTURE_2D, texture->OpenGLId());
            }

            if (shine) {
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, static_cast<float>(shine));
                GLfloat spec_v[] = {spec.r / 255.0, spec.g / 255.0, spec.b / 255.0, spec.a / 255.0};
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_v);
            }
            GLfloat ambient_v[] = {ambient.r / 255.0, ambient.g / 255.0, ambient.b / 255.0, ambient.a / 255.0};
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_v);
            GLfloat diffuse_v[] = {diffuse.r / 255.0, diffuse.g / 255.0, diffuse.b / 255.0, diffuse.a / 255.0};
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_v);
            gluQuadricTexture(quad, texture ? GL_TRUE : GL_FALSE);
            gluQuadricNormals(quad, GLU_SMOOTH);
            gluQuadricOrientation(quad, GLU_OUTSIDE);

            glColor(GG::CLR_WHITE);
            gluSphere(quad, r, 30, 30);
        }
    }

    GLfloat* GetLightPosition()
    {
        static GLfloat retval[] = {0.0, 0.0, 0.0, 0.0};

        if (retval[0] == 0.0 && retval[1] == 0.0 && retval[2] == 0.0) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            retval[0] = lexical_cast<double>(doc.root_node.Child("GLPlanets").Child("light_pos").Child("x").Text());
            retval[1] = lexical_cast<double>(doc.root_node.Child("GLPlanets").Child("light_pos").Child("y").Text());
            retval[2] = lexical_cast<double>(doc.root_node.Child("GLPlanets").Child("light_pos").Child("z").Text());
        }

        return retval;
    }

    const std::map<StarType, std::vector<float> >& GetStarLightColors()
    {
        static std::map<StarType, std::vector<float> > light_colors;

        if (light_colors.empty()) {
            XMLDoc doc;
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLStars") && 0 < doc.root_node.Child("GLStars").NumChildren()) {
                for (XMLElement::child_iterator it = doc.root_node.Child("GLStars").child_begin(); it != doc.root_node.Child("GLStars").child_end(); ++it) {
                    std::vector<float>& color_vec = light_colors[lexical_cast<StarType>(it->Child("star_type").Text())];
                    GG::Clr color(XMLToClr(it->Child("GG::Clr")));
                    color_vec.push_back(color.r / 255.0);
                    color_vec.push_back(color.g / 255.0);
                    color_vec.push_back(color.b / 255.0);
                    color_vec.push_back(color.a / 255.0);
                }
            } else {
                for (int i = STAR_BLUE; i < NUM_STAR_TYPES; ++i) {
                    light_colors[StarType(i)].resize(4, 1.0);
                }
            }
        }

        return light_colors;
    }

    void RenderPlanet(const GG::Pt& center, int diameter, boost::shared_ptr<GG::Texture> texture, double initial_rotation, double RPM, double axial_tilt, double shininess, StarType star_type)
    {
        glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT | GL_SCISSOR_BIT);
        HumanClientApp::GetApp()->Exit2DMode();

        // slide the texture coords to simulate a rotating axis
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(initial_rotation - GG::GUI::GetGUI()->Ticks() / 1000.0 * RPM / 60.0, 0.0, 0.0);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, HumanClientApp::GetApp()->AppWidth(), HumanClientApp::GetApp()->AppHeight(), 0.0, 0.0, HumanClientApp::GetApp()->AppWidth());

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);
        GLfloat* light_position = GetLightPosition();
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        const std::map<StarType, std::vector<float> >& star_light_colors = GetStarLightColors();
        glLightfv(GL_LIGHT0, GL_DIFFUSE, &star_light_colors.find(star_type)->second[0]);
        glLightfv(GL_LIGHT0, GL_SPECULAR, &star_light_colors.find(star_type)->second[0]);
        glEnable(GL_TEXTURE_2D);

        glTranslated(center.x, center.y, -(diameter / 2 + 1));
        glRotated(100.0, -1.0, 0.0, 0.0); // make the poles upright, instead of head-on (we go a bit more than 90 degrees, to avoid some artifacting caused by the GLU-supplied texture coords)
        glRotated(axial_tilt, 0.0, 1.0, 0.0);  // axial tilt
        double intensity = GetRotatingPlanetAmbientIntensity();
        GG::Clr ambient = GG::FloatClr(intensity, intensity, intensity, 1.0);
        intensity = GetRotatingPlanetDiffuseIntensity();
        GG::Clr diffuse = GG::FloatClr(intensity, intensity, intensity, 1.0);

        RenderSphere(diameter / 2, ambient, diffuse, GG::CLR_WHITE, shininess, texture);

        glPopAttrib();

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        HumanClientApp::GetApp()->Enter2DMode();
        glPopAttrib();
    }

    int PlanetDiameter(PlanetSize size)
    {
        double scale = 0.0;
        switch (size)
        {
        case SZ_TINY      : scale = 0.0/5.0; break;
        case SZ_SMALL     : scale = 1.0/5.0; break;
        case SZ_MEDIUM    : scale = 2.0/5.0; break;
        case SZ_LARGE     : scale = 3.0/5.0; break;
        case SZ_HUGE      : scale = 4.0/5.0; break;
        case SZ_GASGIANT  : scale = 5.0/5.0; break;
        case SZ_ASTEROIDS : scale = 5.0/5.0; break;
        default           : scale = 2.0/5.0; break;
        }

        return static_cast<int>(SidePanel::MIN_PLANET_DIAMETER + (SidePanel::MAX_PLANET_DIAMETER - SidePanel::MIN_PLANET_DIAMETER) * scale);
    }

}

/** a single planet's info and controls; several of these may appear at any one time in a SidePanel */
class SidePanel::PlanetPanel : public GG::Wnd
{
public:
    enum HilitingType {
        HILITING_NONE,
        HILITING_CANDIDATE,
        HILITING_SELECTED
    };

    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)>   LeftClickedSignalType;  ///< emitted when the planet graphic is left clicked by the user
    typedef boost::signal<void ()>      ResizedSignalType;      ///< emitted when resized, so external container can redo layout
    //@}
   
    /** \name Slot Types */ //@{
    typedef LeftClickedSignalType::slot_type LeftClickedSlotType; ///< type of functor(s) invoked on a LeftClickedSignalType
    //@}

    /** \name Structors */ //@{
    PlanetPanel(int w, const Planet &planet, StarType star_type); ///< basic ctor
    ~PlanetPanel();
    //@}

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;
    int PlanetID() const {return m_planet_id;}
    HilitingType Hiliting() const;

    const PopulationPanel*  GetPopulationPanel() const;
    const ResourcePanel*    GetResourcePanel() const;
    const BuildingsPanel*   GetBuildingsPanel() const;
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)

    void Refresh();                 ///< updates panels, shows / hides colonize button, redoes layout of infopanels
    void Hilite(HilitingType ht);
    //@}

    mutable LeftClickedSignalType PlanetImageLClickedSignal; ///< returns the left clicked signal object for this Planet panel
    mutable ResizedSignalType ResizedSignal;

private:
    void DoLayout();

    int  PlanetDiameter() const;
    bool InPlanet(const GG::Pt& pt) const;      ///< returns true if pt is within the planet image

    void SetPrimaryFocus  (FocusType focus);    ///< set the primary focus of the planet to focus
    void SetSecondaryFocus(FocusType focus);    ///< set the secondary focus of the planet to focus

    void ClickColonize();   ///< called if colonize button is pressed

    Planet* GetPlanet();    ///< returns the planet with ID m_planet_id
    const Planet* GetPlanet() const;

    int                     m_planet_id;                ///< id for the planet with is representet by this planet panel
    GG::TextControl*        m_planet_name;              ///< planet name
    GG::TextControl*        m_env_size;                 ///< indicates size and planet environment rating un uncolonized planets
    CUIButton*              m_button_colonize;          ///< btn which can be pressed to colonize this planet
    GG::DynamicGraphic*     m_planet_graphic;           ///< image of the planet (can be a frameset); this is now used only for asteroids
    RotatingPlanetControl*  m_rotating_planet_graphic;  ///< a realtime-rendered planet that rotates, with a textured surface mapped onto it
    HilitingType            m_hiliting;
    PopulationPanel*        m_population_panel;         ///< contains info about population and health
    ResourcePanel*          m_resource_panel;           ///< contains info about resources production and focus selection UI
    MilitaryPanel*          m_military_panel;           ///< contains icons representing military-related meters
    BuildingsPanel*         m_buildings_panel;          ///< contains icons representing buildings
    SpecialsPanel*          m_specials_panel;           ///< contains icons representing specials
};

class SidePanel::PlanetPanelContainer : public GG::Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> PlanetSelectedSignalType; ///< emitted when a rotating planet in a planet panel is clicked by the user
    //@}

    /** \name Structors */ //@{
    PlanetPanelContainer(int x, int y, int w, int h);
    //@}

    void Clear();
    void SetPlanets(const std::vector<const Planet*> &plt_vec, StarType star_type);
    void SelectPlanet(int planet_id);
    void SetValidSelectionPredicate(const boost::shared_ptr<UniverseObjectVisitor> &visitor);

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)

    int                PlanetID() const            {return m_planet_id;}
    int                PlanetPanels() const        {return m_planet_panels.size();}
    const PlanetPanel* GetPlanetPanel(int n) const {return m_planet_panels[n];}
    //@}

    /** \name Mutators */ //@{
    PlanetPanel* GetPlanetPanel(int n) {return m_planet_panels[n];}
    
    void RefreshAllPlanetPanels();  ///< updates data displayed in info panels and redoes layout
    //@}

    mutable PlanetSelectedSignalType PlanetSelectedSignal;

private:
    void FindSelectionCandidates();
    void HiliteSelectionCandidates();
    void PlanetSelected(int planet_id);
    void DoPanelsLayout();  // repositions PlanetPanels, accounting for their size, which may have changed
    void VScroll(int from, int to, int range_min, int range_max);

    std::vector<PlanetPanel*> m_planet_panels;
    int                       m_planet_id;
    std::set<int>             m_candidate_ids;

    boost::shared_ptr<UniverseObjectVisitor> m_valid_selection_predicate;

    CUIScroll*        m_vscroll; ///< the vertical scroll (for viewing all the planet panes)
};

class RotatingPlanetControl : public GG::Control
{
public:
    RotatingPlanetControl(int x, int y, const Planet& planet, StarType star_type, const RotatingPlanetData& planet_data) :
        GG::Control(x, y, PlanetDiameter(planet.Size()), PlanetDiameter(planet.Size()), GG::Flags<GG::WndFlag>()),
        m_planet_data(planet_data),
        m_planet(planet),
        m_surface_texture(ClientUI::GetTexture(ClientUI::ArtDir() / m_planet_data.filename, true)),
        m_atmosphere_texture(),
        m_initial_rotation(RandZeroToOne()),
        m_star_type(star_type)
    {
        const std::map<std::string, PlanetAtmosphereData>& atmosphere_data = GetPlanetAtmosphereData();
        std::map<std::string, PlanetAtmosphereData>::const_iterator it = atmosphere_data.find(m_planet_data.filename);
        if (it != atmosphere_data.end()) {
            const PlanetAtmosphereData::Atmosphere& atmosphere = it->second.atmospheres[RandSmallInt(0, it->second.atmospheres.size() - 1)];
            m_atmosphere_texture = ClientUI::GetTexture(ClientUI::ArtDir() / atmosphere.filename, true);
            m_atmosphere_alpha = atmosphere.alpha;
            m_atmosphere_planet_rect = GG::Rect(1, 1, m_atmosphere_texture->DefaultWidth() - 4, m_atmosphere_texture->DefaultHeight() - 4);
        }
    }

    virtual void Render()
    {
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        // these values ensure that wierd GLUT-sphere artifacts do not show themselves
        double axial_tilt = std::max(-75.0, std::min(static_cast<double>(m_planet.AxialTilt()), 88.0));
        RenderPlanet(ul + GG::Pt(Width() / 2, Height() / 2), Width(), m_surface_texture, m_initial_rotation,
                     1.0 / m_planet.RotationalPeriod(), axial_tilt, m_planet_data.shininess, m_star_type);
        if (m_atmosphere_texture) {
            int texture_w = m_atmosphere_texture->DefaultWidth();
            int texture_h = m_atmosphere_texture->DefaultHeight();
            double x_scale = PlanetDiameter(m_planet.Size()) / static_cast<double>(texture_w);
            double y_scale = PlanetDiameter(m_planet.Size()) / static_cast<double>(texture_h);
            glColor4ub(255, 255, 255, m_atmosphere_alpha);
            m_atmosphere_texture->OrthoBlit(GG::Pt(static_cast<int>(ul.x - m_atmosphere_planet_rect.ul.x * x_scale),
                                                   static_cast<int>(ul.y - m_atmosphere_planet_rect.ul.y * y_scale)),
                                            GG::Pt(static_cast<int>(lr.x + (texture_w - m_atmosphere_planet_rect.lr.x) * x_scale),
                                                   static_cast<int>(lr.y + (texture_h - m_atmosphere_planet_rect.lr.y) * y_scale)));
        }
    }

    void SetRotatingPlanetData(const RotatingPlanetData& planet_data)
    {
        m_planet_data = planet_data;
        m_surface_texture = ClientUI::GetTexture(ClientUI::ArtDir() / m_planet_data.filename, true);
    }

private:
    RotatingPlanetData              m_planet_data;
    const Planet&                   m_planet;
    boost::shared_ptr<GG::Texture>  m_surface_texture;
    boost::shared_ptr<GG::Texture>  m_atmosphere_texture;
    int                             m_atmosphere_alpha;
    GG::Rect                        m_atmosphere_planet_rect;
    double                          m_initial_rotation;
    StarType                        m_star_type;
};

////////////////////////////////////////////////
// SidePanel::PlanetPanel
////////////////////////////////////////////////
namespace {
    int SystemNameFontSize()
    {
        return static_cast<int>(ClientUI::Pts()*3/2);
    }
  
    boost::shared_ptr<GG::Texture> IconPopulation() {return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "pop.png"        );}
    boost::shared_ptr<GG::Texture> IconIndustry  () {return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "industry.png"   );}
    boost::shared_ptr<GG::Texture> IconTrade     () {return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "trade.png"      );}
    boost::shared_ptr<GG::Texture> IconResearch  () {return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "research.png"   );}
    boost::shared_ptr<GG::Texture> IconMining    () {return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "mining.png"     );}
    boost::shared_ptr<GG::Texture> IconFarming   () {return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "farming.png"    );}
    boost::shared_ptr<GG::Texture> IconDefense   () {return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "defensebase.png");}

    struct SystemRow : public GG::ListBox::Row
    {
    public:
        SystemRow(int system_id) : m_system_id(system_id) {SetDragDropDataType("SystemID");}
        int m_system_id;
    };

    XMLElement GetXMLChild(XMLElement &node,const std::string &child_path)
    {
        int index;

        if (-1 == (index=child_path.find_first_of('.')))
            return node.ContainsChild(child_path)?node.Child(child_path):XMLElement();
        else
            return node.ContainsChild(child_path.substr(0,index)) ? 
                GetXMLChild(node.Child(child_path.substr(0, index)), child_path.substr(index + 1, child_path.length() - index - 1))
              : XMLElement();
    }

    void GetAsteroidTextures(int planet_id, std::vector<boost::shared_ptr<GG::Texture> > &textures)
    {
        const int NUM_ASTEROID_SETS = 3;
        const int NUM_IMAGES_PER_SET = 256;
        const int SET = (planet_id % NUM_ASTEROID_SETS) + 1;

        for (int i = 0; i < NUM_IMAGES_PER_SET; ++i)
            textures.push_back(ClientUI::GetTexture(ClientUI::ArtDir() / "planets" / "asteroids" / boost::io::str(boost::format("asteroids%d_%03d.png") % SET % i)));
    }

    std::string GetPlanetSizeName(const Planet &planet)
    {
        if (planet.Size() == SZ_ASTEROIDS || planet.Size() == SZ_GASGIANT)
            return "";
        return UserString(lexical_cast<std::string>(planet.Size()));
    }

    std::string GetPlanetTypeName(const Planet &planet)
    {
        return UserString(lexical_cast<std::string>(planet.Type()));
    }

    std::string GetPlanetEnvironmentName(const Planet &planet)
    {
        return UserString(lexical_cast<std::string>(planet.Environment()));
    }

    Ship* FindColonyShip(int system_id)
    {
        const System *system = GetUniverse().Object<const System>(system_id);
        if (!system) return 0;

        std::vector<const Fleet*> flt_vec = system->FindObjects<Fleet>();


        int empire_id = HumanClientApp::GetApp()->EmpireID();

        // check all fleets in this system...
        for (unsigned int i = 0; i < flt_vec.size(); i++) {
            // reject fleets not owned by this empire
            if (flt_vec[i]->Owners().find(empire_id) == flt_vec[i]->Owners().end()) continue;

            // reject fleets that are moving
            if (!flt_vec[i]->Accept(StationaryFleetVisitor(*flt_vec[i]->Owners().begin()))) continue;
            
            // check if any of the ship in this fleet is a colony ship
            for (Fleet::const_iterator it = flt_vec[i]->begin(); it != flt_vec[i]->end(); ++it) {
                Ship* s = GetUniverse().Object<Ship>(*it);

                if (!s) {
                    Logger().errorStream() << "coudln't find ship with id: " << *it;
                    continue;
                }

                const ShipDesign* design = s->Design();

                if (!design) {
                    Logger().errorStream() << "coudln't get ship design of ship " << *it << " with design id: " << s->DesignID();
                    continue;
                }
                
                if (design->Colonize()) return s;
            }
        }
        return 0;   // no ships found...
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
SidePanel::PlanetPanel::PlanetPanel(int w, const Planet &planet, StarType star_type) :
    Wnd(0, 0, w, MAX_PLANET_DIAMETER, GG::CLICKABLE),
    m_planet_id(planet.ID()),
    m_planet_name(0),
    m_env_size(0),
    m_button_colonize(0),
    m_planet_graphic(0),
    m_rotating_planet_graphic(0),
    m_hiliting(HILITING_NONE),
    m_population_panel(0),
    m_resource_panel(0),
    m_military_panel(0),
    m_buildings_panel(0),
    m_specials_panel(0)
{
    SetText(UserString("PLANET_PANEL"));

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    int planet_image_sz = PlanetDiameter();
    GG::Pt planet_image_pos(MAX_PLANET_DIAMETER / 2 - planet_image_sz / 2 + 3, Height() / 2 - planet_image_sz / 2);

    if (planet.Type() == PT_ASTEROIDS)
    {
        std::vector<boost::shared_ptr<GG::Texture> > textures;
        GetAsteroidTextures(planet.ID(), textures);
        m_planet_graphic = new GG::DynamicGraphic(planet_image_pos.x,planet_image_pos.y,planet_image_sz,planet_image_sz,true,textures[0]->DefaultWidth(),textures[0]->DefaultHeight(),0,textures, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        m_planet_graphic->SetFPS(GetAsteroidsFPS());
        m_planet_graphic->SetFrameIndex(RandSmallInt(0, textures.size() - 1));
        AttachChild(m_planet_graphic);
        m_planet_graphic->Play();
    }
    else if (planet.Type() < NUM_PLANET_TYPES)
    {
        const std::map<PlanetType, std::vector<RotatingPlanetData> >& planet_data = GetRotatingPlanetData();
        std::map<PlanetType, std::vector<RotatingPlanetData> >::const_iterator it = planet_data.find(planet.Type());
        int num_planets_of_type;
        if (it != planet_data.end() && (num_planets_of_type = planet_data.find(planet.Type())->second.size()))
        {
            // using algorithm from Thomas Wang's 32 bit Mix Function; assumes that only the lower 16 bits of the system and
            // planet ID's are significant
            unsigned int hash_value =
                (static_cast<unsigned int>(planet.SystemID()) & 0xFFFF) + (static_cast<unsigned int>(planet.ID()) & 0xFFFF);
            hash_value += ~(hash_value << 15);
            hash_value ^= hash_value >> 10;
            hash_value += hash_value << 3;
            hash_value ^= hash_value >> 6;
            hash_value += ~(hash_value << 11);
            hash_value ^= hash_value >> 16;
            m_rotating_planet_graphic =
                new RotatingPlanetControl(planet_image_pos.x, planet_image_pos.y, planet, star_type,
                                          it->second[hash_value % num_planets_of_type]);
            AttachChild(m_rotating_planet_graphic);
        }
    }

    m_planet_name = new GG::TextControl(MAX_PLANET_DIAMETER + 3, 5, planet.Name(), GG::GUI::GetGUI()->GetFont(ClientUI::FontBold(), ClientUI::Pts()*4/3), ClientUI::TextColor());
    AttachChild(m_planet_name);

    std::string env_size_text = GetPlanetSizeName(planet) + " " + GetPlanetTypeName(planet) + " (" + GetPlanetEnvironmentName(planet) + ")";

    m_population_panel = new PopulationPanel(w - MAX_PLANET_DIAMETER, planet);
    AttachChild(m_population_panel);
    GG::Connect(m_population_panel->ExpandCollapseSignal, &SidePanel::PlanetPanel::DoLayout, this);

    m_resource_panel = new ResourcePanel(w - MAX_PLANET_DIAMETER, planet);
    AttachChild(m_resource_panel);
    GG::Connect(m_resource_panel->ExpandCollapseSignal, &SidePanel::PlanetPanel::DoLayout, this);
    GG::Connect(m_resource_panel->PrimaryFocusChangedSignal, &SidePanel::PlanetPanel::SetPrimaryFocus, this);
    GG::Connect(m_resource_panel->SecondaryFocusChangedSignal, &SidePanel::PlanetPanel::SetSecondaryFocus, this);

    m_military_panel = new MilitaryPanel(w - MAX_PLANET_DIAMETER, planet);
    AttachChild(m_military_panel);
    GG::Connect(m_military_panel->ExpandCollapseSignal, &SidePanel::PlanetPanel::DoLayout, this);

    m_buildings_panel = new BuildingsPanel(w - MAX_PLANET_DIAMETER, 4, planet);
    AttachChild(m_buildings_panel);
    GG::Connect(m_buildings_panel->ExpandCollapseSignal, &SidePanel::PlanetPanel::DoLayout, this);

    m_specials_panel = new SpecialsPanel(w - MAX_PLANET_DIAMETER, planet);
    AttachChild(m_specials_panel);
    m_specials_panel->MoveTo(GG::Pt(GG::Pt(Width() - m_population_panel->Width(), m_planet_name->LowerRight().y - UpperLeft().y)));

    m_env_size = new GG::TextControl(MAX_PLANET_DIAMETER, m_specials_panel->LowerRight().y - UpperLeft().y, env_size_text, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor());
    AttachChild(m_env_size);


    m_button_colonize = new CUIButton(MAX_PLANET_DIAMETER, m_env_size->LowerRight().y - UpperLeft().y + 1, 80, UserString("PL_COLONIZE"),
                                      GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                      ClientUI::ButtonColor(), ClientUI::CtrlBorderColor(), 1, 
                                      ClientUI::TextColor(), GG::CLICKABLE);

    GG::Connect(m_button_colonize->ClickedSignal, &SidePanel::PlanetPanel::ClickColonize, this);
    AttachChild(m_button_colonize);


    if (planet.Type() == PT_ASTEROIDS) 
        MoveChildDown(m_planet_graphic);

    const Planet *plt = GetUniverse().Object<const Planet>(m_planet_id);

    if (System* system = plt->GetSystem())
        GG::Connect(system->StateChangedSignal, &SidePanel::PlanetPanel::Refresh, this);
    GG::Connect(plt->StateChangedSignal, &SidePanel::PlanetPanel::Refresh, this);

    Refresh();
}

SidePanel::PlanetPanel::~PlanetPanel()
{
    delete m_button_colonize;
    delete m_env_size;

    delete m_population_panel;
    delete m_resource_panel;
    delete m_military_panel;
    delete m_buildings_panel;
    delete m_specials_panel;
}

Planet* SidePanel::PlanetPanel::GetPlanet()
{
    Planet *planet = GetUniverse().Object<Planet>(m_planet_id);
    if (!planet) throw std::runtime_error("SidePanel::PlanetPanel::GetPlanet: planet not found!");
    return planet;
}

const Planet* SidePanel::PlanetPanel::GetPlanet() const
{
    const Planet *planet = GetUniverse().Object<const Planet>(m_planet_id);
    if (!planet) throw std::runtime_error("SidePanel::PlanetPanel::GetPlanet: planet not found!");
    return planet;
}

void SidePanel::PlanetPanel::Hilite(HilitingType ht)
{
    m_hiliting = ht;
}

void SidePanel::PlanetPanel::DoLayout()
{
    const int INTERPANEL_SPACE = 3;
    int y = m_specials_panel->LowerRight().y - UpperLeft().y;
    int x = Width() - m_population_panel->Width();

    if (m_population_panel->Parent() == this) {
        m_population_panel->MoveTo(GG::Pt(x, y));
        y += m_population_panel->Height() + INTERPANEL_SPACE;
    }

    if (m_resource_panel->Parent() == this) {
        m_resource_panel->MoveTo(GG::Pt(x, y));
        y += m_resource_panel->Height() + INTERPANEL_SPACE;
    }

    if (m_military_panel->Parent() == this) {
        m_military_panel->MoveTo(GG::Pt(x, y));
        y += m_military_panel->Height() + INTERPANEL_SPACE;
    }

    if (m_buildings_panel->Parent() == this) {
        m_buildings_panel->MoveTo(GG::Pt(x, y));
        y += m_buildings_panel->Height();
    }

    Resize(GG::Pt(Width(), std::max(y, MAX_PLANET_DIAMETER)));
    ResizedSignal();
}

void SidePanel::PlanetPanel::Refresh()
{
    const Planet *planet = GetPlanet();

    enum OWNERSHIP {OS_NONE, OS_FOREIGN, OS_SELF} owner = OS_NONE;

    if (planet->Owners().empty() || planet->IsAboutToBeColonized()) {
        owner = OS_NONE;
    } else {
        if (!planet->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
            owner = OS_FOREIGN;
        else
            owner = OS_SELF;
    }

    if (!planet->Owners().empty()) {
        Empire* planet_empire = Empires().Lookup(*(planet->Owners().begin()));
        m_planet_name->SetTextColor(planet_empire ? planet_empire->Color() : ClientUI::TextColor());
    }

    if (owner == OS_NONE) {
        AttachChild(m_env_size);
        DetachChild(m_population_panel);
        DetachChild(m_resource_panel);
        DetachChild(m_military_panel);
    } else {
        DetachChild(m_env_size);
        AttachChild(m_population_panel);
        m_population_panel->Refresh();
        AttachChild(m_resource_panel);
        m_resource_panel->Refresh();
        AttachChild(m_military_panel);
        m_military_panel->Refresh();
    }

    if (owner == OS_NONE && planet->GetMeter(METER_POPULATION)->Max() > 0 && !planet->IsAboutToBeColonized() && FindColonyShip(planet->SystemID())) {
        AttachChild(m_button_colonize);
        m_button_colonize->SetText(UserString("PL_COLONIZE") + " " + boost::lexical_cast<std::string>(planet->GetMeter(METER_POPULATION)->Max()));
    
    } else if (planet->IsAboutToBeColonized()) {
        AttachChild(m_button_colonize);
        m_button_colonize->SetText(UserString("CANCEL"));
    
    } else {
        DetachChild(m_button_colonize);
    }

    m_buildings_panel->Refresh();
    m_specials_panel->Update();

    // BuildingsPanel::Refresh (and other panels) emit ExpandCollapseSignal, which should be connected to SidePanel::PlanetPanel::DoLayout
}

void SidePanel::PlanetPanel::SetPrimaryFocus(FocusType focus)
{
    Planet *planet = GetPlanet();
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ChangeFocusOrder(HumanClientApp::GetApp()->EmpireID(),planet->ID(),focus,true)));
}

void SidePanel::PlanetPanel::SetSecondaryFocus(FocusType focus)
{
    Planet *planet = GetPlanet();
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ChangeFocusOrder(HumanClientApp::GetApp()->EmpireID(),planet->ID(),focus,false)));
} 

void SidePanel::PlanetPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Wnd *parent;
    if ((parent=Parent()))
        parent->MouseWheel(pt,move,mod_keys);
}

bool SidePanel::PlanetPanel::InWindow(const GG::Pt& pt) const
{
    // The mouse is in this window if it is in the rightmost Width() - MAX_PLANET_DIAMETER portion, or if it is over the
    // planet graphic, or if it is over the specials panel.  That is, it falls through to the MapWnd if it is over the
    // empty space around the planet on the left side of the panel.
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    ul.x += MAX_PLANET_DIAMETER;
    return (ul <= pt && pt < lr || m_specials_panel->InWindow(pt) || InPlanet(pt));
}

SidePanel::PlanetPanel::HilitingType SidePanel::PlanetPanel:: Hiliting() const
{
    return m_hiliting;
}

void SidePanel::PlanetPanel::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) 
{
    if (InPlanet(pt))
    {
        if (GetOptionsDB().Get<bool>("UI.sound.enabled"))
            HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.planet-button-click"));
        PlanetImageLClickedSignal(m_planet_id);
    }
}

void SidePanel::PlanetPanel::Render()
{
    GG::Clr DARK_GREY = GG::Clr(26, 26, 26, 255);
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul.x + SidePanel::MAX_PLANET_DIAMETER, m_planet_name->UpperLeft().y, lr.x, m_planet_name->LowerRight().y, DARK_GREY, DARK_GREY, 0);   // top title filled background

    const Planet *planet = GetPlanet();

    if (m_hiliting == HILITING_CANDIDATE && planet->Type() != PT_ASTEROIDS) {
        GG::Rect planet_rect(m_rotating_planet_graphic->UpperLeft(), m_rotating_planet_graphic->LowerRight());
        double PERIOD_MS = 2000.0;
        double PI = 3.14159;
        double factor = 0.5 + std::cos(HumanClientApp::GetApp()->Ticks() / PERIOD_MS * 2 * PI) / 2;
        int alpha = static_cast<int>(255 * factor);
        GG::FlatCircle(planet_rect.ul.x - 3, planet_rect.ul.y - 3, planet_rect.lr.x + 3, planet_rect.lr.y + 3, GG::Clr(0, 100, 0, alpha), GG::CLR_ZERO, 0);
    } else if (m_hiliting == HILITING_SELECTED && planet->Type() != PT_ASTEROIDS) {
        GG::Rect planet_rect(m_rotating_planet_graphic->UpperLeft(), m_rotating_planet_graphic->LowerRight());
        GG::FlatCircle(planet_rect.ul.x - 3, planet_rect.ul.y - 3, planet_rect.lr.x + 3, planet_rect.lr.y + 3, GG::CLR_WHITE, GG::CLR_ZERO, 0);
    }
}

int SidePanel::PlanetPanel::PlanetDiameter() const
{
    return ::PlanetDiameter(GetPlanet()->Size());
}

bool SidePanel::PlanetPanel::InPlanet(const GG::Pt& pt) const
{
    GG::Pt center = UpperLeft() + GG::Pt(MAX_PLANET_DIAMETER / 2, MAX_PLANET_DIAMETER / 2);
    GG::Pt diff = pt - center;
    int r_squared = PlanetDiameter() * PlanetDiameter() / 4;
    return diff.x * diff.x + diff.y * diff.y <= r_squared;
}

void SidePanel::PlanetPanel::ClickColonize()
{
    const Planet *planet = GetPlanet();
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    std::map<int, int> pending_colonization_orders = HumanClientApp::GetApp()->PendingColonizationOrders();
    std::map<int, int>::const_iterator it = pending_colonization_orders.find(planet->ID());
    if (it == pending_colonization_orders.end()) // colonize
    {
        Ship *ship=FindColonyShip(planet->SystemID());
        if (ship==0)
            throw std::runtime_error("SidePanel::PlanetPanel::ClickColonize ship not found!");

        if (!ship->GetFleet()->Accept(StationaryFleetVisitor(*ship->GetFleet()->Owners().begin())))
        {
            GG::ThreeButtonDlg dlg(320,200,UserString("SP_USE_DEPARTING_COLONY_SHIPS_QUESTION"),
                                   GG::GUI::GetGUI()->GetFont(ClientUI::Font(),ClientUI::Pts()),ClientUI::WndColor(),ClientUI::CtrlBorderColor(),ClientUI::CtrlColor(),ClientUI::TextColor(),2,
                                   UserString("YES"),UserString("NO"));
            dlg.Run();

            if (dlg.Result()!=0)
            return;
        }

        HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new FleetColonizeOrder( empire_id, ship->ID(), planet->ID())));
    }
    else // cancel colonization
    {
        boost::shared_ptr<FleetColonizeOrder> col_order =
            boost::dynamic_pointer_cast<FleetColonizeOrder>(HumanClientApp::GetApp()->Orders().ExamineOrder(it->second));
        int ship_id = col_order ? col_order->ShipID() : UniverseObject::INVALID_OBJECT_ID;

        HumanClientApp::GetApp()->Orders().RecindOrder(it->second);
    
        // if the ship now buils a fleet of its own, make sure that fleet appears
        // at a possibly opened FleetWnd
        Ship* ship = GetUniverse().Object<Ship>(ship_id);
        Fleet* fleet = ship ? GetUniverse().Object<Fleet>(ship->FleetID()) : NULL;
        if (fleet) {
            for (FleetUIManager::iterator it = FleetUIManager::GetFleetUIManager().begin(); it != FleetUIManager::GetFleetUIManager().end(); ++it) {
                FleetWnd* fleet_wnd = *it;
                if (fleet->SystemID() == fleet_wnd->SystemID()
                    && !fleet_wnd->ContainsFleet(fleet->ID()))
                {
                    fleet_wnd->AddFleet(GetUniverse().Object<Fleet>(fleet->ID()));
                    break;
                }
            }
        }
    }
}

void SidePanel::PlanetPanel::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    const Planet *planet = GetPlanet();
  
    if (!planet->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
        return;


    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("SP_RENAME_PLANET"), 1, false, false));
    GG::PopupMenu popup(pt.x, pt.y, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), menu_contents, ClientUI::TextColor());

    if (popup.Run()) 
        switch (popup.MenuID())
        {
        case 1: 
        { // rename planet
            std::string plt_name = planet->Name();
            CUIEditWnd edit_wnd(350, UserString("SP_ENTER_NEW_PLANET_NAME"), plt_name);
            edit_wnd.Run();
            if (edit_wnd.Result() != "")
            {
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new RenameOrder(HumanClientApp::GetApp()->EmpireID(), planet->ID(), edit_wnd.Result())));
                m_planet_name->SetText(planet->Name());
            }
            break;
        }
        default:
        break;
    }
}

////////////////////////////////////////////////
// SidePanel::PlanetPanelContainer
////////////////////////////////////////////////
SidePanel::PlanetPanelContainer::PlanetPanelContainer(int x, int y, int w, int h) :
    Wnd(x, y, w, h, GG::CLICKABLE),
    m_planet_panels(),
    m_planet_id(UniverseObject::INVALID_OBJECT_ID),
    m_vscroll(new CUIScroll(Width()-14,0,14,Height(),GG::VERTICAL))
{
    SetText("PlanetPanelContainer");
    EnableChildClipping(true);
    GG::Connect(m_vscroll->ScrolledSignal, &SidePanel::PlanetPanelContainer::VScroll, this);
}

bool SidePanel::PlanetPanelContainer::InWindow(const GG::Pt& pt) const
{
    for (std::vector<PlanetPanel*>::const_iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it) {
        if ((*it)->InWindow(pt))
            return true;
    }

    return UpperLeft() + GG::Pt(MAX_PLANET_DIAMETER, 0) <= pt && pt < LowerRight();
}

void SidePanel::PlanetPanelContainer::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_vscroll)
        move < 0 ? m_vscroll->ScrollLineIncr() : m_vscroll->ScrollLineDecr();
}

void SidePanel::PlanetPanelContainer::Clear()
{
    m_planet_panels.clear();
    m_planet_id = UniverseObject::INVALID_OBJECT_ID;
    PlanetSelectedSignal(m_planet_id);
    DetachChild(m_vscroll);
    DeleteChildren();
    AttachChild(m_vscroll);
}

void SidePanel::PlanetPanelContainer::SetPlanets(const std::vector<const Planet*> &plt_vec, StarType star_type)
{
    Clear();
    for (unsigned int i = 0; i < plt_vec.size(); ++i) {
        const Planet* planet = plt_vec[i];
        PlanetPanel* planet_panel = new PlanetPanel(Width() - m_vscroll->Width(), *planet, star_type);
        AttachChild(planet_panel);
        m_planet_panels.push_back(planet_panel);
        GG::Connect(m_planet_panels.back()->PlanetImageLClickedSignal, &SidePanel::PlanetPanelContainer::PlanetSelected, this);
        GG::Connect(m_planet_panels.back()->ResizedSignal, &SidePanel::PlanetPanelContainer::DoPanelsLayout, this);
    }
    DoPanelsLayout();
    VScroll(m_vscroll->PosnRange().first, 0, 0, 0);
    FindSelectionCandidates();
    HiliteSelectionCandidates();
}

void SidePanel::PlanetPanelContainer::DoPanelsLayout()
{
    int y = 0;
    for (std::vector<PlanetPanel*>::iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it) {
        PlanetPanel* panel = *it;
        panel->MoveTo(GG::Pt(0, y));
        y += panel->Height();   // may be different for each panel depending whether that panel has been previously left expanded or collapsed
    }

    int available_height = y;
    GG::Wnd* parent = Parent();
    if (parent) {
        int containing_height = parent->Height();
        available_height = containing_height - 300;  // height of visible "page" of panels
    }

    m_vscroll->SizeScroll(0, y, MAX_PLANET_DIAMETER, available_height);   // adjust size of scrollbar
    m_vscroll->ScrolledSignal(m_vscroll->PosnRange().first, m_vscroll->PosnRange().second, 0, 0);   // fake a scroll event in order to update scrollbar and panel container position

    if (y < available_height + 1) {
        DetachChild(m_vscroll);
    }
    else {
        AttachChild(m_vscroll);
        m_vscroll->Show();
    }
}

void SidePanel::PlanetPanelContainer::SelectPlanet(int planet_id)
{
    PlanetSelected(planet_id);
}

void SidePanel::PlanetPanelContainer::SetValidSelectionPredicate(const boost::shared_ptr<UniverseObjectVisitor> &visitor)
{
    m_valid_selection_predicate = visitor;
    FindSelectionCandidates();
    HiliteSelectionCandidates();
}

void SidePanel::PlanetPanelContainer::FindSelectionCandidates()
{
    m_candidate_ids.clear();
    if (m_valid_selection_predicate) {
        for (std::vector<PlanetPanel*>::iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it) {
            Planet* planet = GetUniverse().Object<Planet>((*it)->PlanetID());
            if (planet->Accept(*m_valid_selection_predicate))
                m_candidate_ids.insert(planet->ID());
        }
    }
}

void SidePanel::PlanetPanelContainer::HiliteSelectionCandidates()
{
    if (m_candidate_ids.find(m_planet_id) != m_candidate_ids.end())
        m_planet_id = UniverseObject::INVALID_OBJECT_ID;
    if (m_planet_id == UniverseObject::INVALID_OBJECT_ID) {
        for (std::vector<PlanetPanel*>::iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it) {
            (*it)->Hilite(m_candidate_ids.find((*it)->PlanetID()) != m_candidate_ids.end() ? PlanetPanel::HILITING_CANDIDATE : PlanetPanel::HILITING_NONE);
        }
    }
}

void SidePanel::PlanetPanelContainer::PlanetSelected(int planet_id)
{
    if (planet_id != m_planet_id && m_candidate_ids.find(planet_id) != m_candidate_ids.end()) {
        m_planet_id = planet_id;
        bool planet_id_match_found = false;
        for (std::vector<PlanetPanel*>::iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it) {
            bool match = (*it)->PlanetID() == m_planet_id;
            (*it)->Hilite(match ? PlanetPanel::HILITING_SELECTED : PlanetPanel::HILITING_NONE);
            if (match)
                planet_id_match_found = true;
        }
        if (!planet_id_match_found)
            m_planet_id = UniverseObject::INVALID_OBJECT_ID;
        else
            PlanetSelectedSignal(m_planet_id);
    }
}

void SidePanel::PlanetPanelContainer::VScroll(int from, int to, int range_min, int range_max)
{
    int y = -from;
    for (unsigned int i = 0; i < m_planet_panels.size(); ++i) {
        m_planet_panels[i]->MoveTo(GG::Pt(0, y));
        y += m_planet_panels[i]->Height();
    }
}

void SidePanel::PlanetPanelContainer::RefreshAllPlanetPanels()
{
    for (std::vector<PlanetPanel*>::iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it)
        (*it)->Refresh();
}

////////////////////////////////////////////////
// SidePanel
////////////////////////////////////////////////
// static(s)
const int SidePanel::MAX_PLANET_DIAMETER = 128; // size of a huge planet, in on-screen pixels
const int SidePanel::MIN_PLANET_DIAMETER = MAX_PLANET_DIAMETER / 4; // size of a tiny planet, in on-screen pixels

const System*        SidePanel::s_system = 0;
std::set<SidePanel*> SidePanel::s_side_panels;

SidePanel::SidePanel(int x, int y, int w, int h) : 
    Wnd(x, y, w, h, GG::CLICKABLE),
    m_system_name(0),
    m_button_prev(0),
    m_button_next(0),
    m_star_graphic(0),
    m_planet_panel_container(new PlanetPanelContainer(0, 140, w, h-170)),
    m_system_resource_summary(0)
{
    const boost::shared_ptr<GG::Font>& font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), SystemNameFontSize());
    const int DROP_HEIGHT = SystemNameFontSize();

    m_button_prev = new GG::Button(MAX_PLANET_DIAMETER + EDGE_PAD, EDGE_PAD, DROP_HEIGHT, DROP_HEIGHT, "", font, GG::CLR_WHITE);
    m_button_next = new GG::Button(w - DROP_HEIGHT - EDGE_PAD,     EDGE_PAD, DROP_HEIGHT, DROP_HEIGHT, "", font, GG::CLR_WHITE);
    m_system_name = new CUIDropDownList(MAX_PLANET_DIAMETER, 0, w - MAX_PLANET_DIAMETER, DROP_HEIGHT, 10*SystemNameFontSize(), GG::CLR_ZERO, GG::FloatClr(0.0, 0.0, 0.0, 0.5));

    TempUISoundDisabler sound_disabler;

    SetText(UserString("SIDE_PANEL"));

    m_system_name->DisableDropArrow();
    m_system_name->SetStyle(GG::LIST_CENTER);
    m_system_name->SetInteriorColor(GG::Clr(0, 0, 0, 200));

    m_button_prev->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "leftarrownormal.png"   ), 0, 0, 32, 32));
    m_button_prev->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "leftarrowclicked.png"  ), 0, 0, 32, 32));
    m_button_prev->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "leftarrowmouseover.png"), 0, 0, 32, 32));

    m_button_next->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "rightarrownormal.png"  ), 0, 0, 32, 32));
    m_button_next->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "rightarrowclicked.png"   ), 0, 0, 32, 32));
    m_button_next->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "rightarrowmouseover.png"), 0, 0, 32, 32));

    m_system_resource_summary = new MultiIconValueIndicator(w - MAX_PLANET_DIAMETER - EDGE_PAD*2);
    m_system_resource_summary->MoveTo(GG::Pt(MAX_PLANET_DIAMETER + EDGE_PAD, 140 - m_system_resource_summary->Height()));


    AttachChild(m_system_name);
    AttachChild(m_button_prev);
    AttachChild(m_button_next);
    AttachChild(m_system_resource_summary);
    AttachChild(m_planet_panel_container);

    GG::Connect(m_system_name->SelChangedSignal, &SidePanel::SystemSelectionChanged, this);
    GG::Connect(m_button_prev->ClickedSignal, &SidePanel::PrevButtonClicked, this);
    GG::Connect(m_button_next->ClickedSignal, &SidePanel::NextButtonClicked, this);
    GG::Connect(m_planet_panel_container->PlanetSelectedSignal, &SidePanel::PlanetSelected, this);

    Hide();

    s_side_panels.insert(this);
}

SidePanel::~SidePanel()
{
    // disconnect any existing stored signals
    while (!m_system_connections.empty()) {
        m_system_connections.begin()->disconnect();
        m_system_connections.erase(m_system_connections.begin());
    }
    while (!m_fleet_connections.empty()) {
        m_fleet_connections.begin()->second.disconnect();
        m_fleet_connections.erase(m_fleet_connections.begin());
    }
    s_side_panels.erase(this);
}

bool SidePanel::InWindow(const GG::Pt& pt) const
{
    return (UpperLeft() + GG::Pt(MAX_PLANET_DIAMETER, 0) <= pt && pt < LowerRight()) || m_planet_panel_container->InWindow(pt);
}

void SidePanel::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    FlatRectangle(ul.x + MAX_PLANET_DIAMETER, ul.y, lr.x, lr.y, ClientUI::SidePanelColor(), GG::CLR_CYAN, 0);
}

void SidePanel::Refresh()
{
    for (std::set<SidePanel*>::iterator it = s_side_panels.begin(); it != s_side_panels.end(); ++it)
        (*it)->RefreshImpl();
}

void SidePanel::RefreshImpl()
{
    m_system_resource_summary->Update();
    // update individual PlanetPanels in PlanetPanelContainer, then redo layout of panel container
    m_planet_panel_container->RefreshAllPlanetPanels();
}
void SidePanel::SetSystemImpl()
{
    TempUISoundDisabler sound_disabler;

    m_fleet_icons.clear();
    m_planet_panel_container->Clear();
    m_system_name->Clear();

    DeleteChild(m_star_graphic);    m_star_graphic = 0;
    
    // disconnect any existing system signals
    while (!m_system_connections.empty()) {
        m_system_connections.begin()->disconnect();
        m_system_connections.erase(m_system_connections.begin());
    }
    while (!m_fleet_connections.empty()) {
        m_fleet_connections.begin()->second.disconnect();
        m_fleet_connections.erase(m_fleet_connections.begin());
    }

    if (s_system)
    {
        m_system_connections.insert(GG::Connect(s_system->FleetAddedSignal, &SidePanel::SystemFleetAdded, this));
        m_system_connections.insert(GG::Connect(s_system->FleetRemovedSignal, &SidePanel::SystemFleetRemoved, this));

        std::vector<const System*> sys_vec = GetUniverse().FindObjects<const System>();
        GG::ListBox::Row *select_row = 0;

        int system_names_in_droplist = 0;
        for (unsigned int i = 0; i < sys_vec.size(); i++) 
        {
            GG::ListBox::Row *row = new SystemRow(sys_vec[i]->ID());

            if (sys_vec[i]->Name().length()==0) {
                if (sys_vec[i] == s_system) {
                    row->push_back(UserString("SP_UNKNOWN_SYSTEM"), ClientUI::Font(), SystemNameFontSize(), ClientUI::TextColor());
                    GG::Control* control = row->at(0);
                    GG::TextControl* text_control = dynamic_cast<GG::TextControl*>(control);
                    text_control->SetTextFormat(GG::FORMAT_VCENTER | GG::FORMAT_CENTER);
                } else {
                    delete row;
                    continue;
                }
            } else {
                row->push_back(new OwnerColoredSystemName(sys_vec[i], HumanClientApp::GetApp()->GetFont(ClientUI::Font(), SystemNameFontSize()), UserString("SP_SYSTEM_NAME")));
            }
            
            m_system_name->Insert(row);
            ++system_names_in_droplist;

            if (sys_vec[i] == s_system)
                select_row = row;
        }
        const int TEXT_ROW_HEIGHT = CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT;
        const int MAX_DROPLIST_DROP_HEIGHT = TEXT_ROW_HEIGHT * 10;
        const int TOTAL_LISTBOX_MARGIN = 4;
        int drop_height = std::min(TEXT_ROW_HEIGHT * system_names_in_droplist, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
        m_system_name->SetDropHeight(drop_height);

        for (int i = 0; i < m_system_name->NumRows(); i++) {
            if (select_row == &m_system_name->GetRow(i))
            {
                m_system_name->Select(i);
                break;
            }
        }

        boost::shared_ptr<GG::Texture> graphic = ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars_sidepanel", ClientUI::StarTypeFilePrefixes()[s_system->Star()], s_system->ID());
        std::vector<boost::shared_ptr<GG::Texture> > textures;
        textures.push_back(graphic);

        int star_dim = (Width()*4)/5;
        m_star_graphic = new GG::DynamicGraphic(Width()-(star_dim*2)/3,-(star_dim*1)/3,star_dim,star_dim,true,textures[0]->DefaultWidth(),textures[0]->DefaultHeight(),0,textures, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

        AttachChild(m_star_graphic);
        MoveChildDown(m_star_graphic);

        // TODO: add fleet icons
        std::pair<System::const_orbit_iterator, System::const_orbit_iterator> range = s_system->non_orbit_range();
        std::vector<const Fleet*> flt_vec = s_system->FindObjects<Fleet>();
        for (unsigned int i = 0; i < flt_vec.size(); i++) 
            m_fleet_connections.insert(std::pair<int, boost::signals::connection>(flt_vec[i]->ID(), GG::Connect(flt_vec[i]->StateChangedSignal, &SidePanel::FleetsChanged, this)));

        // add planets
        std::vector<const Planet*> plt_vec = s_system->FindObjects<Planet>();

        std::vector<const UniverseObject*> owned_planets;
        for (std::vector<const Planet*>::const_iterator it = plt_vec.begin(); it != plt_vec.end(); ++it)
            if ((*it)->WhollyOwnedBy(HumanClientApp::GetApp()->EmpireID()))
                owned_planets.push_back(dynamic_cast<const UniverseObject*>(*it));

        std::vector<MeterType> meter_types;
        meter_types.push_back(METER_FARMING);   meter_types.push_back(METER_MINING);    meter_types.push_back(METER_INDUSTRY);
        meter_types.push_back(METER_RESEARCH);  meter_types.push_back(METER_TRADE);

        delete m_system_resource_summary;
        m_system_resource_summary = new MultiIconValueIndicator(Width() - MAX_PLANET_DIAMETER - 8, owned_planets, meter_types);
        m_system_resource_summary->MoveTo(GG::Pt(MAX_PLANET_DIAMETER + 4, 140 - m_system_resource_summary->Height()));
        AttachChild(m_system_resource_summary);

        m_planet_panel_container->SetPlanets(plt_vec, s_system->Star());
        for (unsigned int i = 0; i < plt_vec.size(); i++) {
            m_system_connections.insert(GG::Connect(plt_vec[i]->StateChangedSignal, &MultiIconValueIndicator::Update, m_system_resource_summary));
            m_system_connections.insert(GG::Connect(plt_vec[i]->ResourceCenterChangedSignal, SidePanel::ResourceCenterChangedSignal));
        }

        m_system_resource_summary->Update();
    }
}

void SidePanel::SystemSelectionChanged(int selection)
{
    int system_id = UniverseObject::INVALID_OBJECT_ID;
    if (0 <= selection && selection < m_system_name->NumRows())
        system_id = static_cast<const SystemRow&>(m_system_name->GetRow(selection)).m_system_id;
    if (SystemID() != system_id)
        SystemSelectedSignal(system_id);
}

void SidePanel::PrevButtonClicked()
{
    int selected = m_system_name->CurrentItemIndex();
    m_system_name->Select(selected ? selected - 1 : m_system_name->NumRows() - 1);
}

void SidePanel::NextButtonClicked()
{
    int selected = m_system_name->CurrentItemIndex();
    m_system_name->Select((selected < m_system_name->NumRows() - 1) ? selected + 1 : 0);
}

void SidePanel::PlanetSelected(int planet_id)
{
    PlanetSelectedSignal(planet_id);
}

int SidePanel::PlanetPanels() const
{
    return m_planet_panel_container->PlanetPanels();
}

const SidePanel::PlanetPanel* SidePanel::GetPlanetPanel(int n) const
{
    return m_planet_panel_container->GetPlanetPanel(n);
}

int SidePanel::SystemID() const
{
    return s_system ? s_system->ID() : UniverseObject::INVALID_OBJECT_ID;
}

int SidePanel::PlanetID() const
{
    return m_planet_panel_container->PlanetID();
}

void SidePanel::SelectPlanet(int planet_id)
{
    m_planet_panel_container->SelectPlanet(planet_id);
}

void SidePanel::SetValidSelectionPredicate(const boost::shared_ptr<UniverseObjectVisitor> &visitor)
{
    m_planet_panel_container->SetValidSelectionPredicate(visitor);
}

void SidePanel::SetSystem(int system_id)
{
    const System* system = GetUniverse().Object<const System>(system_id);
    if (system && system != s_system)
        PlaySidePanelOpenSound();
    s_system = system;
    for (std::set<SidePanel*>::iterator it = s_side_panels.begin(); it != s_side_panels.end(); ++it) {
        (*it)->SetSystemImpl();
    }
}

void SidePanel::SystemFleetAdded(const Fleet& flt)
{
    m_fleet_connections.insert(std::pair<int, boost::signals::connection>(flt.ID(), GG::Connect(flt.StateChangedSignal, &SidePanel::FleetsChanged, this)));
    FleetsChanged();
}

void SidePanel::SystemFleetRemoved(const Fleet& flt)
{
    std::map<int, boost::signals::connection>::iterator map_it = m_fleet_connections.find(flt.ID());
    if (map_it != m_fleet_connections.end()) {
        map_it->second.disconnect();
        m_fleet_connections.erase(map_it);
    }
    FleetsChanged();
}

void SidePanel::FleetsChanged()
{
    // may need to add or remove colonize buttons
    m_planet_panel_container->RefreshAllPlanetPanels();

    // TODO: if there are fleet status indicators on the SidePanel, update them
}
