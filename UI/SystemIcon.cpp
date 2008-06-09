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
#include "../util/OptionsDB.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace {
    bool PlaySounds() {return GetOptionsDB().Get<bool>("UI.sound.enabled");}
    void PlaySystemIconRolloverSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.system-icon-rollover"));}
}

////////////////////////////////////////////////
// OwnerColoredSystemName
////////////////////////////////////////////////
OwnerColoredSystemName::OwnerColoredSystemName(const System* system, const boost::shared_ptr<GG::Font>& font, const std::string& format_text/* = ""*/, GG::Flags<GG::WndFlag> flags/* = GG::Flags<GG::WndFlag>()*/) :
    Control(0, 0, 1, 1, flags)
{
    // TODO: Have this make a single call per color.  Set up texture coord and vertex buffers (quads) for the glyphs.  Consider extending GG::Font to do similar.

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
        GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE;
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
// static(s)
const int SystemIcon::TINY_SIZE = 8;

SystemIcon::SystemIcon(GG::Wnd* parent, int x, int y, int w, int id) :
    GG::Control(x, y, w, w, GG::CLICKABLE),
    m_system(*GetUniverse().Object<const System>(id)),
    m_disc_texture(ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[m_system.Star()], id)),
    m_halo_texture(ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars", ClientUI::HaloStarTypeFilePrefixes()[m_system.Star()], id)),
    m_tiny_texture(ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars", "tiny_" + ClientUI::StarTypeFilePrefixes()[m_system.Star()], id)),
    m_tiny_graphic(0),
    m_selection_indicator(0),
    m_mouseover_indicator(0),
    m_selected(false),
    m_name(0),
    m_showing_name(false)
{
    parent->AttachChild(this);
    Init();
}

void SystemIcon::Init() {
    Connect(m_system.StateChangedSignal, &SystemIcon::Refresh, this);

    SetText(m_system.Name());

    // everything is resized by SizeMove
    const int DEFAULT_SIZE = 10;

    if (m_tiny_texture)
        m_tiny_graphic = new GG::StaticGraphic(0, 0, m_tiny_texture->Width(), m_tiny_texture->Height(), m_tiny_texture);
    AttachChild(m_tiny_graphic);
    m_tiny_graphic->Hide();

    // selection indicator graphic
    boost::shared_ptr<GG::Texture> selection_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_selection.png");
    m_selection_indicator = new GG::StaticGraphic(0, 0, DEFAULT_SIZE, DEFAULT_SIZE, selection_texture, GG::GRAPHIC_FITGRAPHIC);

    // mouseover indicator graphic
    boost::shared_ptr<GG::Texture> mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover.png");
    m_mouseover_indicator = new GG::StaticGraphic(0, 0, DEFAULT_SIZE, DEFAULT_SIZE, mouseover_texture, GG::GRAPHIC_FITGRAPHIC);

    Refresh();
}

SystemIcon::~SystemIcon()
{
    AttachChild(m_selection_indicator);
    AttachChild(m_mouseover_indicator);
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

GG::Pt SystemIcon::FleetButtonCentre(int empire_id, bool moving) const
{
    // determine if the specified empire has the specified fleetbutton, and if so, where it is
    std::map<int, FleetButton*>::const_iterator it, end;
    if (moving) {
        it = m_moving_fleet_markers.find(empire_id);
        end = m_moving_fleet_markers.end();
    } else {
        it = m_stationary_fleet_markers.find(empire_id);
        end = m_stationary_fleet_markers.end();
    }

    if (it == end) {  // no such fleetbutton
        return GG::Pt(static_cast<int>(UniverseObject::INVALID_POSITION),
                      static_cast<int>(UniverseObject::INVALID_POSITION));
    }

    const int HALF_SIZE = static_cast<int>(it->second->Width() * 0.5);
    return it->second->UpperLeft() + GG::Pt(HALF_SIZE, HALF_SIZE);
}

const boost::shared_ptr<GG::Texture>& SystemIcon::DiscTexture() const
{ return m_disc_texture; }

const boost::shared_ptr<GG::Texture>& SystemIcon::HaloTexture() const
{ return m_halo_texture; }

const boost::shared_ptr<GG::Texture>& SystemIcon::TinyTexture() const
{ return m_tiny_texture; }

GG::Pt SystemIcon::NthFleetButtonUpperLeft(int n, bool moving) const
{
    assert(n > 0);
    // determine where the nth fleetbutton should be located
    GG::Pt retval = GG::Pt(0, 0);//this->UpperLeft();
    const int FLEETBUTTON_SIZE = FleetButtonSize();

    if (moving) {   // moving at bottom left
        retval += GG::Pt(-FLEETBUTTON_SIZE, this->Height());
        retval += GG::Pt(0, FLEETBUTTON_SIZE*(n - 1));
        return retval;
    } else {        // stationary at top right
        retval += GG::Pt(this->Width(), -FLEETBUTTON_SIZE*n);
        return retval;
    }
}

int SystemIcon::FleetButtonSize() const
{
    return static_cast<int>(Height() * ClientUI::FleetButtonSize());
}

void SystemIcon::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);

    if (m_tiny_graphic && lr.x - ul.x < TINY_SIZE) {
        GG::Pt tiny_size = m_tiny_graphic->Size();
        GG::Pt middle = GG::Pt(Width() / 2, Height() / 2);
        GG::Pt tiny_ul(static_cast<int>(middle.x - tiny_size.x / 2.0 + 0.5),
                       static_cast<int>(middle.y - tiny_size.y / 2.0 + 0.5));
        m_tiny_graphic->SizeMove(tiny_ul, tiny_ul + tiny_size);
        m_tiny_graphic->Show();
    } else {
        m_tiny_graphic->Hide();
    }

    int ind_size = static_cast<int>(ClientUI::SystemSelectionIndicatorSize() * Width());
    GG::Pt ind_ul((Width() - ind_size) / 2, (Height() - ind_size) / 2);
    GG::Pt ind_lr = ind_ul + GG::Pt(ind_size, ind_size);

    if (m_selection_indicator && m_selected)
        m_selection_indicator->SizeMove(ind_ul, ind_lr);

    if (m_mouseover_indicator)
        m_mouseover_indicator->SizeMove(ind_ul, ind_lr);

    PositionSystemName();

    DoFleetButtonLayout();
}

void SystemIcon::ManualRender(double halo_scale_factor)
{
    if (!Visible())
        return;
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (0.5 < halo_scale_factor && m_halo_texture) {
        GG::Pt size = lr - ul;
        GG::Pt half_size = GG::Pt(size.x / 2, size.y / 2);
        GG::Pt middle = ul + half_size;
        GG::Pt halo_size = GG::Pt(size.x*halo_scale_factor, size.y*halo_scale_factor);
        GG::Pt halo_half_size = GG::Pt(halo_size.x / 2, halo_size.y / 2);
        GG::Pt halo_ul = middle - halo_half_size;
        GG::Pt halo_lr = halo_ul + halo_size;
        m_halo_texture->OrthoBlit(halo_ul, halo_lr);
    }
    if (m_disc_texture)
        m_disc_texture->OrthoBlit(ul, lr);
}

void SystemIcon::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled())
        LeftClickedSignal(m_system.ID());
}

void SystemIcon::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled())
        RightClickedSignal(m_system.ID());
}

void SystemIcon::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled())
        LeftDoubleClickedSignal(m_system.ID());
}

void SystemIcon::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    // indicate mouseover
    if (m_mouseover_indicator) {
        AttachChild(m_mouseover_indicator);
        MoveChildUp(m_mouseover_indicator);
    }
    if (m_name)
        m_name->Show();

    PlaySystemIconRolloverSound();

    MouseEnteringSignal(m_system.ID());
}

void SystemIcon::MouseLeave()
{
    // un-indicate mouseover
    if (m_mouseover_indicator) {
        DetachChild(m_mouseover_indicator);
    }
    if (!m_showing_name && m_name)
        m_name->Hide();

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
        boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts() + 3);
        m_name = new OwnerColoredSystemName(&m_system, font);
        AttachChild(m_name);
        m_showing_name = true;
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
            it->second->LClick(GG::Pt(), GG::MOD_KEY_NONE);
            return;
        }
    }
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it) {
        if (std::find(it->second->Fleets().begin(), it->second->Fleets().end(), fleet) != it->second->Fleets().end()) {
            it->second->LClick(GG::Pt(), GG::MOD_KEY_NONE);
            return;
        }
    }
}

void SystemIcon::ShowName()
{
    if (m_name)
        m_name->Show();
    m_showing_name = true;
}

void SystemIcon::HideName()
{
    if (m_name)
        m_name->Hide();
    m_showing_name = false;
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

    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    FleetButton* fb;

    // create new fleet buttons
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        const int empire_id = it->first;
        const Empire* empire = it->second;

        std::vector<int> fleet_IDs = m_system.FindObjectIDs(StationaryFleetVisitor(empire_id));        
        if (!fleet_IDs.empty()) {
            fb = new FleetButton(empire->Color(), fleet_IDs);
            m_stationary_fleet_markers[empire_id] = fb;
            AttachChild(m_stationary_fleet_markers[empire_id]);
            GG::Connect(fb->ClickedSignal, FleetButtonClickedFunctor(*fb, *this, false));
        }

        fleet_IDs = m_system.FindObjectIDs(OrderedMovingFleetVisitor(it->first));
        if (!fleet_IDs.empty()) {
            fb = new FleetButton(empire->Color(), fleet_IDs);
            m_moving_fleet_markers[empire_id] = fb;
            AttachChild(m_moving_fleet_markers[empire_id]);
            GG::Connect(fb->ClickedSignal, FleetButtonClickedFunctor(*fb, *this, true));
        }
    }

    // position new fleet buttons
    DoFleetButtonLayout();

    // create movement lines (after positioning buttons, so lines will originate from button location)
    for (EmpireManager::const_iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it) {
        const int empire_id = empire_it->first;
        
        std::map<int, FleetButton*>::iterator button_it = m_stationary_fleet_markers.find(empire_id);
        if (button_it != m_stationary_fleet_markers.end())
            map_wnd->SetFleetMovement(button_it->second);
        
        button_it = m_moving_fleet_markers.find(empire_id);
        if (button_it != m_moving_fleet_markers.end())
            map_wnd->SetFleetMovement(button_it->second);
    }
}

void SystemIcon::DoFleetButtonLayout()
{
    const int FLEETBUTTON_SIZE = FleetButtonSize();
    const GG::Pt SIZE = GG::Pt(FLEETBUTTON_SIZE, FLEETBUTTON_SIZE);
    
    // stationary fleet buttons
    int empire_num = 1;
    for (std::map<int, FleetButton*>::iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it) {
        GG::Pt ul = NthFleetButtonUpperLeft(empire_num, false);
        ++empire_num;
        it->second->SizeMove(ul, ul + SIZE);
    }
    
    // departing fleet buttons
    empire_num = 1;
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it) {
        GG::Pt ul = NthFleetButtonUpperLeft(empire_num, true);
        ++empire_num;
        it->second->SizeMove(ul, ul + SIZE);
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
    for (std::map<int, FleetButton*>::const_iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it) {
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
