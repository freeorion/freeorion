#include "CensusBrowseWnd.h"

#include "../util/i18n.h"
#include "CUIControls.h"

class CensusRowPanel;

namespace {
    /** How big we want meter icons with respect to the current UI font size.
      * Meters should scale along font size, but not below the size for the
      * default 12 points font. */
    GG::Pt MeterIconSize() {
        const int icon_size = std::max(ClientUI::Pts(), 12) * 4/3;
        return GG::Pt(GG::X(icon_size), GG::Y(icon_size));
    }

    /** Returns height of rows of text in InfoTextBrowseWnd. */
    int IconTextBrowseWndRowHeight() {
        return ClientUI::Pts()*3/2;
    }

    constexpr int   EDGE_PAD(3);
    constexpr GG::Y ICON_BROWSE_ICON_HEIGHT{64};

    const GG::X BrowseTextWidth()
    { return GG::X(FontBasedUpscale(230)); }
}

class CensusRowPanel : public GG::Control {
public:
    CensusRowPanel(GG::X w, GG::Y h, const std::string& name, double census_val, int worlds, bool show_icon) :
        GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
        m_show_icon(show_icon)
    {
        if (m_show_icon)
            m_icon = GG::Wnd::Create<GG::StaticGraphic>(
                ClientUI::SpeciesIcon(name), GG::GRAPHIC_FITGRAPHIC);

        m_name = GG::Wnd::Create<CUILabel>(UserString(name), GG::FORMAT_RIGHT);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << census_val;
        m_census_val = GG::Wnd::Create<CUILabel>(ss.str(), GG::FORMAT_RIGHT);
        m_worlds = GG::Wnd::Create<CUILabel>(std::to_string(worlds), GG::FORMAT_RIGHT);
    }

    void CompleteConstruction() override {
        GG::Control::CompleteConstruction();

        SetChildClippingMode(ChildClippingMode::ClipToClient);

        if (m_show_icon)
            AttachChild(m_icon);

        AttachChild(m_name);
        AttachChild(m_census_val);
        AttachChild(m_worlds);

        DoLayout();
    }

    void Render() override {
        GG::Pt ul = UpperLeft();

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(static_cast<GLfloat>(Value(ul.x)), static_cast<GLfloat>(Value(ul.y)), 0.0f);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(1.0);
        glEnableClientState(GL_VERTEX_ARRAY);

        m_buffer.activate();
        glColor(ClientUI::WndColor());
        glDrawArrays(GL_TRIANGLE_FAN,   0, 4);
        glColor(ClientUI::WndOuterBorderColor());
        glDrawArrays(GL_LINE_LOOP,      0, 4);

        glEnable(GL_TEXTURE_2D);
        glPopMatrix();
        glDisableClientState(GL_VERTEX_ARRAY);
    }

private:
    void DoLayout() {
        const GG::X SPECIES_NAME_WIDTH{ClientUI::Pts() * 9};
        const GG::X SPECIES_CENSUS_WIDTH{ClientUI::Pts() * 5};
        const GG::X SPECIES_WORLDS_WIDTH{ClientUI::Pts() * 3};

        GG::X left(GG::X0);
        GG::Y bottom(MeterIconSize().y - GG::Y(EDGE_PAD));

        if (m_show_icon)
            m_icon->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + MeterIconSize().x, bottom));
        left += MeterIconSize().x + GG::X(EDGE_PAD);

        m_name->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + SPECIES_NAME_WIDTH, bottom));
        left += SPECIES_NAME_WIDTH;

        m_census_val->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + SPECIES_CENSUS_WIDTH, bottom));
        left += SPECIES_CENSUS_WIDTH;

        m_worlds->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + SPECIES_WORLDS_WIDTH, bottom));
        left += SPECIES_WORLDS_WIDTH;

        InitBuffer();
    }

    void InitBuffer() {
        const auto sz = Size();
        m_buffer.clear();

        m_buffer.store(0.0f,                            0.0f);
        m_buffer.store(static_cast<float>(Value(sz.x)), 0.0f);
        m_buffer.store(static_cast<float>(Value(sz.x)), static_cast<float>(Value(sz.y)));
        m_buffer.store(0.0f,                            static_cast<float>(Value(sz.y)));
        m_buffer.createServerBuffer();
    }

    GG::GL2DVertexBuffer                m_buffer;
    std::shared_ptr<GG::StaticGraphic>  m_icon;
    std::shared_ptr<GG::Label>          m_name;
    std::shared_ptr<GG::Label>          m_census_val;
    std::shared_ptr<GG::Label>          m_worlds;
    bool                                m_show_icon;
};

CensusBrowseWnd::CensusBrowseWnd(std::string                  title_text,
                                 float                        target_population,
                                 std::map<std::string, float> population_counts,
                                 std::map<std::string, int>   population_worlds,
                                 std::map<std::string, float> tag_counts,
                                 std::map<std::string, int>   tag_worlds,
                                 std::vector<std::string>     census_order
                                ):
    GG::BrowseInfoWnd(GG::X0, GG::Y0, BrowseTextWidth(), GG::Y1),
    m_title_text(GG::Wnd::Create<CUILabel>(std::move(title_text), GG::FORMAT_LEFT)),
    m_species_text(GG::Wnd::Create<CUILabel>(UserString("CENSUS_SPECIES_HEADER"), GG::FORMAT_BOTTOM)),
    m_list(GG::Wnd::Create<CUIListBox>()),
    m_tags_text(GG::Wnd::Create<CUILabel>(UserString("CENSUS_TAG_HEADER"), GG::FORMAT_BOTTOM)),
    m_tags_list(GG::Wnd::Create<CUIListBox>()),
    m_target_population(GG::Wnd::Create<CUILabel>(UserString("CENSUS_TARGET_POPULATION"), GG::FORMAT_BOTTOM)),
    m_total_population(GG::Wnd::Create<CUILabel>(UserString("CENSUS_TOTAL_POPULATION"), GG::FORMAT_BOTTOM)),
    m_total_worlds(GG::Wnd::Create<CUILabel>(UserString("CENSUS_TOTAL_WORLDS"), GG::FORMAT_BOTTOM)),
    m_offset(GG::X0, ICON_BROWSE_ICON_HEIGHT/2),
    m_target_population_value(std::move(target_population)),
    m_population_counts(std::move(population_counts)),
    m_population_worlds(std::move(population_worlds)),
    m_tag_counts(std::move(tag_counts)),
    m_tag_worlds(std::move(tag_worlds)),
    m_census_order(std::move(census_order))
{}

void CensusBrowseWnd::CompleteConstruction() {
    GG::BrowseInfoWnd::CompleteConstruction();

    const GG::Y ROW_HEIGHT(MeterIconSize().y);
    const GG::Y HALF_HEIGHT(GG::Y(int(ClientUI::Pts()/2)));

    GG::Y top = GG::Y0;

    m_title_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top + m_offset.y));
    m_title_text->Resize(GG::Pt(BrowseTextWidth(), ROW_HEIGHT));
    m_title_text->SetFont(ClientUI::GetBoldFont());

    top += ROW_HEIGHT;
    m_species_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top + m_offset.y));
    m_species_text->Resize(GG::Pt(BrowseTextWidth(), ROW_HEIGHT + HALF_HEIGHT));

    top += ROW_HEIGHT + HALF_HEIGHT;
    m_list->MoveTo(GG::Pt(m_offset.x, top + m_offset.y));
    m_list->Resize(GG::Pt(BrowseTextWidth(), ROW_HEIGHT));
    m_list->SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);
    m_list->SetInteriorColor(ClientUI::WndColor());
    // preinitialize listbox/row column widths, because what ListBox::Insert does on default is not suitable for this case
    m_list->SetNumCols(1);
    m_list->SetColWidth(0, GG::X0);
    m_list->LockColWidths();

    // put into multimap to sort by population, ascending
    std::multimap<float, std::string> counts_species;
    for (const auto& [name, count] : m_population_counts)
        counts_species.emplace(count, name);
    m_population_counts.clear();

    float total_population = 0;
    int total_worlds = 0;

    // add species rows
    for (auto it = counts_species.rbegin(); it != counts_species.rend(); ++it) {
        total_population += it->first;
        total_worlds += m_population_worlds[it->second];
        auto row = GG::Wnd::Create<GG::ListBox::Row>(m_list->Width(), ROW_HEIGHT);
        row->SetDragDropDataType("Census Species Row");
        row->push_back(GG::Wnd::Create<CensusRowPanel>(m_list->Width(), ROW_HEIGHT,
                                                       it->second, it->first,
                                                       m_population_worlds[it->second], true));
        m_list->Insert(row);
        row->Resize(GG::Pt(m_list->Width(), ROW_HEIGHT));
        top += ROW_HEIGHT;
    }
    top += (EDGE_PAD*3);
    m_list->Resize(GG::Pt(BrowseTextWidth(), top - 2* ROW_HEIGHT - HALF_HEIGHT));

    GG::Y top2 = top;
    m_tags_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top2 + m_offset.y));
    m_tags_text->Resize(GG::Pt(BrowseTextWidth(), ROW_HEIGHT + HALF_HEIGHT));

    top2 += ROW_HEIGHT + HALF_HEIGHT;
    m_tags_list->MoveTo(GG::Pt(m_offset.x, top2 + m_offset.y));
    m_tags_list->Resize(GG::Pt(BrowseTextWidth(), ROW_HEIGHT));
    m_tags_list->SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);
    m_tags_list->SetInteriorColor(ClientUI::WndColor());
    // preinitialize listbox/row column widths, because what ListBox::Insert does on default is not suitable for this case
    m_tags_list->SetNumCols(1);
    m_tags_list->SetColWidth(0, GG::X0);
    m_tags_list->LockColWidths();

    AttachChild(m_title_text);
    AttachChild(m_species_text);
    AttachChild(m_list);
    AttachChild(m_tags_text);
    AttachChild(m_tags_list);

    // add tags/characteristics rows
    for (const std::string& tag_ord : m_census_order) {
        //DebugLogger() << "Census checking for tag '"<< tag_ord <<"'";
        auto it2 = m_tag_counts.find(tag_ord);
        if (it2 != m_tag_counts.end()) {
            auto row = GG::Wnd::Create<GG::ListBox::Row>(m_list->Width(), ROW_HEIGHT);
            row->SetDragDropDataType("Census Characteristics Row");
            row->push_back(GG::Wnd::Create<CensusRowPanel>(m_tags_list->Width(), ROW_HEIGHT, it2->first, it2->second, m_tag_worlds[it2->first], false));
            m_tags_list->Insert(row);
            row->Resize(GG::Pt(m_list->Width(), ROW_HEIGHT));
            top2 += ROW_HEIGHT;
        }
    }
    m_tag_counts.clear();

    m_tags_list->Resize(GG::Pt(BrowseTextWidth(), top2 -top -ROW_HEIGHT - HALF_HEIGHT + (EDGE_PAD*3)));

    top2 += ROW_HEIGHT/2;

    m_total_worlds->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top2 + m_offset.y));
    m_total_worlds->Resize(GG::Pt(BrowseTextWidth(), ROW_HEIGHT + HALF_HEIGHT));
    m_total_worlds->SetText(boost::io::str(FlexibleFormat(UserString("CENSUS_TOTAL_WORLDS")) % total_worlds));
    top2 += ROW_HEIGHT;

    m_total_population->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top2 + m_offset.y));
    m_total_population->Resize(GG::Pt(BrowseTextWidth(), ROW_HEIGHT + HALF_HEIGHT));
    std::stringstream s1;
    s1 << std::fixed << std::setprecision(1) << total_population;
    m_total_population->SetText(boost::io::str(FlexibleFormat(UserString("CENSUS_TOTAL_POPULATION")) % s1.str() ));
    top2 += ROW_HEIGHT;

    m_target_population->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top2 + m_offset.y));
    m_target_population->Resize(GG::Pt(BrowseTextWidth(), ROW_HEIGHT + HALF_HEIGHT));
    std::stringstream s2;
    s2 << std::fixed << std::setprecision(1) << m_target_population_value;
    m_target_population->SetText(boost::io::str(FlexibleFormat(UserString("CENSUS_TARGET_POPULATION")) % s2.str() ));
    top2 += ROW_HEIGHT;

    AttachChild(m_total_worlds);
    AttachChild(m_total_population);
    AttachChild(m_target_population);

    Resize(GG::Pt(BrowseTextWidth(), top2  + (EDGE_PAD*3)));

    InitBuffer();
}

bool CensusBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void CensusBrowseWnd::InitBuffer() {
    const auto sz = Size();
    const float ROW_HEIGHT(IconTextBrowseWndRowHeight());

    m_buffer.clear();

    m_buffer.store(static_cast<float>(sz.x), ROW_HEIGHT);
    m_buffer.store(0.0f,                     ROW_HEIGHT);

    m_buffer.store(0.0f,                     0.0f);
    m_buffer.store(static_cast<float>(sz.x), 0.0f);
    m_buffer.store(static_cast<float>(sz.x), static_cast<float>(sz.y));
    m_buffer.store(0.0f,                     static_cast<float>(sz.y));
    m_buffer.createServerBuffer();
}

void CensusBrowseWnd::Render() {
    const GG::Pt ul = UpperLeft() + m_offset;

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(ul.x), static_cast<GLfloat>(ul.y), 0.0f);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0);
    glEnableClientState(GL_VERTEX_ARRAY);

    m_buffer.activate();
    glColor(ClientUI::WndColor());
    glDrawArrays(GL_TRIANGLE_FAN,   2, 4);
    glColor(ClientUI::WndOuterBorderColor());
    glDrawArrays(GL_LINE_LOOP,      2, 4);
    glDrawArrays(GL_TRIANGLE_FAN,   0, 4);

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
}
