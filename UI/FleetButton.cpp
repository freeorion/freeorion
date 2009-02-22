#include "FleetButton.h"

#include "../util/AppInterface.h"
#include "../universe/Fleet.h"
#include "FleetWnd.h"
#include "../client/human/HumanClientApp.h"
#include "MapWnd.h"
#include "Sound.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/System.h" 
#include "../Empire/Empire.h"
#include "CUIDrawUtil.h"

#include <GG/DrawUtil.h>

#include <algorithm>


namespace {
    void PlayFleetButtonOpenSound() {Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.fleet-button-click"), true);}
    void PlayFleetButtonRolloverSound() {Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.fleet-button-rollover"), true);}

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
}

///////////////////////////
// FleetButton
///////////////////////////
FleetButton::FleetButton(const std::vector<int>& fleet_IDs, SizeType size_type) :
    GG::Button(GG::X0, GG::Y0, GG::X1, GG::Y1, "", boost::shared_ptr<GG::Font>(), GG::CLR_ZERO),
    m_fleets(),
    m_head_icon(),
    m_size_icon()
{
    Init(fleet_IDs, size_type);
}

FleetButton::FleetButton(int fleet_id, SizeType size_type) :
    GG::Button(GG::X0, GG::Y0, GG::X1, GG::Y1, "", boost::shared_ptr<GG::Font>(), GG::CLR_ZERO),
    m_fleets(),
    m_head_icon(),
    m_size_icon()
{
    std::vector<int> fleet_IDs;
    fleet_IDs.push_back(fleet_id);
    Init(fleet_IDs, size_type);
}

void FleetButton::Init(const std::vector<int>& fleet_IDs, SizeType size_type) {
    // get fleets
    Universe& universe = GetUniverse();
    for (std::vector<int>::const_iterator it = fleet_IDs.begin(); it != fleet_IDs.end(); ++it) {
        Fleet* fleet = universe.Object<Fleet>(*it);
        if (!fleet) {
            Logger().errorStream() << "FleetButton::FleetButton couldn't get fleet with id " << *it;
            continue;
        }
        m_fleets.push_back(fleet);
    }

    // determine owner(s) of fleet(s).  Only care whether or not there is more than one owner, as owner
    // is used to determine colouration
    int owner_id = ALL_EMPIRES;

    for (std::vector<Fleet*>::const_iterator it = m_fleets.begin(); it != m_fleets.end(); ++it) {
        const Fleet* fleet = *it;
        const std::set<int>& fleet_owners = fleet->Owners();

        // not sure how a fleet can have no owners, but ignore this case
        if (fleet_owners.empty())
            continue;

        // check if multiple empires own this fleet
        if (fleet_owners.size() != 1) {
            owner_id = ALL_EMPIRES;
            break;
        }

        // check if owner of this fleet is a second owner of fleets
        int fleet_owner = *(fleet_owners.begin());
        if (owner_id != ALL_EMPIRES && fleet_owner != owner_id) {
            owner_id = ALL_EMPIRES;
            break;
        }

        // default: current fleet owner is first owner found, or is the same as previously found fleet owner
        owner_id = fleet_owner;
    }


    // get fleet colour
    const Empire* empire = Empires().Lookup(owner_id);
    if (!empire)
        SetColor(GG::CLR_WHITE);
    else
        SetColor(empire->Color());


    // select icon(s) for fleet(s)
    if (m_fleets.size() != 1) {
        m_head_icon = FleetHeadIcon(NULL, size_type);
        int num_ships = 0;
        for (std::vector<Fleet*>::const_iterator it = m_fleets.begin(); it != m_fleets.end(); ++it)
            num_ships += (*it)->NumShips();
        m_size_icon = FleetSizeIcon(num_ships, size_type);
    } else {
        const Fleet* fleet = *m_fleets.begin();
        m_head_icon = FleetHeadIcon(fleet, size_type);
        m_size_icon = FleetSizeIcon(fleet, size_type);
    }

    // resize to fit icon by first determining necessary size, and then resizing
    GG::X width(0);
    GG::Y height(0);

    if (m_head_icon) {
        width = m_head_icon->DefaultWidth();
        height = m_head_icon->DefaultHeight();
    }
    if (m_size_icon) {
        width = std::max(width, m_size_icon->DefaultWidth());
        height = std::max(height, m_size_icon->DefaultHeight());
    }

    Resize(GG::Pt(width, height));
}

bool FleetButton::InWindow(const GG::Pt& pt) const {
    // find if cursor is within required distance of centre of icon
    const int RADIUS = Value(Width()) / 2;
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

void FleetButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode())) {
        if (State() != BN_ROLLOVER)
            PlayFleetButtonRolloverSound();
        SetState(BN_ROLLOVER);
    }
}

void FleetButton::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode()))
        PlayFleetButtonOpenSound();
    GG::Button::LClick(pt, mod_keys);
}

void FleetButton::RenderUnpressed() {
    glColor(Color());
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_head_icon)
        m_head_icon->OrthoBlit(ul);
    if (m_size_icon)
        m_size_icon->OrthoBlit(ul);
}

void FleetButton::RenderPressed() {
    const double TWO_PI = 2.0*3.14159;
    glDisable(GL_TEXTURE_2D);
    glColor(Color());
    CircleArc(UpperLeft(), LowerRight(), 0.0, TWO_PI, true);
    glEnable(GL_TEXTURE_2D);

    RenderUnpressed();  // TODO: do something else
}

void FleetButton::RenderRollover() {
    const double TWO_PI = 2.0*3.14159;
    glDisable(GL_TEXTURE_2D);
    glColor(GG::CLR_WHITE);
    CircleArc(UpperLeft(), LowerRight(), 0.0, TWO_PI, true);
    glEnable(GL_TEXTURE_2D);

    RenderUnpressed();  // TODO: do something else
}

/////////////////////
// Free Functions
/////////////////////
boost::shared_ptr<GG::Texture> FleetHeadIcon(const Fleet* fleet, FleetButton::SizeType size_type) {
    if (size_type == FleetButton::FLEET_BUTTON_NONE || size_type == FleetButton::FLEET_BUTTON_TINY)
        return boost::shared_ptr<GG::Texture>();

    // get file name prefix for appropriate size of icon
    std::string size_prefix = FleetIconSizePrefix(size_type);
    if (size_prefix.empty())
        return boost::shared_ptr<GG::Texture>();

    // get file name main part depending on type of fleet
    std::string main_filename = "head-scout.png";
    if (fleet && fleet->HasColonyShips())
        main_filename = "head-colony.png";
    else if (fleet && fleet->HasArmedShips())
        main_filename = "head-warship.png";

    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "fleet" / (size_prefix + main_filename), false);
}

boost::shared_ptr<GG::Texture> FleetSizeIcon(const Fleet* fleet, FleetButton::SizeType size_type) {
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

    return ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "icons" / "fleet", (size_prefix + "tail-"), FleetSizeIconNumber(fleet_size), false);
}