#include "SystemIcon.h"

#include "ClientUI.h"
#include "FleetButton.h"
#include "FleetWnd.h"
#include "MapWnd.h"
#include "Sound.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "../universe/Predicates.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/Species.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace {
    bool PlaySounds() {return GetOptionsDB().Get<bool>("UI.sound.enabled");}
    void PlaySystemIconRolloverSound() {if (PlaySounds()) Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.system-icon-rollover"));}

    const std::vector<boost::shared_ptr<GG::Texture> >& GetSelectionIndicatorTextures() {
        static std::vector<boost::shared_ptr<GG::Texture> > normal_textures =
            ClientUI::GetClientUI()->GetPrefixedTextures(ClientUI::ArtDir() / "misc" / "system_selection", "system_selection", true);
        return normal_textures;
    }
    const std::vector<boost::shared_ptr<GG::Texture> >& GetSelectionIndicatorTinyTextures() {
        static std::vector<boost::shared_ptr<GG::Texture> > tiny_textures =
            ClientUI::GetClientUI()->GetPrefixedTextures(ClientUI::ArtDir() / "misc" / "system_selection_tiny", "system_selection_tiny", false);
        return tiny_textures;
    }

    const double        PI = 3.1415926535;
    const unsigned int  MAX_TRIES = 128;     // most allowed unique fleetbutton locations
}

////////////////////////////////////////////////
// OwnerColoredSystemName
////////////////////////////////////////////////
OwnerColoredSystemName::OwnerColoredSystemName(int system_id, int font_size, bool blank_unexplored_and_none) :
    Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::Flags<GG::WndFlag>())
{
    // TODO: Have this make a single call per color.
    // Set up texture coord and vertex buffers (quads) for the glyphs.
    // Consider extending GG::Font to do similar.

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    const ObjectMap& objects = GetUniverse().EmpireKnownObjects(client_empire_id);
    const SpeciesManager& species_manager = GetSpeciesManager();
    const EmpireManager& empire_manager = Empires();

    const System* system = objects.Object<System>(system_id);
    if (!system)
        return;

    // get system name
    const std::string& system_name = system->ApparentName(client_empire_id, blank_unexplored_and_none);

    // loop through planets in system, checking if any are a homeworld, capital
    // or have a shipyard, or have neutral population
    bool capital = false, homeworld = false, has_shipyard = false, has_neutrals = false, has_player_planet = false;

    std::vector<const Planet*> system_planets;
    std::vector<const Planet*> planets = objects.FindObjects<Planet>();
    for (std::vector<const Planet*>::const_iterator it = planets.begin(); it != planets.end(); ++it)
        if ((*it)->SystemID() == system_id)
            system_planets.push_back(*it);

    std::set<int> owner_empire_ids;
    for (std::vector<const Planet*>::const_iterator it = system_planets.begin(); it != system_planets.end(); ++it) {
        const Planet* planet = *it;
        int planet_id = planet->ID();

        // is planet a capital?
        if (!capital) {
            for (EmpireManager::const_iterator empire_it = empire_manager.begin(); empire_it != empire_manager.end(); ++empire_it) {
                if (empire_it->second->CapitalID() == planet_id)
                    capital = true;
            }
        }

        // is planet a homeworld? (for any species)
        if (!homeworld) {
            for (SpeciesManager::iterator species_it = species_manager.begin(); species_it != species_manager.end(); ++species_it) {
                if (const Species* species = species_it->second) {
                    const std::set<int>& homeworld_ids = species->Homeworlds();
                    if (homeworld_ids.find(planet_id) != homeworld_ids.end()) {
                        homeworld = true;
                    }
                }
            }
        }

        // does planet contain a shipyard?
        if (!has_shipyard) {
            const std::set<int>& buildings = planet->Buildings();
            for (std::set<int>::const_iterator building_it = buildings.begin(); building_it != buildings.end(); ++building_it) {
                const Building* building = objects.Object<Building>(*building_it);
                if (!building)
                    continue;
                // annoying hard-coded building name here... not sure how better to deal with it
                if (building->BuildingTypeName() == "BLD_SHIPYARD_BASE") {
                    has_shipyard = true;
                }
            }
        }

        if (!has_neutrals) {
            if (planet->Unowned() && !planet->SpeciesName().empty() && planet->CurrentMeterValue(METER_POPULATION) > 0.0)
                has_neutrals = true;
        }

        if (!has_player_planet) {
            if (!planet->Unowned()) {
                has_player_planet = true;
                owner_empire_ids.insert(planet->Owner());
            }
        }
    }


    // apply formatting tags around planet name to indicate:
    //    Italic for homeworlds
    //    Bold for capital(s)
    //    Underline for shipyard(s), and
    // need to check all empires for homeworld or capitals

    // wrap with formatting tags
    std::string wrapped_system_name = system_name;
    if (homeworld)
        wrapped_system_name = "<i>" + wrapped_system_name + "</i>";
    if (has_shipyard)
        wrapped_system_name = "<u>" + wrapped_system_name + "</u>";
    boost::shared_ptr<GG::Font> font;
    if (capital)
        font = ClientUI::GetBoldFont(font_size);
    else
        font = ClientUI::GetFont(font_size);

    GG::Clr text_color = ClientUI::SystemNameTextColor();
    if (has_player_planet) {
        if (owner_empire_ids.size() == 1)
            text_color = Empires().Lookup(*owner_empire_ids.begin())->Color();
    } else if (has_neutrals) {
            text_color = ClientUI::TextColor();
    }

    GG::TextControl* text = new ShadowedTextControl(GG::X0, GG::Y0, wrapped_system_name, font, text_color);
    AttachChild(text);
    Resize(GG::Pt(text->Width(), text->Height()));
}

void OwnerColoredSystemName::Render()
{}

////////////////////////////////////////////////
// SystemIcon
////////////////////////////////////////////////
SystemIcon::SystemIcon(GG::X x, GG::Y y, GG::X w, int system_id) :
    GG::Control(x, y, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_system_id(system_id),
    m_tiny_graphic(0),
    m_selection_indicator(0),
    m_tiny_selection_indicator(0),
    m_mouseover_indicator(0),
    m_tiny_mouseover_indicator(0),
    m_selected(false),
    m_colored_name(0),
    m_showing_name(false)
{
    ClientUI* ui = ClientUI::GetClientUI();
    if (const System* system = GetEmpireKnownObject<System>(m_system_id, HumanClientApp::GetApp()->EmpireID())) {
        StarType star_type = system->GetStarType();
        m_disc_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              ClientUI::StarTypeFilePrefixes()[star_type],
                                              system_id);
        m_halo_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              ClientUI::HaloStarTypeFilePrefixes()[star_type],
                                              system_id);
        m_tiny_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              "tiny_" + ClientUI::StarTypeFilePrefixes()[star_type],
                                              system_id);
    } else {
        m_disc_texture = ui->GetTexture(ClientUI::ArtDir() / "misc" / "missing.png");
        m_halo_texture = m_disc_texture;
        m_tiny_texture = m_disc_texture;
    }

    if (m_tiny_texture)
        m_tiny_graphic = new GG::StaticGraphic(GG::X0, GG::Y0, m_tiny_texture->Width(), m_tiny_texture->Height(),
                                               m_tiny_texture);
    AttachChild(m_tiny_graphic);
    m_tiny_graphic->Hide();

    // selection indicator graphic
    const std::vector<boost::shared_ptr<GG::Texture> >& textures = GetSelectionIndicatorTextures();
    GG::X texture_width = textures.at(0)->DefaultWidth();
    GG::Y texture_height = textures.at(0)->DefaultHeight();
    m_selection_indicator = new GG::DynamicGraphic(GG::X0, GG::Y0,
                                                   texture_width, texture_height, true,
                                                   texture_width, texture_height, 0, textures,
                                                   GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_selection_indicator->SetFPS(ClientUI::SystemSelectionIndicatorFPS());
    AttachChild(m_selection_indicator);
    m_selection_indicator->Play();

    // tiny selection indicator graphic
    const std::vector<boost::shared_ptr<GG::Texture> >& tiny_textures = GetSelectionIndicatorTinyTextures();
    texture_width = tiny_textures.at(0)->DefaultWidth();
    texture_height = tiny_textures.at(0)->DefaultHeight();
    m_tiny_selection_indicator = new GG::DynamicGraphic(GG::X0, GG::Y0,
                                                        texture_width, texture_height, true,
                                                        texture_width, texture_height, 0, tiny_textures,
                                                        GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_tiny_selection_indicator->SetFPS(ClientUI::SystemSelectionIndicatorFPS());
    AttachChild(m_tiny_selection_indicator);
    m_tiny_selection_indicator->Play();

    // mouseover indicator graphic
    boost::shared_ptr<GG::Texture> mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover.png");
    texture_width = mouseover_texture->DefaultWidth();
    texture_height = mouseover_texture->DefaultHeight();
    m_mouseover_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, texture_width, texture_height,
                                                  mouseover_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    // tiny mouseover indicator graphic
    boost::shared_ptr<GG::Texture> tiny_mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover_tiny.png");
    texture_width = tiny_mouseover_texture->DefaultWidth();
    texture_height = tiny_mouseover_texture->DefaultHeight();
    m_tiny_mouseover_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, texture_width,
                                                       texture_height, tiny_mouseover_texture,
                                                       GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    Refresh();
}

SystemIcon::~SystemIcon()
{
    delete m_tiny_graphic;
    delete m_selection_indicator;
    delete m_tiny_selection_indicator;
    delete m_mouseover_indicator;
    delete m_tiny_mouseover_indicator;
    delete m_colored_name;
}

int SystemIcon::SystemID() const
{ return m_system_id; }

const boost::shared_ptr<GG::Texture>& SystemIcon::DiscTexture() const
{ return m_disc_texture; }

const boost::shared_ptr<GG::Texture>& SystemIcon::HaloTexture() const
{ return m_halo_texture; }

const boost::shared_ptr<GG::Texture>& SystemIcon::TinyTexture() const
{ return m_tiny_texture; }

GG::Pt SystemIcon::NthFleetButtonUpperLeft(unsigned int button_number, bool moving) const
{
    if (button_number < 1) {
        Logger().errorStream() << "SystemIcon::NthFleetButtonUpperLeft passed button number less than 1... treating as if = 1";
        button_number = 1;
    } else if (button_number >= MAX_TRIES) {
        Logger().errorStream() << "SystemIcon::NthFleetButtonUpperLeft passed too large a button number.  treating as if = 1";
        button_number = 1;
    }


    // get fleetbutton radius to use for layout
    FleetButton::SizeType fb_size_type = ClientUI::GetClientUI()->GetMapWnd()->FleetButtonSizeType();
    GG::Pt button_size = GG::Pt();
    double FB_RADIUS = 0.0;
    const boost::shared_ptr<GG::Texture>& icon = FleetSizeIcon(1u, fb_size_type);
    if (icon) {
        button_size = GG::Pt(icon->DefaultWidth(), icon->DefaultHeight());
        FB_RADIUS = static_cast<double>(Value(button_size.x) + Value(button_size.y))/4.0;   // average half of two side lengths to get radius
    }


    // pick an angle and radius for where to put (centre of) icon
    const double SYSTEM_RADIUS = static_cast<double>(EnclosingCircleDiameter()) / 2.0;
    double button_angle = PI/4.0;
    double arc_radius = std::max(SYSTEM_RADIUS + FB_RADIUS, 1.0);


    // loop through shells, trying to find a place where a fleet icon will fit
    int buttons_in_previous_shells = 0;
    unsigned int tries = 0;
    double first_fb_angle = PI/4.0;

    while (tries < MAX_TRIES) {
        //std::cout << std::endl << std::endl << "System: " << m_system.Name() << " button_number: " << button_number << std::endl;
        //std::cout << "arc_radius: " << arc_radius << std::endl;
        //std::cout << "FB_RADIUS: " << FB_RADIUS << std::endl;

        // determine how many buttons will fit in this shell
        double half_fb_arc_angle = FB_RADIUS/arc_radius;  // small angle approximation theta ~= tan(theta) = opposite / adjacent
        double min = 0.0    + half_fb_arc_angle;
        double max = PI/2.0 - half_fb_arc_angle;
        double angle_range = max - min;
        int max_buttons_at_current_radius = std::max(1, static_cast<int>(floor(angle_range / (half_fb_arc_angle * 2.0))));

        //std::cout << "90 degres: " << PI/2.0 << std::endl;
        //std::cout << "half_fb_arc_angle*2.0: " << half_fb_arc_angle * 2.0 << std::endl;
        //std::cout << "angle max: " << max << " min: " << min << " range: " << angle_range << std::endl;

        //std::cout << "max buttons in shell: " << max_buttons_at_current_radius << std::endl;

        // can button fit in this shell?
        int button_number_in_shell = button_number - buttons_in_previous_shells;
        //std::cout << "button_number_in_shell: " << button_number_in_shell << std::endl;

        if (button_number_in_shell > max_buttons_at_current_radius) {
            // button can't fit in this shell, so jump up to the next bigger shell
            arc_radius += 2.0*FB_RADIUS;
            buttons_in_previous_shells += max_buttons_at_current_radius;
            ++tries;
            continue;
        }


        // centre buttons in this shell.  if odd numbered, first can go at PI/4.0.  If even numbered,
        // need to offset first in shell by half a radius' worth of angle
        if (max_buttons_at_current_radius % 2 == 0)
            first_fb_angle += half_fb_arc_angle;


        unsigned int offset_steps = button_number_in_shell - 1;
        // is button at a higher or lower angle than 45 degree place of first button in each shell?
        if (offset_steps == 0) {
             button_angle = first_fb_angle;
        } else if (offset_steps % 2 == 0) {
            button_angle = first_fb_angle + half_fb_arc_angle*(offset_steps);
        } else {
            button_angle = first_fb_angle - half_fb_arc_angle*(offset_steps + 1);
        }
        break;
    }


    // position at top left of system icon (relative)
    GG::Pt retval = GG::Pt();
    if (moving)
        button_angle += PI / 2.0;

    GG::X button_centre_x = GG::X(static_cast<int>( std::cos(button_angle) * arc_radius));
    GG::Y button_centre_y = GG::Y(static_cast<int>(-std::sin(button_angle) * arc_radius));

    // reposition at centre of system icon
    retval += GG::Pt(Width()/2, Height()/2);

    // reposition at centre of fleet icon
    retval += GG::Pt(button_centre_x, button_centre_y);

    // reposition at top-left of fleet icon
    retval -= GG::Pt(button_size.x/2, button_size.y/2);

    return retval;
}

int SystemIcon::EnclosingCircleDiameter() const
{ return static_cast<const int>(Value(Width()) * GetOptionsDB().Get<double>("UI.system-circle-size")) + 1; }

void SystemIcon::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);

    const bool USE_TINY_GRAPHICS = Value(Width()) < GetOptionsDB().Get<int>("UI.system-tiny-icon-size-threshold");
    GG::Pt middle = GG::Pt(Width() / 2, Height() / 2);

    // tiny graphic?
    if (m_tiny_graphic && USE_TINY_GRAPHICS) {
        GG::Pt tiny_size = m_tiny_graphic->Size();
        GG::Pt tiny_ul(static_cast<GG::X>(middle.x - tiny_size.x / 2.0),
                       static_cast<GG::Y>(middle.y - tiny_size.y / 2.0));
        m_tiny_graphic->SizeMove(tiny_ul, tiny_ul + tiny_size);
        AttachChild(m_tiny_graphic);
        m_tiny_graphic->Show();
    } else if (m_tiny_graphic) {
        DetachChild(m_tiny_graphic);
        m_tiny_graphic->Hide();
    }

    // tiny selection indicator
    if (m_selected && m_tiny_selection_indicator && USE_TINY_GRAPHICS) {
        GG::Pt tiny_sel_ind_size = m_tiny_selection_indicator->Size();
        GG::Pt tiny_sel_ind_ul(static_cast<GG::X>(middle.x - tiny_sel_ind_size.x / 2.0),
                               static_cast<GG::Y>(middle.y - tiny_sel_ind_size.y / 2.0));
        m_tiny_selection_indicator->SizeMove(tiny_sel_ind_ul, tiny_sel_ind_ul + tiny_sel_ind_size);
        AttachChild(m_tiny_selection_indicator);
        m_tiny_selection_indicator->Show();
    } else if (m_tiny_selection_indicator) {
        DetachChild(m_tiny_selection_indicator);
        m_tiny_selection_indicator->Hide();
    }

    const int SEL_IND_WIDTH_HEIGHT = ClientUI::SystemSelectionIndicatorSize() * Value(Width()) / ClientUI::SystemIconSize();
    const GG::Pt SEL_IND_SIZE = GG::Pt(GG::X(SEL_IND_WIDTH_HEIGHT), GG::Y(SEL_IND_WIDTH_HEIGHT));

    // normal selection indicator
    if (m_selected && m_selection_indicator && !USE_TINY_GRAPHICS) {
        GG::Pt sel_ind_ul(static_cast<GG::X>(middle.x - SEL_IND_SIZE.x / 2.0),
                          static_cast<GG::Y>(middle.y - SEL_IND_SIZE.y / 2.0));
        m_selection_indicator->SizeMove(sel_ind_ul, sel_ind_ul + SEL_IND_SIZE);
        AttachChild(m_selection_indicator);
        m_selection_indicator->Show();
    } else if (m_selection_indicator) {
        DetachChild(m_selection_indicator);
        m_selection_indicator->Hide();
    }

    const bool USE_TINY_MOUSEOVER_INDICATOR = Value(Width()) < m_tiny_mouseover_indicator->Width();

    // normal mouseover indicator - attach / detach / show / hide done by MouseEnter and MouseLeave
    if (m_mouseover_indicator && !USE_TINY_MOUSEOVER_INDICATOR) {
        GG::Pt mouse_ind_ul(static_cast<GG::X>(middle.x - SEL_IND_SIZE.x / 2.0),
                            static_cast<GG::Y>(middle.y - SEL_IND_SIZE.y / 2.0));
        m_mouseover_indicator->SizeMove(mouse_ind_ul, mouse_ind_ul + SEL_IND_SIZE);
    } else if (m_mouseover_indicator) {
        DetachChild(m_mouseover_indicator);
    }

    // tiny mouseover indicator - attach / detach / show / hide done by MouseEnter and MouseLeave
    if (m_tiny_mouseover_indicator && USE_TINY_MOUSEOVER_INDICATOR) {
        GG::Pt tiny_mouse_ind_size = m_tiny_mouseover_indicator->Size();
        GG::Pt tiny_mouse_ind_ul(static_cast<GG::X>(middle.x - tiny_mouse_ind_size.x / 2.0),
                                 static_cast<GG::Y>(middle.y - tiny_mouse_ind_size.y / 2.0));
        m_tiny_mouseover_indicator->SizeMove(tiny_mouse_ind_ul, tiny_mouse_ind_ul + tiny_mouse_ind_size);
    } else if (m_tiny_mouseover_indicator) {
        DetachChild(m_tiny_mouseover_indicator);
    }

    Refresh();
}

void SystemIcon::Render()
{}

void SystemIcon::ManualRender(double halo_scale_factor)
{
    if (!Visible())
        return;
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (0.5 < halo_scale_factor && m_halo_texture) {
        GG::Pt size = lr - ul;
        GG::Pt half_size = GG::Pt(size.x / 2, size.y / 2);
        GG::Pt middle = ul + half_size;
        GG::Pt halo_size = GG::Pt(static_cast<GG::X>(size.x * halo_scale_factor),
                                  static_cast<GG::Y>(size.y * halo_scale_factor));
        GG::Pt halo_half_size = GG::Pt(halo_size.x / 2, halo_size.y / 2);
        GG::Pt halo_ul = middle - halo_half_size;
        GG::Pt halo_lr = halo_ul + halo_size;
        m_halo_texture->OrthoBlit(halo_ul, halo_lr);
    }
    if (m_disc_texture)
        m_disc_texture->OrthoBlit(ul, lr);
}

void SystemIcon::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) LeftClickedSignal(m_system_id); }

void SystemIcon::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) RightClickedSignal(m_system_id); }

void SystemIcon::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) LeftDoubleClickedSignal(m_system_id); }

void SystemIcon::RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) RightDoubleClickedSignal(m_system_id); }

void SystemIcon::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    const bool USE_TINY_MOUSEOVER_INDICATOR = Value(Width()) < m_tiny_mouseover_indicator->Width();
    // indicate mouseover
    if (m_mouseover_indicator && !USE_TINY_MOUSEOVER_INDICATOR) {
        AttachChild(m_mouseover_indicator);
        MoveChildUp(m_mouseover_indicator);
    } else if (m_mouseover_indicator) {
        DetachChild(m_mouseover_indicator);
    }
    if (m_tiny_mouseover_indicator && USE_TINY_MOUSEOVER_INDICATOR) {
        AttachChild(m_tiny_mouseover_indicator);
        MoveChildUp(m_tiny_mouseover_indicator);
    } else if (m_tiny_mouseover_indicator) {
        DetachChild(m_tiny_mouseover_indicator);
    }

    // show system name if not by default
    if (!m_showing_name)
        AttachChild(m_colored_name);

    PlaySystemIconRolloverSound();

    MouseEnteringSignal(m_system_id);
}

void SystemIcon::MouseLeave()
{
    // un-indicate mouseover
    if (m_mouseover_indicator)
        DetachChild(m_mouseover_indicator);
    if (m_tiny_mouseover_indicator)
        DetachChild(m_tiny_mouseover_indicator);

    // hide name if not showing by default
    if (!m_showing_name)
        DetachChild(m_colored_name);

    MouseLeavingSignal(m_system_id);
}

void SystemIcon::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void SystemIcon::SetSelected(bool selected)
{
    m_selected = selected;
    Resize(Size());
}

void SystemIcon::Refresh()
{
    std::string name;
    m_system_connection.disconnect();

    const System* system = GetObject<System>(m_system_id);
    if (!system)
        system = GetEmpireKnownObject<System>(m_system_id, HumanClientApp::GetApp()->EmpireID());
    if (system) {
        name = system->Name();
        m_system_connection = GG::Connect(system->StateChangedSignal,   &SystemIcon::Refresh,   this,   boost::signals::at_front);
    }

    SetName(name);   // sets GG::Control name.  doesn't affect displayed system name


    // remove existing system name control
    delete m_colored_name;  m_colored_name = 0;

    // create new system name control
    if (!name.empty()) {
        // get font size
        int name_pts = ClientUI::Pts();
        if (const MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd()) {
            name_pts = map_wnd->SystemNamePts();
        }

        // create and position
        m_colored_name = new OwnerColoredSystemName(m_system_id, name_pts, true);
        PositionSystemName();

        // attach if appropriate, to display
        if (m_showing_name)
            AttachChild(m_colored_name);
    }
}

void SystemIcon::ShowName()
{
    m_showing_name = true;
    if (!m_colored_name)
        Refresh();
    else
        AttachChild(m_colored_name);
}

void SystemIcon::HideName()
{
    m_showing_name = false;
    DetachChild(m_colored_name);
}

void SystemIcon::PositionSystemName()
{
    if (m_colored_name) {
        GG::X name_left = ( Width()  - m_colored_name->Width()          )/2;
        GG::Y name_top =  ( Height() + GG::Y(EnclosingCircleDiameter()) )/2;
        m_colored_name->MoveTo(GG::Pt(name_left, name_top));
    }
}

bool SystemIcon::InWindow(const GG::Pt& pt) const
{
    // find if cursor is within required distance of centre of icon
    const int RADIUS = EnclosingCircleDiameter() / 2;
    const int RADIUS2 = RADIUS*RADIUS;

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt size = lr - ul;
    GG::Pt half_size = GG::Pt(size.x / 2, size.y / 2);
    GG::Pt middle = ul + half_size;

    GG::Pt delta = pt - middle;

    const int distx = Value(delta.x);
    const int disty = Value(delta.y);

    return distx*distx + disty*disty <= RADIUS2;
}
