#include "SystemIcon.h"

#include "ClientUI.h"
#include "FleetButton.h"
#include "FleetWnd.h"
#include "MapWnd.h"
#include "Sound.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "../universe/Fleet.h"
#include "../universe/Predicates.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/Species.h"
#include "../universe/Enums.h"
#include "../client/human/HumanClientApp.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/utf8/checked.h>
#include <GG/utf8/core.h>

#include <boost/format.hpp>

namespace {
    bool PlaySounds()                   { return GetOptionsDB().Get<bool>("audio.effects.enabled"); }
    void PlaySystemIconRolloverSound()  { if (PlaySounds()) Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.map.system.icon.rollover.sound.path")); }

    // Wrap content int an rgba tag with color color. Opacity ignored.
    std::string ColorTag(const std::string& content, GG::Clr color){
        boost::format templ("<rgba %d %d %d 255>%s</rgba>");
        return (templ % static_cast<int>(color.r) % static_cast<int>(color.g) % static_cast<int>(color.b) % content).str();
    }

    /// Adds color tags to name_o according to the empires in owner_empire_ids
    std::string ColorNameByOwners(const std::string& name_o, std::set<int>& owner_empire_ids, const EmpireManager& empires) {
        if (owner_empire_ids.size() < 1) {
            return name_o;

        } else if(owner_empire_ids.size() == 1) {
            const Empire* empire = GetEmpire(*owner_empire_ids.begin());
            if (!empire)
                return name_o;
            return ColorTag(name_o, empire->Color());

        } else {
            // We will split the name into pieces.
            // To avoid splitting multi-byte glyphs, we need to convert it to 32-bit form,
            // where a single element always corresponds to a single glyph
            std::vector<utf8::uint32_t> name;
            utf8::utf8to32(name_o.begin(), name_o.end(), std::back_inserter(name));
            const unsigned owner_count = owner_empire_ids.size();
            const unsigned piece_length = name.size() / owner_count;
            unsigned extra = name.size() - owner_count*piece_length; // letters that would be left over
            std::string retval;
            int start = 0;

            for (int empire_id : owner_empire_ids) {
                int current_length = piece_length;
                if (extra > 0) { // Use left over letters as long as we have them
                    ++current_length;
                    --extra;
                }
                // Now we convert a piece of the name back into utf8 and wrap it in tags.
                std::string  piece;
                utf8::utf32to8(name.begin() + start, name.begin() + start + current_length, std::back_inserter(piece));

                GG::Clr empire_clr = ClientUI::TextColor();
                const Empire* empire = GetEmpire(empire_id);
                if (empire)
                    empire_clr = empire->Color();

                retval += ColorTag(piece, empire_clr);
                start += current_length;
            }
            return retval;
        }
    }

    const double        PI = 3.1415926535;
    const unsigned int  MAX_TRIES = 128;     // most allowed unique fleetbutton locations
}

////////////////////////////////////////////////
// OwnerColoredSystemName
////////////////////////////////////////////////
OwnerColoredSystemName::OwnerColoredSystemName(int system_id, int font_size,
                                               bool blank_unexplored_and_none) :
    Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS)
{
    // TODO: Have this make a single call per color.
    // Set up texture coord and vertex buffers (quads) for the glyphs.
    // Consider extending GG::Font to do similar.

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    auto system = GetSystem(system_id);
    if (!system)
        return;

    const auto& known_destroyed_object_ids = GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id);
    if (known_destroyed_object_ids.count(system_id))
        return;

    const SpeciesManager& species_manager = GetSpeciesManager();
    const EmpireManager& empire_manager = Empires();

    // get system name
    const std::string& system_name = system->ApparentName(client_empire_id, blank_unexplored_and_none);

    // loop through planets in system, checking if any are a homeworld, capital
    // or have a shipyard, or have neutral population
    bool capital = false, homeworld = false, has_shipyard = false, has_neutrals = false, has_player_planet = false;

    std::set<int> owner_empire_ids;
    auto system_planets = Objects().FindObjects<const Planet>(system->PlanetIDs());

    for (auto& planet : system_planets) {
        int planet_id = planet->ID();

        if (known_destroyed_object_ids.count(planet_id))
            continue;

        // is planet a capital?
        if (!capital) {
            for (const auto& entry : empire_manager) {
                if (entry.second->CapitalID() == planet_id) {
                    capital = true;
                    break;
                }
            }
        }

        // is planet a homeworld? (for any species)
        if (!homeworld) {
            for (const auto& entry : species_manager) {
                if (const auto& species = entry.second) {
                    const auto& homeworld_ids = species->Homeworlds();
                    if (homeworld_ids.count(planet_id)) {
                        homeworld = true;
                        break;
                    }
                }
            }
        }

        // does planet contain a shipyard?
        if (!has_shipyard) {
            for (auto& building : Objects().FindObjects<const Building>(planet->BuildingIDs())) {
                int building_id = building->ID();

                if (known_destroyed_object_ids.count(building_id))
                    continue;

                if (building->HasTag(TAG_SHIPYARD)) {
                    has_shipyard = true;
                    break;
                }
            }
        }

        // is planet populated by neutral species
        if (!has_neutrals) {
            if (planet->Unowned() && !planet->SpeciesName().empty() && planet->InitialMeterValue(METER_POPULATION) > 0.0f)
                has_neutrals = true;
        }

        // remember if this system has a player-owned planet
        if (!planet->Unowned()) {
            has_player_planet = true;
            owner_empire_ids.insert(planet->Owner());
        }
    }


    // apply formatting tags around planet name to indicate:
    //    Italic for homeworlds
    //    Bold for capital(s)
    //    Underline for shipyard(s), and
    // need to check all empires for homeworld or capitals

    // wrap with formatting tags
    std::string wrapped_system_name = ColorNameByOwners(system_name, owner_empire_ids, empire_manager);
    if (homeworld)
        wrapped_system_name = "<i>" + wrapped_system_name + "</i>";
    if (has_shipyard)
        wrapped_system_name = "<u>" + wrapped_system_name + "</u>";
    std::shared_ptr<GG::Font> font;
    if (capital)
        font = ClientUI::GetBoldFont(font_size);
    else
        font = ClientUI::GetFont(font_size);

    GG::Clr text_color = ClientUI::SystemNameTextColor();
    if (has_player_planet) {
        if (owner_empire_ids.size() == 1)
            text_color = GetEmpire(*owner_empire_ids.begin())->Color();
    } else if (has_neutrals) {
        text_color = ClientUI::TextColor();
    }

    if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
        wrapped_system_name = wrapped_system_name + " (" + std::to_string(system_id) + ")";
    }

    m_text = GG::Wnd::Create<GG::TextControl>(
        GG::X0, GG::Y0, GG::X1, GG::Y1,
        "<s>" + wrapped_system_name + "</s>",
        font, text_color);
}

void OwnerColoredSystemName::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    if (!m_text) {
        ErrorLogger() << "OwnerColoredSystemName::CompleteConstruction had empty m_text";
        return;
    }
    AttachChild(m_text);
    GG::Pt text_size(m_text->TextLowerRight() - m_text->TextUpperLeft());
    m_text->SizeMove(GG::Pt(GG::X0, GG::Y0), text_size);
    Resize(text_size);
}

void OwnerColoredSystemName::Render()
{}

void OwnerColoredSystemName::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Control::SizeMove(ul, lr);

    // Center text
    if (!Children().empty())
        if (GG::TextControl* text = dynamic_cast<GG::TextControl*>(Children().begin()->get())) {
            text->MoveTo(GG::Pt((Width() - text->Width()) / 2, (Height() - text->Height()) / 2));
        }
}

////////////////////////////////////////////////
// SystemIcon
////////////////////////////////////////////////
SystemIcon::SystemIcon(GG::X x, GG::Y y, GG::X w, int system_id) :
    GG::Control(x, y, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_system_id(system_id),
    m_tiny_graphic(nullptr),
    m_selection_indicator(nullptr),
    m_tiny_selection_indicator(nullptr),
    m_mouseover_indicator(nullptr),
    m_mouseover_unexplored_indicator(nullptr),
    m_tiny_mouseover_indicator(nullptr),
    m_selected(false),
    m_colored_name(nullptr),
    m_showing_name(false)
{}

void SystemIcon::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    ClientUI* ui = ClientUI::GetClientUI();
    if (auto system = GetSystem(m_system_id)) {
        StarType star_type = system->GetStarType();
        m_disc_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              ClientUI::StarTypeFilePrefixes()[star_type],
                                              m_system_id);
        m_halo_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              ClientUI::HaloStarTypeFilePrefixes()[star_type],
                                              m_system_id);
        m_tiny_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              "tiny_" + ClientUI::StarTypeFilePrefixes()[star_type],
                                              m_system_id);
    } else {
        m_disc_texture = ui->GetTexture(ClientUI::ArtDir() / "misc" / "missing.png");
        m_halo_texture = m_disc_texture;
        m_tiny_texture = m_disc_texture;
    }

    m_tiny_graphic = GG::Wnd::Create<GG::StaticGraphic>(m_tiny_texture);
    m_tiny_graphic->Resize(GG::Pt(m_tiny_texture->Width(), m_tiny_texture->Height()));
    AttachChild(m_tiny_graphic);
    m_tiny_graphic->Hide();

    // selection indicator graphic
    const std::shared_ptr<GG::Texture>& texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_selection" / "system_selection2.png", true);

    GG::X texture_width = texture->DefaultWidth();
    GG::Y texture_height = texture->DefaultHeight();
    m_selection_indicator = GG::Wnd::Create<RotatingGraphic>(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_selection_indicator->SetRPM(ClientUI::SystemSelectionIndicatorRPM());
    AttachChild(m_selection_indicator);
    m_selection_indicator->Resize(GG::Pt(texture_width, texture_height));

    // tiny selection indicator graphic
    const std::shared_ptr<GG::Texture>& tiny_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_selection_tiny" / "system_selection_tiny2.png", true);
    texture_width = tiny_texture->DefaultWidth();
    texture_height = tiny_texture->DefaultHeight();
    m_tiny_selection_indicator = GG::Wnd::Create<RotatingGraphic>(tiny_texture, GG::GRAPHIC_NONE);
    m_tiny_selection_indicator->SetRPM(ClientUI::SystemSelectionIndicatorRPM());
    AttachChild(m_tiny_selection_indicator);
    m_tiny_selection_indicator->Resize(GG::Pt(texture_width, texture_height));

    // mouseover indicator graphic
    const std::shared_ptr<GG::Texture> mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover.png");
    texture_width = mouseover_texture->DefaultWidth();
    texture_height = mouseover_texture->DefaultHeight();
    m_mouseover_indicator = GG::Wnd::Create<GG::StaticGraphic>(mouseover_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_mouseover_indicator->Resize(GG::Pt(texture_width, texture_height));

    // unexplored mouseover indicator graphic
    std::shared_ptr<GG::Texture> unexplored_mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover_unexplored.png");
    texture_width = unexplored_mouseover_texture->DefaultWidth();
    texture_height = unexplored_mouseover_texture->DefaultHeight();
    m_mouseover_unexplored_indicator = GG::Wnd::Create<GG::StaticGraphic>(unexplored_mouseover_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_mouseover_unexplored_indicator->Resize(GG::Pt(texture_width, texture_height));

    // tiny mouseover indicator graphic
    std::shared_ptr<GG::Texture> tiny_mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover_tiny.png");
    texture_width = tiny_mouseover_texture->DefaultWidth();
    texture_height = tiny_mouseover_texture->DefaultHeight();
    m_tiny_mouseover_indicator = GG::Wnd::Create<GG::StaticGraphic>(tiny_mouseover_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_tiny_mouseover_indicator->Resize(GG::Pt(texture_width, texture_height));

    Refresh();
}

SystemIcon::~SystemIcon()
{}

int SystemIcon::SystemID() const
{ return m_system_id; }

const std::shared_ptr<GG::Texture>& SystemIcon::DiscTexture() const
{ return m_disc_texture; }

const std::shared_ptr<GG::Texture>& SystemIcon::HaloTexture() const
{ return m_halo_texture; }

const std::shared_ptr<GG::Texture>& SystemIcon::TinyTexture() const
{ return m_tiny_texture; }

GG::Pt SystemIcon::NthFleetButtonUpperLeft(unsigned int button_number, bool moving) const {
    if (button_number < 1) {
        ErrorLogger() << "SystemIcon::NthFleetButtonUpperLeft passed button number less than 1... treating as if = 1";
        button_number = 1;
    } else if (button_number >= MAX_TRIES) {
        ErrorLogger() << "SystemIcon::NthFleetButtonUpperLeft passed too large a button number.  treating as if = 1";
        button_number = 1;
    }


    // get fleetbutton radius to use for layout
    FleetButton::SizeType fb_size_type = ClientUI::GetClientUI()->GetMapWnd()->FleetButtonSizeType();
    GG::Pt button_size = GG::Pt();
    double FB_RADIUS = 0.0;
    const std::shared_ptr<GG::Texture>& icon = FleetSizeIcon(1u, fb_size_type);
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
{ return static_cast<const int>(Value(Width()) * GetOptionsDB().Get<double>("ui.map.system.circle.size")) + 1; }

void SystemIcon::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    Wnd::SizeMove(ul, lr);

    const bool USE_TINY_GRAPHICS = Value(Width()) < GetOptionsDB().Get<int>("ui.map.system.icon.tiny.threshold");
    GG::Pt middle = GG::Pt(Width() / 2, Height() / 2);

    // tiny graphic?
    if (m_tiny_graphic && USE_TINY_GRAPHICS) {
        const GG::SubTexture& tiny_texture = m_tiny_graphic->GetTexture();
        const GG::Texture* tiny_texture2 = tiny_texture.GetTexture();
        GG::Pt tiny_size = GG::Pt(tiny_texture2->DefaultWidth(), tiny_texture2->DefaultHeight());
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
        const GG::SubTexture& tiny_texture = m_tiny_selection_indicator->GetTexture();
        const GG::Texture* tiny_texture2 = tiny_texture.GetTexture();
        GG::Pt tiny_sel_ind_size = GG::Pt(tiny_texture2->DefaultWidth(), tiny_texture2->DefaultHeight());
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

    const bool USE_TINY_MOUSEOVER_INDICATOR = m_tiny_mouseover_indicator &&
                                              (Value(Width()) < m_tiny_mouseover_indicator->Width());

    // normal mouseover indicator - attach / detach / show / hide done by MouseEnter and MouseLeave
    if (m_mouseover_indicator && !USE_TINY_MOUSEOVER_INDICATOR) {
        GG::Pt mouse_ind_ul(static_cast<GG::X>(middle.x - SEL_IND_SIZE.x / 2.0),
                            static_cast<GG::Y>(middle.y - SEL_IND_SIZE.y / 2.0));
        m_mouseover_indicator->SizeMove(mouse_ind_ul, mouse_ind_ul + SEL_IND_SIZE);
        if (m_mouseover_unexplored_indicator)
            m_mouseover_unexplored_indicator->SizeMove(mouse_ind_ul, mouse_ind_ul + SEL_IND_SIZE);
    } else {
        if (m_mouseover_indicator)
            DetachChild(m_mouseover_indicator);
        if (m_mouseover_unexplored_indicator)
            DetachChild(m_mouseover_unexplored_indicator);
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

void SystemIcon::RenderHalo(double halo_scale_factor) {
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
}

void SystemIcon::RenderDisc() {
    if (m_disc_texture)
        m_disc_texture->OrthoBlit(UpperLeft(), LowerRight());
}

void SystemIcon::RenderOverlay(double zoom_factor) {
    if (!Visible() || !m_overlay_texture)
        return;
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt size = lr - ul;
    GG::Pt half_size = GG::Pt(size.x / 2, size.y / 2);
    GG::Pt middle = ul + half_size;
    GG::Pt overlay_size = GG::Pt(GG::X(static_cast<int>(m_overlay_size * zoom_factor)),
                                 GG::Y(static_cast<int>(m_overlay_size * zoom_factor *
                                                        Value(m_overlay_texture->DefaultHeight()) /
                                                        std::max<double>(1.0, Value(m_overlay_texture->DefaultWidth())))));
    GG::Pt overlay_half_size = GG::Pt(overlay_size.x / 2, overlay_size.y / 2);
    GG::Pt overlay_ul = middle - overlay_half_size;
    GG::Pt overlay_lr = overlay_ul + overlay_size;
    m_overlay_texture->OrthoBlit(overlay_ul, overlay_lr);
}

void SystemIcon::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) LeftClickedSignal(m_system_id); }

void SystemIcon::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) RightClickedSignal(m_system_id, mod_keys); }

void SystemIcon::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) LeftDoubleClickedSignal(m_system_id); }

void SystemIcon::RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) RightDoubleClickedSignal(m_system_id); }

void SystemIcon::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    const bool USE_TINY_MOUSEOVER_INDICATOR = m_tiny_mouseover_indicator &&
                                              (Value(Width()) < m_tiny_mouseover_indicator->Width());
    // indicate mouseover
    if (m_mouseover_indicator && !USE_TINY_MOUSEOVER_INDICATOR) {
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        Empire* this_empire = GetEmpire(client_empire_id);
        bool explored = !this_empire || (this_empire && this_empire->HasExploredSystem(m_system_id)) ||
                !m_mouseover_unexplored_indicator;
        if (explored || !GetOptionsDB().Get<bool>("ui.map.system.unexplored.rollover.enabled")) {
            AttachChild(m_mouseover_indicator);
            MoveChildUp(m_mouseover_indicator);
        } else {
            AttachChild(m_mouseover_unexplored_indicator);
            MoveChildUp(m_mouseover_unexplored_indicator);
        }
    } else {
        if (m_mouseover_indicator)
            DetachChild(m_mouseover_indicator);
        if (m_mouseover_unexplored_indicator)
            DetachChild(m_mouseover_unexplored_indicator);
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

    MouseEnteringSignal(m_system_id, mod_keys);
}

void SystemIcon::MouseLeave() {
    // un-indicate mouseover
    if (m_mouseover_indicator)
        DetachChild(m_mouseover_indicator);
    if (m_mouseover_unexplored_indicator)
        DetachChild(m_mouseover_unexplored_indicator);
    if (m_tiny_mouseover_indicator)
        DetachChild(m_tiny_mouseover_indicator);

    // hide name if not showing by default
    if (!m_showing_name)
        DetachChild(m_colored_name);

    MouseLeavingSignal(m_system_id);
}

void SystemIcon::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void SystemIcon::SetSelected(bool selected) {
    m_selected = selected;
    Resize(Size());
}

void SystemIcon::Refresh() {
    std::string name;
    m_system_connection.disconnect();

    auto system = GetSystem(m_system_id);
    if (system) {
        name = system->Name();
        m_system_connection = system->StateChangedSignal.connect(
            boost::bind(&SystemIcon::Refresh, this), boost::signals2::at_front);
    }

    SetName(name);   // sets GG::Control name.  doesn't affect displayed system name


    // remove existing system name control
    DetachChildAndReset(m_colored_name);

    // create new system name control
    if (!name.empty()) {
        // get font size
        int name_pts = ClientUI::Pts();
        if (const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd()) {
            name_pts = map_wnd->SystemNamePts();
        }

        // create and position
        m_colored_name = GG::Wnd::Create<OwnerColoredSystemName>(m_system_id, name_pts, true);
        PositionSystemName();

        // attach if appropriate, to display
        if (m_showing_name)
            AttachChild(m_colored_name);
    }

    if (system && !system->OverlayTexture().empty())
        m_overlay_texture = ClientUI::GetTexture(ClientUI::ArtDir() / system->OverlayTexture());
    else
        m_overlay_texture.reset();
    if (system)
        m_overlay_size = system->OverlaySize();
}

void SystemIcon::ShowName() {
    m_showing_name = true;
    if (!m_colored_name)
        Refresh();
    else
        AttachChild(m_colored_name);
}

void SystemIcon::HideName() {
    m_showing_name = false;
    DetachChild(m_colored_name);
}

void SystemIcon::PositionSystemName() {
    if (m_colored_name) {
        GG::X name_left = ( Width()  - m_colored_name->Width()                                      )/2;
        GG::Y name_top =  ( Height() + GG::Y(EnclosingCircleDiameter()*2) - m_colored_name->Height())/2;
        m_colored_name->MoveTo(GG::Pt(name_left, name_top));
    }
}

bool SystemIcon::InWindow(const GG::Pt& pt) const {
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
