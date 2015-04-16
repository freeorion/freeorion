#include "CensusBrowseWnd.h"

#include "../util/i18n.h"

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

    const int       EDGE_PAD(3);

    const GG::Y     ICON_BROWSE_ICON_HEIGHT(64);
    const GG::X     BROWSE_TEXT_WIDTH(200);
}

class CensusRowPanel : public GG::Control {
public:
    CensusRowPanel(GG::X w, GG::Y h, const std::string& name, double census_val, bool show_icon) :
        GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
        m_icon(0),
        m_name(0),
        m_census_val(0)
    {
        m_show_icon = show_icon;
        SetChildClippingMode(ClipToClient);

        if (m_show_icon) {
            m_icon = new GG::StaticGraphic(ClientUI::SpeciesIcon(name), GG::GRAPHIC_FITGRAPHIC);
            AttachChild(m_icon);
        }

        m_name = new CUILabel(UserString(name), GG::FORMAT_RIGHT);

        int num_digits = census_val < 10 ? 1 : 2; // this allows the decimal point to line up when there number above and below 10.
        num_digits =    census_val < 100 ? num_digits : 3; // this allows the decimal point to line up when there number above and below 100.
        num_digits =   census_val < 1000 ? num_digits : 4; // this allows the decimal point to line up when there number above and below 1000.
        m_census_val = new CUILabel(DoubleToString(census_val, num_digits, false), GG::FORMAT_RIGHT);

        AttachChild(m_name);
        AttachChild(m_census_val);

        DoLayout();
    }

    virtual void    Render()
    { GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), ClientUI::WndColor(), 1u); }

private:
    void            DoLayout() {
        const GG::X SPECIES_NAME_WIDTH(ClientUI::Pts() * 9);
        const GG::X SPECIES_CENSUS_WIDTH(ClientUI::Pts() * 5);

        GG::X left(GG::X0);
        GG::Y bottom(MeterIconSize().y - GG::Y(EDGE_PAD));

        if (m_show_icon)
            m_icon->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + MeterIconSize().x, bottom));
        left += MeterIconSize().x + GG::X(EDGE_PAD);

        m_name->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + SPECIES_NAME_WIDTH, bottom));
        left += SPECIES_NAME_WIDTH;

        m_census_val->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + SPECIES_CENSUS_WIDTH, bottom));
        left += SPECIES_CENSUS_WIDTH;
    }

    GG::StaticGraphic*  m_icon;
    CUILabel*           m_name;
    CUILabel*           m_census_val;
    bool                m_show_icon;
};

CensusBrowseWnd::CensusBrowseWnd(const std::string& title_text,
                                 const std::map<std::string, float>& population_counts, const std::map<std::string, float>& tag_counts) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, BROWSE_TEXT_WIDTH, GG::Y1),
    m_title_text(0),
    m_species_text(0),
    m_list(0),
    m_tags_text(0),
    m_tags_list(0)
    {
    const GG::Y ROW_HEIGHT(MeterIconSize().y);
    const GG::Y HALF_HEIGHT(GG::Y(int(ClientUI::Pts()/2)));
    
    GG::Y top = GG::Y0;

    m_row_height = GG::Y(MeterIconSize().y);

    m_offset = GG::Pt(GG::X0, ICON_BROWSE_ICON_HEIGHT/2); //lower the window

    m_title_text = new CUILabel(title_text, GG::FORMAT_LEFT);
    m_title_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top + m_offset.y));
    m_title_text->Resize(GG::Pt(BROWSE_TEXT_WIDTH, ROW_HEIGHT));
    m_title_text->SetFont(ClientUI::GetBoldFont());

    top += ROW_HEIGHT;
    m_species_text = new CUILabel(UserString("CENSUS_SPECIES_HEADER"), GG::FORMAT_BOTTOM);
    m_species_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top + m_offset.y));
    m_species_text->Resize(GG::Pt(BROWSE_TEXT_WIDTH, ROW_HEIGHT+HALF_HEIGHT));

    top += ROW_HEIGHT+HALF_HEIGHT;
    m_list = new CUIListBox();
    m_list->MoveTo(GG::Pt(m_offset.x, top + m_offset.y));
    m_list->Resize(GG::Pt(BROWSE_TEXT_WIDTH, ROW_HEIGHT));
    m_list->SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);
    m_list->SetInteriorColor(ClientUI::WndColor());
    // preinitialize listbox/row column widths, because what ListBox::Insert does on default is not suitable for this case
    m_list->SetNumCols(1);
    m_list->SetColWidth(0, GG::X0);
    m_list->LockColWidths();

    // put into multimap to sort by population, ascending
    std::multimap<float, std::string> counts_species;
    for (std::map<std::string, float>::const_iterator it = population_counts.begin();
         it != population_counts.end(); ++it)
    { counts_species.insert(std::make_pair(it->second, it->first)); }

    // add species rows
    for (std::multimap<float, std::string>::const_reverse_iterator it = counts_species.rbegin();
         it != counts_species.rend(); ++it)
    {
        GG::ListBox::Row* row = new GG::ListBox::Row(m_list->Width(), ROW_HEIGHT, "Census Species Row");
        row->push_back(new CensusRowPanel(m_list->Width(), ROW_HEIGHT, it->second, it->first, true));
        m_list->Insert(row);
        row->Resize(GG::Pt(m_list->Width(), ROW_HEIGHT));
        top += m_row_height;
    }
    top += (EDGE_PAD*3);
    m_list->Resize(GG::Pt(BROWSE_TEXT_WIDTH, top - 2* ROW_HEIGHT - HALF_HEIGHT));

    GG::Y top2 = top;
    m_tags_text = new CUILabel(UserString("CENSUS_TAG_HEADER"), GG::FORMAT_BOTTOM);
    m_tags_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, top2 + m_offset.y));
    m_tags_text->Resize(GG::Pt(BROWSE_TEXT_WIDTH, ROW_HEIGHT + HALF_HEIGHT));

    top2 += ROW_HEIGHT + HALF_HEIGHT;
    m_tags_list = new CUIListBox();
    m_tags_list->MoveTo(GG::Pt(m_offset.x, top2 + m_offset.y));
    m_tags_list->Resize(GG::Pt(BROWSE_TEXT_WIDTH, ROW_HEIGHT));
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

    // determine tag order
    //DebugLogger() << "Census Tag Order: " << UserString("CENSUS_TAG_ORDER");
    std::istringstream tag_stream(UserString("CENSUS_TAG_ORDER"));
    std::vector<std::string> tag_order;
    std::copy(std::istream_iterator<std::string>(tag_stream), 
        std::istream_iterator<std::string>(),
        std::back_inserter<std::vector<std::string> >(tag_order));

    // add tags/characteristics rows
    //for (std::map<std::string, float>::const_iterator it = tag_counts.begin(); it != tag_counts.end(); ++it) {
    for (std::vector<std::string>::iterator it = tag_order.begin(); it != tag_order.end(); ++it) {
        //DebugLogger() << "Census checking for tag '"<< *it <<"'";
        std::map<std::string, float>::const_iterator it2 = tag_counts.find(*it);
        if (it2 != tag_counts.end()) {
            GG::ListBox::Row* row = new GG::ListBox::Row(m_list->Width(), ROW_HEIGHT, "Census Characteristics Row");
            row->push_back(new CensusRowPanel(m_tags_list->Width(), ROW_HEIGHT, it2->first, it2->second, false));
            m_tags_list->Insert(row);
            row->Resize(GG::Pt(m_list->Width(), ROW_HEIGHT));
            top2 += m_row_height;
        }
    }

    m_tags_list->Resize(GG::Pt(BROWSE_TEXT_WIDTH, top2 -top -ROW_HEIGHT - HALF_HEIGHT + (EDGE_PAD*3)));

    Resize(GG::Pt(BROWSE_TEXT_WIDTH, top2  + (EDGE_PAD*3)));
}

bool CensusBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void CensusBrowseWnd::Render() {
    GG::Pt      ul = UpperLeft();
    GG::Pt      lr = LowerRight();
    const GG::Y ROW_HEIGHT(IconTextBrowseWndRowHeight());
    // main background
    GG::FlatRectangle(ul + m_offset, lr + m_offset, ClientUI::WndColor(),
                      ClientUI::WndOuterBorderColor(), 1);
    // top title filled background
    GG::FlatRectangle(ul + m_offset, GG::Pt(lr.x, ul.y + ROW_HEIGHT) + m_offset,
                      ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);
}
