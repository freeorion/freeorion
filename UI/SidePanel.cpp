#include "SidePanel.h"

#include "CUIWnd.h"
#include "CUIControls.h"

#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Predicates.h"
#include "../universe/ShipDesign.h"
#include "../util/Random.h"
#include "FocusSelector.h"
#include "../util/XMLDoc.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/DynamicGraphic.h>
#include <GG/Scroll.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../util/OptionsDB.h"

#include "../UI/FleetWindow.h"

#include <boost/format.hpp>

#include "MapWnd.h"

#include <fstream>

using boost::lexical_cast;

#define ROTATING_PLANET_IMAGES 1 // set this to 1 to use the OpenGL-rendered rotating planets code

namespace {
    bool PlaySounds() {return GetOptionsDB().Get<bool>("UI.sound.enabled");}
    void PlaySidePanelOpenSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.sidepanel-open"));}
    void PlayFarmingFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.farming-focus"));}
    void PlayIndustryFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.industry-focus"));}
    void PlayResearchFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.research-focus"));}
    void PlayMiningFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.mining-focus"));}
    void PlayTradeFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.trade-focus"));}
    void PlayBalancedFocusClickSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.balanced-focus"));}

    int CircleXFromY(double y, double r) {return static_cast<int>(std::sqrt(r * r - y * y) + 0.5);}

    struct RotatingPlanetData
    {
        RotatingPlanetData(const XMLElement& elem)
        {
            if (elem.Tag() != "RotatingPlanetData")
                throw std::invalid_argument("Attempted to construct a RotatingPlanetData from an XMLElement that had a tag other than \"RotatingPlanetData\"");

            planet_type = lexical_cast<PlanetType>(elem.Child("planet_type").Text());
            filename = elem.Child("filename").Text();
            RPM = lexical_cast<double>(elem.Child("RPM").Text());
            axis_angle = lexical_cast<double>(elem.Child("axis_angle").Text());
            shininess = lexical_cast<double>(elem.Child("shininess").Text());

            // ensure proper bounds
            axis_angle = std::max(-75.0, std::min(axis_angle, 88.0)); // these values ensure that GLUT-sphere artifacting does not show itself
            shininess = std::max(0.0, std::min(shininess, 128.0));
        }

        XMLElement XMLEncode() const
        {
            XMLElement retval("RotatingPlanetData");
            retval.AppendChild(XMLElement("planet_type", lexical_cast<std::string>(planet_type)));
            retval.AppendChild(XMLElement("filename", filename));
            retval.AppendChild(XMLElement("RPM", lexical_cast<std::string>(RPM)));
            retval.AppendChild(XMLElement("axis_angle", lexical_cast<std::string>(axis_angle)));
            retval.AppendChild(XMLElement("shininess", lexical_cast<std::string>(shininess)));
            return retval;
        }

        PlanetType planet_type; ///< the type of planet for which this data may be used
        std::string filename;   ///< the filename of the image used to texture a rotating image
        double RPM;             ///< the rotation of this planet, in revolutions per minute (may be negative, which will cause CW rotation)
        double axis_angle;      ///< the angle, in degrees, of the axis on which the planet rotates, measured CCW from straight up (may be negative)
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
                    throw std::invalid_argument("Attempted to construct a Atmosphere from an XMLElement that had a tag other than \"Atmosphere\"");
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
            std::ifstream ifs((ClientUI::ART_DIR + "planets/planets.xml").c_str());
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
            std::ifstream ifs((ClientUI::ART_DIR + "planets/atmospheres.xml").c_str());
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
            std::ifstream ifs((ClientUI::ART_DIR + "planets/planets.xml").c_str());
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
            std::ifstream ifs((ClientUI::ART_DIR + "planets/planets.xml").c_str());
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
            std::ifstream ifs((ClientUI::ART_DIR + "planets/planets.xml").c_str());
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

            glColor4ubv(GG::CLR_WHITE.v);
            gluSphere(quad, r, 100, 100);
        }
    }

    GLfloat* GetLightPosition()
    {
        static GLfloat retval[] = {0.0, 0.0, 0.0, 0.0};

        if (retval[0] == 0.0 && retval[1] == 0.0 && retval[2] == 0.0) {
            XMLDoc doc;
            std::ifstream ifs((ClientUI::ART_DIR + "planets/planets.xml").c_str());
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
            std::ifstream ifs((ClientUI::ART_DIR + "planets/planets.xml").c_str());
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

    void RenderPlanet(const GG::Pt& center, int diameter, boost::shared_ptr<GG::Texture> texture, double initial_rotation, double RPM, double axis_tilt, double shininess, StarType star_type)
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
        glRotated(axis_tilt, 0.0, 1.0, 0.0);  // axis tilt
        double intensity = GetRotatingPlanetAmbientIntensity();
        GG::Clr ambient(intensity, intensity, intensity, 1.0);
        intensity = GetRotatingPlanetDiffuseIntensity();
        GG::Clr diffuse(intensity, intensity, intensity, 1.0);
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
        default                   : scale = 2.0/5.0; break;
        }

        return static_cast<int>(SidePanel::MIN_PLANET_DIAMETER + (SidePanel::MAX_PLANET_DIAMETER - SidePanel::MIN_PLANET_DIAMETER) * scale);
    }

    bool temp_header_bool = RecordHeaderFile(SidePanelRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

/** a single planet's info and controls; several of these may appear at any one time in a SidePanel */
class SidePanel::PlanetPanel : public GG::Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> LeftClickedSignalType; ///< emitted when the planet graphic is left clicked by the user
    //@}
   
    /** \name Slot Types */ //@{
    typedef LeftClickedSignalType::slot_type LeftClickedSlotType; ///< type of functor(s) invoked on a LeftClickedSignalType
    //@}

    /** \name Structors */ //@{
    PlanetPanel(int x, int y, int w, int h, const Planet &planet, StarType star_type); ///< basic ctor
    ~PlanetPanel();
    //@}

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;
    int PlanetID() const {return m_planet_id;}
    bool Hilited() const;
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, Uint32 keys);
    virtual void RClick(const GG::Pt& pt, Uint32 keys);
    virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    void Update();
    void Hilite(bool b);
    //@}

    mutable LeftClickedSignalType PlanetImageLClickedSignal; ///< returns the left clicked signal object for this Planet panel

private:
    /** some of the elements at planet panel are only used if a specific
        planet ownership state is present, some others are only used if
        additional conditions applies. If a control is being enabled, it's
        moved from the list of disabled controls (m_vec_unused_controls) to
        the child list of planet panel, if the control isn't found at m_vec_unused_controls
        is assumed that it is already enable. after that control->Show() is called. Disabling a
        control is done in reverse.
        for example: colonize btn is only enable/visible if there is a colony ship in orbit 
        and the planet is unowned and inhabitable*/
    void EnableControl(GG::Wnd *control, bool enable);


    bool RenderUnhabited(const Planet &planet); ///< it's call if the planet isn't inhabited
    bool RenderInhabited(const Planet &planet); ///< it's call if the planet is inhabited by someone else
    bool RenderOwned    (const Planet &planet); ///< it's call if the planet is inhabited by te player

    int  PlanetDiameter() const;
    bool InPlanet(const GG::Pt& pt) const;///< returns true if pt is within the planet image

    void PlanetChanged();                 ///< called when a planet was changed to handle rendering and which controls are enabled
    void PlanetResourceCenterChanged();   ///< called when a planet resource production was changed

    void SetPrimaryFocus  (FocusType focus); ///< set the primary focus of the planet to focus
    void SetSecondaryFocus(FocusType focus); ///< set the secondary focus of the planet to focus

    void ClickColonize();///< called if btn colonize is pressed

    Planet* GetPlanet(); ///< returns the planet with ID m_planet_id
    const Planet* GetPlanet() const;

    int                   m_planet_id;                ///< id for the planet with is representet by this planet panel
    GG::TextControl       *m_planet_name;             ///< planet name
    GG::TextControl       *m_planet_info;             ///< planet size and type info
    FocusSelector         *m_focus_selector;          ///< buttons and displays for foci and associated meters
    CUIButton             *m_button_colonize;         ///< btn which can be pressed to colonize this planet
    GG::DynamicGraphic    *m_planet_graphic;          ///< image of the planet (can be a frameset); this is now used only for asteroids
    RotatingPlanetControl *m_rotating_planet_graphic; ///< a realtime-rendered planet that rotates, with a textured surface mapped onto it
    bool                  m_hilited;

    boost::signals::connection m_connection_system_changed;           ///< stores connection used to handle a system change
    boost::signals::connection m_connection_planet_changed;           ///< stores connection used to handle a planet change
    boost::signals::connection m_connection_planet_production_changed;///< stores connection used to handle a planet resource production change

    /** planet panel is constructed without taking care of which controls
        are needed by current planet ownership state. All controls which aren't
        needed by current planet ownership state are stored in m_vec_unused_controls
        and can be used when for instance planet ownership changes
    */
    std::vector<GG::Wnd*> m_vec_unused_controls;
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
    void HiliteSelectedPlanet(bool b); ///< enables/disables hiliting the currently-selected planet in the container
    void SelectPlanet(int planet_id);

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;
    virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)

    int                PlanetID() const            {return m_planet_id;}
    int                PlanetPanels() const        {return m_planet_panels.size();}
    const PlanetPanel* GetPlanetPanel(int n) const {return m_planet_panels[n];}
    //@}

    PlanetPanel* GetPlanetPanel(int n) {return m_planet_panels[n];}

    mutable PlanetSelectedSignalType PlanetSelectedSignal;

private:
    void PlanetSelected(int planet_id);

    std::vector<PlanetPanel*> m_planet_panels;
    int                       m_planet_id;
    bool                      m_hilite_selected_planet;

    void VScroll(int,int,int,int);
    CUIScroll*        m_vscroll; ///< the vertical scroll (for viewing all the planet panes)
};

class SidePanel::SystemResourceSummary : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    SystemResourceSummary(int x, int y, int w, int h);
    //@}

    /** \name Mutators */ //@{
    virtual void Render();

    void SetFarming (int farming ) {m_farming = farming;}
    void SetMining  (int mining  ) {m_mining  = mining;}
    void SetTrade   (int trade   ) {m_trade   = trade;}
    void SetResearch(int research) {m_research= research;}
    void SetIndustry(int industry) {m_industry= industry;}
    void SetDefense (int defense ) {m_defense = defense;}
    //@}

private:
    int m_farming,m_mining,m_trade,m_research,m_industry,m_defense;
};

class RotatingPlanetControl : public GG::Control
{
public:
    RotatingPlanetControl(int x, int y, PlanetSize size, StarType star_type, const RotatingPlanetData& planet_data) :
        GG::Control(x, y, PlanetDiameter(size), PlanetDiameter(size), 0),
        m_planet_data(planet_data),
        m_size(size),
        m_surface_texture(GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + m_planet_data.filename, true)),
        m_atmosphere_texture(),
        m_initial_rotation(RandZeroToOne()),
        m_star_type(star_type)
    {
        m_surface_texture->SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        const std::map<std::string, PlanetAtmosphereData>& atmosphere_data = GetPlanetAtmosphereData();
        std::map<std::string, PlanetAtmosphereData>::const_iterator it = atmosphere_data.find(m_planet_data.filename);
        if (it != atmosphere_data.end()) {
            const PlanetAtmosphereData::Atmosphere& atmosphere = it->second.atmospheres[RandSmallInt(0, it->second.atmospheres.size() - 1)];
            m_atmosphere_texture = GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + atmosphere.filename, true);
            m_atmosphere_texture->SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
            m_atmosphere_alpha = atmosphere.alpha;
            m_atmosphere_planet_rect = GG::Rect(1, 1, m_atmosphere_texture->DefaultWidth() - 4, m_atmosphere_texture->DefaultHeight() - 4);
        }
    }

    virtual void Render()
    {
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        RenderPlanet(ul + GG::Pt(Width() / 2, Height() / 2), Width(), m_surface_texture, m_initial_rotation,
                     SizeRotationFactor(m_size) * m_planet_data.RPM, m_planet_data.axis_angle, m_planet_data.shininess, m_star_type);
        if (m_atmosphere_texture) {
            int texture_w = m_atmosphere_texture->DefaultWidth();
            int texture_h = m_atmosphere_texture->DefaultHeight();
            double x_scale = PlanetDiameter(m_size) / static_cast<double>(texture_w);
            double y_scale = PlanetDiameter(m_size) / static_cast<double>(texture_h);
            glColor4ub(255, 255, 255, m_atmosphere_alpha);
            m_atmosphere_texture->OrthoBlit(static_cast<int>(ul.x - m_atmosphere_planet_rect.ul.x * x_scale),
                                            static_cast<int>(ul.y - m_atmosphere_planet_rect.ul.y * y_scale),
                                            static_cast<int>(lr.x + (texture_w - m_atmosphere_planet_rect.lr.x) * x_scale),
                                            static_cast<int>(lr.y + (texture_h - m_atmosphere_planet_rect.lr.y) * y_scale),
                                            0, false);
        }
    }

    void SetRotatingPlanetData(const RotatingPlanetData& planet_data)
    {
        m_planet_data = planet_data;
        m_surface_texture = GG::GUI::GetGUI()->GetTexture(m_planet_data.filename);
    }

private:
    double SizeRotationFactor(PlanetSize size) const
    {
        switch (size) {
        case SZ_TINY:     return 2.0;
        case SZ_SMALL:    return 1.5;
        case SZ_MEDIUM:   return 1.0;
        case SZ_LARGE:    return 0.75;
        case SZ_HUGE:     return 0.5;
        case SZ_GASGIANT: return 0.25;
        default:          return 1.0;
        }
        return 1.0;
    }

    RotatingPlanetData              m_planet_data;
    PlanetSize                      m_size;
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
  const int IMAGES_PER_PLANET_TYPE = 3; // number of planet images available per planet type (named "type1.png", "type2.png", ...)
  const int SYSTEM_NAME_FONT_SIZE = static_cast<int>(ClientUI::PTS*1.4);
  
  boost::shared_ptr<GG::Texture> GetTexture(const std::string& name, bool mipmap = false)
  {
    try
    {
      return HumanClientApp::GetApp()->GetTexture(name,mipmap);
    }
    catch(...)
    {
      return HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "misc/missing.png",mipmap);
    }
  }

  boost::shared_ptr<GG::Texture> IconPopulation() {return GetTexture(ClientUI::ART_DIR + "icons/pop.png"        );}
  boost::shared_ptr<GG::Texture> IconIndustry  () {return GetTexture(ClientUI::ART_DIR + "icons/industry.png"   );}
  boost::shared_ptr<GG::Texture> IconTrade     () {return GetTexture(ClientUI::ART_DIR + "icons/trade.png"      );}
  boost::shared_ptr<GG::Texture> IconResearch  () {return GetTexture(ClientUI::ART_DIR + "icons/research.png"   );}
  boost::shared_ptr<GG::Texture> IconMining    () {return GetTexture(ClientUI::ART_DIR + "icons/mining.png"     );}
  boost::shared_ptr<GG::Texture> IconFarming   () {return GetTexture(ClientUI::ART_DIR + "icons/farming.png"    );}
  boost::shared_ptr<GG::Texture> IconDefense   () {return GetTexture(ClientUI::ART_DIR + "icons/defensebase.png");}

  struct SystemRow : public GG::ListBox::Row
  {
    public:
      SystemRow(int system_id) : m_system_id(system_id) {SetDragDropDataType("SystemID");}
      int m_system_id;
  };

  XMLElement GetXMLChild(XMLElement &node,const std::string &child_path)
  {
    int index;

    if(-1==(index=child_path.find_first_of('.')))
      return node.ContainsChild(child_path)?node.Child(child_path):XMLElement();
    else
      return node.ContainsChild(child_path.substr(0,index))
              ?GetXMLChild(node.Child(child_path.substr(0,index)),child_path.substr(index+1,child_path.length()-index-1))
              :XMLElement();
  }

  void GetAsteroidTextures(int planet_id, std::vector<boost::shared_ptr<GG::Texture> > &textures)
  {
    const int NUM_ASTEROID_SETS = 3;
    const int NUM_IMAGES_PER_SET = 256;
    const int SET = (planet_id % NUM_ASTEROID_SETS) + 1;

    for (int i = 0; i < NUM_IMAGES_PER_SET; ++i) {
      char buf[256];
      sprintf(buf, "asteroids%d_%03d.png", SET, i);
      textures.push_back(HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "planets/asteroids/" + buf));
    }
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

  Ship* FindColonyShip(int system_id)
  {
    const System *system = GetUniverse().Object<const System>(system_id);
    if(system==0)
      return 0;

    std::vector<const Fleet*> flt_vec = system->FindObjects<Fleet>();

    Ship* ship=0;

    for(unsigned int i=0;i<flt_vec.size();i++)
      if(flt_vec[i]->Owners().find(HumanClientApp::GetApp()->EmpireID()) != flt_vec[i]->Owners().end())
      {
        Ship* s=0;
        for(Fleet::const_iterator it = flt_vec[i]->begin(); it != flt_vec[i]->end(); ++it)
          if(   (s=GetUniverse().Object<Ship>(*it))
             && s->Design()->colonize)
          {
            ship = s;

            // prefere non moving colony ship
            if(!flt_vec[i]->Accept(StationaryFleetVisitor(*flt_vec[i]->Owners().begin())))
              break;
            return s;
          }
      }

    return ship;
  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
SidePanel::PlanetPanel::PlanetPanel(int x, int y, int w, int h, const Planet &planet, StarType star_type) :
  Wnd(0, y, w, h, GG::CLICKABLE),
  m_planet_id(planet.ID()),
  m_planet_name(0),
  m_planet_info(0),
  m_focus_selector(0),
  m_button_colonize(0),
  m_planet_graphic(0),
  m_rotating_planet_graphic(0),
  m_hilited(false)
{
  SetText(UserString("PLANET_PANEL"));

  m_planet_name = new GG::TextControl(MAX_PLANET_DIAMETER-15,10,planet.Name(),GG::GUI::GetGUI()->GetFont(ClientUI::FONT,ClientUI::SIDE_PANEL_PLANET_NAME_PTS),ClientUI::TEXT_COLOR);
  AttachChild(m_planet_name);

  GG::Pt ul = UpperLeft(), lr = LowerRight();
  int planet_image_sz = PlanetDiameter();
  GG::Pt planet_image_pos(MAX_PLANET_DIAMETER / 2 - planet_image_sz / 2 + 3, Height() / 2 - planet_image_sz / 2);

  if (planet.Type() == PT_ASTEROIDS)
  {
      std::vector<boost::shared_ptr<GG::Texture> > textures;
      GetAsteroidTextures(planet.ID(), textures);
      m_planet_graphic = new GG::DynamicGraphic(planet_image_pos.x,planet_image_pos.y,planet_image_sz,planet_image_sz,true,textures[0]->DefaultWidth(),textures[0]->DefaultHeight(),0,textures, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
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
          Uint32 hash_value = (static_cast<Uint32>(planet.SystemID()) & 0xFFFF) + (static_cast<Uint32>(planet.ID()) & 0xFFFF);
          hash_value += ~(hash_value << 15);
          hash_value ^= hash_value >> 10;
          hash_value += hash_value << 3;
          hash_value ^= hash_value >> 6;
          hash_value += ~(hash_value << 11);
          hash_value ^= hash_value >> 16;
          m_rotating_planet_graphic = new RotatingPlanetControl(planet_image_pos.x, planet_image_pos.y, planet.Size(), star_type,
                                                                it->second[hash_value % num_planets_of_type]);
          AttachChild(m_rotating_planet_graphic);
      }
  }

  m_planet_info = new GG::TextControl(m_planet_name->UpperLeft().x-UpperLeft().x+10,m_planet_name->LowerRight().y-UpperLeft().y,"",GG::GUI::GetGUI()->GetFont(ClientUI::FONT,ClientUI::SIDE_PANEL_PTS),ClientUI::TEXT_COLOR,GG::TF_LEFT|GG::TF_TOP);
  AttachChild(m_planet_info);

  m_button_colonize = new CUIButton((Width()/3)*2,(Height()-ClientUI::SIDE_PANEL_PTS)/2,60,UserString("PL_COLONIZE"),GG::GUI::GetGUI()->GetFont(ClientUI::FONT,ClientUI::SIDE_PANEL_PTS),ClientUI::BUTTON_COLOR,ClientUI::CTRL_BORDER_COLOR,1,ClientUI::TEXT_COLOR,GG::CLICKABLE);
  Connect(m_button_colonize->ClickedSignal, &SidePanel::PlanetPanel::ClickColonize, this);
  AttachChild(m_button_colonize);

  m_focus_selector = new FocusSelector(175, planet);
  m_focus_selector->MoveTo(GG::Pt(Width() - m_focus_selector->Width(),
                                  (Height() - m_focus_selector->Height()) / 2));
  AttachChild(m_focus_selector);
  GG::Connect(m_focus_selector->PrimaryFocusChangedSignal, &SidePanel::PlanetPanel::SetPrimaryFocus, this);
  GG::Connect(m_focus_selector->SecondaryFocusChangedSignal, &SidePanel::PlanetPanel::SetSecondaryFocus, this);

  if (planet.Type() == PT_ASTEROIDS) 
  {
    MoveChildDown(m_planet_graphic);
  }

  const Planet *plt = GetUniverse().Object<const Planet>(m_planet_id);

  if (System* system = plt->GetSystem())
    m_connection_system_changed = GG::Connect(system->StateChangedSignal, &SidePanel::PlanetPanel::PlanetChanged, this);
  m_connection_planet_changed = GG::Connect(plt->StateChangedSignal, &SidePanel::PlanetPanel::PlanetChanged, this);
  m_connection_planet_production_changed= GG::Connect(plt->ResourceCenterChangedSignal, &SidePanel::PlanetPanel::PlanetResourceCenterChanged, this);

  Update();
}

SidePanel::PlanetPanel::~PlanetPanel()
{
  for(unsigned int i=0;i<m_vec_unused_controls.size();i++)
    delete m_vec_unused_controls[i];
  m_vec_unused_controls.clear();
}

Planet* SidePanel::PlanetPanel::GetPlanet()
{
  Planet *planet = GetUniverse().Object<Planet>(m_planet_id);
  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::GetPlanet: planet not found!");
  return planet;
}

const Planet* SidePanel::PlanetPanel::GetPlanet() const
{
  const Planet *planet = GetUniverse().Object<const Planet>(m_planet_id);
  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::GetPlanet: planet not found!");
  return planet;
}

void SidePanel::PlanetPanel::Update()
{
  PlanetChanged();
  PlanetResourceCenterChanged();
}

void SidePanel::PlanetPanel::Hilite(bool b)
{
    m_hilited = b;
}

void SidePanel::PlanetPanel::EnableControl(GG::Wnd *control,bool enable)
{
  std::vector<GG::Wnd*>::iterator it = std::find(m_vec_unused_controls.begin(),m_vec_unused_controls.end(),control);
  if(it != m_vec_unused_controls.end())
  {
    if(enable)
    {
      m_vec_unused_controls.erase(it);
      AttachChild(control);
      control->Show();
    }
  }
  else
  {
    if(!enable)
    {
      m_vec_unused_controls.push_back(control);
      DetachChild(control);
      control->Hide();
    }
  }
}

void SidePanel::PlanetPanel::PlanetChanged()
{
  const Planet *planet = GetPlanet();

  enum OWNERSHIP {OS_NONE,OS_FOREIGN,OS_SELF} owner = OS_NONE;

  std::string text;
  if(planet->Owners().size()==0 || planet->IsAboutToBeColonized()) 
  { //uninhabited
    owner = OS_NONE;
    text = GetPlanetSizeName(*planet);
    if(text.length()>0)
      text+=" ";
    text+= GetPlanetTypeName(*planet);
  
    text+="\n";
    if(planet->MaxPop()==0) text+= UserString("PE_UNINHABITABLE");
    else                    text+= UserString("PL_SIZE") + " " + lexical_cast<std::string>(static_cast<int>(planet->MaxPop()));

    m_planet_info->SetText(text);
  }
  else 
    if(!planet->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
    {//inhabited
      owner = OS_FOREIGN;
      text = GetPlanetSizeName(*planet);
      if(text.length()>0)
        text+=" ";
      text+= GetPlanetTypeName(*planet);
    
      m_planet_info->SetText(text);
    }
    else
    {//Owned
      owner = OS_SELF;
      text = GetPlanetSizeName(*planet);
      if(text.length()>0)
        text+="\n";
      text+= GetPlanetTypeName(*planet);
      
      m_planet_info->SetText(text);
    }

  if (!planet->Owners().empty()) {
      Empire* planet_empire = HumanClientApp::Empires().Lookup(*(planet->Owners().begin()));
      m_planet_name->SetTextColor(planet_empire?planet_empire->Color():ClientUI::TEXT_COLOR);
  }

  // visibility
  if (owner==OS_NONE 
      && planet->MaxPop()>0 
      && !planet->IsAboutToBeColonized()
      && FindColonyShip(planet->SystemID()))
  {
    std::vector<GG::Wnd*>::iterator it = std::find(m_vec_unused_controls.begin(),m_vec_unused_controls.end(),m_button_colonize);
    if(it != m_vec_unused_controls.end())
    {
      m_vec_unused_controls.erase(it);
      AttachChild(m_button_colonize);
      m_button_colonize->Show();
    }
    m_button_colonize->SetText(UserString("PL_COLONIZE"));
  }
  else if (planet->IsAboutToBeColonized())
  {
    std::vector<GG::Wnd*>::iterator it = std::find(m_vec_unused_controls.begin(),m_vec_unused_controls.end(),m_button_colonize);
    if(it != m_vec_unused_controls.end())
    {
      m_vec_unused_controls.erase(it);
      AttachChild(m_button_colonize);
      m_button_colonize->Show();
    }
    m_button_colonize->SetText(UserString("CANCEL"));
  }
  else
  {
     std::vector<GG::Wnd*>::iterator it = std::find(m_vec_unused_controls.begin(),m_vec_unused_controls.end(),m_button_colonize);
     if(it == m_vec_unused_controls.end())
     {
        m_vec_unused_controls.push_back(m_button_colonize);
        DetachChild(m_button_colonize);
        m_button_colonize->Hide();
     }
  }

  EnableControl(m_focus_selector, (owner==OS_SELF));
}

void SidePanel::PlanetPanel::PlanetResourceCenterChanged()
{
  TempUISoundDisabler sound_disabler;

  const Planet *planet = GetPlanet();
  m_focus_selector->Update(*planet);
}

void SidePanel::PlanetPanel::SetPrimaryFocus(FocusType focus)
{
  Planet *planet = GetPlanet();
  HumanClientApp::Orders().IssueOrder(new ChangeFocusOrder(HumanClientApp::GetApp()->EmpireID(),planet->ID(),focus,true));
}

void SidePanel::PlanetPanel::SetSecondaryFocus(FocusType focus)
{
  Planet *planet = GetPlanet();
  HumanClientApp::Orders().IssueOrder(new ChangeFocusOrder(HumanClientApp::GetApp()->EmpireID(),planet->ID(),focus,false));
} 

void SidePanel::PlanetPanel::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
  GG::Wnd *parent;
  if((parent=Parent()))
    parent->MouseWheel(pt,move,keys);
}

bool SidePanel::PlanetPanel::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    ul.x += MAX_PLANET_DIAMETER / 2;
    return ((ul <= pt && pt < lr) || InPlanet(pt));
}

bool SidePanel::PlanetPanel::Hilited() const
{
    return m_hilited;
}

void SidePanel::PlanetPanel::LClick(const GG::Pt& pt, Uint32 keys) 
{
  if(InPlanet(pt))
  {
    if(GetOptionsDB().Get<bool>("UI.sound.enabled"))
      HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.planet-button-click"));
	PlanetImageLClickedSignal(m_planet_id);
  }
}

bool SidePanel::PlanetPanel::RenderUnhabited(const Planet &planet)
{
  return true;
}

bool SidePanel::PlanetPanel::RenderInhabited(const Planet &planet)
{
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT,ClientUI::SIDE_PANEL_PTS);
  Uint32 format = GG::TF_LEFT | GG::TF_BOTTOM;

  std::string text; int x,y;

  x = m_planet_name->UpperLeft ().x+10;
  y = m_planet_name->LowerRight().y+ 5;

  //text = GetPlanetSizeName(planet);
  //font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0);
  y+=font->Height();

  //text = GetPlanetTypeName(planet);
  //font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0);
  y+=font->Height();

  int population=static_cast<int>(planet.PopPoints());

  boost::shared_ptr<GG::Texture> icon;
  const int ICON_MARGIN    =  5;
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.2));

  //population
  //x = m_planet_name->UpperLeft ().x+10; y = m_planet_name->LowerRight().y + RESOURCE_DISPLAY_HEIGHT+3;
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconPopulation(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  x+=font->Height();
  text = lexical_cast<std::string>(population)+"/"+lexical_cast<std::string>(static_cast<int>(planet.MaxPop()));
  font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0);
  x+=font->TextExtent(text, format).x+ICON_MARGIN;

  return true;
}

bool SidePanel::PlanetPanel::RenderOwned(const Planet &planet)
{
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT,ClientUI::SIDE_PANEL_PTS);
  Uint32 format = GG::TF_LEFT | GG::TF_BOTTOM;

  std::string text; int x,y;

  x = m_planet_name->UpperLeft ().x+10;
  y = m_planet_name->LowerRight().y+ 5;

  //text = GetPlanetSizeName(planet);
  //font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0);
  y+=font->Height();

  //text = GetPlanetTypeName(planet);
  //font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0);
  y+=font->Height();

  boost::shared_ptr<GG::Texture> icon;
  const int ICON_MARGIN    =  5;
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.2));

  //population
  //x = m_planet_name->UpperLeft ().x+10; y = m_planet_name->LowerRight().y + RESOURCE_DISPLAY_HEIGHT+3;
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconPopulation(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  x+=font->Height();

  double future_pop_growth = static_cast<int>(planet.FuturePopGrowth()*100.0) / 100.0;
  if     (future_pop_growth<0.0)  text=GG::RgbaTag(GG::CLR_RED);
  else if(future_pop_growth>0.0)  text=GG::RgbaTag(GG::CLR_GREEN);
       else                       text=GG::RgbaTag(ClientUI::TEXT_COLOR);

  text+= lexical_cast<std::string>(static_cast<int>(planet.PopPoints())) + "</rgba>/"+lexical_cast<std::string>(static_cast<int>(planet.MaxPop()));
  font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0);
  x+=font->TextExtent(text, format).x+ICON_MARGIN;

  return true;
}

void SidePanel::PlanetPanel::Render()
{
    const Planet *planet = GetPlanet();

    if(planet->Owners().size()==0 || planet->IsAboutToBeColonized()) {
        RenderUnhabited(*planet);
    } else {
        if(!planet->OwnedBy(HumanClientApp::GetApp()->EmpireID()))     
            RenderInhabited(*planet);
        else
            RenderOwned    (*planet);
    }

    if (m_hilited && planet->Type() != PT_ASTEROIDS) {
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
    GG::Pt center = UpperLeft() + GG::Pt(MAX_PLANET_DIAMETER / 2, Height() / 2);
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
  if(it == pending_colonization_orders.end()) // colonize
  {
    Ship *ship=FindColonyShip(planet->SystemID());
    if(ship==0)
      throw std::runtime_error("SidePanel::PlanetPanel::ClickColonize ship not found!");

    if(!ship->GetFleet()->Accept(StationaryFleetVisitor(*ship->GetFleet()->Owners().begin())))
    {
      GG::ThreeButtonDlg dlg(320,200,UserString("SP_USE_DEPARTING_COLONY_SHIPS_QUESTION"),
                             GG::GUI::GetGUI()->GetFont(ClientUI::FONT,ClientUI::PTS),ClientUI::WND_COLOR,ClientUI::CTRL_BORDER_COLOR,ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR,2,
                             UserString("YES"),UserString("NO"));
      dlg.Run();

      if(dlg.Result()!=0)
        return;
    }

    HumanClientApp::Orders().IssueOrder(new FleetColonizeOrder( empire_id, ship->ID(), planet->ID()));
  }
  else // cancel colonization
  {
    const FleetColonizeOrder *col_order = dynamic_cast<const FleetColonizeOrder*>(HumanClientApp::Orders().ExamineOrder(it->second));
    int ship_id = col_order?col_order->ShipID():UniverseObject::INVALID_OBJECT_ID;

    HumanClientApp::Orders().RecindOrder(it->second);
    
    // if the ship now buils a fleet of its own, make sure that fleet appears
    // at a possibly opend FleetWnd
    Ship  *ship = GetUniverse().Object<Ship>(ship_id);
    Fleet *fleet= ship?GetUniverse().Object<Fleet>(ship->FleetID()):NULL;
    if(fleet)
      for( FleetWnd::FleetWndItr it = FleetWnd::FleetWndBegin();it != FleetWnd::FleetWndEnd();++it)
      {
        FleetWnd *fleet_wnd = *it;
        if(   fleet->SystemID() == fleet_wnd->SystemID()
          && !fleet_wnd->ContainsFleet(fleet->ID()))
        {
          fleet_wnd->AddFleet(GetUniverse().Object<Fleet>(fleet->ID()));
          break;
        }
      }
  }
}

void SidePanel::PlanetPanel::RClick(const GG::Pt& pt, Uint32 keys)
{
  const Planet *planet = GetPlanet();
  
  if(!planet->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
    return;


  GG::MenuItem menu_contents;
  menu_contents.next_level.push_back(GG::MenuItem(UserString("SP_RENAME_PLANET"), 1, false, false));
  GG::PopupMenu popup(pt.x, pt.y, GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), menu_contents, ClientUI::TEXT_COLOR);

  if(popup.Run()) 
    switch (popup.MenuID())
    {
      case 1: 
      { // rename planet
        std::string plt_name = planet->Name();
        CUIEditWnd edit_wnd(350, UserString("SP_ENTER_NEW_PLANET_NAME"), plt_name);
        edit_wnd.Run();
        if(edit_wnd.Result() != "")
        {
          HumanClientApp::Orders().IssueOrder(new RenameOrder(HumanClientApp::GetApp()->EmpireID(), planet->ID(), edit_wnd.Result()));
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
    Wnd(x-MAX_PLANET_DIAMETER/2, y, w+MAX_PLANET_DIAMETER/2, h, GG::CLICKABLE),
    m_planet_panels(),
    m_planet_id(UniverseObject::INVALID_OBJECT_ID),
    m_hilite_selected_planet(false),
    m_vscroll(new CUIScroll(Width()-10,0,10,Height(),GG::VERTICAL))
{
  SetText("PlanetPanelContainer");
  EnableChildClipping(true);
  AttachChild(m_vscroll);
  GG::Connect(m_vscroll->ScrolledSignal, &SidePanel::PlanetPanelContainer::VScroll,this);
}

bool SidePanel::PlanetPanelContainer::InWindow(const GG::Pt& pt) const
{
  if(pt.y<UpperLeft().y)
    return false;

  bool retval = UpperLeft()+GG::Pt(MAX_PLANET_DIAMETER/2,0) <= pt && pt < LowerRight();
  for(unsigned int i = 0; i < m_planet_panels.size() && !retval; ++i)
    if(m_planet_panels[i]->InWindow(pt))
      retval = true;

  return retval;
}
void SidePanel::PlanetPanelContainer::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
  if(m_vscroll)
    move<0?m_vscroll->ScrollLineIncr():m_vscroll->ScrollLineDecr();
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

  int y = 0;
  const int PLANET_PANEL_HT = MAX_PLANET_DIAMETER;
  for (unsigned int i = 0; i < plt_vec.size(); ++i, y += PLANET_PANEL_HT) 
  {
    const Planet* planet = plt_vec[i];
    PlanetPanel* planet_panel = new PlanetPanel(0, y, Width()-m_vscroll->Width(), PLANET_PANEL_HT, *planet, star_type);
    AttachChild(planet_panel);
    m_planet_panels.push_back(planet_panel);
    GG::Connect(m_planet_panels.back()->PlanetImageLClickedSignal, &SidePanel::PlanetPanelContainer::PlanetSelected, this);
  }
  m_vscroll->SizeScroll(0,plt_vec.size()*PLANET_PANEL_HT,PLANET_PANEL_HT,Height());
  VScroll(m_vscroll->PosnRange().first, 0, 0, 0);
}

void SidePanel::PlanetPanelContainer::HiliteSelectedPlanet(bool b)
{
    m_hilite_selected_planet = b;
    PlanetPanel* current_panel = 0;
    for (std::vector<PlanetPanel*>::iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it) {
        if ((*it)->PlanetID() == m_planet_id) {
            current_panel = *it;
            break;
        }
    }
    if (current_panel)
        current_panel->Hilite(m_hilite_selected_planet);
}

void SidePanel::PlanetPanelContainer::SelectPlanet(int planet_id)
{
    PlanetSelected(planet_id);
}

void SidePanel::PlanetPanelContainer::PlanetSelected(int planet_id)
{
    if (planet_id != m_planet_id) {
        if (m_hilite_selected_planet) {
            PlanetPanel* current_panel = 0;
            for (std::vector<PlanetPanel*>::iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it) {
                if ((*it)->PlanetID() == m_planet_id) {
                    current_panel = *it;
                    break;
                }
            }
            if (current_panel)
                current_panel->Hilite(false);
            m_planet_id = planet_id;
            current_panel = 0;
            for (std::vector<PlanetPanel*>::iterator it = m_planet_panels.begin(); it != m_planet_panels.end(); ++it) {
                if ((*it)->PlanetID() == m_planet_id) {
                    current_panel = *it;
                    break;
                }
            }
            if (current_panel)
                current_panel->Hilite(true);
        }
        PlanetSelectedSignal(m_planet_id);
    }
}

void SidePanel::PlanetPanelContainer::VScroll(int from,int to,int range_min,int range_max)
{
  int y = -from;
  const int PLANET_PANEL_HT = MAX_PLANET_DIAMETER;
  for (unsigned int i = 0; i < m_planet_panels.size(); ++i, y += PLANET_PANEL_HT)
      m_planet_panels[i]->MoveTo(GG::Pt(UpperLeft().x-m_planet_panels[i]->UpperLeft().x,y));
}

////////////////////////////////////////////////
// SidePanel::SystemResourceSummary
////////////////////////////////////////////////
SidePanel::SystemResourceSummary::SystemResourceSummary(int x, int y, int w, int h)
: Wnd(x, y, w, h, GG::CLICKABLE),
  m_farming(0),m_mining(0),m_trade(0),m_research(0),m_industry(0),m_defense(0)
{
}

void SidePanel::SystemResourceSummary::Render()
{
  GG::FlatRectangle(UpperLeft().x,UpperLeft().y,LowerRight().x,LowerRight().y,GG::Clr(0.0,0.0,0.0,0.5),GG::CLR_ZERO,1);

  int farming=m_farming,mining=m_mining,trade=m_trade,research=m_research,industry=m_industry,defense=m_defense;

  std::string text; int x,y; boost::shared_ptr<GG::Texture> icon;
  boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.2));
  Uint32 format = GG::TF_LEFT | GG::TF_VCENTER;
  const int ICON_MARGIN    =  5;
  
  x=UpperLeft().x;y=UpperLeft().y;

  int info_elem_width = (Width()-(6+1)*ICON_MARGIN)/6;

  //farming
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconFarming(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (farming<0?"-":"+") + lexical_cast<std::string>(farming);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //mining
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconMining(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (mining<0?"-":"+") + lexical_cast<std::string>(mining);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //trade
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconTrade(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (trade<0?"-":"+") + lexical_cast<std::string>(trade);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //research
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconResearch(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (research<0?"-":"+") + lexical_cast<std::string>(research);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //industy
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconIndustry(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (industry<0?"-":"+") + lexical_cast<std::string>(industry);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //defense
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconDefense(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = lexical_cast<std::string>(defense)+"/"+lexical_cast<std::string>(defense*3);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;
}

////////////////////////////////////////////////
// SidePanel
////////////////////////////////////////////////
// static(s)
const int SidePanel::MAX_PLANET_DIAMETER = 128; // size of a huge planet, in on-screen pixels
const int SidePanel::MIN_PLANET_DIAMETER = MAX_PLANET_DIAMETER / 3; // size of a tiny planet, in on-screen pixels

SidePanel::SidePanel(int x, int y, int w, int h) : 
    Wnd(x, y, w, h, GG::CLICKABLE),
    m_system(0),
    m_system_name(new CUIDropDownList(40, 0, w-80,SYSTEM_NAME_FONT_SIZE, 10*SYSTEM_NAME_FONT_SIZE,GG::CLR_ZERO,GG::Clr(0.0, 0.0, 0.0, 0.5),ClientUI::SIDE_PANEL_COLOR)),
    m_system_name_unknown(new GG::TextControl(40, 0, w-80,SYSTEM_NAME_FONT_SIZE,UserString("SP_UNKNOWN_SYSTEM"),GG::GUI::GetGUI()->GetFont(ClientUI::FONT,static_cast<int>(ClientUI::PTS*1.4)),ClientUI::TEXT_COLOR)),
    m_button_prev(new GG::Button(40-SYSTEM_NAME_FONT_SIZE,4,SYSTEM_NAME_FONT_SIZE,SYSTEM_NAME_FONT_SIZE,"",GG::GUI::GetGUI()->GetFont(ClientUI::FONT,SYSTEM_NAME_FONT_SIZE),GG::CLR_WHITE,GG::CLICKABLE)),
    m_button_next(new GG::Button(40+w-80                 ,4,SYSTEM_NAME_FONT_SIZE,SYSTEM_NAME_FONT_SIZE,"",GG::GUI::GetGUI()->GetFont(ClientUI::FONT,SYSTEM_NAME_FONT_SIZE),GG::CLR_WHITE,GG::CLICKABLE)),
    m_star_graphic(0),
    m_static_text_systemproduction(new GG::TextControl(0,100-20-ClientUI::PTS-5,UserString("SP_SYSTEM_PRODUCTION"),GG::GUI::GetGUI()->GetFont(ClientUI::FONT,ClientUI::PTS),ClientUI::TEXT_COLOR)),
    m_next_pltview_fade_in(0),m_next_pltview_planet_id(UniverseObject::INVALID_OBJECT_ID),m_next_pltview_fade_out(-1),
    m_planet_panel_container(new PlanetPanelContainer(0,100,w,h-100-30)),
    m_system_resource_summary(new SystemResourceSummary(0,100-20,w,20))
{
  TempUISoundDisabler sound_disabler;

  SetText(UserString("SIDE_PANEL"));

  m_system_name->DisableDropArrow();
  m_system_name->SetStyle(GG::LB_CENTER);

  m_button_prev->SetUnpressedGraphic(GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/leftarrownormal.png"   ), 0, 0, 32, 32));
  m_button_prev->SetPressedGraphic  (GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/leftarrowclicked.png"  ), 0, 0, 32, 32));
  m_button_prev->SetRolloverGraphic (GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/leftarrowmouseover.png"), 0, 0, 32, 32));

  m_button_next->SetUnpressedGraphic(GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/rightarrownormal.png"  ), 0, 0, 32, 32));
  m_button_next->SetPressedGraphic  (GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/rightarrowclicked.png"   ), 0, 0, 32, 32));
  m_button_next->SetRolloverGraphic (GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/rightarrowmouseover.png"), 0, 0, 32, 32));

  AttachChild(m_system_name_unknown);
  AttachChild(m_system_name);
  AttachChild(m_button_prev);
  AttachChild(m_button_next);
  AttachChild(m_static_text_systemproduction);
  AttachChild(m_system_resource_summary);
  AttachChild(m_planet_panel_container);

  GG::Connect(m_system_name->SelChangedSignal, &SidePanel::SystemSelectionChanged, this);
  GG::Connect(m_button_prev->ClickedSignal, &SidePanel::PrevButtonClicked, this);
  GG::Connect(m_button_next->ClickedSignal, &SidePanel::NextButtonClicked, this);
  GG::Connect(m_planet_panel_container->PlanetSelectedSignal, &SidePanel::PlanetSelected, this);

  Hide();
}

bool SidePanel::InWindow(const GG::Pt& pt) const
{
  return (UpperLeft() <= pt && pt < LowerRight()) || m_planet_panel_container->InWindow(pt);
}

void SidePanel::Render()
{
  GG::Pt ul = UpperLeft(), lr = LowerRight();
  FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::SIDE_PANEL_COLOR, GG::CLR_ZERO, 0);
}

void SidePanel::SystemSelectionChanged(int selection)
{
  int system_id = UniverseObject::INVALID_OBJECT_ID;

  if(0<= selection && selection<m_system_name->NumRows())
    system_id = static_cast<const SystemRow&>(m_system_name->GetRow(selection)).m_system_id;

  if(SystemID()!=system_id)
    SetSystem(system_id);
}

void SidePanel::PrevButtonClicked()
{
  int selected = m_system_name->CurrentItemIndex();

  if(0< selected && selected<m_system_name->NumRows())
    m_system_name->Select(selected-1);
}

void SidePanel::NextButtonClicked()
{
  int selected = m_system_name->CurrentItemIndex();

  if(0<=selected && selected<m_system_name->NumRows()-1)
    m_system_name->Select(selected+1);
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
    return m_system!=0?m_system->ID():UniverseObject::INVALID_OBJECT_ID;
}

int SidePanel::PlanetID() const
{
    return m_planet_panel_container->PlanetID();
}

void SidePanel::SetSystem(int system_id)
{
    const System* new_system = HumanClientApp::GetUniverse().Object<const System>(system_id);
    if (new_system && new_system != m_system)
        PlaySidePanelOpenSound();
    TempUISoundDisabler sound_disabler;

    m_fleet_icons.clear();
    m_planet_panel_container->Clear();
    m_system_name->Clear();

    DeleteChild(m_star_graphic);m_star_graphic=0;

    Hide();

    m_system = new_system;

    if (m_system)
    {
      GG::Connect(m_system->FleetAddedSignal  , &SidePanel::SystemFleetAdded  , this);
      GG::Connect(m_system->FleetRemovedSignal, &SidePanel::SystemFleetRemoved, this);

      std::vector<const System*> sys_vec = GetUniverse().FindObjects<const System>();
      GG::ListBox::Row *select_row=0;

      for (unsigned int i = 0; i < sys_vec.size(); i++) 
      {
        GG::ListBox::Row *row = new SystemRow(sys_vec[i]->ID());

        if(sys_vec[i]->Name().length()==0)
          continue;
 
        row->push_back(boost::io::str(boost::format(UserString("SP_SYSTEM_NAME")) % sys_vec[i]->Name()), ClientUI::FONT,static_cast<int>(ClientUI::PTS*1.4), ClientUI::TEXT_COLOR);
        m_system_name->Insert(row);

        if(sys_vec[i]->ID() == system_id)
          select_row = row;
      }

      for (int i = 0; i < m_system_name->NumRows(); i++) 
        if(select_row == &m_system_name->GetRow(i))
        {
          m_system_name->Select(i);
          break;
        }

      std::vector<boost::shared_ptr<GG::Texture> > textures;
      boost::shared_ptr<GG::Texture> graphic;
     
      std::string star_image = ClientUI::ART_DIR + "stars_sidepanel/";
      switch (m_system->Star())
      {
        case STAR_BLUE     : star_image += "blue0"     ; break;
        case STAR_WHITE    : star_image += "white0"    ; break;
        case STAR_YELLOW   : star_image += "yellow0"   ; break;
        case STAR_ORANGE   : star_image += "orange0"   ; break;
        case STAR_RED      : star_image += "red0"      ; break;
        case STAR_NEUTRON  : star_image += "neutron0"  ; break;
        case STAR_BLACK    : star_image += "blackhole0"; break;
        default            : star_image += "white0"    ; break;
      }
      star_image += lexical_cast<std::string>(m_system->ID()%2)+".png";

      graphic = GetTexture(star_image);
      
      textures.push_back(graphic);

      int star_dim = (Width()*4)/5;
      m_star_graphic = new GG::DynamicGraphic(Width()-(star_dim*2)/3,-(star_dim*1)/3,star_dim,star_dim,true,textures[0]->DefaultWidth(),textures[0]->DefaultHeight(),0,textures, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);

      AttachChild(m_star_graphic);MoveChildDown(m_star_graphic);

      // TODO: add fleet icons
      std::pair<System::const_orbit_iterator, System::const_orbit_iterator> range = m_system->non_orbit_range();
      std::vector<const Fleet*> flt_vec = m_system->FindObjects<Fleet>();
      for(unsigned int i = 0; i < flt_vec.size(); i++) 
        GG::Connect(flt_vec[i]->StateChangedSignal, &SidePanel::FleetsChanged, this);

      // add planets
      std::vector<const Planet*> plt_vec = m_system->FindObjects<Planet>();

      m_planet_panel_container->SetPlanets(plt_vec, m_system->Star());
      for(unsigned int i = 0; i < plt_vec.size(); i++) 
      {
        GG::Connect(plt_vec[i]->StateChangedSignal, &SidePanel::PlanetsChanged, this);
        GG::Connect(plt_vec[i]->ResourceCenterChangedSignal, &SidePanel::PlanetsChanged, this);
      }

      m_planet_panel_container->SetPlanets(plt_vec, m_system->Star());

      Show();PlanetsChanged();
      if(select_row==0)
      {
        m_system_name_unknown->Show();
        m_system_name->Hide();
        m_button_prev->Hide();
        m_button_next->Hide();
      }
      else
      {
        m_system_name_unknown->Hide();
        m_system_name->Show();
        m_button_prev->Show();
        m_button_next->Show();
      }
    }
}

void SidePanel::SelectPlanet(int planet_id)
{
    m_planet_panel_container->SelectPlanet(planet_id);
}

void SidePanel::HiliteSelectedPlanet(bool b)
{
    m_planet_panel_container->HiliteSelectedPlanet(b);
}

void SidePanel::SystemFleetAdded  (const Fleet &flt)
{
  GG::Connect(flt.StateChangedSignal, &SidePanel::FleetsChanged, this);
  FleetsChanged();
}

void SidePanel::SystemFleetRemoved(const Fleet &)
{
  FleetsChanged();
}

void SidePanel::FleetsChanged()
{
  for(int i=0;i<m_planet_panel_container->PlanetPanels();i++)
    m_planet_panel_container->GetPlanetPanel(i)->Update();
}

void SidePanel::PlanetsChanged()
{
  if(m_system)
  {
    std::vector<const Planet*> plt_vec = m_system->FindObjects<Planet>();
    int farming=0,mining=0,trade=0,research=0,industry=0,defense=0,num_empire_planets=0;

    for(unsigned int i=0;i<plt_vec.size();i++)
      if(plt_vec[i]->Owners().find(HumanClientApp::GetApp()->EmpireID()) != plt_vec[i]->Owners().end())
      {
        farming   +=static_cast<int>(plt_vec[i]->FarmingPoints());
        mining    +=static_cast<int>(plt_vec[i]->MiningPoints());
        trade     +=static_cast<int>(plt_vec[i]->TradePoints());
        industry  +=static_cast<int>(plt_vec[i]->IndustryPoints());
        research  +=static_cast<int>(plt_vec[i]->ResearchPoints());
        defense   +=plt_vec[i]->DefBases();
        
        num_empire_planets++;
      }

    m_system_resource_summary->SetFarming (farming );
    m_system_resource_summary->SetMining  (mining  );
    m_system_resource_summary->SetTrade   (trade   );
    m_system_resource_summary->SetResearch(research);
    m_system_resource_summary->SetIndustry(industry);
    m_system_resource_summary->SetDefense (defense );

    if(num_empire_planets==0)
    {
      m_system_resource_summary->Hide();
      m_static_text_systemproduction->Hide();
    }
    else
    {
      m_system_resource_summary->Show();
      m_static_text_systemproduction->Show();
    }
  }
}
