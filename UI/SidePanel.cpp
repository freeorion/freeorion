#include "SidePanel.h"

#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../Empire/TechLevel.h"


namespace {
int CircleXFromY(double y, double r) {return static_cast<int>(std::sqrt(r * r - y * y) + 0.5);}
}


////////////////////////////////////////////////
// SidePanel::PlanetPanel
////////////////////////////////////////////////
namespace {
const int HUGE_PLANET_SIZE = 128; // size of a huge planet, in on-screen pixels
const int TINY_PLANET_SIZE = HUGE_PLANET_SIZE / 3; // size of a tiny planet, in on-screen pixels
const int IMAGES_PER_PLANET_TYPE = 3; // number of planet images available per planet type (named "type1.png", "type2.png", ...)
const int PLANET_IMAGE_SIZE = 256; // size of the images in the source files

const int CONSTR_DROP_LIST_WIDTH = 150;
const int CONSTR_PROGRESS_BAR_HT = 4;
}

// static(s)
GG::SubTexture SidePanel::PlanetPanel::m_pop_icon;
GG::SubTexture SidePanel::PlanetPanel::m_industry_icon;
GG::SubTexture SidePanel::PlanetPanel::m_research_icon;
GG::SubTexture SidePanel::PlanetPanel::m_mining_icon;
GG::SubTexture SidePanel::PlanetPanel::m_farming_icon;

SidePanel::PlanetPanel::PlanetPanel(const Planet& planet, int y, int parent_width, int h) : 
    Wnd(-HUGE_PLANET_SIZE / 2, y, parent_width + HUGE_PLANET_SIZE / 2, h, GG::Wnd::CLICKABLE),
    m_planet_id(planet.ID()),
    m_construction(0)
{
    // initialize statics if we're the first instance of a PlanetPanel
    if (m_pop_icon.Empty()) {
        const std::string ICON_DIR = ClientUI::ART_DIR + "icons/";
        m_pop_icon = GG::SubTexture(GG::App::GetApp()->GetTexture(ICON_DIR + "pop.png"), 0, 0, 63, 63);
        m_industry_icon = GG::SubTexture(GG::App::GetApp()->GetTexture(ICON_DIR + "industry.png"), 0, 0, 63, 63);
        m_research_icon = GG::SubTexture(GG::App::GetApp()->GetTexture(ICON_DIR + "research.png"), 0, 0, 63, 63);
        m_mining_icon = GG::SubTexture(GG::App::GetApp()->GetTexture(ICON_DIR + "mining.png"), 0, 0, 63, 63);
        m_farming_icon = GG::SubTexture(GG::App::GetApp()->GetTexture(ICON_DIR + "farming.png"), 0, 0, 63, 63);
    }

    SetText(ClientUI::String("PLANET_PANEL"));

    // planet graphic
    std::string planet_image = ClientUI::ART_DIR + "planets/";
    switch (planet.Type()) {
    case Planet::TOXIC:    planet_image += "toxic"; break;
    case Planet::RADIATED: planet_image += "radiated"; break;
    case Planet::BARREN:   planet_image += "barren"; break;
    case Planet::DESERT:   planet_image += "desert"; break;
    case Planet::TUNDRA:   planet_image += "tundra"; break;
    case Planet::OCEAN:    planet_image += "ocean"; break;
    case Planet::TERRAN:
    case Planet::GAIA:     planet_image += "terran"; break;
    default:               planet_image += "barren"; break;
    }
    planet_image += boost::lexical_cast<std::string>((planet.ID() % IMAGES_PER_PLANET_TYPE) + 1) + ".png";
    m_planet_graphic = GG::SubTexture(HumanClientApp::GetApp()->GetTexture(planet_image), 
                                      0, 0, PLANET_IMAGE_SIZE - 1, PLANET_IMAGE_SIZE - 1);

    // construction drop list
    if (!planet.Owners().empty() && *planet.Owners().begin() == HumanClientApp::GetApp()->PlayerID()) {

        // for v.1 some of these only appear after tech is researched
        
        Empire *empire = HumanClientApp::Empires().Lookup( HumanClientApp::GetApp()->PlayerID() );

        m_construction = new CUIDropDownList(Width() - CONSTR_DROP_LIST_WIDTH - 3, 
                                             Height() - ClientUI::SIDE_PANEL_PTS - CONSTR_PROGRESS_BAR_HT - 6,
                                             CONSTR_DROP_LIST_WIDTH, ClientUI::SIDE_PANEL_PTS + 4, 
                                             (ClientUI::SIDE_PANEL_PTS + 4) * 5, GG::CLR_ZERO);
        m_construction->SetStyle(GG::LB_NOSORT);
        m_construction->OffsetMove(0, -m_construction->Height());
        AttachChild(m_construction);
        ////////////////////// v0.1 only!! (in v0.2 and later build this list from the production capabilities of the planet)
        GG::ListBox::Row* row = new GG::ListBox::Row;
        row->push_back("No Building", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
        m_construction->Insert(row);
        m_construction_prod_idx.push_back( ProdCenter::NOT_BUILDING );

        row = new GG::ListBox::Row;
        row->push_back("Industry", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
        m_construction->Insert(row);
        m_construction_prod_idx.push_back( ProdCenter::INDUSTRY_BUILD );

        row = new GG::ListBox::Row;
        row->push_back("Research", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
        m_construction->Insert(row);
        m_construction_prod_idx.push_back( ProdCenter::RESEARCH_BUILD );

        row = new GG::ListBox::Row;
        row->push_back("Scout", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
        m_construction->Insert(row);
        m_construction_prod_idx.push_back( ProdCenter::SCOUT );

        row = new GG::ListBox::Row;
        row->push_back("Colony Ship", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
        m_construction->Insert(row);
        m_construction_prod_idx.push_back( ProdCenter::COLONY_SHIP );

        row = new GG::ListBox::Row;
        row->push_back("MarkI", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
        m_construction->Insert(row);
        m_construction_prod_idx.push_back( ProdCenter::MARKI );

        if ( empire->HasTech( Tech::TECH_MARK2 ) )
        {
            row = new GG::ListBox::Row;
            row->push_back("MarkII", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
            m_construction->Insert(row);
            m_construction_prod_idx.push_back( ProdCenter::MARKII );
        }

        if ( empire->HasTech( Tech::TECH_MARK3 ) )
        {
            row = new GG::ListBox::Row;
            row->push_back("MarkIII", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
            m_construction->Insert(row);
            m_construction_prod_idx.push_back( ProdCenter::MARKIII );
        }

        if ( empire->HasTech( Tech::TECH_MARK4 ) )
        {
            row = new GG::ListBox::Row;
            row->push_back("MarkIV", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
            m_construction->Insert(row);
            m_construction_prod_idx.push_back( ProdCenter::MARKIV );
        }

        if ( empire->HasTech( Tech::TECH_BASE ) )
        {
            row = new GG::ListBox::Row;
            row->push_back("DefBase", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
            m_construction->Insert(row);
            m_construction_prod_idx.push_back( ProdCenter::DEF_BASE );
        }

        // find the index we need to set in the list 
        int selection_idx = 0;
        for ( std::vector< ProdCenter::BuildType >::iterator it = m_construction_prod_idx.begin(); it != m_construction_prod_idx.end(); ++it, selection_idx++ ) {
            if ( *it ==  planet.CurrentlyBuilding() )
                break;
        }
        
        m_construction->Select( selection_idx );
        Connect(m_construction->SelChangedSignal(), &SidePanel::PlanetPanel::BuildSelected, this);
        ////////////////////// v0.1 only!!
    }
}

bool SidePanel::PlanetPanel::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    ul.x += HUGE_PLANET_SIZE / 2;
    return ((ul <= pt && pt < lr) || InPlanet(pt));
}

int SidePanel::PlanetPanel::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();

    using boost::lexical_cast;
    using std::string;

    const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
    if(!planet)
        throw std::invalid_argument("SidePanel::PlanetPanel::Render planet not found!");

    // planet graphic
    int planet_image_sz = PlanetDiameter();
    GG::Pt planet_image_pos(ul.x + HUGE_PLANET_SIZE / 2 - planet_image_sz / 2, ul.y + Height() / 2 - planet_image_sz / 2);
    glColor4ubv(GG::CLR_WHITE.v);
    m_planet_graphic.OrthoBlit(planet_image_pos, planet_image_pos + GG::Pt(planet_image_sz, planet_image_sz), false);

    // planet description
    const double TEXT_POSITION_RADIUS = HUGE_PLANET_SIZE / 2.0 * 0.9;
    Uint32 format = GG::TF_LEFT | GG::TF_BOTTOM;
    boost::shared_ptr<GG::Font> planet_name_font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS);
    const double PLANET_NAME_FONT_HALF_HT = planet_name_font->Height() / 2.0;
    double y = Height() / 2.0 - TEXT_POSITION_RADIUS + PLANET_NAME_FONT_HALF_HT;
    GG::Pt posn(ul.x + HUGE_PLANET_SIZE / 2 + CircleXFromY(y - Height() / 2.0, TEXT_POSITION_RADIUS), ul.y + static_cast<int>(y));
    GG::Clr planet_name_color = ClientUI::TEXT_COLOR;
    const std::set<int>& owners = planet->Owners();
    if (!owners.empty()) {
        planet_name_color = HumanClientApp::Empires().Lookup(*owners.begin())->Color();
    }
    glColor4ubv(planet_name_color.v);
    int y1 = static_cast<int>(posn.y - PLANET_NAME_FONT_HALF_HT), y2 = static_cast<int>(posn.y + PLANET_NAME_FONT_HALF_HT);
    planet_name_font->RenderText(GG::Pt(posn.x, y1), GG::Pt(posn.x + 500, y2), planet->Name(), format, 0, false);

    glColor4ubv(ClientUI::TEXT_COLOR.v);
    boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, ClientUI::SIDE_PANEL_PTS);
    const double FONT_HALF_HT = font->Height() / 2.0;
    y += PLANET_NAME_FONT_HALF_HT + FONT_HALF_HT;
    posn = GG::Pt(ul.x + HUGE_PLANET_SIZE / 2 + CircleXFromY(y - Height() / 2.0, TEXT_POSITION_RADIUS), ul.y + static_cast<int>(y));
    std::string text;
    switch (planet->Size()) {
    case Planet::SZ_TINY: text += ClientUI::String("PL_SZ_TINY"); break;
    case Planet::SZ_SMALL: text += ClientUI::String("PL_SZ_SMALL"); break;
    case Planet::SZ_MEDIUM: text += ClientUI::String("PL_SZ_MEDIUM"); break;
    case Planet::SZ_LARGE: text += ClientUI::String("PL_SZ_LARGE"); break;
    case Planet::SZ_HUGE: text += ClientUI::String("PL_SZ_HUGE"); break;
    default: text += "ERROR "; break;
    }
    text += " ";
    switch (planet->Type()) {
    case Planet::TOXIC: text += ClientUI::String("PL_TOXIC"); break;
    case Planet::RADIATED: text += ClientUI::String("PL_RADIATED"); break;
    case Planet::BARREN: text += ClientUI::String("PL_BARREN"); break;
    case Planet::DESERT: text += ClientUI::String("PL_DESERT"); break;
    case Planet::TUNDRA: text += ClientUI::String("PL_TUNDRA"); break;
    case Planet::OCEAN: text += ClientUI::String("PL_OCEAN"); break;
    case Planet::TERRAN: text += ClientUI::String("PL_TERRAN"); break;
    case Planet::GAIA: text += ClientUI::String("PL_GAIA"); break;
    default: text += "ERROR "; break;
    }
    y1 = static_cast<int>(posn.y - FONT_HALF_HT);
    y2 = static_cast<int>(posn.y + FONT_HALF_HT);
    font->RenderText(GG::Pt(posn.x, y1), GG::Pt(posn.x + 500, y2), text, format, 0, false);

    // pop
    const int ICON_MARGIN = 5;
    if (!planet->Owners().empty()) {
        y += font->Height();
        posn = GG::Pt(ul.x + HUGE_PLANET_SIZE / 2 + CircleXFromY(y - Height() / 2.0, TEXT_POSITION_RADIUS), ul.y + static_cast<int>(y));
        text = lexical_cast<string>(static_cast<int>(planet->PopPoints()));
        text += "/";
        text += lexical_cast<string>(static_cast<int>(planet->MaxPop()));
        if (planet->PopGrowth()) {
            text += GG::RgbaTag(0 < planet->PopGrowth() ? ClientUI::STAT_INCR_COLOR : ClientUI::STAT_DECR_COLOR);
            text += (0 < planet->PopGrowth() ? " (+" : " (") + lexical_cast<string>(planet->PopGrowth()) + ")</rgba>";
        }
        y1 = static_cast<int>(posn.y - FONT_HALF_HT);
        y2 = static_cast<int>(posn.y + FONT_HALF_HT);
        m_pop_icon.OrthoBlit(posn.x, y1, posn.x + font->Height(), y1 + font->Height(), false);
        posn.x += font->Height() + ICON_MARGIN;
        font->RenderText(GG::Pt(posn.x, y1), GG::Pt(posn.x + 500, y2), text, format, 0, true);

        // industry
        y += font->Height();
        posn = GG::Pt(ul.x + HUGE_PLANET_SIZE / 2 + CircleXFromY(y - Height() / 2.0, TEXT_POSITION_RADIUS), ul.y + static_cast<int>(y));
        text = lexical_cast<string>(static_cast<int>(planet->ProdPoints()));
        y1 = static_cast<int>(posn.y - FONT_HALF_HT);
        y2 = static_cast<int>(posn.y + FONT_HALF_HT);
        m_industry_icon.OrthoBlit(posn.x, y1, posn.x + font->Height(), y1 + font->Height(), false);
        posn.x += font->Height() + ICON_MARGIN;
        font->RenderText(GG::Pt(posn.x, y1), GG::Pt(posn.x + 500, y2), text, format, 0, true);
    }

#if 0
    // TODO: grab the correct values for these from the planet, universe, empires, etc. in versions 0.2 and later

    // research
    y += font->Height();
    posn = GG::Pt(ul.x + HUGE_PLANET_SIZE / 2 + CircleXFromY(y - Height() / 2.0, TEXT_POSITION_RADIUS), ul.y + static_cast<int>(y));
    text = lexical_cast<string>(5);
    y1 = static_cast<int>(posn.y - FONT_HALF_HT);
    y2 = static_cast<int>(posn.y + FONT_HALF_HT);
    m_research_icon.OrthoBlit(posn.x, y1, posn.x + font->Height(), y1 + font->Height(), false);
    posn.x += font->Height() + ICON_MARGIN;
    font->RenderText(GG::Pt(posn.x, y1), GG::Pt(posn.x + 500, y2), text, format, 0, true);

    // mining
    y += font->Height();
    posn = GG::Pt(ul.x + HUGE_PLANET_SIZE / 2 + CircleXFromY(y - Height() / 2.0, TEXT_POSITION_RADIUS), ul.y + static_cast<int>(y));
    text = lexical_cast<string>(6);
    y1 = static_cast<int>(posn.y - FONT_HALF_HT);
    y2 = static_cast<int>(posn.y + FONT_HALF_HT);
    m_mining_icon.OrthoBlit(posn.x, y1, posn.x + font->Height(), y1 + font->Height(), false);
    posn.x += font->Height() + ICON_MARGIN;
    font->RenderText(GG::Pt(posn.x, y1), GG::Pt(posn.x + 500, y2), text, format, 0, true);

    // farming
    y += font->Height();
    posn = GG::Pt(ul.x + HUGE_PLANET_SIZE / 2 + CircleXFromY(y - Height() / 2.0, TEXT_POSITION_RADIUS), ul.y + static_cast<int>(y));
    text = lexical_cast<string>(7);
    if (-1) {
        text += GG::RgbaTag(0 < -1 ? ClientUI::STAT_INCR_COLOR : ClientUI::STAT_DECR_COLOR);
        text += (0 < -1 ? " (+": " (") + lexical_cast<string>(-1) + ")</rgba>";
    }
    y1 = static_cast<int>(posn.y - FONT_HALF_HT);
    y2 = static_cast<int>(posn.y + FONT_HALF_HT);
    m_farming_icon.OrthoBlit(posn.x, y1, posn.x + font->Height(), y1 + font->Height(), false);
    posn.x += font->Height() + ICON_MARGIN;
    font->RenderText(GG::Pt(posn.x, y1), GG::Pt(posn.x + 500, y2), text, format, 0, true);
#endif

    if (ProdCenter::SCOUT <= planet->CurrentlyBuilding()) {
        // construction progress bar
        // TODO : get the costs of the item from the list of available technologies
        const int PROD_COSTS[] = {0, 0, 0, 50, 250, 100, 200, 375, 700, 200};
        int cost = PROD_COSTS[planet->CurrentlyBuilding()];
        double percent_complete = cost ? planet->BuildProgress() / cost : 0.0;
        int x1 = ul.x + Width() - CONSTR_DROP_LIST_WIDTH - 3;
        int x2 = x1 + CONSTR_DROP_LIST_WIDTH;
        y1 = ul.y + Height() - ClientUI::SIDE_PANEL_PTS - CONSTR_PROGRESS_BAR_HT - 3;
        y2 = y1 + CONSTR_PROGRESS_BAR_HT;
        GG::FlatRectangle(x1, y1, x2, y2, GG::CLR_ZERO, ClientUI::CTRL_BORDER_COLOR, 1);
        GG::FlatRectangle(x1, y1, x1 + static_cast<int>((x2 - x1 - 2) * percent_complete), y2, 
                          ClientUI::SIDE_PANEL_BUILD_PROGRESSBAR_COLOR, LightColor(ClientUI::SIDE_PANEL_BUILD_PROGRESSBAR_COLOR), 1);

        // construction progress text
        format = GG::TF_RIGHT | GG::TF_VCENTER;
        if (cost && !planet->ProdPoints()) {
            text = "(never)";
        } else if (cost) {
            int turns_remaining = static_cast<int>(std::ceil((cost - planet->BuildProgress()) / planet->ProdPoints()));
            text = "(" + lexical_cast<string>(turns_remaining) + " turns)";
        } else {
            text = "";
        }
        y1 = ul.y + Height() - font->Height();
        y2 = y1 + font->Height();
        glColor4ubv(ClientUI::TEXT_COLOR.v);
        font->RenderText(x1, y1, x2, y2, text, format, 0, false);
    }

    return 1;
}

int SidePanel::PlanetPanel::PlanetDiameter() const
{
    const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
    if(!planet)
        throw std::invalid_argument("SidePanel::PlanetPanel::PlanetDiameter planet not found!");

    return TINY_PLANET_SIZE + (HUGE_PLANET_SIZE - TINY_PLANET_SIZE) / (Planet::SZ_HUGE - Planet::SZ_TINY) * planet->Size();
}

bool SidePanel::PlanetPanel::InPlanet(const GG::Pt& pt) const
{
    GG::Pt center = UpperLeft() + GG::Pt(HUGE_PLANET_SIZE / 2, Height() / 2);
    GG::Pt diff = pt - center;
    int r_squared = PlanetDiameter() * PlanetDiameter() / 4;
    return diff.x * diff.x + diff.y * diff.y <= r_squared;
}

void SidePanel::PlanetPanel::BuildSelected(int idx) const
{
    const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
    if(!planet)
        throw std::invalid_argument("SidePanel::PlanetPanel::BuildSelected planet not found!");

    HumanClientApp::Orders().IssueOrder(new PlanetBuildOrder(*planet->Owners().begin(), planet->ID(), m_construction_prod_idx[ idx ] ));
}


////////////////////////////////////////////////
// SidePanel
////////////////////////////////////////////////
SidePanel::SidePanel(int x, int y, int w, int h) : 
    Wnd(x, y, w, h, GG::Wnd::CLICKABLE),
    m_system(0),
    m_name_text(new GG::TextControl(0, 0, w, static_cast<int>(ClientUI::PTS * 1.5 + 4), "", ClientUI::FONT, 
                                    static_cast<int>(ClientUI::PTS * 1.5), GG::TF_CENTER | GG::TF_VCENTER, ClientUI::TEXT_COLOR)),
    m_first_row_shown(0),
    m_first_col_shown(0),
    m_vscroll(0),
    m_hscroll(0)
{
    SetText(ClientUI::String("SIDE_PANEL"));
    AttachChild(m_name_text);
    Hide();
}

bool SidePanel::InWindow(const GG::Pt& pt) const
{
    bool retval = UpperLeft() <= pt && pt < LowerRight();
    for (unsigned int i = 0; i < m_planet_panels.size() && !retval; ++i) {
        if (m_planet_panels[i]->InWindow(pt))
            retval = true;
    }
    return retval;
}

int SidePanel::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::SIDE_PANEL_COLOR, GG::CLR_ZERO, 0);
    return 1;
}

void SidePanel::SetSystem(int system_id)
{
    m_fleet_icons.clear();
    m_planet_panels.clear();
    DetachChild(m_name_text);
    DeleteChildren();
    AttachChild(m_name_text);
    m_vscroll = m_hscroll = 0;
    m_first_row_shown = m_first_col_shown = 0;
    Hide();

    m_system = dynamic_cast<const System*>(HumanClientApp::GetUniverse().Object(system_id));
    if (m_system) {
        *m_name_text << m_system->Name() + " System";

        // TODO: add fleet icons

        // add planets
        System::ConstObjectVec planets = m_system->FindObjects(IsPlanet);
        int y = m_name_text->Height();
        const int PLANET_PANEL_HT = HUGE_PLANET_SIZE;
        for (unsigned int i = 0; i < planets.size(); ++i, y += PLANET_PANEL_HT) {
            const Planet* planet = dynamic_cast<const Planet*>(planets[i]);
            PlanetPanel* planet_panel = new PlanetPanel(*planet, y, Width(), PLANET_PANEL_HT);
            AttachChild(planet_panel);
            m_planet_panels.push_back(planet_panel);
        }
        Show();
    }
}
