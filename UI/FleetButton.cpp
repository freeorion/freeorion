#include "FleetButton.h"

#include "FleetWnd.h"
#include "MapWnd.h"
#include "Sound.h"
#include "CUIDrawUtil.h"
#include "CUIControls.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../client/human/GGHumanClientApp.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"

#include <GG/GLClientAndServerBuffer.h>

#include <algorithm>


namespace {
    /* returns number of fleet icon size texture to use to represent fleet(s) with the passed number of ships */
    int FleetSizeIconNumber(int number_ships) {
        // one ship (or zero?) has no marker.  more marker levels are used for each doubling in the number of ships
        number_ships = std::max(number_ships, 1); // smallest size indicator is for 1 ship, largest is for 128 or greater
        if (number_ships < 2)
            return 0;
        else if (number_ships < 4)
            return 1;
        else if (number_ships < 8)
            return 2;
        else if (number_ships < 16)
            return 3;
        else if (number_ships < 32)
            return 4;
        else if (number_ships < 64)
            return 5;
        else if (number_ships < 128)
            return 6;
        else  //(number_ships >= 128)
            return 7;
    }

    /* returns prefix of filename used for icons for the indicated fleet button size type */
    const std::string& FleetIconSizePrefix(FleetButton::SizeType size_type) {
        static const std::string EMPTY_STRING;
        static const std::string BIG_STRING{"big-"};
        static const std::string MED_STRING{"med-"};
        static const std::string SML_STRING{"sml-"};

        //std::cout << "FleetIconSizePrefix size_type: " << int(size_type) << std::endl;
        switch (size_type) {
        case FleetButton::SizeType::LARGE:  return BIG_STRING; break;
        case FleetButton::SizeType::MEDIUM: return MED_STRING; break;
        case FleetButton::SizeType::SMALL:  return SML_STRING; break;
        default:                            return EMPTY_STRING;
        }
    }

    void AddOptions(OptionsDB& db) {
        db.Add("ui.map.fleet.select.indicator.size", UserStringNop("OPTIONS_DB_UI_FLEET_SELECTION_INDICATOR_SIZE"), 1.625, RangedStepValidator<double>(0.125, 0.5, 5));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const float TWO_PI = 2.0*3.14159f;
}

///////////////////////////
// FleetButton           //
///////////////////////////
FleetButton::FleetButton(int fleet_id, SizeType size_type) :
    FleetButton(std::vector<int>{fleet_id}, size_type)
{}

FleetButton::FleetButton(std::vector<int> fleet_IDs, SizeType size_type) :
    GG::Button("", nullptr, GG::CLR_ZERO),
    m_fleets(std::move(fleet_IDs)),
    m_size(size_type)
{}

void FleetButton::CompleteConstruction() {
    Button::CompleteConstruction();

    Refresh(m_size);
}

FleetButton::~FleetButton()
{}

void FleetButton::Refresh(SizeType size_type) {
    auto fleets_shared = Objects().find<Fleet>(m_fleets);
    std::vector<const Fleet*> fleets;
    fleets.reserve(fleets_shared.size());
    std::transform(fleets_shared.begin(), fleets_shared.end(), std::back_inserter(fleets),
                   [](auto& f) { return f.get(); });

    // determine owner(s) of fleet(s).  Only care whether or not there is more than one owner, as owner
    // is used to determine colouration
    int multiple_owners = false;
    int owner_id = fleets.empty() ? ALL_EMPIRES : fleets.front()->Owner();
    if (!fleets.empty()) {
        // use ALL_EMPIRES if there are multiple owners
        for (auto& fleet : fleets) {
            if (fleet->Owner() != owner_id) {
                owner_id = ALL_EMPIRES;
                multiple_owners = true;
                break;
            }
        }
    }


    // get fleet colour
    if (multiple_owners) {
        SetColor(GG::CLR_WHITE);

    } else if (owner_id == ALL_EMPIRES) {
        // all ships owned by no empire
        bool monsters = true;
        // find if any ship in fleets in button is not a monster
        for (const auto& fleet : fleets) {
            for (const auto& ship : Objects().find<Ship>(fleet->ShipIDs())) {
                if (!ship->IsMonster()) {
                    monsters = false;
                    break;
                }
            }
            if (!monsters) break;
        }

        SetColor(monsters ? GG::CLR_RED : GG::CLR_WHITE);

    } else {
        // single empire owner
        const Empire* empire = GetEmpire(owner_id);
        SetColor(empire ? empire->Color() : GG::CLR_GRAY);
    }


    // determine direction button should be rotated to orient along a starlane
    GLfloat pointing_angle = 0.0f;
    const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();

    const Fleet* first_fleet = fleets.empty() ? nullptr : fleets.front();

    if (first_fleet &&
        first_fleet->SystemID() == INVALID_OBJECT_ID &&
        first_fleet->NextSystemID() != INVALID_OBJECT_ID)
    {
        int next_sys_id = first_fleet->NextSystemID();
        if (const auto* obj = Objects().get(next_sys_id).get()) {
            // fleet is not in a system and has a valid next destination, so can orient it in that direction
            // fleet icons might not appear on the screen in the exact place corresponding to their
            // actual universe position, but if they're moving along a starlane, this code will assume
            // their apparent position will only be different from their true position in a direction
            // parallel with the starlane, so the direction from their true position to their destination
            // position can be used to get a direction vector to orient the icon
            float dest_x = obj->X(), dest_y = obj->Y();
            float cur_x = first_fleet->X(), cur_y = first_fleet->Y();
            GG::Pt dest = map_wnd->ScreenCoordsFromUniversePosition(dest_x, dest_y);
            GG::Pt cur = map_wnd->ScreenCoordsFromUniversePosition(cur_x, cur_y);
            GG::Pt direction_vector = dest - cur;

            if (direction_vector.x != GG::X0 || direction_vector.y != GG::Y0)
                pointing_angle = 360.0f / TWO_PI * std::atan2(
                    static_cast<float>(Value(direction_vector.y)),
                    static_cast<float>(Value(direction_vector.x))) + 90;
        }
    }


    // select icon(s) for fleet(s)
    int num_ships = 0;
    m_fleet_blockaded = false;
    for (const auto& fleet : fleets) {
        if (fleet) {
            num_ships += fleet->NumShips();
            if (!m_fleet_blockaded && fleet->Blockaded(ScriptingContext{}))
                m_fleet_blockaded = true;
        }
    }

    // remove and re-create graphics for all icons needed
    for (auto& icon : m_icons)
        DetachChild(icon);
    m_icons.clear();
    m_icons.reserve(4);

    if (m_fleet_blockaded) {
        if (auto texture = FleetBlockadedIcon(size_type)) {
            auto icon = GG::Wnd::Create<GG::StaticGraphic>(
                std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            GG::Clr opposite_clr(255 - this->Color().r, 255 - this->Color().g,
                                 255 - this->Color().b, this->Color().a);
            icon->SetColor(opposite_clr);
            m_icons.emplace_back(std::move(icon));
        }
    }

    if (auto texture = FleetSizeIcon(num_ships, size_type)) {
        GG::Pt sz{texture->DefaultWidth(), texture->DefaultHeight()};
        auto icon = GG::Wnd::Create<RotatingGraphic>(
            std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        icon->SetPhaseOffset(pointing_angle);
        icon->SetRPM(0.0f);
        icon->SetColor(this->Color());
        m_icons.emplace_back(std::move(icon));
        Resize(sz);
    }

    for (auto& texture : FleetHeadIcons(fleets, size_type)) {
        GG::Pt sz{texture->DefaultWidth(), texture->DefaultHeight()};
        auto icon = GG::Wnd::Create<RotatingGraphic>(
            std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        icon->SetPhaseOffset(pointing_angle);
        icon->SetRPM(0.0f);
        icon->SetColor(this->Color());
        m_icons.emplace_back(std::move(icon));
        if (Width() < sz.x)
            Resize(sz);
    }


    // set up selection indicator
    if (!m_selection_indicator) {
        m_selection_indicator = GG::Wnd::Create<RotatingGraphic>(
            FleetSelectionIndicatorIcon(), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    }
    m_selection_indicator->SetRPM(ClientUI::SystemSelectionIndicatorRPM());


    LayoutIcons();
    for (auto& icon : m_icons)
        AttachChild(icon);

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    // Scanlines for not currently-visible objects?
    int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES || !GetOptionsDB().Get<bool>("ui.map.scanlines.shown")) {
        DetachChild(m_scanline_control);
        m_scanline_control.reset();
        return;
    }

    if (!m_scanline_control) {
        // Create scanline renderer control, use opposite color of fleet btn
        GG::Clr opposite_clr(255 - Color().r, 255 - Color().g, 255 - Color().b, 64);
        m_scanline_control = GG::Wnd::Create<ScanlineControl>(GG::X0, GG::Y0, Width(), Height(),
                                                              false, opposite_clr);
    }

    bool at_least_one_fleet_visible = false;
    for (int fleet_id : m_fleets) {
        if (GetUniverse().GetObjectVisibilityByEmpire(fleet_id, empire_id) >= Visibility::VIS_BASIC_VISIBILITY) {
            at_least_one_fleet_visible = true;
            break;
        }
    }

    if (!at_least_one_fleet_visible && !m_scanline_control->Parent())
        AttachChild(m_scanline_control);
    else
        DetachChild(m_scanline_control);
}

bool FleetButton::InWindow(const GG::Pt& pt) const {
    // find if cursor is within required distance of centre of icon
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    const float midX = Value(ul.x + lr.x)/2.0f;
    const float midY = Value(ul.y + lr.y)/2.0f;

    const float RADIUS2 = Value(Width())*Value(Width())/4.0f;

    const float ptX = Value(pt.x);
    const float ptY = Value(pt.y);

    const float distx = ptX - midX, disty = ptY - midY;

    return distx*distx + disty*disty <= RADIUS2;
}

void FleetButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode())) {
        if (State() != ButtonState::BN_ROLLOVER)
            PlayFleetButtonRolloverSound();
        SetState(ButtonState::BN_ROLLOVER);
    }
}

void FleetButton::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt sz = Size();

    Button::SizeMove(ul, lr);

    if (sz == Size())
        return;

    LayoutIcons();
}

void FleetButton::LayoutIcons() {
    const ScriptingContext context;

    GG::Pt middle = GG::Pt(Width() / 2, Height() / 2);
    for (auto& graphic : m_icons) {
        GG::SubTexture subtexture = graphic->GetTexture();
        GG::Pt subtexture_sz = GG::Pt(subtexture.Width(), subtexture.Height());
        GG::Pt graphic_ul = middle - GG::Pt(subtexture_sz.x / 2, subtexture_sz.y / 2);
        graphic->SizeMove(graphic_ul, graphic_ul + subtexture_sz);
    }

    if (m_selection_indicator) {
        //GG::SubTexture subtexture = m_selection_indicator->GetTexture();
        //GG::Pt subtexture_sz = GG::Pt(subtexture.Width(), subtexture.Height());
        double sel_ind_scale = GetOptionsDB().Get<double>("ui.map.fleet.select.indicator.size");
        GG::Pt subtexture_sz = Size() * sel_ind_scale;
        GG::Pt graphic_ul = middle - subtexture_sz / 2;
        m_selection_indicator->SizeMove(graphic_ul, graphic_ul + subtexture_sz);
    }

    // refresh fleet button tooltip
    if (m_fleet_blockaded) {
        std::shared_ptr<const Fleet> fleet;
        std::string available_exits;
        int available_exits_count = 0;

        if (!m_fleets.empty())
            // can just pick first fleet because all fleets in system should have same exits
            fleet = context.ContextObjects().get<Fleet>(*m_fleets.begin());
        else
            return;

        for (const auto& target_system_id : context.ContextObjects().get<System>(fleet->SystemID())->StarlanesWormholes()) {
            if (fleet->BlockadedAtSystem(fleet->SystemID(), target_system_id.first, context))
                continue;

            if (auto target_system = context.ContextObjects().get<System>(target_system_id.first).get()) {
                available_exits += "\n" + target_system->ApparentName(GGHumanClientApp::GetApp()->EmpireID());
                available_exits_count++;
            }
        }

        if (fleet->Owner() == ALL_EMPIRES)  // as above, if first fleet of fleet-button is "monster-owned", all fleets are
            SetBrowseText(UserString("FB_TOOLTIP_BLOCKADE_MONSTER"));
        else if (available_exits_count >= 1)
            SetBrowseText(UserString("FB_TOOLTIP_BLOCKADE_WITH_EXIT") + available_exits);
        else
            SetBrowseText(UserString("FB_TOOLTIP_BLOCKADE_NO_EXIT"));
    } else {
        ClearBrowseInfoWnd();
    }
}

void FleetButton::SetSelected(bool selected) {
    m_selected = selected;

    if (!m_selected) {
        DetachChild(m_selection_indicator);
        m_selection_indicator->Hide();
        return;
    }

    AttachChild(m_selection_indicator);
    m_selection_indicator->Show();
    MoveChildDown(m_selection_indicator);
    LayoutIcons();
}

void FleetButton::RenderUnpressed() {
    // GG::Pt ul = UpperLeft(), lr = LowerRight();
    // const float midX = Value(ul.x + lr.x)/2.0f;
    // const float midY = Value(ul.y + lr.y)/2.0f;

    //// debug
    //GG::FlatRectangle(ul, lr, GG::CLR_ZERO, GG::CLR_RED, 2);
    //// end debug
}

void FleetButton::RenderPressed() {
    glDisable(GL_TEXTURE_2D);
    glColor(Color());
    CircleArc(UpperLeft(), LowerRight(), 0.0, TWO_PI, true);
    glEnable(GL_TEXTURE_2D);

    RenderUnpressed();
}

void FleetButton::RenderRollover() {
    glDisable(GL_TEXTURE_2D);
    glColor(GG::CLR_WHITE);
    CircleArc(UpperLeft(), LowerRight(), 0.0, TWO_PI, true);
    glEnable(GL_TEXTURE_2D);

    RenderUnpressed();
}

void FleetButton::PlayFleetButtonRolloverSound()
{ Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.map.fleet.button.rollover.sound.path"), true); }

void FleetButton::PlayFleetButtonOpenSound()
{ Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.map.fleet.button.press.sound.path"), true); }

/////////////////////
// Free Functions
/////////////////////
std::vector<std::shared_ptr<GG::Texture>> FleetHeadIcons(
    const Fleet* fleet,
    FleetButton::SizeType size_type)
{
    return FleetHeadIcons(std::vector<const Fleet*>{fleet}, size_type);
}

std::vector<std::shared_ptr<GG::Texture>> FleetHeadIcons(
    const std::vector<const Fleet*>& fleets,
    FleetButton::SizeType size_type)
{
    //std::cout << "FleetHeadIcons size_type: " << int(size_type) << std::endl;
    std::vector<std::shared_ptr<GG::Texture>> result;

    // get file name prefix for appropriate size of icon
    auto& size_prefix = FleetIconSizePrefix(size_type);
    if (size_prefix.empty())
        return result;

    // the set of fleets is treated like a fleet that contains all the ships
    bool hasColonyShips = false;
    bool hasOutpostShips = false;
    bool hasTroopShips = false;
    bool hasMonsters = false;
    bool hasArmedShips = false;

    for (auto* fleet : fleets) {
        if (!fleet)
            continue;

        hasColonyShips  = hasColonyShips  || fleet->HasColonyShips(Objects());
        hasOutpostShips = hasOutpostShips || fleet->HasOutpostShips(Objects());
        hasTroopShips   = hasTroopShips   || fleet->HasTroopShips(Objects());
        hasMonsters     = hasMonsters     || fleet->HasMonsters(Objects());
        hasArmedShips   = hasArmedShips   || fleet->HasArmedShips(Objects()) || fleet->HasFighterShips(Objects());
    }

    // get file name main part depending on type of fleet
    // symbol type prioritized by the ship type arbitrarily deemed "most important"
    std::vector<std::string> main_filenames;
    main_filenames.reserve(4);
    if (hasMonsters) {
        if (hasArmedShips)   { main_filenames.emplace_back(size_prefix + "head-monster.png"); }
        else                 { main_filenames.emplace_back(size_prefix + "head-monster-harmless.png"); }
    } else {
        if (hasArmedShips)   { main_filenames.emplace_back(size_prefix + "head-warship.png"); }
        if (hasColonyShips)  { main_filenames.emplace_back(size_prefix + "head-colony.png");  }
        if (hasOutpostShips) { main_filenames.emplace_back(size_prefix + "head-outpost.png"); }
        if (hasTroopShips)   { main_filenames.emplace_back(size_prefix + "head-lander.png");  }
    }
    if (main_filenames.empty()) { main_filenames.emplace_back(size_prefix + "head-scout.png"); }

    result.reserve(main_filenames.size());
    for (const std::string& name : main_filenames) {
        auto texture_temp = ClientUI::GetTexture(
            ClientUI::ArtDir() / "icons" / "fleet" / name, false);
        glBindTexture(GL_TEXTURE_2D, texture_temp->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        result.emplace_back(std::move(texture_temp));
    }

    return result;
}

std::shared_ptr<GG::Texture> FleetSizeIcon(const Fleet* fleet, FleetButton::SizeType size_type) {
    if (!fleet)
        return FleetSizeIcon(1u, size_type);
    return FleetSizeIcon(fleet->NumShips(), size_type);
}

std::shared_ptr<GG::Texture> FleetSizeIcon(unsigned int fleet_size, FleetButton::SizeType size_type) {
    if (size_type == FleetButton::SizeType::NONE)
        return nullptr;

    if (size_type == FleetButton::SizeType::TINY) {
        if (fleet_size > 1u)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "fleet" / "tiny-fleet-multi.png", false);
        else
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "fleet" / "tiny-fleet.png", false);
    }

    auto& size_prefix = FleetIconSizePrefix(size_type);
    if (size_prefix.empty())
        return nullptr;

    auto texture_temp = ClientUI::GetClientUI()->GetModuloTexture(
        ClientUI::ArtDir() / "icons" / "fleet",
        size_prefix + "tail-",
        FleetSizeIconNumber(fleet_size), false);
    if (texture_temp) {
        glBindTexture(GL_TEXTURE_2D, texture_temp->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return texture_temp;
}

std::shared_ptr<GG::Texture> FleetBlockadedIcon(FleetButton::SizeType size_type) {
    std::shared_ptr<GG::Texture> retval;
    if (size_type == FleetButton::SizeType::NONE)
        return retval;

    // don't know why, but this crashes when refactored to use ?:
    std::string size_blockade_prefix;
    if (size_type == FleetButton::SizeType::TINY)
        size_blockade_prefix = "tiny-blockade.png";
    else
        size_blockade_prefix = FleetIconSizePrefix(size_type) + "blockade.png";

    retval = ClientUI::GetClientUI()->GetTexture(
        ClientUI::ArtDir() / "icons" / "fleet" / size_blockade_prefix, false);
    if (retval) {
        glBindTexture(GL_TEXTURE_2D, retval->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return retval;
}

std::shared_ptr<GG::Texture> FleetSelectionIndicatorIcon() {
    static std::shared_ptr<GG::Texture> retval;
    if (!retval) {
        retval = ClientUI::GetClientUI()->GetTexture(
            ClientUI::ArtDir() / "icons" / "fleet" / "fleet_selection.png", false);
        glBindTexture(GL_TEXTURE_2D, retval->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    return retval;
}
