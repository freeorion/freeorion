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

#include <GG/DrawUtil.h>

#include <algorithm>


namespace {
    void PlayFleetButtonOpenSound() {Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.fleet-button-click"), true);}
    void PlayFleetButtonRolloverSound() {Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.fleet-button-rollover"), true);}
}

///////////////////////////
// FleetButton
///////////////////////////
FleetButton::FleetButton(const std::vector<int>& fleet_IDs, SizeType size) :
    GG::Button(),
    m_fleets(),
    m_head_icon(),
    m_size_icon()
{
    Init(fleet_IDs, size);
}

FleetButton::FleetButton(int fleet_id, SizeType size) :
    GG::Button(),
    m_fleets(),
    m_head_icon(),
    m_size_icon()
{
    std::vector<int> fleet_ids;
    fleet_ids.push_back(fleet_id);
    Init(fleet_ids, size);
}

void FleetButton::Init(const std::vector<int>& fleet_IDs, SizeType size) {
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


    // set button size
    int button_size = ClientUI::TinyFleetButtonSize();
    if (size == FLEET_BUTTON_SMALL)
        button_size = ClientUI::SmallFleetButtonSize();
    else if (size == FLEET_BUTTON_LARGE)
        button_size = ClientUI::LargeFleetButtonSize();

    Resize(GG::Pt(GG::X(button_size), GG::Y(button_size)));


    // select icon(s) for fleet
    if (m_fleets.size() != 1) {
        m_size_icon.reset();        // use single icon, not head + parts

        if (size == FLEET_BUTTON_TINY)
            m_head_icon = ClientUI::MultiFleetTinyIcon();
        else
            m_head_icon = ClientUI::MultiFleetTinyIcon();   // should not be needed, but just in case

    } else {
        const Fleet* fleet = *m_fleets.begin();
        m_head_icon = ClientUI::FleetTinyIcon(fleet);

        if (size == FLEET_BUTTON_TINY)
            m_size_icon.reset();    // use single icon, not head + parts
        else
            m_size_icon = ClientUI::FleetSizeIcon(fleet);
    }
}

bool FleetButton::InWindow(const GG::Pt& pt) const {
    return GG::Wnd::InWindow(pt);   // could do something fancy to fit better to shape of icon, but square is probably fine, and easier since shape of icon is a function of its texture
}

void FleetButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode())) {
        if (State() != BN_ROLLOVER)
            PlayFleetButtonRolloverSound();
        SetState(BN_ROLLOVER);
    }
}

void FleetButton::RenderUnpressed() {
    glColor(Color());
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_head_icon)
        m_head_icon->OrthoBlit(ul, lr);
    if (m_size_icon)
        m_size_icon->OrthoBlit(ul, lr);
}

void FleetButton::RenderPressed() {
    RenderUnpressed();  // TODO: do something else
}

void FleetButton::RenderRollover() {
    RenderUnpressed();  // TODO: do something else
}
