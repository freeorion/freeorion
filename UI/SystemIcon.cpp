#include "SystemIcon.h"

#include "ClientUI.h"
#include "../universe/Fleet.h"
#include "FleetButton.h"
#include "FleetWnd.h"
#include "../client/human/HumanClientApp.h"
#include "MapWnd.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Predicates.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>


////////////////////////////////////////////////
// OwnerColoredSystemName
////////////////////////////////////////////////
OwnerColoredSystemName::OwnerColoredSystemName(const System* system, const boost::shared_ptr<GG::Font>& font, const std::string& format_text/* = ""*/, Uint32 flags/* = 0*/) :
    Control(0, 0, 1, 1, flags)
{
    std::string str = format_text == "" ? system->Name() : boost::io::str(boost::format(format_text) % system->Name());
    int width = 0;
    const std::set<int>& owners = system->Owners();
    if (owners.size() <= 1) {
        GG::Clr text_color = ClientUI::TextColor();
        if (!owners.empty())
            text_color = Empires().Lookup(*owners.begin())->Color();
        GG::TextControl* text = new GG::TextControl(width, 0, str, font, text_color);
        m_subcontrols.push_back(text);
        AttachChild(m_subcontrols.back());
        width += m_subcontrols.back()->Width();
    } else {
        Uint32 format = 0;
        std::vector<GG::Font::LineData> lines;
        GG::Pt extent = font->DetermineLines(str, format, 1000, lines);
        unsigned int first_char_pos = 0;
        unsigned int last_char_pos = 0;
        int pixels_per_owner = extent.x / owners.size() + 1; // the +1 is to make sure there is not a stray character left off the end
        int owner_idx = 1;
        for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it, ++owner_idx) {
            while (last_char_pos < str.size() && lines[0].char_data[last_char_pos].extent < (owner_idx * pixels_per_owner)) {
                ++last_char_pos;
            }
            m_subcontrols.push_back(new GG::TextControl(width, 0, str.substr(first_char_pos, last_char_pos - first_char_pos), 
                                                        font, Empires().Lookup(*it)->Color()));
            AttachChild(m_subcontrols.back());
            first_char_pos = last_char_pos;
            width += m_subcontrols.back()->Width();
        }
    }
    Resize(GG::Pt(width, m_subcontrols[0]->Height()));
}

void OwnerColoredSystemName::Render()
{}

////////////////////////////////////////////////
// SystemIcon
////////////////////////////////////////////////
SystemIcon::SystemIcon(int id, double zoom) :
    GG::Control(0, 0, 1, 1, GG::CLICKABLE),
    m_system(*GetUniverse().Object<const System>(id)),
    m_disc_graphic(0),
    m_halo_graphic(0),
    m_selection_indicator(0),
    m_mouseover_indicator(0),
    m_selected(false),
    m_name(0)
{
    Connect(m_system.StateChangedSignal, &SystemIcon::Refresh, this);

    SetText(m_system.Name());

    StarType star_type = m_system.Star();
    
    // everything is resized at the bottom of this function
    const int DEFAULT_SIZE = 10;

    // disc graphic
    boost::shared_ptr<GG::Texture> disc_texture = ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[star_type], m_system.ID());
    m_disc_graphic = new GG::StaticGraphic(0, 0, DEFAULT_SIZE, DEFAULT_SIZE, disc_texture, GG::GR_FITGRAPHIC);
    m_disc_graphic->SetColor(GG::CLR_WHITE);
    AttachChild(m_disc_graphic);

    // halo graphic
    boost::shared_ptr<GG::Texture> halo_texture = ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars", ClientUI::HaloStarTypeFilePrefixes()[star_type], m_system.ID());
    if (halo_texture)
        m_halo_graphic = new GG::StaticGraphic(-DEFAULT_SIZE, -DEFAULT_SIZE, DEFAULT_SIZE, DEFAULT_SIZE, halo_texture, GG::GR_FITGRAPHIC);

    // selection indicator graphic
    boost::shared_ptr<GG::Texture> selection_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_selection.png");
    m_selection_indicator = new GG::StaticGraphic(0, 0, DEFAULT_SIZE, DEFAULT_SIZE, selection_texture, GG::GR_FITGRAPHIC);

    // mouseover indicator graphic
    boost::shared_ptr<GG::Texture> mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover.png");
    m_mouseover_indicator = new GG::StaticGraphic(0, 0, DEFAULT_SIZE, DEFAULT_SIZE, mouseover_texture, GG::GR_FITGRAPHIC);

    // resize icon, along with indicators and halo (which are also done in SizeMove(...))
    GG::Pt ul(static_cast<int>((m_system.X() - ClientUI::SystemIconSize() / 2) * zoom),
              static_cast<int>((m_system.Y() - ClientUI::SystemIconSize() / 2) * zoom));
    GG::Pt lr(static_cast<int>(ul.x + ClientUI::SystemIconSize() * zoom + 0.5),
              static_cast<int>(ul.y + ClientUI::SystemIconSize() * zoom + 0.5));
    SizeMove(ul, lr);
}

SystemIcon::~SystemIcon()
{
    AttachChild(m_selection_indicator);
    AttachChild(m_mouseover_indicator);
    AttachChild(m_halo_graphic);
}

const System& SystemIcon::GetSystem() const
{
    return m_system;
}

const FleetButton* SystemIcon::GetFleetButton(Fleet* fleet) const
{
    std::map<int, FleetButton*>::const_iterator it = m_stationary_fleet_markers.find(*fleet->Owners().begin());
    if (it != m_stationary_fleet_markers.end()) {
        const std::vector<Fleet*>& fleets = it->second->Fleets();
        if (std::find(fleets.begin(), fleets.end(), fleet) != fleets.end())
            return it->second;
    }
    it = m_moving_fleet_markers.find(*fleet->Owners().begin());
    if (it != m_moving_fleet_markers.end()) {
        const std::vector<Fleet*>& fleets = it->second->Fleets();
        if (std::find(fleets.begin(), fleets.end(), fleet) != fleets.end())
            return it->second;
    }
    return 0;
}

void SystemIcon::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    if (m_disc_graphic)
        m_disc_graphic->SizeMove(GG::Pt(0, 0), lr - ul);

    int ind_size = static_cast<int>(ClientUI::SystemSelectionIndicatorSize() * Width());
    GG::Pt ind_ul = GG::Pt((Width() - ind_size) / 2, (Height() - ind_size) / 2);
    GG::Pt ind_lr = ind_ul + GG::Pt(ind_size, ind_size);

    if (m_selection_indicator && m_selected)
        m_selection_indicator->SizeMove(ind_ul, ind_lr);        

    if (m_mouseover_indicator)
        m_mouseover_indicator->SizeMove(ind_ul, ind_lr);

    if (m_halo_graphic) {
        double halo_size_factor = 1 + log10( static_cast<double>(Width()) / static_cast<double>(ClientUI::SystemIconSize()) );
        if (halo_size_factor > 0.5) {
            int halo_size = static_cast<int>(Width() * halo_size_factor);
            //Logger().errorStream() << "Core Size: " << Width() << " halo size: " << halo_size;
            GG::Pt halo_ul = GG::Pt((Width() - halo_size) / 2, (Height() - halo_size) / 2);
            GG::Pt halo_lr = halo_ul + GG::Pt(halo_size, halo_size);
            AttachChild(m_halo_graphic);
            m_halo_graphic->SizeMove(halo_ul, halo_lr);
            MoveChildDown(m_halo_graphic);
        } else {
            DetachChild(m_halo_graphic);
        }
    }

    PositionSystemName();

    const int BUTTON_SIZE = static_cast<int>(Height() * ClientUI::FleetButtonSize());
    GG::Pt size = Size();
    int stationary_y = 0;
    for (std::map<int, FleetButton*>::iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it) {
        it->second->SizeMove(GG::Pt(size.x - BUTTON_SIZE, stationary_y), GG::Pt(size.x, stationary_y + BUTTON_SIZE));
        stationary_y += BUTTON_SIZE;
    }
    int moving_y = size.y - BUTTON_SIZE;
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it) {
        it->second->SizeMove(GG::Pt(0, moving_y), GG::Pt(BUTTON_SIZE, moving_y + BUTTON_SIZE));
        moving_y -= BUTTON_SIZE;
    }
}

void SystemIcon::LClick(const GG::Pt& pt, Uint32 keys)
{
    if (!Disabled())
        LeftClickedSignal(m_system.ID());
}

void SystemIcon::RClick(const GG::Pt& pt, Uint32 keys)
{
    if (!Disabled())
        RightClickedSignal(m_system.ID());
}

void SystemIcon::LDoubleClick(const GG::Pt& pt, Uint32 keys)
{
    if (!Disabled())
        LeftDoubleClickedSignal(m_system.ID());
}

void SystemIcon::MouseEnter(const GG::Pt& pt, Uint32 keys)
{
    // indicate mouseover
    if (m_mouseover_indicator) {
        AttachChild(m_mouseover_indicator);
        MoveChildUp(m_mouseover_indicator);
    }

    MouseEnteringSignal(m_system.ID());
}

void SystemIcon::MouseLeave()
{
    // un-indicate mouseover
    if (m_mouseover_indicator) {
        DetachChild(m_mouseover_indicator);
    }

    MouseLeavingSignal(m_system.ID());
}

void SystemIcon::SetSelected(bool selected)
{
    m_selected = selected;

    if (m_selected) {
        int size = static_cast<int>(ClientUI::SystemSelectionIndicatorSize() * Width());
        GG::Pt ind_ul = GG::Pt((Width() - size) / 2, (Height() - size) / 2);
        GG::Pt ind_lr = ind_ul + GG::Pt(size, size);
        AttachChild(m_selection_indicator);
        m_selection_indicator->SizeMove(ind_ul, ind_lr);
        MoveChildUp(m_selection_indicator);
    } else {
        DetachChild(m_selection_indicator);
    }
}

void SystemIcon::Refresh()
{
    SetText(m_system.Name());

    // set up the name text controls
    if (!m_system.Name().empty()) {
        delete m_name;
        boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());
        m_name = new OwnerColoredSystemName(&m_system, font);
        AttachChild(m_name);
        PositionSystemName();
    }

    std::vector<const Fleet*> fleets = m_system.FindObjects<Fleet>();
    for (unsigned int i = 0; i < fleets.size(); ++i)
        Connect(fleets[i]->StateChangedSignal, &SystemIcon::CreateFleetButtons, this);
    Connect(m_system.FleetAddedSignal, &SystemIcon::FleetCreatedOrDestroyed, this);
    Connect(m_system.FleetRemovedSignal, &SystemIcon::FleetCreatedOrDestroyed, this);

    CreateFleetButtons();
}

void SystemIcon::ClickFleetButton(Fleet* fleet)
{
    for (std::map<int, FleetButton*>::iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it) {
        if (std::find(it->second->Fleets().begin(), it->second->Fleets().end(), fleet) != it->second->Fleets().end()) {
            it->second->LClick(GG::Pt(), 0);
            return;
        }
    }
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it) {
        if (std::find(it->second->Fleets().begin(), it->second->Fleets().end(), fleet) != it->second->Fleets().end()) {
            it->second->LClick(GG::Pt(), 0);
            return;
        }
    }
}

void SystemIcon::ShowName()
{
    if (m_name)
        m_name->Show();
}

void SystemIcon::HideName()
{
    if (m_name)
        m_name->Hide();
}

void SystemIcon::CreateFleetButtons()
{
    // clear out old fleet buttons
    for (std::map<int, FleetButton*>::iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it)
        DeleteChild(it->second);
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it)
        DeleteChild(it->second);
    m_stationary_fleet_markers.clear();
    m_moving_fleet_markers.clear();

    const int BUTTON_SIZE = static_cast<int>(Height() * ClientUI::FleetButtonSize());
    GG::Pt size = Size();
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    int stationary_y = 0;
    int moving_y = size.y - BUTTON_SIZE;
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        std::vector<int> fleet_IDs = m_system.FindObjectIDs(StationaryFleetVisitor(it->first));
        FleetButton* stationary_fb = 0;
        if (!fleet_IDs.empty()) {
            stationary_fb = new FleetButton(size.x - BUTTON_SIZE, stationary_y, BUTTON_SIZE, BUTTON_SIZE, it->second->Color(), fleet_IDs, SHAPE_LEFT);
            m_stationary_fleet_markers[it->first] = stationary_fb;
            AttachChild(m_stationary_fleet_markers[it->first]);
            map_wnd->SetFleetMovement(stationary_fb);
            GG::Connect(stationary_fb->ClickedSignal, FleetButtonClickedFunctor(*stationary_fb, *this, false));
            stationary_y += BUTTON_SIZE;
        }
        fleet_IDs = m_system.FindObjectIDs(OrderedMovingFleetVisitor(it->first));
        FleetButton* moving_fb = 0;
        if (!fleet_IDs.empty()) {
            moving_fb = new FleetButton(0, moving_y, BUTTON_SIZE, BUTTON_SIZE, it->second->Color(), fleet_IDs, SHAPE_RIGHT);
            m_moving_fleet_markers[it->first] = moving_fb;
            AttachChild(m_moving_fleet_markers[it->first]);
            map_wnd->SetFleetMovement(moving_fb);
            GG::Connect(moving_fb->ClickedSignal, FleetButtonClickedFunctor(*moving_fb, *this, true));
            moving_y -= BUTTON_SIZE;
        }
    }
}

void SystemIcon::PositionSystemName()
{
    if (m_name)
        m_name->MoveTo(GG::Pt((Width() - m_name->Width()) / 2, Height()));
}

void SystemIcon::FleetCreatedOrDestroyed(const Fleet&)
{
    CreateFleetButtons();
}

bool SystemIcon::InWindow(const GG::Pt& pt) const
{
    // Before we blindly check our bounding rect, make sure it doesn't fall in any of our fleets.
    for (std::map<int, FleetButton*>::const_iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it) {
        if (it->second->InWindow(pt))
            return true;
    }

    return Wnd::InWindow(pt);
}

////////////////////////////////////////////////
// SystemIcon::FleetButtonClickedFunctor
////////////////////////////////////////////////
SystemIcon::FleetButtonClickedFunctor::FleetButtonClickedFunctor(FleetButton& fleet_btn, SystemIcon& system_icon, bool fleet_departing) :
    m_fleet_btn(fleet_btn),
    m_system_icon(system_icon),
    m_fleet_departing(fleet_departing)
{}

void SystemIcon::FleetButtonClickedFunctor::operator()()
{
    m_system_icon.FleetButtonClickedSignal(m_fleet_btn, m_fleet_departing);
}