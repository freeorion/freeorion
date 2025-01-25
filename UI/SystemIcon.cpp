#include "SystemIcon.h"

#include "ClientUI.h"
#include "FleetButton.h"
#include "FleetWnd.h"
#include "MapWnd.h"
#include "Sound.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "../universe/Fleet.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/Species.h"
#include "../client/human/GGHumanClientApp.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/ranges.h"
#include "../Empire/Empire.h"

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
    std::string ColorNameByOwners(const std::string& name_o,
                                  std::set<int>& owner_empire_ids,
                                  const EmpireManager& empires)
    {
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
            const auto owner_count = owner_empire_ids.size();
            const auto piece_length = name.size() / owner_count;
            auto extra = name.size() - owner_count*piece_length; // letters that would be left over
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
                if (auto empire = empires.GetEmpire(empire_id))
                    empire_clr = empire->Color();

                retval += ColorTag(piece, empire_clr);
                start += current_length;
            }
            return retval;
        }
    }

    constexpr double       PI = 3.1415926535;
    constexpr unsigned int MAX_TRIES = 128;     // most allowed unique fleetbutton locations
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

    int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const EmpireManager& empire_manager = context.Empires();
    const Universe& universe = context.ContextUniverse();
    const ObjectMap& objects = context.ContextObjects();
    const SpeciesManager& species_manager = context.species;


    auto system = objects.get<System>(system_id);
    if (!system)
        return;

    const auto& known_destroyed_object_ids = universe.EmpireKnownDestroyedObjectIDs(client_empire_id);
    if (known_destroyed_object_ids.contains(system_id))
        return;


    // get system name
    std::string system_name = system->ApparentName(
        client_empire_id, universe, blank_unexplored_and_none);

    // loop through planets in system, checking if any are a homeworld, capital
    // or have a shipyard, or have neutral population
    bool capital = false, homeworld = false, has_shipyard = false,
         has_neutrals = false, has_player_planet = false;

    std::set<int> owner_empire_ids;
    auto system_planets = objects.find<const Planet>(system->PlanetIDs());

    for (auto& planet : system_planets) {
        int planet_id = planet->ID();

        if (known_destroyed_object_ids.contains(planet_id))
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
            const auto is_homeworld = [planet_id](const auto& hw_ids) { return hw_ids.contains(planet_id); };
            homeworld = range_any_of(species_manager.GetSpeciesHomeworldsMap() | range_values,
                                     is_homeworld);
        }

        // does planet contain a shipyard?
        if (!has_shipyard) {
            const auto not_destroyed = [&known_destroyed_object_ids](const auto id)
            { return !known_destroyed_object_ids.contains(id); };

            const auto get_building = [&objects](const auto id) { return objects.getRaw<const Building>(id); };

            static constexpr auto is_shipyard = [](const Building* b) { return b && b->HasTag(TAG_SHIPYARD); };

            has_shipyard = range_any_of(planet->BuildingIDs() | range_filter(not_destroyed) | range_transform(get_building),
                                        is_shipyard);
        }

        // is planet populated by neutral species
        if (!has_neutrals) {
            if (planet->Unowned() && !planet->SpeciesName().empty() && planet->GetMeter(MeterType::METER_POPULATION)->Initial() > 0.0f)
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
    const auto font = capital ? ClientUI::GetBoldFont(font_size) : ClientUI::GetFont(font_size);

    GG::Clr text_color = ClientUI::SystemNameTextColor();
    if (has_player_planet) {
        if (owner_empire_ids.size() == 1) {
            if (const auto* owner_empire = GetEmpire(*owner_empire_ids.begin()))
                text_color = owner_empire->Color();
            else
                DebugLogger() << "OwnerColoredSystemName couldn't get empire with id: " << *owner_empire_ids.begin();
        }
    } else if (has_neutrals) {
        text_color = ClientUI::TextColor();
    }

    if (GetOptionsDB().Get<bool>("ui.name.id.shown"))
        wrapped_system_name = wrapped_system_name + " (" + std::to_string(system_id) + ")";

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
    m_text->SizeMove(GG::Pt0, text_size);
    Resize(text_size);
}

void OwnerColoredSystemName::Render()
{}

void OwnerColoredSystemName::SizeMove(GG::Pt ul, GG::Pt lr) {
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
    m_system_id(system_id)
{}

void SystemIcon::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    ClientUI* ui = ClientUI::GetClientUI();
    if (auto system = Objects().get<System>(m_system_id)) {
        StarType star_type = system->GetStarType();
        m_disc_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              ClientUI::StarTypeFilePrefix(star_type),
                                              m_system_id);
        m_halo_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              ClientUI::HaloStarTypeFilePrefix(star_type),
                                              m_system_id);
        m_tiny_texture = ui->GetModuloTexture(ClientUI::ArtDir() / "stars",
                                              std::string("tiny_").append(ClientUI::StarTypeFilePrefix(star_type)),
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
    auto texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_selection" / "system_selection2.png", true);
    GG::Pt sz{texture->DefaultWidth(), texture->DefaultHeight()};
    m_selection_indicator = GG::Wnd::Create<RotatingGraphic>(
        std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_selection_indicator->SetRPM(static_cast<float>(ClientUI::SystemSelectionIndicatorRPM()));
    AttachChild(m_selection_indicator);
    m_selection_indicator->Resize(sz);

    // tiny selection indicator graphic
    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_selection_tiny" / "system_selection_tiny2.png", true);
    sz = {texture->DefaultWidth(), texture->DefaultHeight()};
    m_tiny_selection_indicator = GG::Wnd::Create<RotatingGraphic>(
        std::move(texture), GG::GRAPHIC_NONE);
    m_tiny_selection_indicator->SetRPM(ClientUI::SystemSelectionIndicatorRPM());
    AttachChild(m_tiny_selection_indicator);
    m_tiny_selection_indicator->Resize(sz);

    // mouseover indicator graphic
    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover.png");
    sz = {texture->DefaultWidth(), texture->DefaultHeight()};
    m_mouseover_indicator = GG::Wnd::Create<GG::StaticGraphic>(
        std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_mouseover_indicator->Resize(sz);

    // unexplored mouseover indicator graphic
    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover_unexplored.png");
    sz = {texture->DefaultWidth(), texture->DefaultHeight()};
    m_mouseover_unexplored_indicator = GG::Wnd::Create<GG::StaticGraphic>(
        std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_mouseover_unexplored_indicator->Resize(sz);

    // tiny mouseover indicator graphic
    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover_tiny.png");
    sz = {texture->DefaultWidth(), texture->DefaultHeight()};
    m_tiny_mouseover_indicator = GG::Wnd::Create<GG::StaticGraphic>(
        std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_tiny_mouseover_indicator->Resize(sz);

    Refresh();
}

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
    auto map_wnd = ClientUI::GetClientUI()->GetMapWndConst();
    FleetButton::SizeType fb_size_type = map_wnd ? map_wnd->FleetButtonSizeType() : FleetButton::SizeType::LARGE;
    GG::Pt button_size = GG::Pt();
    double FB_RADIUS = 0.0;
    if (const auto& icon = FleetSizeIcon(1u, fb_size_type)) {
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

void SystemIcon::SizeMove(GG::Pt ul, GG::Pt lr) {
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
                                              (Width() < m_tiny_mouseover_indicator->Width());

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

void SystemIcon::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) LeftClickedSignal(m_system_id); }

void SystemIcon::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) RightClickedSignal(m_system_id, mod_keys); }

void SystemIcon::LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) LeftDoubleClickedSignal(m_system_id); }

void SystemIcon::RDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ if (!Disabled()) RightDoubleClickedSignal(m_system_id); }

void SystemIcon::MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    const bool USE_TINY_MOUSEOVER_INDICATOR = m_tiny_mouseover_indicator &&
                                              (Width() < m_tiny_mouseover_indicator->Width());
    // indicate mouseover
    if (m_mouseover_indicator && !USE_TINY_MOUSEOVER_INDICATOR) {
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
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
    if (!m_showing_name) {
        // get font size
        int name_pts = ClientUI::Pts();
        if (auto map_wnd = ClientUI::GetClientUI()->GetMapWndConst())
            name_pts = map_wnd->SystemNamePts();
        auto it = m_colored_names.find(name_pts);
        if (it != m_colored_names.end())
            AttachChild(it->second);
    }

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
    if (!m_showing_name) {
        for (auto& pts_name_ptr : m_colored_names)
            DetachChild(pts_name_ptr.second);
    }

    MouseLeavingSignal(m_system_id);
}

void SystemIcon::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void SystemIcon::SetSelected(bool selected) {
    m_selected = selected;
    Resize(Size());
}

void SystemIcon::Refresh() {
    std::string name;
    m_system_connection.disconnect();

    auto system = Objects().get<System>(m_system_id);
    if (system) {
        name = system->Name();
        m_system_connection = system->StateChangedSignal.connect(
            boost::bind(&SystemIcon::Refresh, this), boost::signals2::at_front);
    }

    SetName(name);   // sets GG::Control name.  doesn't affect displayed system name


    // get font size
    int name_pts = ClientUI::Pts();
    if (auto map_wnd = ClientUI::GetClientUI()->GetMapWndConst())
        name_pts = map_wnd->SystemNamePts();

    // remove existing system name control
    for (auto& pts_name_ptrs : m_colored_names) {
        if (!m_showing_name || name.empty() || pts_name_ptrs.first != name_pts)
            DetachChild(pts_name_ptrs.second);
    }

    // create new system name control
    if (m_showing_name && !name.empty()) {
        // create and position
        auto it = m_colored_names.find(name_pts);
        if (it == m_colored_names.end()) {
            it = m_colored_names.emplace(name_pts,
                                         GG::Wnd::Create<OwnerColoredSystemName>(
                                            m_system_id, name_pts, true)).first;
        }

        PositionSystemName(name_pts);
        if (it->second->Parent().get() != this)
            AttachChild(it->second);
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

    // get font size
    int name_pts = ClientUI::Pts();
    if (auto map_wnd = ClientUI::GetClientUI()->GetMapWnd(true))
        name_pts = map_wnd->SystemNamePts();

    auto it = m_colored_names.find(name_pts);
    if (it == m_colored_names.end())
        Refresh();
    else
        AttachChild(it->second);
}

void SystemIcon::HideName() {
    m_showing_name = false;
    for (auto& pts_name_ptr : m_colored_names)
        DetachChild(pts_name_ptr.second);
}

void SystemIcon::PositionSystemName(int pts) {
    auto it = m_colored_names.find(pts);
    if (it == m_colored_names.end())
        return;
    auto& name = *it;
    if (!name.second)
        return;
    GG::X name_left = ( Width() - name.second->Width()                                        )/2;
    GG::Y name_top =  ( Height() + GG::Y(EnclosingCircleDiameter()*2) - name.second->Height() )/2;
    name.second->MoveTo(GG::Pt(name_left, name_top));
}

bool SystemIcon::InWindow(GG::Pt pt) const {
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
