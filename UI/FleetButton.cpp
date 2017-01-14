#include "FleetButton.h"

#include "FleetWnd.h"
#include "MapWnd.h"
#include "Sound.h"
#include "CUIDrawUtil.h"
#include "CUIControls.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/System.h" 
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/GLClientAndServerBuffer.h>

#include <algorithm>


namespace {
    /* returns number of fleet icon size texture to use to represent fleet(s) with the passed number of ships */
    int FleetSizeIconNumber(int number_ships) {
        // one ship (or zero?) has no marker.  more marker levels are used for each doubling in the number of ships
        number_ships = std::min(std::max(number_ships, 1), 129);    // smallest size indicator is for 1 ship, largest is for 128 or greater
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
    std::string FleetIconSizePrefix(FleetButton::SizeType size_type) {
        if (size_type == FleetButton::FLEET_BUTTON_LARGE)
            return "big-";
        else if (size_type == FleetButton::FLEET_BUTTON_MEDIUM)
            return "med-";
        else if (size_type == FleetButton::FLEET_BUTTON_SMALL)
            return "sml-";
        else
            return "";
    }

    void AddOptions(OptionsDB& db) {
        db.Add("UI.fleet-selection-indicator-size", UserStringNop("OPTIONS_DB_UI_FLEET_SELECTION_INDICATOR_SIZE"), 1.625, RangedStepValidator<double>(0.125, 0.5, 5));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const float TWO_PI = 2.0*3.14159f;
}

///////////////////////////
// FleetButton           //
///////////////////////////
FleetButton::FleetButton(const std::vector<int>& fleet_IDs, SizeType size_type) :
    GG::Button("", boost::shared_ptr<GG::Font>(), GG::CLR_ZERO),
    m_fleets(),
    m_icons(),
    m_selection_indicator(0),
    m_scanline_control(0),
    m_selected(false)
{ Init(fleet_IDs, size_type); }

FleetButton::FleetButton(int fleet_id, SizeType size_type) :
    GG::Button("", boost::shared_ptr<GG::Font>(), GG::CLR_ZERO),
    m_fleets(),
    m_icons(),
    m_selection_indicator(0),
    m_scanline_control(0),
    m_selected(false)
{
    std::vector<int> fleet_IDs;
    fleet_IDs.push_back(fleet_id);
    Init(fleet_IDs, size_type);
}

FleetButton::~FleetButton() {
    DetachChild(m_selection_indicator);
    delete m_selection_indicator;

    if (m_scanline_control) {
        DetachChild(m_scanline_control);
        delete m_scanline_control;
    }
}

void FleetButton::Init(const std::vector<int>& fleet_IDs, SizeType size_type) {
    //std::cout << "FleetButton::Init" << std::endl;

    // get fleets
    std::vector<TemporaryPtr<const Fleet> > fleets;
    for (int fleet_id : fleet_IDs) {
        TemporaryPtr<const Fleet> fleet = GetFleet(fleet_id);
        if (!fleet) {
            ErrorLogger() << "FleetButton::FleetButton couldn't get fleet with id " << fleet_id;
            continue;
        }
        m_fleets.push_back(fleet_id);
        fleets.push_back(fleet);
    }

    // determine owner(s) of fleet(s).  Only care whether or not there is more than one owner, as owner
    // is used to determine colouration
    int owner_id = ALL_EMPIRES;
    int multiple_owners = false;
    if (fleets.empty()) {
        // leave as ALL_EMPIRES
    } else if (fleets.size() == 1) {
        owner_id = (*fleets.begin())->Owner();
    } else {
        owner_id = (*fleets.begin())->Owner();
        // use ALL_EMPIRES if there are multiple owners (including no owner and an owner)
        for (TemporaryPtr<const Fleet> fleet : fleets) {
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
        // all ships owned by now empire
        bool monsters = true;
        // find if any ship in fleets in button is not a monster
        for (TemporaryPtr<const Fleet> fleet : fleets) {
            for (int ship_id : fleet->ShipIDs()) {
                if (TemporaryPtr<const Ship> ship = GetShip(ship_id)) {
                    if (!ship->IsMonster()) {
                        monsters = false;
                        break;
                    }
                }
            }
        }

        if (monsters)
            SetColor(GG::CLR_RED);
        else
            SetColor(GG::CLR_WHITE);
    } else {
        // single empire owner
        if (const Empire* empire = GetEmpire(owner_id))
            SetColor(empire->Color());
        else
            SetColor(GG::CLR_GRAY); // should never be necessary... but just in case
    }


    // determine direction button should be rotated to orient along a starlane
    GLfloat pointing_angle = 0.0f;

    TemporaryPtr<const Fleet> first_fleet;
    if (!m_fleets.empty())
        first_fleet = *fleets.begin();
    if (first_fleet && first_fleet->SystemID() == INVALID_OBJECT_ID && first_fleet->NextSystemID() != INVALID_OBJECT_ID) {
        int next_sys_id = first_fleet->NextSystemID();
        if (TemporaryPtr<const UniverseObject> obj = GetUniverseObject(next_sys_id)) {
            // fleet is not in a system and has a valid next destination, so can orient it in that direction
            // fleet icons might not appear on the screen in the exact place corresponding to their
            // actual universe position, but if they're moving along a starlane, this code will assume
            // their apparent position will only be different from their true position in a direction
            // parallel with the starlane, so the direction from their true position to their destination
            // position can be used to get a direction vector to orient the icon
            float dest_x = obj->X(), dest_y = obj->Y();
            float cur_x = first_fleet->X(), cur_y = first_fleet->Y();
            const MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
            GG::Pt dest = map_wnd->ScreenCoordsFromUniversePosition(dest_x, dest_y);
            GG::Pt cur = map_wnd->ScreenCoordsFromUniversePosition(cur_x, cur_y);
            GG::Pt direction_vector = dest - cur;

            if (direction_vector.x != GG::X0 || direction_vector.y != GG::Y0)
                pointing_angle = 360.0f / TWO_PI * std::atan2(static_cast<float>(Value(direction_vector.y)), static_cast<float>(Value(direction_vector.x))) + 90;
        }
    }


    // select icon(s) for fleet(s)
    int num_ships = 0;
    for (TemporaryPtr<const Fleet> fleet : fleets) {
        if (fleet)
            num_ships += fleet->NumShips();
    }
    boost::shared_ptr<GG::Texture> size_texture = FleetSizeIcon(num_ships, size_type);

    // add RotatingGraphics for all icons needed
    if (size_texture) {
        RotatingGraphic* icon = new RotatingGraphic(size_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        icon->SetPhaseOffset(pointing_angle);
        icon->SetRPM(0.0f);
        icon->SetColor(this->Color());
        m_icons.push_back(icon);
        Resize(GG::Pt(size_texture->DefaultWidth(), size_texture->DefaultHeight()));
        AttachChild(icon);
    }

    for (boost::shared_ptr<GG::Texture> texture : FleetHeadIcons(fleets, size_type)) {
        RotatingGraphic* icon = new RotatingGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        icon->SetPhaseOffset(pointing_angle);
        icon->SetRPM(0.0f);
        icon->SetColor(this->Color());
        m_icons.push_back(icon);
        if (Width() < texture->DefaultWidth())
            Resize(GG::Pt(texture->DefaultWidth(), texture->DefaultHeight()));
        AttachChild(icon);
    }

    // set up selection indicator
    m_selection_indicator = new RotatingGraphic(FleetSelectionIndicatorIcon(), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_selection_indicator->SetRPM(ClientUI::SystemSelectionIndicatorRPM());

    LayoutIcons();

    // Scanlines for not currently-visible objects?
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES || !GetOptionsDB().Get<bool>("UI.system-fog-of-war"))
        return;

    bool at_least_one_fleet_visible = false;
    for (int fleet_id : m_fleets) {
        if (GetUniverse().GetObjectVisibilityByEmpire(fleet_id, empire_id) >= VIS_BASIC_VISIBILITY) {
            at_least_one_fleet_visible = true;
            break;
        }
    }

    // Create scanline renderer control, use opposite color of fleet btn
    GG::Clr opposite_clr(255 - Color().r, 255 - Color().g, 255 - Color().b, 64);
    m_scanline_control = new ScanlineControl(GG::X0, GG::Y0, Width(), Height(), false, opposite_clr);

    if (!at_least_one_fleet_visible)
        AttachChild(m_scanline_control);
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
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode())) {
        if (State() != BN_ROLLOVER)
            PlayFleetButtonRolloverSound();
        SetState(BN_ROLLOVER);
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
    GG::Pt middle = GG::Pt(Width() / 2, Height() / 2);
    for (RotatingGraphic* graphic : m_icons) {
        GG::SubTexture subtexture = graphic->GetTexture();
        GG::Pt subtexture_sz = GG::Pt(subtexture.Width(), subtexture.Height());
        //std::cout << "FleetButton::LayoutIcons repositioning icon: sz: " << subtexture_sz << "  tex: " << subtexture.GetTexture()->Filename() << std::endl;
        GG::Pt graphic_ul = middle - GG::Pt(subtexture_sz.x / 2, subtexture_sz.y / 2);
        graphic->SizeMove(graphic_ul, graphic_ul + subtexture_sz);
    }

    if (m_selection_indicator) {
        //GG::SubTexture subtexture = m_selection_indicator->GetTexture();
        //GG::Pt subtexture_sz = GG::Pt(subtexture.Width(), subtexture.Height());
        double sel_ind_scale = GetOptionsDB().Get<double>("UI.fleet-selection-indicator-size");
        GG::Pt subtexture_sz = Size() * sel_ind_scale;
        GG::Pt graphic_ul = middle - subtexture_sz / 2;
        m_selection_indicator->SizeMove(graphic_ul, graphic_ul + subtexture_sz);
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
{ Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.fleet-button-rollover"), true); }

void FleetButton::PlayFleetButtonOpenSound()
{ Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.fleet-button-click"), true); }

/////////////////////
// Free Functions
/////////////////////
std::vector<boost::shared_ptr<GG::Texture> > FleetHeadIcons(TemporaryPtr<const Fleet> fleet, FleetButton::SizeType size_type) {
    std::vector<TemporaryPtr<const Fleet> > fleets(1U, fleet);
    return FleetHeadIcons(fleets, size_type);
}

std::vector<boost::shared_ptr<GG::Texture> > FleetHeadIcons(const std::vector< TemporaryPtr<const Fleet> >& fleets, FleetButton::SizeType size_type) {
    if (size_type == FleetButton::FLEET_BUTTON_NONE || size_type == FleetButton::FLEET_BUTTON_TINY)
        return std::vector<boost::shared_ptr<GG::Texture> >();

    // get file name prefix for appropriate size of icon
    std::string size_prefix = FleetIconSizePrefix(size_type);
    if (size_prefix.empty())
        return std::vector<boost::shared_ptr<GG::Texture> >();

    // the set of fleets is treated like a fleet that contains all the ships
    bool hasColonyShips = false; bool hasOutpostShips = false; bool hasTroopShips = false; bool hasMonsters = false; bool hasArmedShips = false;
    for (TemporaryPtr<const Fleet> fleet : fleets) {
        if (!fleet)
            continue;

        hasColonyShips  = hasColonyShips  || fleet->HasColonyShips();
        hasOutpostShips = hasOutpostShips || fleet->HasOutpostShips();
        hasTroopShips   = hasTroopShips   || fleet->HasTroopShips();
        hasMonsters     = hasMonsters     || fleet->HasMonsters();
        hasArmedShips   = hasArmedShips   || fleet->HasArmedShips() || fleet->HasFighterShips();
    }

    // get file name main part depending on type of fleet
    // symbol type prioritized by the ship type arbitrarily deemed "most important"
    std::vector<std::string> main_filenames;
    if (hasMonsters) {
        if (hasArmedShips)   { main_filenames.push_back("head-monster.png"); }
        else                 { main_filenames.push_back("head-monster-harmless.png"); }
    } else {
        if (hasArmedShips)   { main_filenames.push_back("head-warship.png"); }
        if (hasColonyShips)  { main_filenames.push_back("head-colony.png");  }
        if (hasOutpostShips) { main_filenames.push_back("head-outpost.png"); }
        if (hasTroopShips)   { main_filenames.push_back("head-lander.png");  }
    }
    if (main_filenames.empty()) { main_filenames.push_back("head-scout.png"); }

    std::vector<boost::shared_ptr<GG::Texture> > result;
    for (const std::string& name : main_filenames) {
        boost::shared_ptr<GG::Texture> texture_temp = ClientUI::GetTexture(
            ClientUI::ArtDir() / "icons" / "fleet" / (size_prefix + name), false);
        glBindTexture(GL_TEXTURE_2D, texture_temp->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        result.push_back(texture_temp);
    }

    return result;
}

boost::shared_ptr<GG::Texture> FleetSizeIcon(TemporaryPtr<const Fleet> fleet, FleetButton::SizeType size_type) {
    if (!fleet)
        return FleetSizeIcon(1u, size_type);
    return FleetSizeIcon(fleet->NumShips(), size_type);
}

boost::shared_ptr<GG::Texture> FleetSizeIcon(unsigned int fleet_size, FleetButton::SizeType size_type) {
    if (fleet_size < 1u)
        fleet_size = 1u; // because there's no zero-ship icon, and the one-ship icon is (as of this writing) blank, so is fitting for zero ships

    if (size_type == FleetButton::FLEET_BUTTON_NONE)
        return boost::shared_ptr<GG::Texture>();

    if (size_type == FleetButton::FLEET_BUTTON_TINY) {
        if (fleet_size > 1u)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "fleet" / "tiny-fleet-multi.png", false);
        else
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "fleet" / "tiny-fleet.png", false);
    }

    std::string size_prefix = FleetIconSizePrefix(size_type);

    if (size_prefix.empty())
        return boost::shared_ptr<GG::Texture>();

    boost::shared_ptr<GG::Texture> texture_temp = ClientUI::GetClientUI()->GetModuloTexture(
        ClientUI::ArtDir() / "icons" / "fleet", (size_prefix + "tail-"), FleetSizeIconNumber(fleet_size), false);
    glBindTexture(GL_TEXTURE_2D, texture_temp->OpenGLId());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture_temp;
}

boost::shared_ptr<GG::Texture> FleetSelectionIndicatorIcon() {
    static boost::shared_ptr<GG::Texture> retval;
    if (!retval) {
        retval = ClientUI::GetClientUI()->GetTexture(ClientUI::ArtDir() / "icons" / "fleet" / "fleet_selection.png", false);
        glBindTexture(GL_TEXTURE_2D, retval->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    return retval;
}
