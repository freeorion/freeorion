#include "SidePanel.h"

#include <cmath>
#include <numeric>
#include <fstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/irange.hpp>
#include <GG/DynamicGraphic.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/Scroll.h>
#include <GG/StaticGraphic.h>
#include "BuildingsPanel.h"
#include "CUIControls.h"
#include "CUIWnd.h"
#include "FleetWnd.h"
#include "MapWnd.h"
#include "MilitaryPanel.h"
#include "ModeratorActionsWnd.h"
#include "MultiIconValueIndicator.h"
#include "PopulationPanel.h"
#include "ResourcePanel.h"
#include "ShaderProgram.h"
#include "Sound.h"
#include "SpecialsPanel.h"
#include "SystemIcon.h"
#include "SystemResourceSummaryBrowseWnd.h"
#include "TextBrowseWnd.h"
#include "../client/human/GGHumanClientApp.h"
#include "../Empire/Empire.h"
#include "../universe/Building.h"
#include "../universe/BuildingType.h"
#include "../universe/Condition.h"
#include "../universe/Conditions.h"
#include "../universe/Fleet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Ship.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/ValueRef.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../util/Random.h"
#include "../util/ScopedTimer.h"
#include "../util/XMLDoc.h"
#include "../util/ranges.h"


class RotatingPlanetControl;

namespace {

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif

    constexpr int EDGE_PAD(3);
    boost::container::flat_map<std::pair<int, int>, float, std::less<>>         colony_projections; // indexed by {ship_id, planet_id}
    boost::container::flat_map<std::pair<std::string, int>, float, std::less<>> species_colony_projections;

    /** @content_tag{CTRL_ALWAYS_BOMBARD} Select this ship during automatic ship selection for bombard, regardless of any tags **/
    constexpr std::string_view TAG_BOMBARD_ALWAYS = "CTRL_ALWAYS_BOMBARD";
    /** @content_tag{CTRL_BOMBARD_} Prefix tag allowing automatic ship selection for bombard, must post-fix a valid planet tag **/
    constexpr std::string_view TAG_BOMBARD_PREFIX = "CTRL_BOMBARD_";

    void PlaySidePanelOpenSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.map.sidepanel.open.sound.path"), true); }

    struct RotatingPlanetData {
        RotatingPlanetData(const XMLElement& elem) {
            if (elem.Tag() != "RotatingPlanetData")
                throw std::invalid_argument("Attempted to construct a RotatingPlanetData from an XMLElement that had a tag other than \"RotatingPlanetData\"");

            planet_type = PlanetTypeFromString(elem.Attribute("planet_type"));
            filename = elem.Attribute("filename");
            shininess = boost::lexical_cast<float>(elem.Attribute("shininess"));

            // ensure proper bounds
            shininess = std::max(0.0f, std::min(shininess, 128.0f));
        }

        PlanetType  planet_type;    ///< the type of planet for which this data may be used
        std::string filename;       ///< the filename of the image used to texture a rotating image
        float       shininess;      ///< the exponent of specular (shiny) reflection off of the planet; must be in [0.0, 128.0]
    };

    struct PlanetAtmosphereData {
        struct Atmosphere {
            Atmosphere() = default;
            Atmosphere(const XMLElement& elem) {
                if (elem.Tag() != "Atmosphere")
                    throw std::invalid_argument("Attempted to construct an Atmosphere from an XMLElement that had a tag other than \"Atmosphere\"");
                filename = elem.Attribute("filename");
                alpha = boost::lexical_cast<int>(elem.Attribute("alpha"));
                alpha = std::max(0, std::min(alpha, 255));
            }

            std::string filename;
            int         alpha = 128;
        };

        PlanetAtmosphereData() = default;
        PlanetAtmosphereData(const XMLElement& elem) {
            if (elem.Tag() != "PlanetAtmosphereData")
                throw std::invalid_argument("Attempted to construct a PlanetAtmosphereData from an XMLElement that had a tag other than \"PlanetAtmosphereData\"");
            planet_filename = elem.Attribute("planet_filename");
            for (const XMLElement& atmosphere : elem.Child("atmospheres").Children())
                atmospheres.push_back(Atmosphere(atmosphere));
        }

        std::string             planet_filename; ///< the filename of the planet image that this atmosphere image data goes with
        std::vector<Atmosphere> atmospheres;     ///< the filenames of the atmosphere images suitable for use with this planet image
    };

    const auto& GetRotatingPlanetData() {
        static const auto data = []() {
            boost::container::flat_map<PlanetType, std::vector<RotatingPlanetData>> data;
            ScopedTimer timer("GetRotatingPlanetData initialization", true);
            XMLDoc doc;
            try {
                std::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
                doc.ReadDoc(ifs);
                ifs.close();
            } catch (const std::exception& e) {
                ErrorLogger() << "GetRotatingPlanetData: error reading artdir/planets/planets.xml: " << e.what();
            }

            if (doc.root_node.ContainsChild("GLPlanets")) {
                const XMLElement& elem = doc.root_node.Child("GLPlanets");
                for (const XMLElement& planet_definition : elem.Children()) {
                    if (planet_definition.Tag() == "RotatingPlanetData") {
                        RotatingPlanetData current_data{planet_definition};
                        data[current_data.planet_type].emplace_back(std::move(current_data));
                    }
                }
            }
            return data;
        }();
        return data;
    }

    const auto& GetPlanetAtmosphereData() {
        static const auto data = []() {
            XMLDoc doc;
            std::ifstream ifs(ClientUI::ArtDir() / "planets" / "atmospheres.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            boost::container::flat_map<std::string, PlanetAtmosphereData> data;
            for (const XMLElement& atmosphere_definition : doc.root_node.Children()) {
                if (atmosphere_definition.Tag() == "PlanetAtmosphereData") {
                    try {
                        PlanetAtmosphereData current_data{atmosphere_definition};
                        auto filename{current_data.planet_filename}; // copy due to moving from on next line
                        data.emplace(std::move(filename), std::move(current_data));
                    } catch (const std::exception& e) {
                        ErrorLogger() << "GetPlanetAtmosphereData: " << e.what();
                    }
                }
            }
            return data;
        }();
        return data;
    }

    double GetAsteroidsFPS() {
        static const double retval = []() {
            XMLDoc doc;
            std::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("asteroids_fps"))
                return std::max(0.0, std::min(60.0, boost::lexical_cast<double>(
                    doc.root_node.Child("asteroids_fps").Text())));
            else
                return 15.0;
        }();
        return retval;
    }

    double GetRotatingPlanetAmbientIntensity() {
        static const double retval = []() {
            XMLDoc doc;
            std::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLPlanets") &&
                doc.root_node.Child("GLPlanets").ContainsChild("ambient_intensity"))
            {
                return std::max(0.0, std::min(1.0, boost::lexical_cast<double>(
                    doc.root_node.Child("GLPlanets").Child("ambient_intensity").Text())));
            } else {
                return 0.5;
            }
        }();
        return retval;
    }

    double GetRotatingPlanetDiffuseIntensity() {
        static const double retval = []() {
            XMLDoc doc;
            std::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
            doc.ReadDoc(ifs);
            ifs.close();

            if (doc.root_node.ContainsChild("GLPlanets") &&
                doc.root_node.Child("GLPlanets").ContainsChild("diffuse_intensity"))
            {
                return std::max(0.0, std::min(1.0, boost::lexical_cast<double>(
                    doc.root_node.Child("GLPlanets").Child("diffuse_intensity").Text())));
            } else {
                return 0.5;
            }
        }();
        return retval;
    }

    struct PolarCoordinate {
        GLfloat sin = 0;
        GLfloat cos = 0;
    };
    constexpr double PI = 3.1415926535897932384626433;
    constexpr std::size_t sphere_coords_size = 43; // less and there are (more) weird artifacts with larger sizes

#if defined(__cpp_lib_constexpr_cmath)
    using cxsin = std::sin;
    using cxcos = std::cos;
#else
    constexpr std::array<uint8_t, 7> cxsin_factor_increments{3u*2u, 5u*4u, 7u*6u, 9u*8u, 11u*10u, 13u*12u, 15u*14u};
    constexpr GLfloat cxsin(GLfloat a) {
        if (a == 0 || a == PI) return 0.0;
        if (a == PI/2) return 1.0;
        if (a < 0) return -cxsin(-a);
        if (a > 2*PI) return cxsin(a - 2*PI*static_cast<uint64_t>(a / (2*PI)));
        if (a > PI/2) return cxsin(PI - a);

        GLfloat apow = a;
        GLfloat sum = a;
        uint64_t factorial = 1u;
        int8_t signpart = 1;
        const GLfloat a2 = a*a;
        for (uint8_t pow : cxsin_factor_increments) {
            apow *= a2;
            factorial *= pow;
            signpart = -signpart;
            sum += (apow / factorial * signpart);
        }

        return sum;
    }

    constexpr std::array<uint8_t, 7> cxcos_factor_increments{1u*2u, 3u*4u, 5u*6u, 7u*8u, 9u*10u, 11u*12u, 13u*14u};
    constexpr GLfloat cxcos(GLfloat a) {
        if (a == 0) return 1.0;
        if (a == PI/2) return 0.0;
        if (a < 0) return cxcos(-a);
        if (a > 2*PI) return cxcos(a - 2*PI*static_cast<uint64_t>(a / (2*PI)));
        if (a > PI/2) return -cxcos(PI - a);

        GLfloat apow = 1;
        GLfloat sum = 1;
        uint64_t factorial = 1;
        int8_t signpart = 1;
        const GLfloat a2 = a*a;

        for (uint8_t pow : cxcos_factor_increments) {
            apow *= a2;
            factorial *= pow;
            signpart = -signpart;
            sum += (apow / factorial * signpart);
        }

        return sum;
    }

    consteval uint64_t factorial(uint64_t n) { return (n >= 2u) ? (n * factorial(n-1u)) : 1u; }
    static_assert(factorial(15) == 1307674368000 && 1307674368000 < std::numeric_limits<uint64_t>::max());
#endif

    constexpr auto azimuth = []() {
        std::array<PolarCoordinate, sphere_coords_size> azimuth{};

        // calculate azimuth on unit sphere along equator
        for (std::size_t idx = 0u; idx < sphere_coords_size; ++idx) {
            GLfloat phi = 2 * M_PI * idx / (sphere_coords_size - 1);
            azimuth[idx] = {cxsin(phi), cxcos(phi)};
        }
        // Make sure equator is a closed circle
        azimuth.back() = azimuth.front();

        return azimuth;
    }();
    static_assert(azimuth.front().sin == 0);

    constexpr auto elevation = []() {
        std::array<PolarCoordinate, sphere_coords_size> elevation{};

        // calculate elevation on unit sphere along meridian
        for (std::size_t idx = 0u; idx < sphere_coords_size; ++idx) {
            GLfloat theta = M_PI * idx / (sphere_coords_size - 1);
            elevation[idx] = {cxsin(theta), cxcos(theta)};
        }

        // Make sure sphere poles collapse at true zero
        elevation.front() = {0.0f, 1.0f};
        elevation.back() = {0.0f, -1.0f};

        return elevation;
    }();
    static_assert(elevation.front().sin == 0 && elevation.back().cos == -1);

    const std::tuple<const GG::GL3DVertexBuffer&,
                     const GG::GLNormalBuffer&,
                     const GG::GLTexCoordBuffer&>
    SphereVerticesNormalsTexCoords(GLfloat radius) {
        static const GG::GLNormalBuffer norms = []() {
            GG::GLNormalBuffer norms;
            norms.reserve(elevation.size()*azimuth.size()*2);

            for (std::size_t lat_idx = 0u; lat_idx < elevation.size() - 1; ++lat_idx) {
                for (std::size_t long_idx = 0u; long_idx < azimuth.size(); ++long_idx) {
                    norms.store(azimuth[long_idx].sin * elevation[lat_idx+1].sin,
                                azimuth[long_idx].cos * elevation[lat_idx+1].sin,
                                elevation[lat_idx+1].cos);
                    norms.store(azimuth[long_idx].sin * elevation[lat_idx].sin,
                                azimuth[long_idx].cos * elevation[lat_idx].sin,
                                elevation[lat_idx].cos);
                }
            }

            return norms;
        }();

        static const GG::GLTexCoordBuffer tex = []() {
            GG::GLTexCoordBuffer tex;
            tex.reserve(elevation.size()*azimuth.size()*2);

            for (std::size_t lat_idx = 0u; lat_idx < elevation.size() - 1; ++lat_idx) {
                const float lat1 = 1.0f - static_cast<GLfloat>(lat_idx + 1) / (elevation.size() - 1);
                const float lat0 = 1.0f - static_cast<GLfloat>(lat_idx) / (elevation.size() - 1);

                for (std::size_t long_idx = 0u; long_idx < azimuth.size(); ++long_idx) {
                    const float long0 = 1.0f - static_cast<GLfloat>(long_idx) / (azimuth.size() - 1);
                    tex.store(long0, lat1);
                    tex.store(long0, lat0);
                }
            }

            return tex;
        }();

        static std::map<GLfloat, GG::GL3DVertexBuffer> verts;
        {
            auto it = verts.find(radius);
            if (it != verts.end())
                return {it->second, norms, tex};
        }

        auto& new_verts = [radius]() -> const auto& {
            auto& new_verts = verts[radius];
            new_verts.reserve(elevation.size()*azimuth.size()*2);

            for (std::size_t lat_idx = 0u; lat_idx < elevation.size() - 1; ++lat_idx) {
                for (std::size_t long_idx = 0u; long_idx < azimuth.size(); ++long_idx) {
                    new_verts.store(azimuth[long_idx].sin * elevation[lat_idx+1u].sin * radius,
                                    azimuth[long_idx].cos * elevation[lat_idx+1u].sin * radius,
                                    elevation[lat_idx+1u].cos * radius);
                    new_verts.store(azimuth[long_idx].sin * elevation[lat_idx].sin * radius,
                                    azimuth[long_idx].cos * elevation[lat_idx].sin * radius,
                                    elevation[lat_idx].cos * radius);
                }
            }

            return new_verts;
        }();
            
        return {new_verts, norms, tex};
    }

    void RenderSphere(double radius, const GG::Clr ambient, const GG::Clr diffuse,
                      const GG::Clr spec, float shine, GLuint texture_id)
    {
        if (texture_id != 0)
            glBindTexture(GL_TEXTURE_2D, texture_id);

        if (shine != 0) {
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);
            const auto spec_v = spec.ToNormalizedRGBA();
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_v.data());
        }

        const auto ambient_v = ambient.ToNormalizedRGBA();
        const auto diffuse_v = diffuse.ToNormalizedRGBA();
        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_v.data());
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_v.data());

        glColor(GG::CLR_WHITE);

        const auto& [verts, norms, tex] = SphereVerticesNormalsTexCoords(radius);

        verts.activate();
        norms.activate();
        tex.activate();

        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glDrawArrays(GL_QUAD_STRIP, 0, verts.size());

        glPopClientAttrib();
    }

    const GLfloat* GetLightPosition() {
        static const auto retval = []() -> std::array<GLfloat, 4> {
            try {
                XMLDoc doc;
                std::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
                doc.ReadDoc(ifs);
                ifs.close();
                const auto& lpos = doc.root_node.Child("GLPlanets").Child("light_pos");

                return {boost::lexical_cast<GLfloat>(lpos.Child("x").Text()),
                        boost::lexical_cast<GLfloat>(lpos.Child("y").Text()),
                        boost::lexical_cast<GLfloat>(lpos.Child("z").Text()),
                        0.0f};
            } catch (...) {
                return {10.0f, -7.0f, 8.0f, 0.0f};
            }
        }();
        return retval.data();
    }

    // output should be float RGBA ranging 0f to 1.0f
    const auto& GetStarLightColors() {
        static const auto light_colors{[]() -> std::map<StarType, std::array<float, 4>> {
            std::map<StarType, std::array<float, 4>> retval;
            // pre-fill with defaults
            for (StarType i = StarType::STAR_BLUE; i < StarType::NUM_STAR_TYPES; i = StarType(int(i) + 1))
                retval[i] = {1.0f, 1.0f, 1.0f, 1.0f};

            // replace from config file where specified
            try {
                XMLDoc doc;
                std::ifstream ifs(ClientUI::ArtDir() / "planets" / "planets.xml");
                doc.ReadDoc(ifs);
                ifs.close();

                if (!doc.root_node.ContainsChild("GLStars") || doc.root_node.Child("GLStars").Children().empty())
                    throw std::runtime_error("no GLStars in planets.xml");

                for (const XMLElement& star_definition : doc.root_node.Child("GLStars").Children()) {
                    if (!star_definition.HasAttribute("star_type") || !star_definition.HasAttribute("color"))
                        continue;
                    const auto& star_type_name = star_definition.Attribute("star_type");
                    const auto star_type = StarTypeFromString(star_type_name, StarType::INVALID_STAR_TYPE);
                    if (star_type < StarType::STAR_BLUE || star_type >= StarType::NUM_STAR_TYPES)
                        continue;
                    static_assert(StarType::STAR_BLUE == StarType{0} && StarType::INVALID_STAR_TYPE == StarType{-1});

                    std::string_view colour_string = star_definition.Attribute("color");
                    if (colour_string.size() != 6 && colour_string.size() != 8)
                        continue;
                    const GG::Clr color = GG::Clr::HexClr(colour_string);
                    retval.insert_or_assign(star_type, std::array{color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f});
                }
            } catch (...) { }

            return retval;
        }()};

        return light_colors;
    }

    std::array<float, 4> StarLightColour(StarType star_type) {
        static constexpr std::array<float, 4> white{1.0f, 1.0f, 1.0f, 1.0f};
        const auto& colour_map = GetStarLightColors();
        auto it = colour_map.find(star_type);
        if (it != colour_map.end())
            return it->second;
        return white;
    }

    void RenderPlanet(GG::Pt center, int diameter, GLuint texture_id, GLuint overlay_texture_id,
                      double initial_rotation, double RPM, float axial_tilt, float shininess,
                      StarType star_type)
    {
        glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT | GL_SCISSOR_BIT);
        GetApp().Exit2DMode();

        // slide the texture coords to simulate a rotating axis
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(initial_rotation - GetApp().Ticks() / 1000.0 * RPM / 60.0, 0.0, 0.0);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, Value(GetApp().AppWidth()),
                Value(GetApp().AppHeight()), 0.0,
                0.0, Value(GetApp().AppWidth()));

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);
        const auto light_position = GetLightPosition();
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        const auto& colour = StarLightColour(star_type);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, colour.data());
        glLightfv(GL_LIGHT0, GL_SPECULAR, colour.data());
        glEnable(GL_TEXTURE_2D);

        glTranslated(Value(center.x), Value(center.y), GLdouble(-(diameter / 2 + 1)));// relocate to location on screen where planet is to be rendered
        glRotated(95.0, -1.0, 0.0, 0.0);                                              // make the poles upright, instead of head-on (we go a bit more than 90 degrees, to avoid some artifacting caused by the GLU-supplied texture coords)
        glRotated(axial_tilt, 0.0, 1.0, 0.0);                                         // axial tilt

        float intensity = static_cast<float>(GetRotatingPlanetAmbientIntensity());
        GG::Clr ambient = GG::FloatClr(intensity, intensity, intensity, 1.0f);
        intensity = static_cast<float>(GetRotatingPlanetDiffuseIntensity());
        GG::Clr diffuse = GG::FloatClr(intensity, intensity, intensity, 1.0f);

        RenderSphere(diameter / 2.0, ambient, diffuse, GG::CLR_WHITE, shininess, texture_id);

        if (overlay_texture_id != 0) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glCullFace(GL_FRONT);
            glEnable(GL_CULL_FACE);
            RenderSphere(diameter / 2.0 + 0.1, ambient, diffuse, GG::CLR_WHITE, 0.0, overlay_texture_id);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
        }

        glPopAttrib();

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        GetApp().Enter2DMode();
        glPopAttrib();
    }

    int MaxPlanetDiameter()
    { return GetOptionsDB().Get<int>("ui.map.sidepanel.planet.diameter.max"); }

    int PlanetDiameter(PlanetSize size) {
        double scale = 0.0;
        switch (size) {
        case PlanetSize::SZ_TINY      : scale = 1.0/7.0; break;
        case PlanetSize::SZ_SMALL     : scale = 2.0/7.0; break;
        case PlanetSize::SZ_MEDIUM    : scale = 3.0/7.0; break;
        case PlanetSize::SZ_LARGE     : scale = 4.0/7.0; break;
        case PlanetSize::SZ_HUGE      : scale = 5.0/7.0; break;
        case PlanetSize::SZ_GASGIANT  : scale = 7.0/7.0; break;
        case PlanetSize::SZ_ASTEROIDS : scale = 7.0/7.0; break;
        default                       : scale = 3.0/7.0; break;
        }

        int MAX_PLANET_DIAMETER = MaxPlanetDiameter();
        int MIN_PLANET_DIAMETER = GetOptionsDB().Get<int>("ui.map.sidepanel.planet.diameter.min");
        // sanity check
        if (MIN_PLANET_DIAMETER > MAX_PLANET_DIAMETER)
            MIN_PLANET_DIAMETER = MAX_PLANET_DIAMETER;

        return static_cast<int>(MIN_PLANET_DIAMETER + (MAX_PLANET_DIAMETER - MIN_PLANET_DIAMETER) * scale) - 2 * EDGE_PAD;
    }

    /** Adds options related to sidepanel to Options DB. */
    void AddOptions(OptionsDB& db) {
        db.Add("ui.map.sidepanel.planet.diameter.max",      UserStringNop("OPTIONS_DB_UI_SIDEPANEL_PLANET_MAX_DIAMETER"),
               128,                         RangedValidator<int>(16, 512));
        db.Add("ui.map.sidepanel.planet.diameter.min",      UserStringNop("OPTIONS_DB_UI_SIDEPANEL_PLANET_MIN_DIAMETER"),
               24,                          RangedValidator<int>(8,  128));
        db.Add("ui.map.sidepanel.planet.shown",             UserStringNop("OPTIONS_DB_UI_SIDEPANEL_PLANET_SHOWN"),
               true,                        Validator<bool>());
        db.Add("ui.map.sidepanel.planet.scanlane.color",    UserStringNop("OPTIONS_DB_UI_PLANET_FOG_CLR"),
               GG::Clr(0, 0, 0, 128),       Validator<GG::Clr>());
        db.Add("UI.sidepanel-planet-status-icon-size",      UserStringNop("OPTIONS_DB_UI_PLANET_STATUS_ICON_SIZE"),
               32,                          RangedValidator<int>(8, 128));
        db.Add("ui.map.sidepanel.stale-buildings.shown",     UserStringNop("OPTIONS_DB_UI_SHOW_SIDEPANEL_STALE_BUILDING"), false,         Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /** Returns map from planet ID to issued annex orders affecting it. There
      * should be only one ship colonzing each planet for this client. */
    auto PendingAnnexationOrders(const ClientApp& app) {
        std::vector<std::pair<int, int>> retval;

        for (const auto& [order_id, order] : app.Orders()) {
            if (auto annex_order = std::dynamic_pointer_cast<AnnexOrder>(order)) {
                const auto planet_id = annex_order->PlanetID();
                retval.emplace_back(planet_id, order_id);
            }
        }
        // remove any duplicates.  TODO: limit equality test to planet ID?
        //std::sort(retval.begin(), retval.end());
        //auto unique_it = std::unique(retval.begin(), retval.end());
        //retval.resize(static_cast<size_t>(std::distance(retval.begin(), unique_it)));
        return retval;
    }

    double PlanetAnnexationCost(const Planet* planet, const UniverseObject* source_for_empire,
                                const ScriptingContext& context)
    {
        if (!planet)
            return 0.0;
        const auto* species = context.species.GetSpecies(planet->SpeciesName());
        if (!species)
            return 0.0;
        const auto* ac = species->AnnexationCost();
        if (!ac)
            return 0.0;
        if (ac->ConstantExpr())
            return ac->Eval();
        if (!ac->SourceInvariant() && !source_for_empire)
            return 0.0;

        ScriptingContext source_planet_context{context, ScriptingContext::Source{}, source_for_empire};
        source_planet_context.condition_local_candidate = planet;
        if (!source_planet_context.condition_root_candidate)
            source_planet_context.condition_root_candidate = planet;
        return ac->Eval(source_planet_context);
    }

    /** Returns map from planet ID to cost to annex. */
    auto PendingAnnexationPlanetsCosts(const UniverseObject* source_for_empire, const ScriptingContext& context) { // TODO: pass in app?
        std::vector<std::pair<int, double>> retval;

        const auto& app = GetApp();
        const auto client_empire_id = app.EmpireID();
        if (!source_for_empire ||
            source_for_empire->Owner() == ALL_EMPIRES ||
            source_for_empire->Owner() != client_empire_id)
        { return retval; }
        const auto client_empire = context.GetEmpire(client_empire_id);
        if (!client_empire)
            return retval;

        for (const auto& [order_id, order] : app.Orders()) {
            if (auto annex_order = std::dynamic_pointer_cast<AnnexOrder>(order)) {
                const auto planet_id = annex_order->PlanetID();
                const auto* annexed_planet = context.ContextObjects().getRaw<Planet>(planet_id);
                const double cost = PlanetAnnexationCost(annexed_planet, source_for_empire, context);
                retval.emplace_back(annex_order->PlanetID(), cost);
            }
        }
        // remove any duplicates
        std::sort(retval.begin(), retval.end());
        auto unique_it = std::unique(retval.begin(), retval.end(),
                                     [](const auto& lhs, const auto& rhs) { return lhs.first == rhs.first; });
        retval.resize(static_cast<size_t>(std::distance(retval.begin(), unique_it)));
        return retval;
    }

    double PendingAnnexationOrderCost(const UniverseObject* source_for_empire, const ScriptingContext& context) {
        const auto costs = PendingAnnexationPlanetsCosts(source_for_empire, context);
        auto costs_rng = costs | range_values;
        return std::accumulate(costs_rng.begin(), costs_rng.end(), 0.0);
    }

    /** Returns map from planet ID to issued colonize orders affecting it. There
      * should be only one ship colonzing each planet for this client. */
    auto PendingColonizationOrders() { // TODO: pass in app?
        std::map<int, int> retval;
        for (const auto& [order_id, order] : GetApp().Orders()) {
            if (auto col_order = std::dynamic_pointer_cast<ColonizeOrder>(order))
                retval[col_order->PlanetID()] = order_id;
        }
        return retval;
    }

    /** Returns map from planet ID to issued invasion orders affecting it. There
      * may be multiple ships invading a single planet. */
    auto PendingInvadeOrders() { // TODO: pass in app?
        std::map<int, std::set<int>> retval;
        for (const auto& [order_id, order] : GetApp().Orders()) {
            if (auto invade_order = std::dynamic_pointer_cast<InvadeOrder>(order))
                retval[invade_order->PlanetID()].insert(order_id);
        }
        return retval;
    }

    /** Returns map from planet ID to issued bombard orders affecting it. There
      * may be multiple ships bombarding a single planet. */
    auto PendingBombardOrders() { // TODO: pass in app?
        std::map<int, std::set<int>> retval;
        for (const auto& [order_id, base_order] : GetApp().Orders()) {
            if (auto order = std::dynamic_pointer_cast<const BombardOrder>(base_order)) {
                retval[order->PlanetID()].insert(order_id);
            }
        }
        return retval;
    }

    bool ClientPlayerIsModerator()
    { return Networking::is_mod(GetApp()); }
}


/** A single planet's info and controls; several of these may appear at any
  * one time in a SidePanel */
class SidePanel::PlanetPanel : public GG::Control {
public:
    PlanetPanel(GG::X w, int planet_id, StarType star_type) :
        GG::Control(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
        m_planet_id(planet_id),
        m_star_type(star_type)
    {}

    void CompleteConstruction() override;

    bool InWindow(GG::Pt pt) const override;

    int PlanetID() const noexcept { return m_planet_id; }

    void PreRender() override;

    void Render() override;

    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Select(bool selected);

    void Clear();
    void Refresh(ScriptingContext& context, int empire_id); ///< updates panels, shows / hides colonize button, redoes layout of infopanels

    /** Enables, or disables if \a enable is false, issuing orders via this PlanetPanel. */
    void EnableOrderIssuing(bool enable = true);

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
    mutable boost::signals2::signal<void (std::string_view)> FocusChangedSignal;

    mutable boost::signals2::signal<void (int)> BuildingRightClickedSignal;

    /** Emitted when an order button changes state, used to update controls
     *  of panels for other planets in the same system */
    mutable boost::signals2::signal<void (int)> OrderButtonChangedSignal;

private:
    void DoLayout(PlanetType type, PlanetSize size);
    void RefreshPlanetGraphic(PlanetType type, PlanetSize size);
    void SetFocus(std::string focus) const; ///< set the focus of the planet to \a focus
    void ClickAnnex();                      ///< called if annex button is pressed
    void ClickColonize();                   ///< called if colonize button is pressed
    void ClickInvade();                     ///< called if invade button is pressed
    void ClickBombard();                    ///< called if bombard button is pressed

    void FocusDropListSelectionChangedSlot(GG::DropDownList::iterator selected); ///< called when droplist selection changes, emits FocusChangedSignal

    int                                     m_planet_id = INVALID_OBJECT_ID;///< id for the planet with is represented by this planet panel
    std::shared_ptr<GG::TextControl>        m_planet_name;                  ///< planet name;
    std::shared_ptr<GG::Label>              m_env_size;                     ///< indicates size and planet environment rating uncolonized planets;
    std::shared_ptr<GG::Button>             m_annex_button;                 ///< btn which can be pressed to annex this planet;
    std::shared_ptr<GG::Button>             m_colonize_button;              ///< btn which can be pressed to colonize this planet;
    std::shared_ptr<GG::Button>             m_invade_button;                ///< btn which can be pressed to invade this planet;
    std::shared_ptr<GG::Button>             m_bombard_button;               ///< btn which can be pressed to bombard this planet;
    std::shared_ptr<GG::DynamicGraphic>     m_planet_graphic;               ///< image of the planet (can be a frameset); this is now used only for asteroids;
    std::shared_ptr<GG::StaticGraphic>      m_planet_status_graphic;        ///< gives information about the planet status, like supply disconnection
    std::shared_ptr<RotatingPlanetControl>  m_rotating_planet_graphic;      ///< a realtime-rendered planet that rotates, with a textured surface mapped onto it
    std::shared_ptr<GG::DropDownList>       m_focus_drop;                   ///< displays and allows selection of planetary focus;
    std::shared_ptr<PopulationPanel>        m_population_panel;             ///< contains info about population and health
    std::shared_ptr<ResourcePanel>          m_resource_panel;               ///< contains info about resources production and focus selection UI
    std::shared_ptr<MilitaryPanel>          m_military_panel;               ///< contains icons representing military-related meters
    std::shared_ptr<BuildingsPanel>         m_buildings_panel;              ///< contains icons representing buildings
    std::shared_ptr<SpecialsPanel>          m_specials_panel;               ///< contains icons representing specials
    boost::signals2::scoped_connection      m_planet_connection;

    GG::GL2DVertexBuffer                    m_verts;
    GG::GLRGBAColorBuffer                   m_colours;
    static constexpr std::size_t            s_background_start_idx = 0;
    std::size_t                             m_main_border_sz = 0;
    std::size_t                             m_disable_greyover_start_idx = 0;
    std::size_t                             m_border_line_start_idx = 0;
    std::size_t                             m_title_box_start_idx = 0;
    static constexpr std::size_t            s_title_box_sz = 4u;

    GG::Clr                                 m_empire_colour = GG::CLR_ZERO; ///< colour to use for empire-specific highlighting.  set based on ownership of planet.
    StarType                                m_star_type = StarType::INVALID_STAR_TYPE;
    bool                                    m_selected = false;             ///< is this planet panel selected
    bool                                    m_order_issuing_enabled = true; ///< can orders be issues via this planet panel?
};

/** Container class that holds PlanetPanels.  Creates and destroys PlanetPanel
  * as necessary, and does layout of them after creation and in response to
  * scrolling through them by the user. */
class SidePanel::PlanetPanelContainer : public GG::Wnd {
public:
    PlanetPanelContainer();

    bool InWindow(GG::Pt pt) const override;

    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    int         SelectedPlanetID() const noexcept { return m_selected_planet_id; }
    const auto& SelectionCandidates() const noexcept { return m_candidate_ids; }
    int         ScrollPosition() const;

    auto        NumPanels() const noexcept { return m_planet_panels.size(); }

    void LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void PreRender() override;

    void Clear();
    void SetPlanets(const std::vector<int>& planet_ids, StarType star_type, 
                    ScriptingContext& context, int empire_id);
    void SelectPlanet(int planet_id); //!< programatically selects a planet with id \a planet_id
    void SetValidSelectionPredicate(std::function<bool(const UniverseObject*)> pred);
    void ClearValidSelectionPedicate();
    void ScrollTo(int pos);

    /** Updates data displayed in info panels and redoes layout
     *  @param[in] excluded_planet_id Excludes panels with this planet id
     *  @param[in] require_prerender Set panels to RequirePreRender */
    void RefreshAllPlanetPanels(ScriptingContext& context,
                                int empire_id,
                                int excluded_planet_id = INVALID_OBJECT_ID,
                                bool require_prerender = false);

    virtual void ShowScrollbar();
    virtual void HideScrollbar();

    /** Enables, or disables if \a enable is false, issuing orders via the
      * PlanetPanels in this PlanetPanelContainer. */
    void EnableOrderIssuing(bool enable = true);

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
    int                                         m_selected_planet_id = INVALID_OBJECT_ID;
    std::set<int>                               m_candidate_ids;
    std::function<bool(const UniverseObject*)>  m_valid_selection_predicate;
    std::shared_ptr<GG::Scroll>                 m_vscroll; ///< the vertical scroll (for viewing all the planet panes);
    bool                                        m_ignore_recursive_resize = false;
};

class RotatingPlanetControl : public GG::Control {
public:
    RotatingPlanetControl(GG::X x, GG::Y y, int planet_id, StarType star_type) :
        GG::Control(x, y, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
        m_planet_id(planet_id),
        m_initial_rotation(fmod(planet_id / 7.352535, 1.0)), // arbitrary scale number applied to id to give consistent by varied angles
        m_star_type(star_type)
    {
        Refresh(GetApp().GetContext(), GetApp().EmpireID());
    }

    void Render() override {
        const auto ul = UpperLeft();
        const auto lr = LowerRight();
        // render rotating base planet texture
        RenderPlanet(ul + Size() / 2, Value(Width()),
                     m_surface_texture ? m_surface_texture->OpenGLId() : 0,
                     m_overlay_texture ? m_overlay_texture->OpenGLId() : 0,
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
        if ((m_visibility <= Visibility::VIS_BASIC_VISIBILITY) && GetOptionsDB().Get<bool>("ui.map.scanlines.shown")) {
            s_scanline_shader.SetColor(GetOptionsDB().Get<GG::Clr>("ui.map.sidepanel.planet.scanlane.color"));
            s_scanline_shader.RenderCircle(ul, lr);
        }
    }

    void Refresh(const ScriptingContext& context, int client_empire_id) {
        ScopedTimer timer("RotatingPlanetControl::Refresh", true);

        auto planet = context.ContextObjects().get<Planet>(m_planet_id);
        if (!planet) return;

        // these values ensure that wierd GLUT-sphere artifacts do not show themselves
        const double period = static_cast<double>(planet->RotationalPeriod());// gives about one rpm for a 1 "Day" rotational period
        m_rpm = (std::abs(period) < 0.1) ? 10.0 : (1.0 / period); // prevent divide by zero or extremely fast rotations
        m_diameter = PlanetDiameter(planet->Size());
        m_axial_tilt = std::max(-30.0, std::min(static_cast<double>(planet->AxialTilt()), 60.0));
        m_visibility = (client_empire_id == ALL_EMPIRES) ? 
            Visibility::VIS_FULL_VISIBILITY : context.ContextVis(m_planet_id, client_empire_id);

        const auto& planet_data = GetRotatingPlanetData();

        const auto planet_data_it = planet_data.find(planet->Type());
        if (planet_data_it != planet_data.end() && planet_data_it->second.size() > 0) {
            const auto num_planets_of_type = planet_data_it->second.size();
            const auto hash_value = static_cast<unsigned int>(m_planet_id);
            const RotatingPlanetData& rpd = planet_data_it->second[hash_value % num_planets_of_type];
            m_surface_texture = GetApp().GetUI().GetTexture(ClientUI::ArtDir() / rpd.filename, true);
            m_shininess = rpd.shininess;

            const auto& atmosphere_data = GetPlanetAtmosphereData();
            const auto it = atmosphere_data.find(rpd.filename);
            if (it != atmosphere_data.end()) {
                const auto& atmosphere = it->second.atmospheres[static_cast<std::size_t>(RandInt(0, it->second.atmospheres.size() - 1))];
                m_atmosphere_texture = GetApp().GetUI().GetTexture(ClientUI::ArtDir() / atmosphere.filename, true);
                m_atmosphere_alpha = atmosphere.alpha;
                m_atmosphere_planet_rect = GG::Rect(GG::X1, GG::Y1, m_atmosphere_texture->DefaultWidth() - 4, m_atmosphere_texture->DefaultHeight() - 4);
            }
        }

        if (!planet->SurfaceTexture().empty())
            m_overlay_texture = GetApp().GetUI().GetTexture(ClientUI::ArtDir() / planet->SurfaceTexture(), true);

        Resize(GG::Pt(GG::X(PlanetDiameter(planet->Size())), GG::Y(PlanetDiameter(planet->Size()))));
    }

private:
    int                             m_planet_id = INVALID_OBJECT_ID;
    double                          m_rpm = 1.0;
    int                             m_diameter = 1;
    double                          m_axial_tilt = 0.0;
    Visibility                      m_visibility = Visibility::VIS_BASIC_VISIBILITY;
    std::shared_ptr<GG::Texture>    m_surface_texture;
    double                          m_shininess = 0.0;
    std::shared_ptr<GG::Texture>    m_overlay_texture;
    std::shared_ptr<GG::Texture>    m_atmosphere_texture;
    int                             m_atmosphere_alpha = 1;
    GG::Rect                        m_atmosphere_planet_rect;
    double                          m_initial_rotation = 0.0;
    StarType                        m_star_type = StarType::STAR_WHITE;

    static ScanlineRenderer         s_scanline_shader;
};

ScanlineRenderer RotatingPlanetControl::s_scanline_shader;


namespace {
    int SystemNameFontSize()
    { return ClientUI::Pts()*3/2; }

    class SystemRow : public GG::ListBox::Row {
    public:
        SystemRow(int system_id, GG::Y h) :
            GG::ListBox::Row(GG::X1, h),
            m_system_id(system_id)
        {
            SetDragDropDataType("SystemRow");
            SetName("SystemRow");
            RequirePreRender();
        }

        void Init() {
            push_back(GG::Wnd::Create<OwnerColoredSystemName>(m_system_id, SystemNameFontSize(), false));
            SetColAlignment(0, GG::ALIGN_CENTER);
            GetLayout()->PreRender();
        }

        void PreRender() override {
            // If there is no control add it.
            if (GetLayout()->Children().empty())
                Init();

            GG::ListBox::Row::PreRender();
        }

        [[nodiscard]] int SystemID() const noexcept { return m_system_id; }

        [[nodiscard]] SortKeyType SortKey(std::size_t) const noexcept override { return EMPTY_STRING; }

    private:
        int m_system_id;
    };
}
/** A class to display all of the system names*/
class SidePanel::SystemNameDropDownList : public CUIDropDownList {
    public:
    explicit SystemNameDropDownList(std::size_t num_shown_elements) :
        CUIDropDownList(num_shown_elements)
    {}

    /** Enable/disable the ability to give orders that modify the system name.*/
    void EnableOrderIssuing(bool enable = true) noexcept { m_order_issuing_enabled = enable; }

    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override {
        if (CurrentItem() == end())
            return;

        SystemRow* system_row(dynamic_cast<SystemRow*>(CurrentItem()->get()));
        if (!system_row)
            return;

        auto& app = GetApp();
        auto& context = app.GetContext();
        const auto system = context.ContextObjects().get<const System>(system_row->SystemID());
        if (!system)
            return;


        auto popup = GG::Wnd::Create<GG::PopupMenu>(
            pt.x, pt.y, app.GetUI().GetFont(), ClientUI::TextColor(),
            ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

        auto rename_action = [this, system_id{system_row->SystemID()}]() {
            auto& app = GetApp();
            auto& context = app.GetContext();
            const auto system = context.ContextObjects().get<const System>(system_id);
            if (!system)
                return;

            auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(GG::X(350), UserString("SP_ENTER_NEW_SYSTEM_NAME"),
                                                        system->Name());
            edit_wnd->Run();

            if (!m_order_issuing_enabled)
                return;

            if (!RenameOrder::Check(app.EmpireID(), system->ID(), edit_wnd->Result(), context))
                return;

            app.Orders().IssueOrder<RenameOrder>(context, app.EmpireID(), system->ID(), edit_wnd->Result());

            if (SidePanel* side_panel = dynamic_cast<SidePanel*>(Parent().get()))
                side_panel->Refresh();
        };

        if (m_order_issuing_enabled && system->OwnedBy(app.EmpireID()))
            popup->AddMenuItem(GG::MenuItem(UserString("SP_RENAME_SYSTEM"), false, false, rename_action));

        popup->Run();
    }

    bool m_order_issuing_enabled = true;
};

namespace {
    const std::vector<std::shared_ptr<GG::Texture>>& GetAsteroidTextures() {
        static std::vector<std::shared_ptr<GG::Texture>> retval;
        if (retval.empty()) {
            retval = GetApp().GetUI().GetPrefixedTextures(
                ClientUI::ArtDir() / "planets" / "asteroids", "asteroids1_", false);
        }
        return retval;
    }

    const std::string& GetPlanetSizeName(const Planet* planet) {
        if (!planet || planet->Size() == PlanetSize::SZ_ASTEROIDS || planet->Size() == PlanetSize::SZ_GASGIANT)
            return EMPTY_STRING;
        return UserString(to_string(planet->Size()));
    }

    auto& GetPlanetTypeName(const Planet* planet)
    { return UserString(to_string(planet->Type())); }

    auto& GetPlanetEnvironmentName(const Planet* planet, std::string_view species_name,
                                   const SpeciesManager& species)
    { return UserString(to_string(planet->EnvironmentForSpecies(species, species_name))); }

    const std::string& GetStarTypeName(const System* system) {
        if (!system || system->GetStarType() == StarType::INVALID_STAR_TYPE)
            return EMPTY_STRING;
        return UserString(to_string(system->GetStarType()));
    }

    const GG::Y PLANET_PANEL_TOP = GG::Y(140);
}

////////////////////////////////////////////////
// SidePanel::PlanetPanel
////////////////////////////////////////////////
namespace {
    constexpr bool SHOW_ALL_PLANET_PANELS = false;   //!< toggles whether to show population, resource, military and building info panels on planet panels that this player doesn't control

    /** How big we want meter icons with respect to the current UI font size.
      * Meters should scale along font size, but not below the size for the
      * default 12 points font. */
    GG::Pt MeterIconSize() {
        const int icon_size = std::max(ClientUI::Pts(), 12) * 4/3;
        return GG::Pt(GG::X(icon_size), GG::Y(icon_size));
    }
}

void SidePanel::PlanetPanel::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    SetName(UserString("PLANET_PANEL"));

    auto& app = GetApp();
    auto& ui = app.GetUI();
    const ScriptingContext& context = app.GetContext();
    auto* planet = context.ContextObjects().getRaw<const Planet>(m_planet_id);
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
    for (const auto& [loop_empire_id, loop_empire] : context.Empires()) {
        if (!loop_empire) {
            ErrorLogger() << "PlanetPanel::PlanetPanel got null empire pointer for id " << loop_empire_id;
            continue;
        }
        if (loop_empire->CapitalID() == m_planet_id) {
            capital = true;
            break;
        }
    }

    // determine font based on whether planet is a capital...
    auto font{capital ? ui.GetBoldFont(ClientUI::Pts()*4/3) : ui.GetFont(ClientUI::Pts()*4/3)};

    GG::X panel_width = Width() - MaxPlanetDiameter() - 2*EDGE_PAD;

    // create planet name control
    m_planet_name = GG::Wnd::Create<GG::TextControl>(
        GG::X0, GG::Y0, GG::X1, GG::Y1, " ", font, ClientUI::TextColor());
    m_planet_name->MoveTo(GG::Pt(GG::X(MaxPlanetDiameter() + EDGE_PAD), GG::Y0));
    m_planet_name->Resize(m_planet_name->MinUsableSize());
    AttachChild(m_planet_name);

    namespace ph = boost::placeholders;

    // focus-selection droplist
    m_focus_drop = GG::Wnd::Create<CUIDropDownList>(6);
    AttachChild(m_focus_drop);
    m_focus_drop->SelChangedSignal.connect(boost::bind(
        &SidePanel::PlanetPanel::FocusDropListSelectionChangedSlot, this, ph::_1));
    m_focus_drop->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_focus_drop->SetStyle(GG::LIST_NOSORT | GG::LIST_SINGLESEL);
    m_focus_drop->ManuallyManageColProps();
    m_focus_drop->SetNumCols(2);
    m_focus_drop->SetColWidth(0, m_focus_drop->DisplayedRowWidth());
    m_focus_drop->SetColStretch(0, 0.0);
    m_focus_drop->SetColStretch(1, 1.0);
    m_focus_drop->SetOnlyMouseScrollWhenDropped(true);

    this->FocusChangedSignal.connect([this](std::string_view sv) { SetFocus(std::string{sv}); });

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

    m_env_size = GG::Wnd::Create<CUILabel>("", ui, GG::FORMAT_NOWRAP);
    m_env_size->MoveTo(GG::Pt(GG::X(MaxPlanetDiameter()), GG::Y0));
    AttachChild(m_env_size);

    m_annex_button = Wnd::Create<CUIButton>(UserString("PL_ANNEX"));
    m_annex_button ->LeftClickedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::ClickAnnex, this));

    m_colonize_button = Wnd::Create<CUIButton>(UserString("PL_COLONIZE"));
    m_colonize_button->LeftClickedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::ClickColonize, this));

    m_invade_button = Wnd::Create<CUIButton>(UserString("PL_INVADE"));
    m_invade_button->LeftClickedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::ClickInvade, this));

    m_bombard_button = Wnd::Create<CUIButton>(UserString("PL_BOMBARD"));
    m_bombard_button->LeftClickedSignal.connect(
        boost::bind(&SidePanel::PlanetPanel::ClickBombard, this));

    SetChildClippingMode(ChildClippingMode::ClipToWindow);

    ScriptingContext planet_context(context, ScriptingContext::Source{}, planet);
    Refresh(planet_context, app.EmpireID());

    RequirePreRender();

    m_verts.reserve(3*8+4);
    m_colours.reserve(3*8+4);
}

void SidePanel::PlanetPanel::DoLayout(PlanetType type, PlanetSize size) {
    GG::X left = GG::X0 + MaxPlanetDiameter() + EDGE_PAD;
    GG::X right = left + Width() - MaxPlanetDiameter() - 2*EDGE_PAD;
    GG::Y y = GG::Y0;

    if (m_planet_name) {
        m_planet_name->MoveTo(GG::Pt(left, y));
        y += m_planet_name->Height(); // no interpanel space needed here, I declare arbitrarily
    }

    if (m_specials_panel) {
        m_specials_panel->SizeMove(GG::Pt(left, y), GG::Pt(right, y + m_specials_panel->Height())); // assumed to always be this Wnd's child
        y += m_specials_panel->Height() + EDGE_PAD;
    }

    if (m_env_size && m_env_size->Parent().get() == this) {
        m_env_size->MoveTo(GG::Pt(left, y));
        y += m_env_size->Height() + EDGE_PAD;
    }

    if (m_annex_button && m_annex_button->Parent().get() == this) {
        m_annex_button->MoveTo(GG::Pt(left, y));
        m_annex_button->Resize(GG::Pt(GG::X(ClientUI::Pts()*15), m_annex_button->MinUsableSize().y));
        y += m_annex_button->Height() + EDGE_PAD;
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


    RefreshPlanetGraphic(type, size);

    GG::Y min_height = m_planet_graphic ? 
        m_planet_graphic->Height() : GG::Y{MaxPlanetDiameter()};

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

void SidePanel::PlanetPanel::RefreshPlanetGraphic(PlanetType type, PlanetSize size) {
    if (!GetOptionsDB().Get<bool>("ui.map.sidepanel.planet.shown"))
        return;

    DetachChildAndReset(m_planet_graphic);
    DetachChildAndReset(m_rotating_planet_graphic);

    if (type == PlanetType::PT_ASTEROIDS) {
        const auto& textures = GetAsteroidTextures();
        if (textures.empty())
            return;
        GG::X texture_width = textures.front()->DefaultWidth();
        GG::Y texture_height = textures.front()->DefaultHeight();
        GG::Pt planet_image_pos(GG::X(MaxPlanetDiameter() / 2 - texture_width / 2 + 3), GG::Y0);

        m_planet_graphic = GG::Wnd::Create<GG::DynamicGraphic>(
            planet_image_pos.x, planet_image_pos.y, texture_width, texture_height,
            true, texture_width, texture_height, 0, textures,
            GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        m_planet_graphic->SetFPS(GetAsteroidsFPS());
        m_planet_graphic->SetFrameIndex(static_cast<std::size_t>(RandInt(0, textures.size() - 1)));
        AttachChild(m_planet_graphic);
        m_planet_graphic->Play();

    } else if (type < PlanetType::NUM_PLANET_TYPES) {
        const int planet_image_sz = PlanetDiameter(size);
        GG::Pt planet_image_pos(GG::X(MaxPlanetDiameter() / 2 - planet_image_sz / 2 + 3),
                                GG::Y(MaxPlanetDiameter() / 2 - planet_image_sz / 2));
        m_rotating_planet_graphic =
            GG::Wnd::Create<RotatingPlanetControl>(planet_image_pos.x, planet_image_pos.y, m_planet_id, m_star_type);
        AttachChild(m_rotating_planet_graphic);
    }
}

namespace {
    bool IsAvailable(const Ship* ship, int system_id, int empire_id, const ScriptingContext& context) {
        if (!ship)
            return false;
        const Universe& universe = context.ContextUniverse();
        const ObjectMap& objects = context.ContextObjects();
        const auto* fleet = objects.getRaw<Fleet>(ship->FleetID());
        return fleet &&
            ship->SystemID() == system_id &&
            ship->OwnedBy(empire_id) &&
            ship->GetVisibility(empire_id, universe) >= Visibility::VIS_PARTIAL_VISIBILITY &&
            ship->OrderedScrapped() == false &&
            fleet->FinalDestinationID() == INVALID_OBJECT_ID;
    }

    bool AvailableToColonize(const Ship* ship, int system_id, int empire_id,
                             const ScriptingContext& context)
    {
        if (!ship)
            return false;
        const Universe& u = context.ContextUniverse();
        const ObjectMap& o = context.ContextObjects();
        const SpeciesManager& sm = context.species;
        return o.getRaw<Fleet>(ship->FleetID()) &&
            IsAvailable(ship, system_id, empire_id, context) &&
            ship->CanColonize(u, sm) &&
            ship->OrderedColonizePlanet() == INVALID_OBJECT_ID;
    };

    bool AvailableToInvade(const Ship* ship, int system_id, int empire_id,
                           const ScriptingContext& context)
    {
        if (!ship)
            return false;
        const Universe& u = context.ContextUniverse();
        const ObjectMap& o = context.ContextObjects();
        return o.getRaw<Fleet>(ship->FleetID()) &&
            IsAvailable(ship, system_id, empire_id, context) &&
            ship->HasTroops(u) &&
            ship->OrderedInvadePlanet() == INVALID_OBJECT_ID;
    };

    bool AvailableToBombard(const Ship* ship, int system_id, int empire_id,
                            const ScriptingContext& context)
    {
        if (!ship)
            return false;
        const Universe& u = context.ContextUniverse();
        const ObjectMap& o = context.ContextObjects();
        return o.getRaw<Fleet>(ship->FleetID()) &&
            IsAvailable(ship, system_id, empire_id, context) &&
            ship->CanBombard(u) &&
            ship->OrderedBombardPlanet() == INVALID_OBJECT_ID;
    };

    /** Content tags that note if a Ship should be auto-selected for bombarding a Planet.
     *  These tags are determined from the TAG_BOMBARD_PREFIX tags of @a ship and potentially match those of a Planet.
     *  For example, a planet might have a "ROBOTIC" tag because its species does, and if the ship has
     *  the corresponding "CTRL_BOMBARD_ROBOTIC" tag, the ship would be auto-selected to bombard that planet.
     *  If the Ship contains the content tag defined in TAG_BOMBARD_ALWAYS, only that tag will be returned.
     */
    std::vector<std::string_view> BombardTagsForShip(const Ship* ship, const ScriptingContext& context) {
        std::vector<std::string_view> retval;
        if (!ship)
            return retval;
        if (ship->HasTag(TAG_BOMBARD_ALWAYS, context)) {
            retval.push_back(TAG_BOMBARD_ALWAYS);
            return retval;
        }
        auto tags{ship->Tags(context)};

        retval.reserve(tags.size());
        std::copy_if(tags.first.begin(), tags.first.end(), std::back_inserter(retval),
                     [len{TAG_BOMBARD_PREFIX.length()}](std::string_view t) { return t.substr(0, len) == TAG_BOMBARD_PREFIX; });
        std::copy_if(tags.second.begin(), tags.second.end(), std::back_inserter(retval),
                     [len{TAG_BOMBARD_PREFIX.length()}](std::string_view t) { return t.substr(0, len) == TAG_BOMBARD_PREFIX; });
        std::for_each(retval.begin(), retval.end(),
                      [len{TAG_BOMBARD_PREFIX.length()}](auto& tag) { tag = tag.substr(len); });
        return retval;
    }

    bool CanColonizePlanetType(const Ship* ship, PlanetType planet_type, const ScriptingContext context) {
        if (!ship || planet_type == PlanetType::INVALID_PLANET_TYPE)
            return false;

        float colony_ship_capacity = 0.0f;
        const Universe& universe = context.ContextUniverse();
        const SpeciesManager& sm = context.species;

        const auto design = universe.GetShipDesign(ship->DesignID());
        if (design) {
            colony_ship_capacity = design->ColonyCapacity();
            if (colony_ship_capacity == 0.0f && ship->CanColonize(universe, sm))
                return true;    // outpost ship; planet type doesn't matter as there is no species to check against
        }

        if (const Species* colony_ship_species = sm.GetSpecies(ship->SpeciesName())) {
            PlanetEnvironment planet_env_for_colony_species =
                colony_ship_species->GetPlanetEnvironment(planet_type);

            // One-Click Colonize planets that are colonizable (even if they are
            // not hospitable), and One-Click Outpost planets that are not
            // colonizable.
            if (colony_ship_capacity > 0.0f)
                return planet_env_for_colony_species >= PlanetEnvironment::PE_HOSTILE &&
                    planet_env_for_colony_species <= PlanetEnvironment::PE_GOOD;
            else
                return planet_env_for_colony_species < PlanetEnvironment::PE_HOSTILE ||
                    planet_env_for_colony_species > PlanetEnvironment::PE_GOOD;
        }
        return false;
    }

    auto ValidSelectedInvasionShips(int system_id, const ScriptingContext& context) {
        std::vector<const Ship*> retval;

        // if not looking in a valid system, no valid invasion ship can be available
        if (system_id == INVALID_OBJECT_ID)
            return retval;

        const Universe& u = context.ContextUniverse();
        const ObjectMap& o = context.ContextObjects();
        const auto client_empire_id = GetApp().EmpireID();
        const auto selected_ids = FleetUIManager::GetFleetUIManager().SelectedShipIDs();
        retval.reserve(selected_ids.size());

        // is there a valid single selected ship in the active FleetWnd?
        for (const auto* ship : o.findRaw<Ship>(selected_ids)) {
            if (ship &&
                ship->SystemID() == system_id &&
                ship->HasTroops(u) &&
                ship->OwnedBy(client_empire_id))
            { retval.push_back(ship); }
        }

        return retval;
    }

    auto ValidSelectedBombardShips(int system_id, const ScriptingContext& context) {
        std::vector<const Ship*> retval;

        // if not looking in a valid system, no valid bombard ship can be available
        if (system_id == INVALID_OBJECT_ID)
            return retval;

        const Universe& u = context.ContextUniverse();
        const ObjectMap& o = context.ContextObjects();
        const auto client_empire_id = GetApp().EmpireID();
        const auto selected_ids = FleetUIManager::GetFleetUIManager().SelectedShipIDs();
        retval.reserve(selected_ids.size());

        // is there a valid single selected ship in the active FleetWnd?
        for (const auto* ship : o.findRaw<Ship>(selected_ids)) {
            if (ship &&
                ship->SystemID() != system_id &&
                ship->CanBombard(u) &&
                ship->OwnedBy(client_empire_id))
            { retval.push_back(ship); }
        }

        return retval;
    }

    const Ship* ValidSelectedColonyShip(int system_id, const ScriptingContext& context) {
        // if not looking in a valid system, no valid colony ship can be available
        if (system_id == INVALID_OBJECT_ID)
            return nullptr;

        const SpeciesManager& sm = context.species;
        const Universe& u = context.ContextUniverse();
        const ObjectMap& o = context.ContextObjects();

        // is there a valid selected ship in the active FleetWnd?
        for (const auto* ship : o.findRaw<Ship>(FleetUIManager::GetFleetUIManager().SelectedShipIDs())) {
            if (ship && 
                ship->SystemID() == system_id &&
                ship->CanColonize(u, sm) &&
                ship->OwnedBy(GetApp().EmpireID()))
            { return ship; }
        }
        return nullptr;
    }

    int AutomaticallyChosenColonyShip(int target_planet_id, ScriptingContext& context) {
        const int empire_id = GetApp().EmpireID();
        if (empire_id == ALL_EMPIRES)
            return INVALID_OBJECT_ID;
        const Universe& u = context.ContextUniverse();
        ObjectMap& o = context.ContextObjects(); // mutable to allow sims of planet pop after colonization

        if (u.GetObjectVisibilityByEmpire(target_planet_id, empire_id) < Visibility::VIS_PARTIAL_VISIBILITY)
            return INVALID_OBJECT_ID;
        auto* target_planet = o.getRaw<Planet>(target_planet_id); // mutable to allow sims of pop after colonization
        if (!target_planet)
            return INVALID_OBJECT_ID;
        const int system_id = target_planet->SystemID();
        const auto* system = o.getRaw<const System>(system_id);
        if (!system)
            return INVALID_OBJECT_ID;
        // is planet a valid colonization target?
        if (target_planet->GetMeter(MeterType::METER_POPULATION)->Initial() > 0.0f ||
            (!target_planet->Unowned() && !target_planet->OwnedBy(empire_id)))
        { return INVALID_OBJECT_ID; }

        const PlanetType target_planet_type = target_planet->Type();

        // TODO: return vector of ships from system ids using new Objects().findRaw<Ship>(system->FindObjectIDs())
        auto ships = o.findRaw<const Ship>(system->ShipIDs());
        std::vector<const Ship*> capable_and_available_colony_ships;
        capable_and_available_colony_ships.reserve(ships.size());

        // get all ships that can colonize and that are free to do so in the
        // specified planet'ssystem and that can colonize the requested planet
        for (const auto* ship : ships) {
            if (!AvailableToColonize(ship, system_id, empire_id, context))
                continue;
            if (!CanColonizePlanetType(ship, target_planet_type, context))
                continue;
            capable_and_available_colony_ships.emplace_back(ship);
        }

        // simple case early exits: no ships, or just one capable ship
        if (capable_and_available_colony_ships.empty())
            return INVALID_OBJECT_ID;
        if (capable_and_available_colony_ships.size() == 1)
            return capable_and_available_colony_ships.front()->ID();

        // have more than one ship capable and available to colonize.
        // pick the "best" one.
        const auto& orig_species = target_planet->SpeciesName(); //should be just ""
        const int orig_owner = target_planet->Owner();
        const float orig_initial_target_pop = target_planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Initial();
        int best_ship = INVALID_OBJECT_ID;
        float best_capacity = -999;
        bool changed_planet = false;

        Universe& universe = context.ContextUniverse();

        universe.InhibitUniverseObjectSignals(true);
        for (const auto* ship : capable_and_available_colony_ships) {
            // TODO: Also tabulate estimates stabilities of potential colonies
            if (!ship)
                continue;
            const int ship_id = ship->ID();
            float planet_capacity = -999.9f;

            const auto pair_it = colony_projections.find({ship_id, target_planet_id});
            if (pair_it != colony_projections.end()) {
                planet_capacity = pair_it->second;

            } else {
                float colony_ship_capacity = 0.0f;
                const ShipDesign* design = universe.GetShipDesign(ship->DesignID());
                if (!design)
                    continue;
                colony_ship_capacity = design->ColonyCapacity();
                if (colony_ship_capacity > 0.0f) {
                    auto& ship_species_name = ship->SpeciesName();
                    const auto spec_pair_it = species_colony_projections.find({ship_species_name, target_planet_id});
                    if (spec_pair_it != species_colony_projections.end()) {
                        planet_capacity = spec_pair_it->second;
                    } else {
                        const Species* species = context.species.GetSpecies(ship_species_name);
                        PlanetEnvironment planet_environment = PlanetEnvironment::PE_UNINHABITABLE;
                        if (species)
                            planet_environment = species->GetPlanetEnvironment(target_planet->Type());
                        if (planet_environment != PlanetEnvironment::PE_UNINHABITABLE) {
                            changed_planet = true;
                            target_planet->SetOwner(empire_id);
                            target_planet->SetSpecies(ship_species_name, context.current_turn, context.species);
                            target_planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Reset();

                            // temporary meter update with currently set species
                            universe.UpdateMeterEstimates(target_planet_id, context);
                            planet_capacity = target_planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Current();  // want value after meter update, so check current, not initial value
                        }
                        species_colony_projections[{ship_species_name, target_planet_id}] = planet_capacity;
                    }
                } else {
                    planet_capacity = 0.0f;
                }
                colony_projections[{ship_id, target_planet_id}] = planet_capacity;
            }
            if (planet_capacity > best_capacity) {
                best_capacity = planet_capacity;
                best_ship = ship_id;
            }
        }
        if (changed_planet) {
            target_planet->SetOwner(orig_owner);
            target_planet->SetSpecies(orig_species, context.current_turn, context.species);
            target_planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Set(orig_initial_target_pop,
                                                                             orig_initial_target_pop);
            universe.UpdateMeterEstimates(target_planet_id, context);
        }
        universe.InhibitUniverseObjectSignals(false);

        return best_ship;
    }

    auto AutomaticallyChosenInvasionShips(int target_planet_id, const ScriptingContext& context) {
        std::vector<const Ship*> retval;

        const int empire_id = GetApp().EmpireID();
        if (empire_id == ALL_EMPIRES)
            return retval;

        const Universe& u = context.ContextUniverse();
        const ObjectMap& o = context.ContextObjects();

        const auto* target_planet = o.getRaw<Planet>(target_planet_id);
        if (!target_planet)
            return retval;
        const int system_id = target_planet->SystemID();
        const auto* system = o.getRaw<System>(system_id);
        if (!system)
            return retval;

        //Can't invade owned-by-self planets; early exit
        if (target_planet->OwnedBy(empire_id))
            return retval;


        // get "just enough" ships that can invade and that are free to do so
        const double defending_troops = target_planet->GetMeter(MeterType::METER_TROOPS)->Initial();

        retval.reserve(10); // guesstimate
        double invasion_troops = 0;
        for (const auto* ship : o.allRaw<Ship>()) {
            if (!AvailableToInvade(ship, system_id, empire_id, context))
                continue;

            invasion_troops += ship->TroopCapacity(u);

            retval.push_back(ship);

            if (invasion_troops > defending_troops)
                break;
        }

        return retval;
    }

    /** Returns valid Ship%s capable of bombarding a given Planet.
      * @param target_planet_id ID of Planet to potentially bombard */
    auto AutomaticallyChosenBombardShips(int target_planet_id, const ScriptingContext& context) {
        std::vector<const Ship*> retval;

        const int empire_id = GetApp().EmpireID();
        if (empire_id == ALL_EMPIRES)
            return retval;

        const ObjectMap& o = context.ContextObjects();

        const auto* target_planet = o.getRaw<Planet>(target_planet_id);
        if (!target_planet)
            return retval;
        const int system_id = target_planet->SystemID();
        const auto* system = o.getRaw<System>(system_id);
        if (!system)
            return retval;

        // Can't bombard owned-by-self planets; early exit
        if (target_planet->OwnedBy(empire_id))
            return retval;

        retval.reserve(10); // guesstimate
        for (const auto* ship : o.allRaw<Ship>()) {
            // owned ship is capable of bombarding a planet in this system
            if (!AvailableToBombard(ship, system_id, empire_id, context))
                continue;

            // Select ship if the planet contains a content tag specified by the ship,
            // or ship is tagged to always be selected
            for (std::string_view tag : BombardTagsForShip(ship, context)) {
                if ((tag == TAG_BOMBARD_ALWAYS) || (target_planet->HasTag(tag, context))) {
                    retval.push_back(ship);
                    break;
                }
            }
        }

        return retval;
    }
}

void SidePanel::PlanetPanel::Clear() {
    m_planet_connection.disconnect();
    m_focus_drop->Close();

    DetachChild(m_focus_drop);
    DetachChild(m_population_panel);
    DetachChild(m_resource_panel);
    DetachChild(m_military_panel);
    DetachChild(m_buildings_panel);
    DetachChild(m_annex_button);
    DetachChild(m_colonize_button);
    DetachChild(m_invade_button);
    DetachChild(m_bombard_button);
    DetachChild(m_specials_panel);
    DetachChild(m_planet_status_graphic);
}

namespace {
    std::string AnnexConditionDescription(const Condition::Condition* annexation_condition,
                                          const ScriptingContext& source_context,
                                          const UniverseObject* candidate)
    {
        if (!annexation_condition || !source_context.source || ! candidate)
            return EMPTY_STRING;
        return ConditionDescription(std::vector{annexation_condition}, source_context, candidate);
    }

    constexpr auto to_id = [](const auto& o) noexcept {
        if constexpr (requires { o->ID(); })
            return o->ID();
        else if constexpr ( requires { o.ID(); })
            return o.ID();
    };

    constexpr bool FlexibleContains(const auto& container, const auto num) {
        if constexpr (requires { container.contains(num); })
            return container.contains(num);
        else if constexpr (requires { container.find(num); container.end(); })
            return container.find(num) != container.end();
        else if constexpr (requires { container.begin(); container.end(); *container.begin() == num; })
            return range_contains(container, num);
        else if constexpr (requires { container.begin(); container.end(); to_id(*container.begin()) == num; })
            return range_contains(container | range_transform(to_id), num);
        else
            return false;
    }

    static_assert(FlexibleContains(std::array{1,2,3}, 2));
#if defined(USING_STD_RANGES) && USING_STD_RANGES
    static_assert([](){
        constexpr struct { constexpr int ID() const { return 42; } } thing;
        return FlexibleContains(std::array{&thing}, thing.ID());
    }());
#endif
#if defined(__cpp_lib_constexpr_vector)
    static_assert(FlexibleContains(std::vector{1,2,3}, 2));
#endif
}

void SidePanel::PlanetPanel::Refresh(ScriptingContext& context_in, int empire_id) {
    Clear();

    Universe& u = context_in.ContextUniverse();       // must be mutable for object signal inhibition
    ObjectMap& objects = context_in.ContextObjects(); // must be mutable to allow setting species and updating to estimate colonize button numbers
    const SpeciesManager& sm = context_in.species;
    const SupplyManager& supply = context_in.supply;

    auto* planet = objects.getRaw<Planet>(m_planet_id); // not const to allow updates for meter estimates
    if (!planet) {
        RequirePreRender();
        return;
    }

    const auto empire = empire_id != ALL_EMPIRES ? context_in.GetEmpire(empire_id).get() : nullptr;

    // use source object owned by client empire, unless there isn't one
    const auto* const source_for_empire = [empire, planet, &objects]() {
        const auto* retval = empire ? empire->Source(objects).get() : nullptr;
        if (!retval && !planet->Unowned())
            retval = planet;
        return retval;
    }();
    const ScriptingContext source_context{context_in, ScriptingContext::Source{}, source_for_empire};


    // set planet name, formatted to indicate presense of shipyards / homeworlds

    // apply formatting tags around planet name to indicate:
    //    Italic for homeworlds
    //    Underline for shipyard(s)

    const bool is_homeworld = range_any_of(sm.GetSpeciesHomeworldsMap() | range_values,
                                           [this](const auto& ids) { return FlexibleContains(ids, m_planet_id); });

    const auto& known_destroyed_object_ids = u.EmpireKnownDestroyedObjectIDs(empire_id);
    const auto not_destroyed_is_shipyard_tag = [&known_destroyed_object_ids](const Building* building) {
        return building &&
            !known_destroyed_object_ids.contains(building->ID()) &&
            building->HasTag(TAG_SHIPYARD);
    };
    const bool has_shipyard = range_any_of(objects.findRaw<const Building>(planet->BuildingIDs()),
                                           not_destroyed_is_shipyard_tag);

    // wrap with formatting tags
    std::string wrapped_planet_name;
    wrapped_planet_name.reserve(planet->Name().length() + 32);  // extra space for wrappings 3*3 + 3*4 + 2 + 1 = 24 + 8 for number
    wrapped_planet_name = planet->Name();
    if (is_homeworld)
        wrapped_planet_name = "<i>" + wrapped_planet_name + "</i>";
    static_assert(GG::Font::ITALIC_TAG == "i");
    if (has_shipyard)
        wrapped_planet_name = "<u>" + wrapped_planet_name + "</u>";
    static_assert(GG::Font::UNDERLINE_TAG == "u");
    if (GetOptionsDB().Get<bool>("ui.name.id.shown"))
        wrapped_planet_name = wrapped_planet_name + " (" + std::to_string(m_planet_id) + ")";


    // set name
    m_planet_name->SetText("<s>" + wrapped_planet_name + "</s>");
    static_assert(GG::Font::SHADOW_TAG == "s");
    m_planet_name->MoveTo(GG::Pt(GG::X(MaxPlanetDiameter() + EDGE_PAD), GG::Y0));
    m_planet_name->Resize(m_planet_name->MinUsableSize());


    // colour planet name with owner's empire colour
    m_empire_colour = GG::CLR_ZERO;
    if (!planet->Unowned() && m_planet_name) {
        if (auto planet_empire = source_context.GetEmpire(planet->Owner())) {
            m_empire_colour = planet_empire->Color();
            m_planet_name->SetTextColor(planet_empire->Color());
        } else {
            m_planet_name->SetTextColor(ClientUI::TextColor());
        }
    }

    const auto sys_id = SidePanel::SystemID();

    auto selected_colony_ship = ValidSelectedColonyShip(sys_id, source_context);
    if (!selected_colony_ship && FleetUIManager::GetFleetUIManager().SelectedShipIDs().empty())
        selected_colony_ship = objects.getRaw<Ship>(AutomaticallyChosenColonyShip(m_planet_id, context_in)); // need mutable context

    auto invasion_ships = ValidSelectedInvasionShips(sys_id, source_context);
    if (invasion_ships.empty())
        invasion_ships = AutomaticallyChosenInvasionShips(m_planet_id, source_context);

    auto bombard_ships = ValidSelectedBombardShips(sys_id, source_context);
    if (bombard_ships.empty())
        bombard_ships = AutomaticallyChosenBombardShips(m_planet_id, source_context);

    const std::string_view colony_ship_species_name = selected_colony_ship ? selected_colony_ship->SpeciesName() : EMPTY_STRING;
    const float colony_ship_capacity = selected_colony_ship ? selected_colony_ship->ColonyCapacity(u) : 0.0f;
    const Species* colony_ship_species = sm.GetSpecies(colony_ship_species_name);
    const PlanetEnvironment planet_env_for_colony_species = colony_ship_species ?
        colony_ship_species->GetPlanetEnvironment(planet->Type()) : PlanetEnvironment::PE_UNINHABITABLE;

    const auto& planet_species_name = planet->SpeciesName();
    const Species* species = sm.GetSpecies(planet_species_name); // may be nullptr
    const auto* annexation_condition = species ? species->AnnexationCondition() : nullptr;


    // calculate truth tables for planet colonization and invasion
    const bool has_owner =       !planet->Unowned();
    const bool mine =             planet->OwnedBy(empire_id);
    const bool populated =        planet->GetMeter(MeterType::METER_POPULATION)->Initial() > 0.0f;
    const bool habitable =        planet_env_for_colony_species >= PlanetEnvironment::PE_HOSTILE &&
                                  planet_env_for_colony_species <= PlanetEnvironment::PE_GOOD;
    const bool visible =          (empire_id == ALL_EMPIRES) ||
                                    (u.GetObjectVisibilityByEmpire(m_planet_id, empire_id) >= Visibility::VIS_PARTIAL_VISIBILITY);
    const bool shielded =         planet->GetMeter(MeterType::METER_SHIELD)->Initial() > 0.0f;
    const bool has_defenses =     planet->GetMeter(MeterType::METER_MAX_SHIELD)->Initial() > 0.0f ||
                                  planet->GetMeter(MeterType::METER_MAX_DEFENSE)->Initial() > 0.0f ||
                                  planet->GetMeter(MeterType::METER_MAX_TROOPS)->Initial() > 0.0f;
    const bool being_colonized =  planet->IsAboutToBeColonized();
    const bool outpostable =                   !populated && (  !has_owner /*&& !shielded*/         ) && visible && !being_colonized;
    const bool colonizable =      habitable && !populated && ( (!has_owner /*&& !shielded*/) || mine) && visible && !being_colonized;
    const bool can_colonize =     selected_colony_ship && (   (colonizable  && (colony_ship_capacity > 0.0f))
                                                           || (outpostable && (colony_ship_capacity == 0.0f)));
    const bool at_war_with_me =   !mine && (
                                    (!has_owner && populated) ||
                                    (has_owner && source_context.ContextDiploStatus(empire_id, planet->Owner()) == DiplomaticStatus::DIPLO_WAR)
                                  );

    const bool being_invaded =    planet->IsAboutToBeInvaded();
    const bool invadable =        at_war_with_me && !shielded && visible && !being_invaded && !invasion_ships.empty();


    const bool being_annexed =    planet->IsAboutToBeAnnexed();
    const auto annexability_test = [ac{annexation_condition}, &source_context](const auto* planet)
    { return ac && source_context.source && ac->EvalOne(source_context, planet); }; // ignores cost
    const bool potentially_annexable = annexability_test(planet);
    const auto this_planet_annexation_cost = source_for_empire ? PlanetAnnexationCost(planet, source_for_empire, source_context) : 0.0;
    const double empire_annexations_cost = source_for_empire ? PendingAnnexationOrderCost(source_for_empire, source_context) : 0.0;
    const double empire_adopted_policies_cost = empire ? empire->ThisTurnAdoptedPoliciesCost(source_context) : 0.0;
    const double total_costs = empire_annexations_cost + empire_adopted_policies_cost + this_planet_annexation_cost;
    const double available_ip = empire ? empire->ResourceStockpile(ResourceType::RE_INFLUENCE) : 0.0;
    const bool annexation_affordable = total_costs <= available_ip;
    const bool annexable =        visible && !being_annexed && populated && !being_invaded &&
                                  species && potentially_annexable && annexation_affordable;
    const bool show_annex_button = !mine && (being_annexed || annexable || (populated && !being_invaded && species));


    const bool being_bombarded =  planet->IsAboutToBeBombarded();
    const bool bombardable =      at_war_with_me && visible && !being_bombarded && !bombard_ships.empty();


    if (populated || SHOW_ALL_PLANET_PANELS) {
        AttachChild(m_population_panel);
        if (m_population_panel)
            m_population_panel->Refresh();
    }

    if (populated || has_owner || SHOW_ALL_PLANET_PANELS) {
        AttachChild(m_resource_panel);
        if (m_resource_panel)
            m_resource_panel->Refresh();
    }

    if (populated || has_owner || has_defenses || SHOW_ALL_PLANET_PANELS) {
        AttachChild(m_military_panel);
        if (m_military_panel)
            m_military_panel->Refresh();
    }

    if (m_annex_button && show_annex_button) {
        AttachChild(m_annex_button);
        auto txt = [=]() -> std::string {
            if (annexable) // specify cost
                return (this_planet_annexation_cost == 0.0) ? UserString("PL_ANNEX_FREE") :
                    boost::io::str(FlexibleFormat(UserString("PL_ANNEX")) %
                                   DoubleToString(this_planet_annexation_cost, 2, false));
            else if (being_annexed) // show cancel text
                return boost::io::str(FlexibleFormat(UserString("PL_CANCEL_ANNEX")) %
                                      DoubleToString(this_planet_annexation_cost, 2, false));
            else if (!annexation_affordable && potentially_annexable) // show cost
                return boost::io::str(FlexibleFormat(UserString("PL_ANNEXABLE")) %
                                      DoubleToString(this_planet_annexation_cost, 2, false));
            else if (!potentially_annexable)
                return UserString("PL_ANNEXATION_RESTRICTED"); // TODO: tooltip for failed condition
            else
                return "???";
        }();

        m_annex_button->SetText(std::move(txt));
        m_annex_button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

        // checked above for non-null planeted
        auto annex_tooltip = [planet, &planet_species_name, empire, empire_id, &sm,
                              &source_context, this_planet_annexation_cost, annexation_condition]() -> std::string
        {
            const auto& empire_name = empire ? empire->Name() : UserString("UNOWNED");
            const auto op = sm.SpeciesEmpireOpinion(planet_species_name, empire_id, false, true);
            auto opinion_of_empire = DoubleToString(op, 3, false);

            const auto owner_empire = source_context.GetEmpire(planet->Owner());
            const auto& owner_name = owner_empire ? owner_empire->Name() : UserString("UNOWNED");
            const auto owner_op = sm.SpeciesEmpireOpinion(planet_species_name, planet->Owner(), false, true);
            auto opinion_of_owner = DoubleToString(owner_op, 3, false);

            auto stability = DoubleToString(planet->GetMeter(MeterType::METER_HAPPINESS)->Initial(), 3, false);
            auto population = DoubleToString(planet->GetMeter(MeterType::METER_POPULATION)->Initial(), 3, false);

            const auto building_costs = [&source_context, planet, empire_id]() {
                const auto to_building_type_name = [](const Building* building) -> const auto&
                { return building ? building->BuildingTypeName() : EMPTY_STRING; };

                const auto to_cost = [empire_id, location_id{planet->ID()}, &source_context](const std::string& bt_name) {
                    if (bt_name.empty())
                        return 0.0;
                    const auto* building_type = GetBuildingType(bt_name);
                    return building_type ? building_type->ProductionCost(empire_id, location_id, source_context) : 0.0;
                };

                auto buildings = source_context.ContextObjects().findRaw<Building>(planet->BuildingIDs());
                auto cost_rng = buildings | range_transform(to_building_type_name) | range_transform(to_cost);
                return std::accumulate(cost_rng.begin(), cost_rng.end(), 0.0);
            }();
            auto building_costs_pp = DoubleToString(building_costs, 3, false);
            auto annex_cost_ip = DoubleToString(this_planet_annexation_cost, 3, false);

            auto condition_txt = AnnexConditionDescription(annexation_condition, source_context, planet);

            return boost::io::str(FlexibleFormat(UserString("ANNEX_BUTTON_TOOLTIP"))
                                  % UserString(planet_species_name) % empire_name % opinion_of_empire
                                  % owner_name % opinion_of_owner
                                  % stability % population % building_costs_pp % annex_cost_ip
                                  % condition_txt);
        }();
        m_annex_button->SetBrowseText(std::move(annex_tooltip));

        const bool clickable = annexable || being_annexed;
        m_annex_button->Disable(!clickable);
    }

    if (can_colonize) {
        // show colonize button; in case the chosen colony ship is not actually
        // selected, but has been chosen by AutomaticallyChosenColonyShip, determine
        // what population capacity to put on the colonize button by temporarily
        // setting ownership and species of the planet, calculating the target
        // population, then setting the planet back as it was. The results are
        // cached for the duration of the turn in the colony_projections map.
        AttachChild(m_colonize_button);

        double planet_capacity;
        const std::pair<int, int> ship_id_planet_id{selected_colony_ship->ID(), m_planet_id};
        const auto pair_it = colony_projections.find(ship_id_planet_id);
        if (pair_it != colony_projections.end()) {
            planet_capacity = pair_it->second;

        } else if (colony_ship_capacity == 0.0f) {
            planet_capacity = 0.0f;
            colony_projections.emplace(ship_id_planet_id, planet_capacity);

        } else {
            u.InhibitUniverseObjectSignals(true);
            const auto orig_species{planet_species_name}; //want to store by value, not reference. should be just ""
            const int orig_owner = planet->Owner();
            const float orig_initial_target_pop = planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Initial();
            planet->SetOwner(empire_id);
            planet->SetSpecies(std::string{colony_ship_species_name}, source_context.current_turn, sm);
            planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Reset();

            // temporary meter updates for curently set species
            u.UpdateMeterEstimates(m_planet_id, context_in);
            planet_capacity = ((planet_env_for_colony_species == PlanetEnvironment::PE_UNINHABITABLE) ? 0.0 : planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Current());   // want target pop after meter update, so check current value of meter
            planet->SetOwner(orig_owner);
            planet->SetSpecies(orig_species, context_in.current_turn, sm);
            planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Set(orig_initial_target_pop, orig_initial_target_pop);
            u.UpdateMeterEstimates(m_planet_id, context_in);

            colony_projections.emplace(ship_id_planet_id, planet_capacity);
            u.InhibitUniverseObjectSignals(false);
        }

        std::string colonize_text;
        if (colony_ship_capacity > 0.0f) {
            std::string initial_pop = DoubleToString(colony_ship_capacity, 2, false);

            std::string clr_tag;
            if (planet_capacity < colony_ship_capacity && colony_ship_capacity > 0.0f)
                clr_tag = GG::RgbaTag(ClientUI::StatDecrColor());
            else if (planet_capacity > colony_ship_capacity && colony_ship_capacity > 0.0f)
                clr_tag = GG::RgbaTag(ClientUI::StatIncrColor());

            std::string_view clr_tag_close = (clr_tag.empty() ? "" : "</rgba>");

            std::string target_pop = clr_tag + DoubleToString(planet_capacity, 2, false).append(clr_tag_close);

            colonize_text = boost::io::str(FlexibleFormat(UserString("PL_COLONIZE")) % initial_pop % target_pop);
        } else {
            colonize_text = UserString("PL_OUTPOST");
        }
        if (m_colonize_button)
            m_colonize_button->SetText(std::move(colonize_text));

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
        for (auto& invasion_ship : invasion_ships)
            invasion_troops += invasion_ship->TroopCapacity(u);

        const std::string invasion_troops_text = DoubleToString(invasion_troops, 3, false);

        // adjust defending troops number before passing into DoubleToString to ensure
        // rounding up, as it's better to slightly overestimate defending troops than
        // underestimate, since one needs to drop more droops than there are defenders
        // to capture a planet
        float defending_troops = planet->GetMeter(MeterType::METER_TROOPS)->Initial();
        const float log10_df = floor(std::log10(defending_troops));
        const float rounding_adjustment = std::pow(10.0f, log10_df - 2.0f);
        defending_troops += rounding_adjustment;

        std::string defending_troops_text = DoubleToString(defending_troops, 3, false);
        std::string invasion_text = boost::io::str(FlexibleFormat(UserString("PL_INVADE"))
                                                   % invasion_troops_text % defending_troops_text);
        // todo: tooltip breaking down which or how many ships are being used to invade, their troop composition / contribution
        if (m_invade_button)
            m_invade_button->SetText(std::move(invasion_text));

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

    const std::string_view type_size_species_name =
        !planet_species_name.empty() ? planet_species_name : colony_ship_species_name; // may be empty

    std::string env_size_text{
        type_size_species_name.empty() ?
            boost::io::str(FlexibleFormat(UserString("PL_TYPE_SIZE"))
                           % GetPlanetSizeName(planet)
                           % GetPlanetTypeName(planet)) :
            boost::io::str(FlexibleFormat(UserString("PL_TYPE_SIZE_ENV"))
                           % GetPlanetSizeName(planet)
                           % GetPlanetTypeName(planet)
                           % GetPlanetEnvironmentName(planet, type_size_species_name, source_context.species)
                           % UserString(type_size_species_name))};
    m_env_size->SetText(std::move(env_size_text));

    if (!planet->SpeciesName().empty()) {
        AttachChild(m_focus_drop);

        const auto available_foci = planet->AvailableFoci(source_context);

        // refresh items in list

        std::vector<std::shared_ptr<GG::DropDownList::Row>> rows;
        rows.reserve(available_foci.size());
        for (const auto& focus_name : available_foci) {
            auto texture = GetApp().GetUI().GetTexture(
                ClientUI::ArtDir() / planet->FocusIcon(focus_name, source_context), true);
            auto graphic = GG::Wnd::Create<GG::StaticGraphic>(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            graphic->Resize(GG::Pt(MeterIconSize().x*3/2, MeterIconSize().y*3/2));
            auto row = GG::Wnd::Create<GG::DropDownList::Row>(graphic->Width(), graphic->Height());
            row->SetDragDropDataType("FOCUS");

            row->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
            row->SetBrowseText(
                boost::io::str(FlexibleFormat(UserString("RP_FOCUS_TOOLTIP"))
                               % UserString(focus_name)));

            row->push_back(std::move(graphic));
            rows.push_back(std::move(row));
        }

        if (m_focus_drop->Dropped())
            m_focus_drop->Close();

        if (!m_focus_drop->Dropped()) {
            m_focus_drop->Clear();
            m_focus_drop->Insert(std::move(rows));

            // set browse text and select appropriate focus in droplist
            std::string focus_text;
            if (!planet->Focus().empty()) {
                for (unsigned int i = 0; i < available_foci.size(); ++i) {
                    if (available_foci[i] == planet->Focus()) {
                        m_focus_drop->Select(i);
                        focus_text = boost::io::str(FlexibleFormat(UserString("RP_FOCUS_TOOLTIP"))
                                                    % UserString(planet->Focus()));
                        break;
                    }
                }
            } else {
                m_focus_drop->Select(m_focus_drop->end());
            }
            m_focus_drop->SetBrowseText(std::move(focus_text));

            // prevent manipulation for unowned planets
            if (!planet->OwnedBy(empire_id))
                m_focus_drop->Disable();
        }
    }


    // other panels...
    AttachChild(m_buildings_panel);
    m_buildings_panel->Refresh();
    AttachChild(m_specials_panel);
    m_specials_panel->Update();

    // create planet status marker
    if (planet->OwnedBy(empire_id)) {
        std::vector<std::string> planet_status_messages;
        std::shared_ptr<GG::Texture> planet_status_texture;

        // status: no supply
        if (!supply.SystemHasFleetSupply(planet->SystemID(), planet->Owner(), true,
                                         source_context.diplo_statuses))
        {
            planet_status_messages.emplace_back(boost::io::str(FlexibleFormat(
                UserString("OPTIONS_DB_UI_PLANET_STATUS_NO_SUPPLY")) % planet->Name()));
            planet_status_texture = GetApp().GetUI().GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_supply.png", true);
        }

        // status: attacked on previous turn
        if (planet->LastTurnAttackedByShip() == source_context.current_turn - 1) {
            planet_status_messages.emplace_back(boost::io::str(FlexibleFormat(
                                                UserString("OPTIONS_DB_UI_PLANET_STATUS_ATTACKED")) % planet->Name()));
            planet_status_texture = GetApp().GetUI().GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_attacked.png", true);
        }

        // status: conquered on previous turn
        if (planet->LastTurnConquered() == source_context.current_turn - 1) {
            planet_status_messages.emplace_back(boost::io::str(FlexibleFormat(
                                                UserString("OPTIONS_DB_UI_PLANET_STATUS_CONQUERED")) % planet->Name()));
            planet_status_texture = GetApp().GetUI().GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_conquered.png", true);
        }

        // status: very unhappy
        if ((planet->GetMeter(MeterType::METER_HAPPINESS)->Current() <= 5) && (planet->GetMeter(MeterType::METER_POPULATION)->Current() > 0)) {
            planet_status_messages.emplace_back(boost::io::str(FlexibleFormat(
                UserString("OPTIONS_DB_UI_PLANET_STATUS_UNHAPPY")) % planet->Name()));
            planet_status_texture = GetApp().GetUI().GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_unhappy.png", true);
        }

        // status: bombarded (TBD)

        // status: several
        if (planet_status_messages.size() > 1) {
            planet_status_texture = GetApp().GetUI().GetTexture(ClientUI::ArtDir() / "icons" / "planet_status_warning.png", true);
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
                                                  tooltip_message.substr(0, tooltip_message.size()-2)));
            m_planet_status_graphic->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
            AttachChild(m_planet_status_graphic);
        }
    }

    // set planetpanel stealth browse text
    ClearBrowseInfoWnd();

    if (empire_id != ALL_EMPIRES) {
        const Visibility visibility = u.GetObjectVisibilityByEmpire(m_planet_id, empire_id);
        const auto& visibility_turn_map = u.GetObjectVisibilityTurnMapByEmpire(m_planet_id, empire_id);
        const float empire_detection_strength = empire ? empire->GetMeter("METER_DETECTION_STRENGTH")->Current() : 0.0f;
        const float apparent_stealth = planet->GetMeter(MeterType::METER_STEALTH)->Initial();

        std::string visibility_info;
        std::string detection_info;

        if (visibility == Visibility::VIS_NO_VISIBILITY) {
            visibility_info = UserString("PL_NO_VISIBILITY");

            auto last_turn_visible_it = visibility_turn_map.find(Visibility::VIS_BASIC_VISIBILITY);
            if (last_turn_visible_it != visibility_turn_map.end() && last_turn_visible_it->second > 0) {
                visibility_info += "  " + boost::io::str(FlexibleFormat(UserString("PL_LAST_TURN_SEEN")) %
                                                                        std::to_string(last_turn_visible_it->second));
            }
            else {
                visibility_info += "  " + UserString("PL_NEVER_SEEN");
                ErrorLogger() << "Empire " << empire_id << " knows about planet " << planet->Name() <<
                                 " (id: " << m_planet_id << ") without having seen it before!";
            }

            auto system = objects.get<const System>(planet->SystemID());
            if (system && system->GetVisibility(empire_id, u) <= Visibility::VIS_BASIC_VISIBILITY) { // HACK: system is basically visible or less, so we must not be in detection range of the planet.
                detection_info = UserString("PL_NOT_IN_RANGE");
            }
            else if (apparent_stealth > empire_detection_strength) {
                detection_info = boost::io::str(FlexibleFormat(UserString("PL_APPARENT_STEALTH_EXCEEDS_DETECTION")) %
                                                std::to_string(apparent_stealth) %
                                                std::to_string(empire_detection_strength));
            }
            else {
                detection_info = boost::io::str(FlexibleFormat(UserString("PL_APPARENT_STEALTH_DOES_NOT_EXCEED_DETECTION")) %
                                                std::to_string(empire_detection_strength));
            }

            std::string info = visibility_info + "\n\n" + detection_info;
            SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString("METER_STEALTH"), std::move(info)));
        }
        else if (visibility == Visibility::VIS_BASIC_VISIBILITY) {
            visibility_info = UserString("PL_BASIC_VISIBILITY");

            const auto last_turn_visible_it = visibility_turn_map.find(Visibility::VIS_PARTIAL_VISIBILITY);
            if (last_turn_visible_it != visibility_turn_map.end() && last_turn_visible_it->second > 0) {
                visibility_info += "  " + boost::io::str(FlexibleFormat(UserString("PL_LAST_TURN_SCANNED")) %
                                                                        std::to_string(last_turn_visible_it->second));
            }
            else {
                visibility_info += "  " + UserString("PL_NEVER_SCANNED");
            }

            if (apparent_stealth > empire_detection_strength) {
                detection_info = boost::io::str(FlexibleFormat(UserString("PL_APPARENT_STEALTH_EXCEEDS_DETECTION")) %
                                                std::to_string(apparent_stealth) %
                                                std::to_string(empire_detection_strength));
            }
            else {
                detection_info = boost::io::str(FlexibleFormat(UserString("PL_APPARENT_STEALTH_DOES_NOT_EXCEED_DETECTION")) %
                                                std::to_string(empire_detection_strength));
            }

            std::string info = visibility_info + "\n\n" + detection_info;
            SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString("METER_STEALTH"), std::move(info)));
        }
    }


    // BuildingsPanel::Refresh (and other panels) emit ExpandCollapseSignal,
    // which should be connected to SidePanel::PlanetPanel::DoLayout

    m_planet_connection = planet->StateChangedSignal.connect(
        [this]() {
            auto& app = GetApp();
            Refresh(app.GetContext(), app.EmpireID());
        },
        boost::signals2::at_front);
}

void SidePanel::PlanetPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void SidePanel::PlanetPanel::SetFocus(std::string focus) const {
    auto& app = GetApp();
    const int app_empire_id = app.EmpireID();
    ScriptingContext context = app.GetContext();
    const auto planet = context.ContextObjects().get<const Planet>(m_planet_id);
    if (!planet || !planet->OwnedBy(app_empire_id))
        return;
    // TODO: if focus is already equal to planet's focus, return early.

    colony_projections.clear();// in case new or old focus was Growth (important that be cleared BEFORE Order is issued)
    species_colony_projections.clear();

    app.Orders().IssueOrder<ChangeFocusOrder>(context, app_empire_id, m_planet_id, std::move(focus));
}

bool SidePanel::PlanetPanel::InWindow(GG::Pt pt) const {
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

void SidePanel::PlanetPanel::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    //std::cout << "SidePanel::PlanetPanel::LClick m_planet_id: " << m_planet_id << std::endl;
    if (!Disabled())
        LeftClickedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        LeftDoubleClickedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto& app = GetApp();
    ScriptingContext& context = app.GetContext();
    const ObjectMap& objects = context.ContextObjects();
    auto planet = objects.getRaw<const Planet>(m_planet_id);
    if (!planet)
        return;
    const int empire_id = app.EmpireID();


    auto system = objects.getRaw<const System>(planet->SystemID());

    // determine which other empires are at peace with client empire and have
    // an owned object in this fleet's system
    std::set<int> peaceful_empires_in_system;
    if (system) {
        auto system_objects = objects.findRaw<UniverseObject>(system->ObjectIDs());
        for (auto* obj : system_objects) {
            if (obj->GetVisibility(empire_id, context.ContextUniverse()) < Visibility::VIS_PARTIAL_VISIBILITY)
                continue;
            if (obj->OwnedBy(empire_id)|| obj->Unowned())
                continue;
            if (peaceful_empires_in_system.contains(obj->Owner()))
                continue;
            if (context.ContextDiploStatus(empire_id, obj->Owner()) < DiplomaticStatus::DIPLO_PEACE)
                continue;
            peaceful_empires_in_system.insert(obj->Owner());
        }
    }

    if (ClientPlayerIsModerator()) {
        if (const auto map_wnd = GetApp().GetUI().GetMapWndConst()) {
            if (map_wnd->GetModeratorActionSetting() != ModeratorActionSetting::MAS_NoAction) {
                RightClickedSignal(planet->ID());  // response handled in MapWnd
                return;
            }
        }
    }

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    if (planet->OwnedBy(empire_id) && m_order_issuing_enabled
        && m_planet_name->InClient(pt))
    {
        auto rename_action = [this, planet, empire_id]() mutable { // rename planet
            auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(
                GG::X(350), UserString("SP_ENTER_NEW_PLANET_NAME"), planet->Name());
            edit_wnd->Run();

            if (!m_order_issuing_enabled)
                return;

            auto result{edit_wnd->Result()};

            auto& app = GetApp();
            auto& context = app.GetContext();

            if (!RenameOrder::Check(empire_id, planet->ID(), result, context))
                return;

            app.Orders().IssueOrder<RenameOrder>(context, empire_id, planet->ID(), std::move(result));

            Refresh(context, empire_id);
        };
        popup->AddMenuItem(GG::MenuItem(UserString("SP_RENAME_PLANET"), false, false, rename_action));
    }

    auto pedia_to_planet_action = [this]()
    { GetApp().GetUI().ZoomToPlanetPedia(m_planet_id, GetApp().GetContext().ContextObjects()); };

    popup->AddMenuItem(GG::MenuItem(UserString("SP_PLANET_SUITABILITY"), false, false, pedia_to_planet_action));

    if (planet->OwnedBy(empire_id)
        && !peaceful_empires_in_system.empty()
        && !ClientPlayerIsModerator())
    {
        popup->AddMenuItem(GG::MenuItem(true));

        // submenus for each available recipient empire
        GG::MenuItem give_away_menu(UserString("ORDER_GIVE_PLANET_TO_EMPIRE"), false, false);
        for (auto& [recipient_empire_id, recipient_empire] : context.Empires()) {
            auto gift_action = [recipient_empire_id{recipient_empire_id}, empire_id, planet]()
            {
                auto& app = GetApp();
                app.Orders().IssueOrder<GiveObjectToEmpireOrder>(
                    app.GetContext(), empire_id, planet->ID(), recipient_empire_id);
            };
            if (!peaceful_empires_in_system.contains(recipient_empire_id))
                continue;
            give_away_menu.next_level.emplace_back(recipient_empire->Name(), false, false, gift_action);
        }
        popup->AddMenuItem(std::move(give_away_menu));

        if (planet->OrderedGivenToEmpire() != ALL_EMPIRES) {
            auto ungift_action = [planet_id{planet->ID()}]() { // cancel give away order for this fleet
                auto& app = GetApp();
                const OrderSet orders = app.Orders(); // copy to avoid changing container being iteratred
                for (auto& [order_id, order] : orders) {
                    if (auto give_order = std::dynamic_pointer_cast<GiveObjectToEmpireOrder>(order)) {
                        if (give_order->ObjectID() == planet_id) {
                            app.Orders().RescindOrder(order_id, app.GetContext());
                            // could break here, but won't to ensure there are no problems with doubled orders
                        }
                    }
                }

            };
            GG::MenuItem cancel_give_away_menu(UserString("ORDER_CANCEL_GIVE_PLANET"),
                                               false, false, ungift_action);
            popup->AddMenuItem(std::move(cancel_give_away_menu));
        }
    }

    popup->Run();
}

void SidePanel::PlanetPanel::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void SidePanel::PlanetPanel::PreRender() {
    GG::Control::PreRender();
    if (const auto* planet = GetApp().GetContext().ContextObjects().getRaw<Planet>(m_planet_id))
        DoLayout(planet->Type(), planet->Size());
}

void SidePanel::PlanetPanel::Render() {
    const GG::Pt ul = UpperLeft(), lr = LowerRight() - GG::Pt{GG::X1, GG::Y0};
    const GG::Pt name_ul = m_planet_name->UpperLeft() - GG::Pt(GG::X(EDGE_PAD), GG::Y0);
    const GG::Pt name_lr = GG::Pt(lr.x, m_planet_name->Bottom());

    const bool show_planet_box = m_rotating_planet_graphic || m_planet_graphic;
    const GG::Pt planet_box_lr = m_planet_graphic ?
        m_planet_graphic->LowerRight() :
        ul + [mpd{MaxPlanetDiameter()}](){ return GG::Pt(GG::X(mpd), GG::Y(mpd)); }();

    const GG::Clr background_colour = ClientUI::CtrlColor();
    const GG::Clr title_background_colour = ClientUI::WndOuterBorderColor();
    const GG::Clr border_colour = (m_selected ? m_empire_colour : ClientUI::WndOuterBorderColor());


    m_verts.clear();
    m_colours.clear();

    // main border / background
    const auto store_main_border_fan = [&](GG::Clr clr) {
        m_verts.store(lr.x,                 ul.y);                      // top right corner
        if (show_planet_box) {
            static constexpr int OFFSET = 15;                           // size of corners cut off sticky-out bit of background around planet render

            m_verts.store(ul.x + OFFSET,    ul.y);                      // top left, offset right to cut off corner
            m_verts.store(ul.x,             ul.y + OFFSET);             // top left, offset down to cut off corner
            m_verts.store(ul.x,             planet_box_lr.y - OFFSET);  // bottom left, offset up to cut off corner
            m_verts.store(ul.x + OFFSET,    planet_box_lr.y);           // bottom left, offset right to cut off corner
            m_verts.store(planet_box_lr.x,  planet_box_lr.y);           // inner corner between planet box and rest of panel
        } else {
            m_verts.store(planet_box_lr.x,  ul.y);                      // top left of main panel, excluding planet box
        }
        m_verts.store(planet_box_lr.x,      lr.y);                      // bottom left of main panel
        m_verts.store(lr.x,                 lr.y);                      // bottom right

        m_colours.store(show_planet_box ? 8u : 4u, clr);
    };

    store_main_border_fan(background_colour);
    m_main_border_sz = m_verts.size() - s_background_start_idx;

    m_disable_greyover_start_idx = m_verts.size();
    static constexpr GG::Clr HALF_GREY(128, 128, 128, 128);
    store_main_border_fan(HALF_GREY);

    m_border_line_start_idx = m_verts.size();
    store_main_border_fan(border_colour);

    m_title_box_start_idx = m_verts.size();
    m_verts.store(name_lr.x, name_ul.y);
    m_verts.store(name_ul.x, name_ul.y);
    m_verts.store(name_ul.x, name_lr.y);
    m_verts.store(name_lr.x, name_lr.y);
    m_colours.store(s_title_box_sz, title_background_colour);


    m_verts.activate();
    m_colours.activate();


    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);


    // background for whole panel
    glDrawArrays(GL_TRIANGLE_FAN, s_background_start_idx, m_main_border_sz);

    // title background box
    glDrawArrays(GL_TRIANGLE_FAN, m_title_box_start_idx, s_title_box_sz);

    // disable greyover
    if (Disabled())
        glDrawArrays(GL_TRIANGLE_FAN, m_disable_greyover_start_idx, m_main_border_sz);

    // border
    glLineWidth(1.5f);
    glDrawArrays(GL_LINE_LOOP, m_border_line_start_idx, m_main_border_sz);
    glLineWidth(1.0f);


    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}

void SidePanel::PlanetPanel::Select(bool selected)
{ m_selected = selected; } // TODO: consider setting text box colours?

namespace {
    void CancelColonizeInvadeBombardScrapShipOrders(const Ship& ship, ScriptingContext& context,
                                                    OrderSet& mutable_orders)
    {
        const auto orders_const{mutable_orders}; // copy to preserve iterators while rescinding

        // is selected ship already ordered to colonize?  If so, recind that order.
        if (ship.OrderedColonizePlanet() != INVALID_OBJECT_ID) {
            for (const auto& [order_id, order] : orders_const) {
                if (auto colonize_order = std::dynamic_pointer_cast<ColonizeOrder>(order)) {
                    if (colonize_order->ShipID() == ship.ID()) {
                        mutable_orders.RescindOrder(order_id, context);
                        // could break here, but won't to ensure there are no problems with doubled orders
                    }
                }
            }
        }

        // is selected ship ordered to invade?  If so, recind that order
        if (ship.OrderedInvadePlanet() != INVALID_OBJECT_ID) {
            for (const auto& [order_id, order] : orders_const) {
               if (auto invade_order = std::dynamic_pointer_cast<InvadeOrder>(order)) {
                    if (invade_order->ShipID() == ship.ID()) {
                        mutable_orders.RescindOrder(order_id, context);
                        // could break here, but won't to ensure there are no problems with doubled orders
                    }
                }
            }
        }

        // is selected ship ordered scrapped?  If so, recind that order
        if (ship.OrderedScrapped()) {
            for (const auto& [order_id, order] : orders_const) {
                if (auto scrap_order = std::dynamic_pointer_cast<ScrapOrder>(order)) {
                    if (scrap_order->ObjectID() == ship.ID()) {
                        mutable_orders.RescindOrder(order_id, context);
                        // could break here, but won't to ensure there are no problems with doubled orders
                    }
                }
            }
        }

        // is selected ship order to bombard?  If so, recind that order
        if (ship.OrderedBombardPlanet() != INVALID_OBJECT_ID) {
            for (const auto& [order_id, order] : orders_const) {
               if (auto bombard_order = std::dynamic_pointer_cast<BombardOrder>(order)) {
                    if (bombard_order->ShipID() == ship.ID()) {
                        mutable_orders.RescindOrder(order_id, context);
                        // could break here, but won't to ensure there are no problems with doubled orders
                    }
                }
            }
        }
    }
}

void SidePanel::PlanetPanel::ClickAnnex() {
    // order or cancel annexation, depending on whether it has previously
    // been ordered

    auto& app = GetApp();
    ScriptingContext& context = app.GetContext();
    const ObjectMap& objects{context.ContextObjects()};
    auto& orders = app.Orders();

    const auto planet = objects.get<const Planet>(m_planet_id);
    if (!planet || !m_order_issuing_enabled)
        return;

    const int empire_id = app.EmpireID();
    if (empire_id == ALL_EMPIRES)
        return;

    const auto pending_annex_orders = PendingAnnexationOrders(app);
    const auto it = std::find_if(pending_annex_orders.begin(), pending_annex_orders.end(),
                                 [this](const auto& order_id_planet_id)
                                 { return m_planet_id == order_id_planet_id.first; });
    if (it != pending_annex_orders.end()) {
        // cancel previous annex order for planet
        orders.RescindOrder(it->second, context);

    } else {
        const auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "No empire with id " << empire_id;
            return;
        }
        const auto source = empire->Source(context.ContextObjects()).get();
        if (!source) {
            ErrorLogger() << "No source for empire " << empire->Name();
            return;
        }
        ScriptingContext empire_context{context, ScriptingContext::Source{}, source};
        orders.IssueOrder<AnnexOrder>(empire_context, empire_id, m_planet_id);
    }
    OrderButtonChangedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::ClickColonize() {
    // order or cancel colonization, depending on whether it has previously
    // been ordered

    auto& app = GetApp();
    ScriptingContext& context = app.GetContext();
    const ObjectMap& objects{context.ContextObjects()};
    auto& orders = app.Orders();

    const auto planet = objects.getRaw<const Planet>(m_planet_id);
    if (!planet || planet->GetMeter(MeterType::METER_POPULATION)->Initial() != 0.0 || !m_order_issuing_enabled)
        return;

    const int empire_id = app.EmpireID();
    if (empire_id == ALL_EMPIRES)
        return;

    const auto pending_colonization_orders = PendingColonizationOrders();
    const auto it = pending_colonization_orders.find(m_planet_id);

    if (it != pending_colonization_orders.end()) {
        // cancel previous colonization order for planet
        orders.RescindOrder(it->second, context);

    } else {
        // find colony ship and order it to colonize
        auto ship = ValidSelectedColonyShip(SidePanel::SystemID(), context);
        if (!ship)
            ship = objects.getRaw<const Ship>(AutomaticallyChosenColonyShip(m_planet_id, context));

        if (!ship) {
            ErrorLogger() << "SidePanel::PlanetPanel::ClickColonize valid colony not found!";
            return;
        }
        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "SidePanel::PlanetPanel::ClickColonize selected colony ship not owned by this client's empire.";
            return;
        }

        CancelColonizeInvadeBombardScrapShipOrders(*ship, context, orders);

        orders.IssueOrder<ColonizeOrder>(context, empire_id, ship->ID(), m_planet_id);
    }
    OrderButtonChangedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::ClickInvade() {
    // order or cancel invasion, depending on whether it has previously been ordered

    auto& app = GetApp();
    ScriptingContext& context = app.GetContext();
    const ObjectMap& objects{context.ContextObjects()};
    auto planet = objects.getRaw<Planet>(m_planet_id);
    if (!planet ||
        !m_order_issuing_enabled ||
        (planet->GetMeter(MeterType::METER_POPULATION)->Initial() <= 0.0 && planet->Unowned()))
    { return; }

    int empire_id = app.EmpireID();
    if (empire_id == ALL_EMPIRES)
        return;

    auto& orders = app.Orders();

    const auto pending_invade_orders = PendingInvadeOrders();
    auto it = pending_invade_orders.find(m_planet_id);

    if (it != pending_invade_orders.end()) {
        const auto& planet_invade_orders = it->second;
        // cancel previous invasion orders for this planet
        for (int order_id : planet_invade_orders)
            orders.RescindOrder(order_id, context);

    } else {
        // order selected invasion ships to invade planet
        auto invasion_ships = ValidSelectedInvasionShips(planet->SystemID(), context);
        if (invasion_ships.empty())
            invasion_ships = AutomaticallyChosenInvasionShips(m_planet_id, context);

        for (const auto* ship : invasion_ships | range_filter([](auto s) { return !!s; })) {
            CancelColonizeInvadeBombardScrapShipOrders(*ship, context, orders);
            orders.IssueOrder<InvadeOrder>(context, empire_id, ship->ID(), m_planet_id);
        }
    }
    OrderButtonChangedSignal(m_planet_id);
}

void SidePanel::PlanetPanel::ClickBombard() {
    // order or cancel bombard, depending on whether it has previously
    // been ordered

    auto& app = GetApp();
    ScriptingContext& context = app.GetContext();
    const ObjectMap& objects{context.ContextObjects()};
    auto planet = objects.getRaw<Planet>(m_planet_id);
    if (!planet ||
        !m_order_issuing_enabled ||
        (planet->GetMeter(MeterType::METER_POPULATION)->Initial() <= 0.0 && planet->Unowned()))
    { return; }

    int empire_id = app.EmpireID();
    if (empire_id == ALL_EMPIRES)
        return;

    auto& orders = app.Orders();

    const auto pending_bombard_orders = PendingBombardOrders();
    const auto it = pending_bombard_orders.find(m_planet_id);

    if (it != pending_bombard_orders.end()) {
        const auto planet_bombard_orders{it->second}; // copy to preserve iterators while rescinding
        // cancel previous bombard orders for this planet
        for (int order_id : planet_bombard_orders)
            orders.RescindOrder(order_id, context);

    } else {
        // order selected bombard ships to bombard planet
        auto bombard_ships = ValidSelectedBombardShips(planet->SystemID(), context);
        if (bombard_ships.empty())
            bombard_ships = AutomaticallyChosenBombardShips(m_planet_id, context);

        for (const auto* ship : bombard_ships | range_filter([](auto s) { return !!s; })) {
            CancelColonizeInvadeBombardScrapShipOrders(*ship, context, orders);
            orders.IssueOrder<BombardOrder>(context, empire_id, ship->ID(), m_planet_id);
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
        ErrorLogger() << "PlanetPanel::FocusDropListSelectionChanged passed end / invalid iterator";
        return;
    }

    const auto& context = GetApp().GetContext();
    const auto res = context.ContextObjects().getRaw<Planet>(m_planet_id);
    if (!res) {
        ErrorLogger() << "PlanetPanel::FocusDropListSelectionChanged couldn't get planet with id " << m_planet_id;
        return;
    }

    const auto foci = res->AvailableFoci(context);

    const auto i = m_focus_drop->IteratorToIndex(selected);
    if (i >= foci.size() || i < 0) {
        ErrorLogger() << "PlanetPanel::FocusDropListSelectionChanged got invalid focus selected index: " << i;
        return;
    }

    const Sound::TempUISoundDisabler sound_disabler;
    DebugLogger() << "About to send focus-changed signal.";
    FocusChangedSignal(foci[i]);
    DebugLogger() << "Returned from sending focus-changed signal.";
}

void SidePanel::PlanetPanel::EnableOrderIssuing(bool enable) {
    m_order_issuing_enabled = enable;

    m_annex_button->Disable(!enable);
    m_colonize_button->Disable(!enable);
    m_invade_button->Disable(!enable);
    m_bombard_button->Disable(!enable);

    m_buildings_panel->EnableOrderIssuing(enable);

    if (!enable) {
        m_focus_drop->Disable();
        return;
    }

    const auto obj = GetApp().GetContext().ContextObjects().get(m_planet_id);
    if (!obj || !obj->OwnedBy(GetApp().EmpireID()))
        m_focus_drop->Disable();
    else
        m_focus_drop->Disable(false);
}

////////////////////////////////////////////////
// SidePanel::PlanetPanelContainer
////////////////////////////////////////////////
SidePanel::PlanetPanelContainer::PlanetPanelContainer() :
    Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
    m_vscroll(GG::Wnd::Create<CUIScroll>(GG::Orientation::VERTICAL))
{
    SetName("PlanetPanelContainer");
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    namespace ph = boost::placeholders;

    m_vscroll->ScrolledSignal.connect(
        boost::bind(&SidePanel::PlanetPanelContainer::VScroll, this, ph::_1, ph::_2, ph::_3, ph::_4));
    RequirePreRender();
}

bool SidePanel::PlanetPanelContainer::InWindow(GG::Pt pt) const {
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

void SidePanel::PlanetPanelContainer::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) {
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

void SidePanel::PlanetPanelContainer::LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys)
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
    for (auto& pp : m_planet_panels)
        pp->Clear();
    m_planet_panels.clear();
    m_selected_planet_id = INVALID_OBJECT_ID;
    DetachChildren();
    AttachChild(m_vscroll);
}

void SidePanel::PlanetPanelContainer::SetPlanets(
    const std::vector<int>& planet_ids, StarType star_type,
    ScriptingContext& context, int empire_id)
{
    int initial_selected_planet_panel = m_selected_planet_id;

    // remove old panels
    Clear();

    const auto& objects = context.ContextObjects();

    std::multimap<int, int> orbits_planets;
    for (const auto* planet : objects.findRaw<const Planet>(planet_ids)) {
        if (!planet)
            continue;
        int system_id = planet->SystemID();
        const auto* system = objects.getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "PlanetPanelContainer::SetPlanets couldn't find system of planet" << planet->Name();
            continue;
        }
        orbits_planets.emplace(system->OrbitOfPlanet(planet->ID()), planet->ID());
    }

    // create new panels and connect their signals
    for (auto orbit_planet : orbits_planets | range_values) {
        auto planet_panel = GG::Wnd::Create<PlanetPanel>(Width() - m_vscroll->Width(),
                                                         orbit_planet, star_type);
        AttachChild(planet_panel);
        m_planet_panels.push_back(std::move(planet_panel));
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
                auto& app = GetApp();
                RefreshAllPlanetPanels(app.GetContext(), app.EmpireID(), excluded_planet_id, true);
            });
    }

    // disable non-selectable planet panels
    DisableNonSelectionCandidates();

    // redo contents and layout of panels, after enabling or disabling, so
    // they take this into account when doing contents
    RefreshAllPlanetPanels(context, empire_id);

    SelectPlanet(initial_selected_planet_panel);

    RequirePreRender();
}

void SidePanel::PlanetPanelContainer::DoPanelsLayout() {
    if (m_ignore_recursive_resize)
        return;

    GG::ScopedAssign prevent_inifinite_recursion(m_ignore_recursive_resize, true);

    // Determine if scroll bar is required for height or expected because it is already present.
    GG::Y available_height = Height();
    GG::Y initially_used_height = GG::Y0;
    for (const auto& panel : m_planet_panels)
        initially_used_height += panel->Height() + EDGE_PAD;

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
    if (planet_id != m_selected_planet_id && m_candidate_ids.contains(planet_id)) {
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
        GetApp().SendPartialOrders();
    }
}

void SidePanel::PlanetPanelContainer::SetValidSelectionPredicate(std::function<bool(const UniverseObject*)> pred)
{ m_valid_selection_predicate = std::move(pred); }

void SidePanel::PlanetPanelContainer::ClearValidSelectionPedicate()
{ m_valid_selection_predicate = nullptr; }

void SidePanel::PlanetPanelContainer::DisableNonSelectionCandidates() {
    //std::cout << "SidePanel::PlanetPanelContainer::DisableNonSelectionCandidates" << std::endl;
    m_candidate_ids.clear();
    std::set<std::shared_ptr<PlanetPanel>> disabled_panels;

    const ObjectMap& objects = GetApp().GetContext().ContextObjects();

    if (m_valid_selection_predicate) {
        // if there is a selection predicate, which determines which planet panels
        // can be selected, refresh the candidiates and disable the non-selectables

        // find selectables
        for (auto& panel : m_planet_panels) {
            int planet_id = panel->PlanetID();
            const auto* planet = objects.getRaw<Planet>(planet_id);

            if (planet && m_valid_selection_predicate(planet)) {
                m_candidate_ids.insert(planet_id);
                //std::cout << " ... planet " << planet->ID() << " is a selection candidate" << std::endl;
            } else {
                disabled_panels.insert(panel);
            }
        }
    }

    // disable and enabled appropriate panels
    for (auto& panel : m_planet_panels) {
        if (disabled_panels.contains(panel)) {
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
    ScriptingContext& context, int empire_id, int excluded_planet_id, bool require_prerender)
{
    for (auto& panel : m_planet_panels) {
        if (!panel)
            continue;
        if (excluded_planet_id > 0 && panel->PlanetID() == INVALID_OBJECT_ID)
            continue;
        panel->Refresh(context, empire_id);
        if (require_prerender)
            panel->RequirePreRender();
    }
}

void SidePanel::PlanetPanelContainer::ShowScrollbar()
{ RequirePreRender(); }

void SidePanel::PlanetPanelContainer::HideScrollbar()
{ DetachChild(m_vscroll); }

void SidePanel::PlanetPanelContainer::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void SidePanel::PlanetPanelContainer::EnableOrderIssuing(bool enable) {
    for (auto& panel : m_planet_panels)
        panel->EnableOrderIssuing(enable);
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
            for (const auto& [first, second] : m_labels_and_amounts) {
                DetachChild(first);
                DetachChild(second);
            }
            m_labels_and_amounts.clear();

            auto& app = GetApp();
            const auto& objects = app.GetContext().ContextObjects();
            const auto system = objects.get<System>(m_system_id);
            if (!system)
                return;
            auto& ui = app.GetUI();

            auto total_meter_value = 0.0f;

            auto title_label = GG::Wnd::Create<CUILabel>(UserString(to_string(m_meter_type)), ui, GG::FORMAT_RIGHT);
            title_label->MoveTo(GG::Pt0);
            title_label->Resize(GG::Pt(LabelWidth(), row_height));
            title_label->SetFont(app.GetUI().GetBoldFont());
            AttachChild(title_label);

            GG::Y top = row_height;
            // add label-value pair for each resource-producing object in system to indicate amount of resource produced
            const auto contained_objects = objects.findRaw<const Planet>(system->ContainedObjectIDs());
            for (const auto* planet : contained_objects) {
                // Ignore empty planets
                if (!planet || (planet->Unowned() && planet->SpeciesName().empty()))
                    continue;

                auto label = GG::Wnd::Create<CUILabel>(planet->Name(), ui, GG::FORMAT_RIGHT);
                label->MoveTo(GG::Pt(GG::X0, top));
                label->Resize(GG::Pt(LabelWidth(), row_height));
                AttachChild(label);

                const auto meter_value = planet->GetMeter(m_meter_type)->Initial();
                if (m_meter_type == MeterType::METER_SUPPLY) {
                    total_meter_value = std::max(total_meter_value, meter_value);
                } else {
                    total_meter_value += meter_value;
                }
                auto value = GG::Wnd::Create<CUILabel>(DoubleToString(meter_value, 3, false), ui);
                value->MoveTo(GG::Pt(LabelWidth(), top));
                value->Resize(GG::Pt(ValueWidth(), row_height));
                AttachChild(value);

                m_labels_and_amounts.emplace_back(std::move(label), std::move(value));

                top += row_height;
            }

            auto title_value = GG::Wnd::Create<CUILabel>(DoubleToString(total_meter_value, 3, false), ui);
            title_value->MoveTo(GG::Pt(LabelWidth(), GG::Y0));
            title_value->Resize(GG::Pt(ValueWidth(), row_height));
            title_value->SetFont(ui.GetBoldFont());
            AttachChild(title_value);
            m_labels_and_amounts.emplace_back(std::move(title_label), std::move(title_value));

            Resize(GG::Pt(LabelWidth() + ValueWidth(), top));
        }

    private:
        MeterType m_meter_type;
        int m_system_id;
        std::vector<std::pair<std::shared_ptr<GG::Label>,
                              std::shared_ptr<GG::Label>>> m_labels_and_amounts;
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
std::set<boost::signals2::scoped_connection>      SidePanel::s_system_connections;
std::map<int, boost::signals2::scoped_connection> SidePanel::s_fleet_state_change_signals;
boost::signals2::signal<void ()>           SidePanel::ResourceCenterChangedSignal;
boost::signals2::signal<void (int)>        SidePanel::PlanetSelectedSignal;
boost::signals2::signal<void (int)>        SidePanel::PlanetRightClickedSignal;
boost::signals2::signal<void (int)>        SidePanel::PlanetDoubleClickedSignal;
boost::signals2::signal<void (int)>        SidePanel::BuildingRightClickedSignal;
boost::signals2::signal<void (int)>        SidePanel::SystemSelectedSignal;

SidePanel::SidePanel(std::string_view config_name) :
    CUIWnd("", GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP, config_name)
{}

void SidePanel::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    m_planet_panel_container = GG::Wnd::Create<PlanetPanelContainer>();
    AttachChild(m_planet_panel_container);

    std::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    m_button_prev = Wnd::Create<CUIButton>(
        GG::SubTexture(GetApp().GetUI().GetTexture(button_texture_dir / "leftarrownormal.png")),
        GG::SubTexture(GetApp().GetUI().GetTexture(button_texture_dir / "leftarrowclicked.png")),
        GG::SubTexture(GetApp().GetUI().GetTexture(button_texture_dir / "leftarrowmouseover.png")));

    m_button_next = Wnd::Create<CUIButton>(
        GG::SubTexture(GetApp().GetUI().GetTexture(button_texture_dir / "rightarrownormal.png")),
        GG::SubTexture(GetApp().GetUI().GetTexture(button_texture_dir / "rightarrowclicked.png")),
        GG::SubTexture(GetApp().GetUI().GetTexture(button_texture_dir / "rightarrowmouseover.png")));

    Sound::TempUISoundDisabler sound_disabler;

    m_system_name = GG::Wnd::Create<SystemNameDropDownList>(6);
    m_system_name->SetStyle(GG::LIST_NOSORT | GG::LIST_SINGLESEL);
    m_system_name->DisableDropArrow();
    m_system_name->SetInteriorColor(GG::Clr(0, 0, 0, 200));
    m_system_name->ManuallyManageColProps();
    m_system_name->NormalizeRowsOnInsert(false);
    m_system_name->SetNumCols(1);
    AttachChild(m_system_name);

    m_star_type_text = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, "", GetApp().GetUI().GetFont(), ClientUI::TextColor());
    AttachChild(m_star_type_text);
    AttachChild(m_button_prev);
    AttachChild(m_button_next);

    m_system_resource_summary = GG::Wnd::Create<MultiIconValueIndicator>(Width() - EDGE_PAD*2);
    AttachChild(m_system_resource_summary);

    using boost::placeholders::_1;

    m_connections[0] = m_system_name->DropDownOpenedSignal.connect(
        [this](auto b) { SystemNameDropListOpenedSlot(b); });
    m_connections[1] = m_system_name->SelChangedSignal.connect(
        [this](auto it) { SystemSelectionChangedSlot(it); });
    m_connections[2] = m_system_name->SelChangedWhileDroppedSignal.connect(
        [this](auto it) { SystemSelectionChangedSlot(it); });
    m_connections[3] = m_button_prev->LeftClickedSignal.connect(
        [this]() { PrevButtonClicked(); });
    m_connections[4] = m_button_next->LeftClickedSignal.connect(
        [this]() { NextButtonClicked(); });
    m_connections[5] = m_planet_panel_container->PlanetClickedSignal.connect(
        [this](auto id) { PlanetClickedSlot(id, GetApp().GetContext().ContextObjects()); });
    m_connections[6] = m_planet_panel_container->PlanetLeftDoubleClickedSignal.connect(
        PlanetDoubleClickedSignal);
    m_connections[7] = m_planet_panel_container->PlanetRightClickedSignal.connect(
        PlanetRightClickedSignal);
    m_connections[8] = m_planet_panel_container->BuildingRightClickedSignal.connect(
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

bool SidePanel::InWindow(GG::Pt pt) const {
    return (UpperLeft() + GG::Pt(GG::X(MaxPlanetDiameter()), GG::Y0) <= pt && pt < LowerRight())
           || (m_planet_panel_container &&
               m_planet_panel_container->InWindow(pt))
           || (m_system_resource_summary &&
               m_system_resource_summary->Parent().get() == this && m_system_resource_summary->InWindow(pt));
}

GG::Pt SidePanel::ClientUpperLeft() const noexcept
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

    auto& app = GetApp();
    ScriptingContext& context = app.GetContext(); // mutable because RefreshInPreRender modifies universe to simulate effects
    const auto empire_id = app.EmpireID();

    // Needs refresh updates all data related to all SizePanels, including system list etc.
    if (s_needs_refresh)
        RefreshInPreRender(app);

    // Update updates the data for each planet tab in all SidePanels
    if (s_needs_update) {
        for (auto& weak_panel : s_side_panels)
            if (auto panel = weak_panel.lock())
                panel->UpdateImpl(context, empire_id);
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

void SidePanel::UpdateImpl(ScriptingContext& context, int empire_id) {
    //std::cout << "SidePanel::UpdateImpl" << std::endl;
    if (m_system_resource_summary)
        m_system_resource_summary->Update(std::as_const(context).ContextObjects());
    // update individual PlanetPanels in PlanetPanelContainer, then redo layout of panel container
    m_planet_panel_container->RefreshAllPlanetPanels(context, empire_id);
}

void SidePanel::Refresh() {
    s_needs_refresh = true;
    for (auto& weak_panel : s_side_panels)
        if (auto panel = weak_panel.lock())
            panel->RequirePreRender();
}

void SidePanel::RefreshInPreRender(GGHumanClientApp& app) {
    // disconnect any existing system and fleet signals
    for (const auto& con : s_system_connections)
        con.disconnect();
    s_system_connections.clear();

    for (auto& scs : s_fleet_state_change_signals | range_values)
        scs.disconnect();
    s_fleet_state_change_signals.clear();

    // clear any previous colony projections
    colony_projections.clear();
    species_colony_projections.clear();

    const auto& context = app.GetContext();
    const auto& objects = context.ContextObjects();

    // refresh individual panels' contents
    for (auto& weak_panel : s_side_panels)
        if (auto panel = weak_panel.lock())
            panel->RefreshImpl(app);


    // early exit if no valid system object to get or connect signals to
    if (s_system_id == INVALID_OBJECT_ID)
        return;


    // connect state changed and insertion signals for planets and fleets in system
    auto* system = objects.getRaw<System>(s_system_id);
    if (!system) {
        ErrorLogger() << "SidePanel::Refresh couldn't get system with id " << s_system_id;
        return;
    }

    for (auto* planet : objects.findRaw<Planet>(system->PlanetIDs())) {
        s_system_connections.insert(planet->ResourceCenterChangedSignal.connect(
            SidePanel::ResourceCenterChangedSignal));
    }

    for (auto* fleet : objects.findRaw<Fleet>(system->FleetIDs())) {
        s_fleet_state_change_signals[fleet->ID()] = fleet->StateChangedSignal.connect(
            &SidePanel::Update);
    }

    //s_system_connections.insert(s_system->StateChangedSignal.connect(&SidePanel::Update));
    s_system_connections.insert(system->FleetsInsertedSignal.connect(&SidePanel::FleetsInserted));
    s_system_connections.insert(system->FleetsRemovedSignal.connect(&SidePanel::FleetsRemoved));
}

namespace {
    constexpr auto sys_name_cmp = [](const auto& lhs_sv_int, const auto& rhs_sv_int) {
        const std::string_view& lhs = lhs_sv_int.first;
        const std::string_view& rhs = rhs_sv_int.first;

#if defined(FREEORION_MACOSX)
        // Collate on OSX seemingly ignores greek characters, resulting in sort order: X α I, X β I, X α II
        return lhs < rhs;
#else
        using collate_t = std::collate<std::string_view::value_type>;
        const auto& collate = std::use_facet<collate_t>(GetLocale());
        return collate.compare(lhs.data(), lhs.data() + lhs.size(), rhs.data(), rhs.data() + rhs.size()) < 0;
#endif
    };

    auto GetSortedRows(GG::Y system_name_height, const ObjectMap& objects, const auto& is_sys) {
        std::vector<std::pair<std::string_view, int>> sorted_systems;
        sorted_systems.reserve(objects.size<System>());

        for (auto* system : objects.allRaw<const System>() | range_filter(is_sys))
            sorted_systems.emplace_back(system->Name(), system->ID());

        if (!sorted_systems.empty()) // sort by name
            std::stable_sort(sorted_systems.begin(), sorted_systems.end(), sys_name_cmp);

        std::vector<std::shared_ptr<GG::DropDownList::Row>> rows;
        rows.reserve(sorted_systems.size());

        for (auto sys_id : sorted_systems | range_values)
            rows.push_back(GG::Wnd::Create<SystemRow>(sys_id, system_name_height));

        return rows;
    };
}

void SidePanel::RefreshSystemNames(const ObjectMap& objects) {
    // Repopulate the system with all of the names of known systems, if it is closed.
    // If it is open do not change the system names because it runs in a seperate modal
    // from the main UI.
    if (m_system_name->Dropped())
        return;

    m_system_name->Clear();

    {
        const auto system_name_font(GetApp().GetUI().GetBoldFont(SystemNameFontSize()));
        const GG::Y system_name_height(system_name_font->Lineskip() + 4);

        static constexpr auto has_name_or_is_s_system = [](const System* sys)
        { return sys && (!sys->Name().empty() || sys->ID() == s_system_id); };

        // Make a vector of sorted rows and insert them in a single operation.
        m_system_name->Insert(GetSortedRows(system_name_height, objects, has_name_or_is_s_system));
    }

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

void SidePanel::RefreshImpl(GGHumanClientApp& app) {
    ScopedTimer sidepanel_refresh_impl_timer("SidePanel::RefreshImpl", true);
    Sound::TempUISoundDisabler sound_disabler;

    // clear out current contents
    m_planet_panel_container->Clear();
    m_star_type_text->SetText("");
    DetachChildAndReset(m_star_graphic);
    DetachChildAndReset(m_system_resource_summary);

    auto& context = app.GetContext();
    const auto& objects = context.ContextObjects();

    RefreshSystemNames(objects);

    DetachChild(m_star_graphic);

    const auto* system = objects.getRaw<const System>(s_system_id);
    // if no system object, there is nothing to populate with.  early abort.
    if (!system)
        return;

    // (re)create top right star graphic
    if (auto graphic = app.GetUI().GetModuloTexture(
        ClientUI::ArtDir() / "stars_sidepanel",
        ClientUI::StarTypeFilePrefix(system->GetStarType()),
        s_system_id))
    {
        const auto def_width = graphic->DefaultWidth();
        const auto def_height = graphic->DefaultHeight();
        const auto graphic_width = Value(Width()) - MaxPlanetDiameter();

        m_star_graphic = GG::Wnd::Create<GG::DynamicGraphic>(
            GG::X(MaxPlanetDiameter()), GG::Y0, GG::X(graphic_width), GG::Y(graphic_width),
            true, def_width, def_height, 0,
            std::vector<std::shared_ptr<GG::Texture>>{std::move(graphic)},
            GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

        AttachChild(m_star_graphic);
        MoveChildDown(m_star_graphic);
    }

    m_star_type_text->SetText("<s>" + GetStarTypeName(system) + "</s>");

    const auto client_empire_id = app.EmpireID();

    // configure selection of planet panels in panel container
    const auto owned_by_client_empire = [client_empire_id](const UniverseObject* obj) noexcept
    { return (client_empire_id != ALL_EMPIRES) && obj && obj->OwnedBy(client_empire_id); };
    if (m_selection_enabled)
        m_planet_panel_container->SetValidSelectionPredicate(std::function(owned_by_client_empire));
    else
        m_planet_panel_container->ClearValidSelectionPedicate();

    // update planet panel container contents (applying just-set selection predicate)
    //std::cout << " ... setting planet panel container planets" << std::endl;
    const std::vector<int> planet_ids = system->PlanetIDs() | range_to_vec;
    m_planet_panel_container->SetPlanets(planet_ids, system->GetStarType(), context, client_empire_id);


    // populate system resource summary

    // If all planets are owned by the same empire, then we show the Shields/Defense/Troops/Supply;
    // regardless, if there are any planets owned by the player in the system, we show
    // Production/Research/Influnce.
    int all_owner_id = ALL_EMPIRES;
    bool all_planets_share_owner = true;
    std::vector<int> all_planets, player_planets;
    for (const auto* planet : objects.findRaw<const Planet>(planet_ids)) {
        // If it is neither owned nor populated with natives, it can be ignored.
        if (planet->Unowned() && planet->SpeciesName().empty())
            continue;

        const int owner = planet->Owner();
        // If all planets have the same owner as each other, then they must have the same owner
        // as the first planet, so store its owner here when finding the first planet.
        if (all_planets.empty())
            all_owner_id = owner;
        if (owner != all_owner_id)
            all_planets_share_owner = false;

        all_planets.push_back(planet->ID());
        if (owner == client_empire_id)
            player_planets.push_back(planet->ID());
    }

    // Resource meters; show only for player planets
    static constexpr std::array<std::pair<MeterType, MeterType>, 4> resource_meters =
        {{{MeterType::METER_INDUSTRY,  MeterType::METER_TARGET_INDUSTRY},
          {MeterType::METER_RESEARCH,  MeterType::METER_TARGET_RESEARCH},
          {MeterType::METER_INFLUENCE, MeterType::METER_TARGET_INFLUENCE},
          {MeterType::METER_STOCKPILE, MeterType::METER_MAX_STOCKPILE}}};
    // general meters; show only if all planets are owned by same empire
    static constexpr std::array<std::pair<MeterType, MeterType>, 4> general_meters =
        {{{MeterType::METER_SHIELD,  MeterType::METER_MAX_SHIELD},
          {MeterType::METER_DEFENSE, MeterType::METER_MAX_DEFENSE},
          {MeterType::METER_TROOPS,  MeterType::METER_MAX_TROOPS},
          {MeterType::METER_SUPPLY,  MeterType::METER_MAX_SUPPLY}}};

    std::vector<std::pair<MeterType, MeterType>> meter_types;
    meter_types.reserve(8);
    if (!player_planets.empty())
        meter_types.insert(meter_types.end(), resource_meters.begin(), resource_meters.end());
    if (all_planets_share_owner)
        meter_types.insert(meter_types.end(), general_meters.begin(), general_meters.end());


    // refresh the system resource summary.
    m_system_resource_summary = GG::Wnd::Create<MultiIconValueIndicator>(
        Width() - MaxPlanetDiameter() - 8,
        all_planets_share_owner ? std::move(all_planets) : std::move(player_planets),
        std::move(meter_types));
    m_system_resource_summary->MoveTo(GG::Pt(GG::X(MaxPlanetDiameter() + 4),
                                             GG::Y{140} - m_system_resource_summary->Height()));
    AttachChild(m_system_resource_summary);


    // add tooltips and show system resource summary if it is not empty
    if (m_system_resource_summary->Empty()) {
        DetachChild(m_system_resource_summary);
    } else {
        // add tooltips to the system resource summary
        for (const auto type : resource_meters | range_keys) {
            m_system_resource_summary->SetToolTip(type,
                GG::Wnd::Create<SystemResourceSummaryBrowseWnd>(
                    MeterToResource(type), s_system_id, client_empire_id));
        }
        // and the other meters
        for (const auto type : general_meters | range_keys) {
            m_system_resource_summary->SetToolTip(type,
                GG::Wnd::Create<SystemMeterBrowseWnd>(type, s_system_id));
        }

        AttachChild(m_system_resource_summary);
        m_system_resource_summary->Update(objects);
    }
}

void SidePanel::DoLayout() {
    if (m_system_name->CurrentItem() == m_system_name->end()) // no system to render
        return;

    const GG::Y name_height((*m_system_name->CurrentItem())->Height());
    const GG::X button_width{Value(name_height)};

    // left button
    GG::Pt ul(GG::X{MaxPlanetDiameter()} + 2*EDGE_PAD, GG::Y0);
    GG::Pt lr(ul + GG::Pt(button_width, name_height));
    m_button_prev->SizeMove(ul, lr);

    // right button
    ul = GG::Pt(ClientWidth() - button_width - 2*EDGE_PAD, GG::Y0);
    lr = ul + GG::Pt(button_width, name_height);
    m_button_next->SizeMove(ul, lr);

    // system name / droplist
    ul = GG::Pt(GG::X{MaxPlanetDiameter()}, GG::Y0);
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
    if (m_planet_panel_container->NumPanels() > 0)
        m_planet_panel_container->HideScrollbar();
    else
        m_planet_panel_container->ShowScrollbar();

    // resize system resource summary
    if (m_system_resource_summary) {
        ul = GG::Pt(GG::X(EDGE_PAD + 1), PLANET_PANEL_TOP - m_system_resource_summary->Height());
        lr = ul + GG::Pt(ClientWidth() - EDGE_PAD - 1, m_system_resource_summary->Height());
        m_system_resource_summary->SizeMove(ul, lr);
    }
}

void SidePanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void SidePanel::SystemNameDropListOpenedSlot(bool is_open) {
    // Refresh the system names when the drop list closes.
    if (!is_open)
        RefreshSystemNames(GetApp().GetContext().ContextObjects());
}

void SidePanel::SystemSelectionChangedSlot(GG::DropDownList::iterator it) {
    /** This handles cases when the list is dropped and not dropped in the
        same way. Refresh should not update the list of systems if the list
        is open. */
    int system_id = INVALID_OBJECT_ID;
    if (it != m_system_name->end()) {
        if (const auto* sys_row = dynamic_cast<const SystemRow*>(it->get()))
            system_id = sys_row->SystemID();
    }
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

void SidePanel::PlanetClickedSlot(int planet_id, const ObjectMap& objects) const {
    if (m_selection_enabled)
        SelectPlanet(planet_id, objects);
}

void SidePanel::FleetsInserted(std::vector<int> fleets, const ObjectMap& objects) {
    for (auto fleet_id: fleets) {
        if (const auto* fleet = objects.getRaw<Fleet>(fleet_id))
            s_fleet_state_change_signals[fleet_id] = fleet->StateChangedSignal.connect(&SidePanel::Update);
    }
    SidePanel::Update();
}

void SidePanel::FleetsRemoved(std::vector<int> fleets) {
    for (auto fleet_id : fleets)
        s_fleet_state_change_signals.erase(fleet_id);
    SidePanel::Update();
}

bool SidePanel::PlanetSelectable(int planet_id, const ObjectMap& objects) const {
    if (!m_selection_enabled)
        return false;

    auto system = objects.get<System>(s_system_id);
    if (!system)
        return false;

    const auto& planet_ids = system->PlanetIDs();
    if (!planet_ids.contains(planet_id))
        return false;

    auto planet = objects.get<Planet>(planet_id);
    if (!planet)
        return false;

    if (planet->Unowned())
        return false;

    int client_empire_id = GetApp().EmpireID();
    return planet->OwnedBy(client_empire_id);
}

void SidePanel::SelectPlanet(int planet_id, const ObjectMap& objects) {
    if (s_planet_id == planet_id)
        return;

    // Use the first sidepanel with selection enabled to determine if planet is selectable.
    bool planet_selectable = false;
    for (auto& weak_panel : s_side_panels) {
        if (auto panel = weak_panel.lock())
            if (panel->m_selection_enabled) {
                planet_selectable = panel->PlanetSelectable(planet_id, objects);
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

void SidePanel::SetSystem(int system_id, const ObjectMap& objects) {
    if (s_system_id == system_id)
        return;

    auto system = objects.getRaw<const System>(system_id);
    if (!system) {
        s_system_id = INVALID_OBJECT_ID;
        return;
    }

    s_system_id = system_id;

    PlaySidePanelOpenSound();

    // refresh sidepanels
    Refresh();
}

void SidePanel::EnableOrderIssuing(bool enable) {
    m_system_name->EnableOrderIssuing(enable);
    m_planet_panel_container->EnableOrderIssuing(enable);
}
