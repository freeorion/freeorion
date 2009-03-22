#include "SystemIcon.h"

#include "ClientUI.h"
#include "../universe/Fleet.h"
#include "FleetButton.h"
#include "FleetWnd.h"
#include "../client/human/HumanClientApp.h"
#include "MapWnd.h"
#include "Sound.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Predicates.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"
#include "../util/OptionsDB.h"
#include "CUIControls.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace {
    bool PlaySounds() {return GetOptionsDB().Get<bool>("UI.sound.enabled");}
    void PlaySystemIconRolloverSound() {if (PlaySounds()) Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.system-icon-rollover"));}

    const double        PI = 3.1415926535;
    const unsigned int  MAX_TRIES = 128;     // most allowed unique fleetbutton locations

    void AddOptions(OptionsDB& db) {
        db.Add("UI.system-tiny-icon-size-threshold",    "OPTIONS_DB_UI_SYSTEM_TINY_ICON_SIZE_THRESHOLD",    10, RangedValidator<int>(1, 16));
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

////////////////////////////////////////////////
// OwnerColoredSystemName
////////////////////////////////////////////////
OwnerColoredSystemName::OwnerColoredSystemName(const System* system, const boost::shared_ptr<GG::Font>& font, const std::string& format_text/* = ""*/, GG::Flags<GG::WndFlag> flags/* = GG::Flags<GG::WndFlag>()*/) :
    Control(GG::X0, GG::Y0, GG::X1, GG::Y1, flags)
{
    // TODO: Have this make a single call per color.  Set up texture coord and vertex buffers (quads) for the glyphs.  Consider extending GG::Font to do similar.

    std::string str = format_text == "" ? system->Name() : boost::io::str(FlexibleFormat(format_text) % system->Name());
    GG::X width(0);
    const std::set<int>& owners = system->Owners();
    if (owners.size() <= 1) {
        GG::Clr text_color = ClientUI::SystemNameTextColor();
        if (!owners.empty())
            text_color = Empires().Lookup(*owners.begin())->Color();
        GG::TextControl* text = new ShadowedTextControl(width, GG::Y0, str, font, text_color);
        m_subcontrols.push_back(text);
        AttachChild(m_subcontrols.back());
        width += m_subcontrols.back()->Width();
    } else {
        GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE;
        std::vector<GG::Font::LineData> lines;
        GG::Pt extent = font->DetermineLines(str, format, GG::X(1000), lines);
        unsigned int first_char_pos = 0;
        unsigned int last_char_pos = 0;
        GG::X pixels_per_owner = extent.x / static_cast<int>(owners.size()) + 1; // the +1 is to make sure there is not a stray character left off the end
        int owner_idx = 1;
        for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it, ++owner_idx) {
            while (last_char_pos < str.size() && lines[0].char_data[last_char_pos].extent < (owner_idx * pixels_per_owner)) {
                ++last_char_pos;
            }
            m_subcontrols.push_back(new ShadowedTextControl(width, GG::Y0, str.substr(first_char_pos, last_char_pos - first_char_pos), 
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
SystemIcon::SystemIcon(GG::Wnd* parent, GG::X x, GG::Y y, GG::X w, int id) :
    GG::Control(x, y, w, GG::Y(Value(w)), GG::CLICKABLE),
    m_system(*GetUniverse().Object<const System>(id)),
    m_disc_texture(ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[m_system.Star()], id)),
    m_halo_texture(ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars", ClientUI::HaloStarTypeFilePrefixes()[m_system.Star()], id)),
    m_tiny_texture(ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "stars", "tiny_" + ClientUI::StarTypeFilePrefixes()[m_system.Star()], id)),
    m_tiny_graphic(0),
    m_selection_indicator(0),
    m_mouseover_indicator(0),
    m_selected(false),
    m_colored_name(0),
    m_showing_name(false)
{
    parent->AttachChild(this);
    Init();
}

void SystemIcon::Init() {
    // state change signals for system itself and fleets in it
    Connect(m_system.StateChangedSignal,    &SystemIcon::Refresh,       this);

    SetName(m_system.Name());

    // everything is resized by SizeMove
    const int DEFAULT_SIZE = 10;

    if (m_tiny_texture)
        m_tiny_graphic = new GG::StaticGraphic(GG::X0, GG::Y0, m_tiny_texture->Width(), m_tiny_texture->Height(), m_tiny_texture);
    AttachChild(m_tiny_graphic);
    m_tiny_graphic->Hide();

    // selection indicator graphic
    boost::shared_ptr<GG::Texture> selection_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_selection.png");
    m_selection_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(DEFAULT_SIZE), GG::Y(DEFAULT_SIZE), selection_texture, GG::GRAPHIC_FITGRAPHIC);

    // mouseover indicator graphic
    boost::shared_ptr<GG::Texture> mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover.png");
    m_mouseover_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(DEFAULT_SIZE), GG::Y(DEFAULT_SIZE), mouseover_texture, GG::GRAPHIC_FITGRAPHIC);

    Refresh();
}

SystemIcon::~SystemIcon()
{
    delete m_selection_indicator;
    delete m_mouseover_indicator;
    delete m_tiny_graphic;
}

const System& SystemIcon::GetSystem() const
{
    return m_system;
}

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
{
    return static_cast<const int>(Value(Width()) * GetOptionsDB().Get<double>("UI.system-circle-size")) + 1;
}

void SystemIcon::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);

    if (m_tiny_graphic && lr.x - ul.x < GetOptionsDB().Get<int>("UI.system-tiny-icon-size-threshold")) {
        GG::Pt tiny_size = m_tiny_graphic->Size();
        GG::Pt middle = GG::Pt(Width() / 2, Height() / 2);
        GG::Pt tiny_ul(static_cast<GG::X>(middle.x - tiny_size.x / 2.0),
                       static_cast<GG::Y>(middle.y - tiny_size.y / 2.0));
        m_tiny_graphic->SizeMove(tiny_ul, tiny_ul + tiny_size);
        m_tiny_graphic->Show();
    } else {
        m_tiny_graphic->Hide();
    }

    int ind_size = static_cast<int>(ClientUI::SystemSelectionIndicatorSize() * Value(Width()));
    GG::Pt ind_ul((Width() - ind_size) / 2, (Height() - ind_size) / 2);
    GG::Pt ind_lr = ind_ul + GG::Pt(GG::X(ind_size), GG::Y(ind_size));

    if (m_selection_indicator && m_selected)
        m_selection_indicator->SizeMove(ind_ul, ind_lr);

    if (m_mouseover_indicator)
        m_mouseover_indicator->SizeMove(ind_ul, ind_lr);

    PositionSystemName();
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

void SystemIcon::RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled())
        RightDoubleClickedSignal(m_system.ID());
}

void SystemIcon::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    // indicate mouseover
    if (m_mouseover_indicator) {
        AttachChild(m_mouseover_indicator);
        MoveChildUp(m_mouseover_indicator);
    }
    if (m_colored_name)
        m_colored_name->Show();

    PlaySystemIconRolloverSound();

    MouseEnteringSignal(m_system.ID());
}

void SystemIcon::MouseLeave()
{
    // un-indicate mouseover
    if (m_mouseover_indicator) {
        DetachChild(m_mouseover_indicator);
    }
    if (!m_showing_name && m_colored_name)
        m_colored_name->Hide();

    MouseLeavingSignal(m_system.ID());
}

void SystemIcon::SetSelected(bool selected)
{
    m_selected = selected;

    if (m_selected) {
        int size = static_cast<int>(ClientUI::SystemSelectionIndicatorSize() * Value(Width()));
        GG::Pt ind_ul = GG::Pt((Width() - size) / 2, (Height() - size) / 2);
        GG::Pt ind_lr = ind_ul + GG::Pt(GG::X(size), GG::Y(size));
        AttachChild(m_selection_indicator);
        m_selection_indicator->SizeMove(ind_ul, ind_lr);
        MoveChildUp(m_selection_indicator);
    } else {
        DetachChild(m_selection_indicator);
    }
}

void SystemIcon::Refresh()
{
    SetName(m_system.Name());

    // set up the name text controls
    if (!m_system.Name().empty()) {
        delete m_colored_name;
        boost::shared_ptr<GG::Font> font = ClientUI::GetFont(ClientUI::Pts() + 3);
        m_colored_name = new OwnerColoredSystemName(&m_system, font);
        AttachChild(m_colored_name);
        m_showing_name = true;
        PositionSystemName();
    }
}

void SystemIcon::ShowName()
{
    if (m_colored_name)
        m_colored_name->Show();
    m_showing_name = true;
}

void SystemIcon::HideName()
{
    if (m_colored_name)
        m_colored_name->Hide();
    m_showing_name = false;
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
