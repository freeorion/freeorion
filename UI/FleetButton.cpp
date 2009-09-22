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

    /* returns corner vertex x and y components of a quad centred at (0, 0) of side length 2 that is rotated
       so the "top" or "up" face is perpendicular to and facing in the direction of \a direction_vector */
    std::vector<double> VectorAlignedQuadVertices(const GG::Pt& direction_vector, int texture_height, int texture_width) {
        // get unit vectors parallel and perpendicular to direction vector
        int x = Value(direction_vector.x), y = Value(direction_vector.y);
        int mag2 = x*x + y*y;

        // first unit vector parallel to direction_vector
        double U1X = static_cast<double>(x), U1Y = static_cast<double>(y);
        if (mag2 > 1) {
            double mag = std::sqrt(static_cast<double>(mag2));
            U1X /= mag;
            U1Y /= mag;
        } else if (mag2 == 0) {
            // default to straight up for zero vector
            U1X =  0.0;
            U1Y = -1.0;
        } // else don't need to rescale if vector length already is 1

        // second unit vector perpendicular to first
        double U2X = -U1Y, U2Y = U1X;

        // multiply unit vectors by (half) texture size to get properly-scaled side vectors
        double V1X = U1X * texture_height / 2.0;
        double V1Y = U1Y * texture_height / 2.0;
        double V2X = U2X * texture_width / 2.0;
        double V2Y = U2Y * texture_width / 2.0;

        // get components of corner points by adding unit vectors
        std::vector<double> retval;
        retval.push_back( V1X - V2X);   retval.push_back( V1Y - V2Y);
        retval.push_back( V1X + V2X);   retval.push_back( V1Y + V2Y);
        retval.push_back(-V1X - V2X);   retval.push_back(-V1Y - V2Y);
        retval.push_back(-V1X + V2X);   retval.push_back(-V1Y + V2Y);

        return retval;
    }

    /* renders quad with passed vertices and texture */
    void RenderTexturedQuad(const std::vector<double>& vertsXY, const boost::shared_ptr<GG::Texture>& texture) {
        if (!texture || vertsXY.size() < 8)
            return;

        glBindTexture(GL_TEXTURE_2D, texture->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        double tex_coord_x = Value(1.0 * texture->DefaultWidth() / texture->Width());
        double tex_coord_y = Value(1.0 * texture->DefaultHeight() / texture->Height());

        // render texture
        glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0.0, 0.0);
            glVertex2d(vertsXY[0], vertsXY[1]);
            glTexCoord2f(tex_coord_x, 0.0);
            glVertex2d(vertsXY[2], vertsXY[3]);
            glTexCoord2f(0.0, tex_coord_y);
            glVertex2d(vertsXY[4], vertsXY[5]);
            glTexCoord2f(tex_coord_x, tex_coord_y);
            glVertex2d(vertsXY[6], vertsXY[7]);
        glEnd();
    }

    void AddOptions(OptionsDB& db) {
        db.Add("UI.fleet-selection-indicator-size", "OPTIONS_DB_UI_FLEET_SELECTION_INDICATOR_SIZE", 1.625, RangedStepValidator<double>(0.125, 0.5, 5));
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

///////////////////////////
// FleetButton
///////////////////////////
FleetButton::FleetButton(const std::vector<int>& fleet_IDs, SizeType size_type) :
    GG::Button(GG::X0, GG::Y0, GG::X1, GG::Y1, "", boost::shared_ptr<GG::Font>(), GG::CLR_ZERO),
    m_fleets(),
    m_head_icon(),
    m_size_icon(),
    m_selection_texture(),
    m_vertex_components(),
    m_selected(false)
{
    Init(fleet_IDs, size_type);
}

FleetButton::FleetButton(int fleet_id, SizeType size_type) :
    GG::Button(GG::X0, GG::Y0, GG::X1, GG::Y1, "", boost::shared_ptr<GG::Font>(), GG::CLR_ZERO),
    m_fleets(),
    m_head_icon(),
    m_size_icon(),
    m_selection_texture(),
    m_vertex_components(),
    m_selected(false)
{
    std::vector<int> fleet_IDs;
    fleet_IDs.push_back(fleet_id);
    Init(fleet_IDs, size_type);
}

void FleetButton::Init(const std::vector<int>& fleet_IDs, SizeType size_type) {
    // get fleets
    Universe& universe = GetUniverse();
    std::vector<const Fleet*> fleets;

    for (std::vector<int>::const_iterator it = fleet_IDs.begin(); it != fleet_IDs.end(); ++it) {
        const Fleet* fleet = universe.Object<Fleet>(*it);
        if (!fleet) {
            Logger().errorStream() << "FleetButton::FleetButton couldn't get fleet with id " << *it;
            continue;
        }
        m_fleets.push_back(*it);
        fleets.push_back(fleet);
    }

    // determine owner(s) of fleet(s).  Only care whether or not there is more than one owner, as owner
    // is used to determine colouration
    int owner_id = ALL_EMPIRES;

    for (std::vector<const Fleet*>::const_iterator it = fleets.begin(); it != fleets.end(); ++it) {
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


    // select icon(s) for fleet(s), and get a fleet for use later
    const Fleet* first_fleet = 0;
    if (m_fleets.size() != 1) {
        first_fleet = *(fleets.begin());

        m_head_icon = FleetHeadIcon(0, size_type);
        int num_ships = 0;
        for (std::vector<const Fleet*>::const_iterator it = fleets.begin(); it != fleets.end(); ++it)
            num_ships += (*it)->NumShips();
        m_size_icon = FleetSizeIcon(num_ships, size_type);

    } else if (!m_fleets.empty()) {
        first_fleet = *fleets.begin();
        m_head_icon = FleetHeadIcon(first_fleet, size_type);
        m_size_icon = FleetSizeIcon(first_fleet, size_type);
    }

    // resize to fit icon by first determining necessary size, and then resizing
    GG::X texture_width(0);
    GG::Y texture_height(0);

    if (m_head_icon) {
        texture_width = m_head_icon->DefaultWidth();
        texture_height = m_head_icon->DefaultHeight();
    }
    if (m_size_icon) {
        texture_width = std::max(texture_width, m_size_icon->DefaultWidth());
        texture_height = std::max(texture_height, m_size_icon->DefaultHeight());
    }


    // determine if fleet icon should be rotated.  this should be done if the fleet is moving along
    // a starlane, which is the case if the fleet is not in a system and has a valid next system
    GG::Pt direction_vector(GG::X(0), GG::Y(1));    // default, unrotated button orientation

    if (first_fleet && !first_fleet->GetSystem()) {
        int next_sys_id = first_fleet->NextSystemID();
        const UniverseObject* obj = GetUniverse().Object(next_sys_id);
        if (obj) {
            // fleet is not in a system and has a valid next destination, so can orient it in that direction
            // fleet icons might not appear on the screen in the exact place corresponding to their 
            // actual universe position, but if they're moving along a starlane, this code will assume
            // their apparent position will only be different from their true position in a direction
            // parallel with the starlane, so the direction from their true position to their destination
            // position can be used to get a direction vector to orient the icon
            double dest_x = obj->X(), dest_y = obj->Y();
            double cur_x = first_fleet->X(), cur_y = first_fleet->Y();
            const MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
            GG::Pt dest = map_wnd->ScreenCoordsFromUniversePosition(dest_x, dest_y);
            GG::Pt cur = map_wnd->ScreenCoordsFromUniversePosition(cur_x, cur_y);
            direction_vector = dest - cur;
        }
    }

    // check for unrotated texture
    if (Value(direction_vector.x) == 0) {
        // not rotated.  can do simple texture blits
        m_vertex_components.clear();
    } else {
        // texture is rotated, so need some extra math
        // get rotated corner vetex x and y components (x1, y1, x2, y2, x3, y3, x4, y4) for texture of appropriate size
        m_vertex_components = VectorAlignedQuadVertices(direction_vector, Value(texture_height), Value(texture_width));
    }

    // size icon according to texture size (average two dimensions)
    double diameter = (Value(texture_width) + Value(texture_height))/2.0;
    Resize(GG::Pt(GG::X(diameter), GG::Y(diameter)));

    // get selection indicator texture
    m_selection_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "fleet" / "fleet_selection.png", true);
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

void FleetButton::SetSelected(bool selected)
{
    m_selected = selected;
}

void FleetButton::RenderUnpressed() {
    glColor(Color());
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    const double midX = static_cast<double>(Value(ul.x + lr.x))/2.0;
    const double midY = static_cast<double>(Value(ul.y + lr.y))/2.0;

    if (m_vertex_components.empty()) {
        if (m_size_icon)
            m_size_icon->OrthoBlit(ul);
        if (m_head_icon)
            m_head_icon->OrthoBlit(ul);
    } else {
        std::vector<double> vertsXY;
        vertsXY.push_back(midX + m_vertex_components[0]);
        vertsXY.push_back(midY + m_vertex_components[1]);
        vertsXY.push_back(midX + m_vertex_components[2]);
        vertsXY.push_back(midY + m_vertex_components[3]);
        vertsXY.push_back(midX + m_vertex_components[4]);
        vertsXY.push_back(midY + m_vertex_components[5]);
        vertsXY.push_back(midX + m_vertex_components[6]);
        vertsXY.push_back(midY + m_vertex_components[7]);

        RenderTexturedQuad(vertsXY, m_head_icon);
        RenderTexturedQuad(vertsXY, m_size_icon);
    }

    if (m_selected && m_selection_texture) {
        double sel_ind_scale = GetOptionsDB().Get<double>("UI.fleet-selection-indicator-size");
        double sel_ind_half_size = Value(Width()) * sel_ind_scale / 2.0;

        GG::Pt button_mid = GG::Pt(GG::X(midX), GG::Y(midY));
        GG::Pt sel_ul = button_mid - GG::Pt(GG::X(sel_ind_half_size), GG::Y(sel_ind_half_size));
        GG::Pt sel_lr = button_mid + GG::Pt(GG::X(sel_ind_half_size), GG::Y(sel_ind_half_size));

        glColor(GG::CLR_WHITE);
        m_selection_texture->OrthoBlit(sel_ul, sel_lr);
    }
}

void FleetButton::RenderPressed() {
    const double TWO_PI = 2.0*3.14159;
    glDisable(GL_TEXTURE_2D);
    glColor(Color());
    CircleArc(UpperLeft(), LowerRight(), 0.0, TWO_PI, true);
    glEnable(GL_TEXTURE_2D);

    RenderUnpressed();
}

void FleetButton::RenderRollover() {
    const double TWO_PI = 2.0*3.14159;
    glDisable(GL_TEXTURE_2D);
    glColor(GG::CLR_WHITE);
    CircleArc(UpperLeft(), LowerRight(), 0.0, TWO_PI, true);
    glEnable(GL_TEXTURE_2D);

    RenderUnpressed();
}

void FleetButton::PlayFleetButtonRolloverSound() {
    Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.fleet-button-rollover"), true);
}

void FleetButton::PlayFleetButtonOpenSound() {
    Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.fleet-button-click"), true);
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