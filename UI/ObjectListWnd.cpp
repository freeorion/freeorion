#include "ObjectListWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "FleetButton.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/AppInterface.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/System.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"

#include <GG/DrawUtil.h>

////////////////////////////////////////////////
// FilterDialog
////////////////////////////////////////////////
class FilterDialog : public CUIWnd {
public:
    FilterDialog(GG::X x, GG::Y y) : CUIWnd(UserString("FILTERS"), x, y, GG::X(200), GG::Y(200),
                                            GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE)
    {}
    ~FilterDialog()
    {}
    //@}
private:
};

////////////////////////////////////////////////
// ObjectListBox
////////////////////////////////////////////////
class ObjectListBox : public CUIListBox {
public:
    ObjectListBox() :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_collapsed_objects()
    {
        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        SetNumCols(1);
        SetColWidth(0, GG::X0);
        LockColWidths();
    }

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "ObjectListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "ObjectListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (GG::ListBox::iterator it = begin(); it != end(); ++it)
                (*it)->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const
    { return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight()); }

    static GG::Y    ListRowHeight()
    { return GG::Y(ClientUI::Pts() * 2); }

    void            CollapseObject(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID) {
            // collapse all objects
        } else {
        }
    }

    void            ExpandObject(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID) {
            // collapse all objects
        } else {
        }
    }
private:
    std::set<int>   m_collapsed_objects;
};

ObjectListWnd::ObjectListWnd(GG::X w, GG::Y h) :
    CUIWnd(UserString("MAP_BTN_OBJECTS"), GG::X1, GG::Y1, w - 1, h - 1, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE),
    m_list_box(0),
    m_filter_button(0),
    m_sort_button(0),
    m_columns_button(0),
    m_collapse_button(0)
{
    m_list_box = new ObjectListBox();
    m_list_box->SetHiliteColor(GG::CLR_ZERO);
    m_list_box->SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);
    GG::Connect(m_list_box->DoubleClickedSignal,    &ObjectListWnd::ObjectDoubleClicked,    this);
    GG::Connect(m_list_box->RightClickedSignal,     &ObjectListWnd::ObjectRightClicked,     this);
    AttachChild(m_list_box);

    m_filter_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("FILTERS"));
    GG::Connect(m_filter_button->ClickedSignal,     &ObjectListWnd::FilterClicked,          this);
    AttachChild(m_filter_button);

    m_sort_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("SORT"));
    GG::Connect(m_sort_button->ClickedSignal,       &ObjectListWnd::SortClicked,            this);
    AttachChild(m_sort_button);
    m_sort_button->Disable();

    m_columns_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("COLUMNS"));
    GG::Connect(m_columns_button->ClickedSignal,    &ObjectListWnd::ColumnsClicked,         this);
    AttachChild(m_columns_button);
    m_columns_button->Disable();

    m_collapse_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("COLLAPSE_ALL"));
    GG::Connect(m_collapse_button->ClickedSignal,   &ObjectListWnd::CollapseExpandClicked,  this);
    AttachChild(m_collapse_button);
    m_collapse_button->Disable();

    DoLayout();
}

void ObjectListWnd::DoLayout() {
    GG::X BUTTON_WIDTH(ClientUI::Pts()*7);
    GG::Y BUTTON_HEIGHT = m_filter_button->Height();
    int PAD(3);

    GG::Pt button_ul(GG::X0, ClientHeight() - BUTTON_HEIGHT);

    m_filter_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_sort_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_columns_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_collapse_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);

    m_list_box->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(ClientWidth(), button_ul.y));

    SetMinSize(GG::Pt(5*BUTTON_WIDTH, 6*BUTTON_HEIGHT));
}

void ObjectListWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

namespace {
    static std::string EMPTY_STRING;

    std::vector<boost::shared_ptr<GG::Texture> > ObjectTextures(const UniverseObject* obj) {
        std::vector<boost::shared_ptr<GG::Texture> > retval;

        if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
            if (const ShipDesign* design = ship->Design())
                retval.push_back(ClientUI::ShipDesignIcon(design->ID()));
            else
                retval.push_back(ClientUI::ShipDesignIcon(INVALID_OBJECT_ID));  // default icon
        } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(obj)) {
            boost::shared_ptr<GG::Texture> head_icon = FleetHeadIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
            retval.push_back(head_icon);
            boost::shared_ptr<GG::Texture> size_icon = FleetSizeIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
            retval.push_back(size_icon);
        } else if (const System* system = universe_object_cast<const System*>(obj)) {
            StarType star_type = system->GetStarType();
            ClientUI* ui = ClientUI::GetClientUI();
            boost::shared_ptr<GG::Texture> disc_texture = ui->GetModuloTexture(
                ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[star_type], system->ID());
            retval.push_back(disc_texture);
            boost::shared_ptr<GG::Texture> halo_texture = ui->GetModuloTexture(
                ClientUI::ArtDir() / "stars", ClientUI::HaloStarTypeFilePrefixes()[star_type], system->ID());
            retval.push_back(halo_texture);
        } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {

        } else if (const Building* building = universe_object_cast<const Building*>(obj)) {
            boost::shared_ptr<GG::Texture> texture = ClientUI::BuildingIcon(building->TypeName());
            retval.push_back(texture);
        }
        if (retval.empty())
            retval.push_back(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "generic_object.png", true));
        return retval;
    }

    const std::string& ObjectName(const UniverseObject* obj) {
        if (!obj)
            return EMPTY_STRING;
        if (const System* system = universe_object_cast<const System*>(obj))
            return system->ApparentName(HumanClientApp::GetApp()->EmpireID());
        return obj->PublicName(HumanClientApp::GetApp()->EmpireID());
    }

    std::pair<std::string, GG::Clr> ObjectEmpireNameAndColour(const UniverseObject* obj) {
        if (!obj)
            return std::make_pair("", ClientUI::TextColor());
        if (const Empire* empire = Empires().Lookup(obj->Owner()))
            return std::make_pair(empire->Name(), empire->Color());
        return std::make_pair("", ClientUI::TextColor());
    }

    class ObjectPanel : public GG::Control {
    public:
        ObjectPanel(GG::X w, GG::Y h, const UniverseObject* obj, int indent = 0) :
            Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
            m_object_id(obj ? obj->ID() : INVALID_OBJECT_ID),
            m_indent(indent),
            m_icon(0),
            m_name_label(0),
            m_empire_label(0)
        {
            Update();
            DoLayout();
        }
        virtual void        Render() {
            GG::Clr background_clr = this->Disabled() ? ClientUI::WndColor() : ClientUI::CtrlColor();
            GG::FlatRectangle(UpperLeft(), LowerRight(), background_clr, ClientUI::WndOuterBorderColor(), 1u);
        }
        virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::Control::SizeMove(ul, lr);
            if (old_size != Size())
                DoLayout();
        }
        void                Update() {
            delete m_icon;              m_icon = 0;
            delete m_empire_label;      m_empire_label = 0;

            const UniverseObject* obj = GetUniverseObject(m_object_id);
            if (!obj)
                return;

            boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
            GG::Clr clr = ClientUI::TextColor();
            int client_empire_id = HumanClientApp::GetApp()->EmpireID();

            GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE;
            std::vector<boost::shared_ptr<GG::Texture> > textures = ObjectTextures(obj);

            m_icon = new MultiTextureStaticGraphic(GG::X0, GG::Y0, GG::X(Value(ClientHeight())), ClientHeight(),
                                                   textures, std::vector<GG::Flags<GG::GraphicStyle> >(textures.size(), style));
            AttachChild(m_icon);


            m_name_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, ObjectName(obj), font, clr, GG::FORMAT_LEFT);
            AttachChild(m_name_label);

            std::pair<std::string, GG::Clr> empire_and_colour = ObjectEmpireNameAndColour(obj);
            m_empire_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, empire_and_colour.first, font, empire_and_colour.second, GG::FORMAT_LEFT);
            AttachChild(m_empire_label);
        }
    private:
        void                DoLayout() {
            const GG::Y ICON_HEIGHT(ClientHeight());
            const GG::X ICON_WIDTH(Value(ClientHeight()));

            GG::X indent(ICON_WIDTH * m_indent / 2);
            GG::X left = indent;
            GG::Y top(GG::Y0);
            GG::Y bottom(ClientHeight());
            GG::X PAD(3);

            GG::X ctrl_width = ICON_WIDTH;
            m_icon->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
            left += ctrl_width + PAD;

            ctrl_width = GG::X(ClientUI::Pts()*12) - indent;    // so second column all line up
            m_name_label->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
            left += ctrl_width + PAD;

            ctrl_width = GG::X(ClientUI::Pts()*8);
            m_empire_label->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
            left += ctrl_width + PAD;
        }

        int                 m_object_id;
        int                 m_indent;

        MultiTextureStaticGraphic*  m_icon;
        GG::TextControl*            m_name_label;
        GG::TextControl*            m_empire_label;
    };

    ////////////////////////////////////////////////
    // ObjectRow
    ////////////////////////////////////////////////
    class ObjectRow : public GG::ListBox::Row {
    public:
        ObjectRow(GG::X w, GG::Y h, const UniverseObject* obj, int indent) :
            GG::ListBox::Row(w, h, "ObjectRow", GG::ALIGN_CENTER, 1),
            m_panel(0),
            m_object_id(obj ? obj->ID() : INVALID_OBJECT_ID)
        {
            SetName("ObjectRow");
            SetChildClippingMode(ClipToClient);
            SetDragDropDataType("ObjectRow");
            m_panel = new ObjectPanel(w, h, obj, indent);
            push_back(m_panel);
        }

        int     ObjectID() const
        { return m_object_id; }

        void    Update() {
            if (m_panel)
                m_panel->Update();
        }

        /** This function overridden because otherwise, rows don't expand
          * larger than their initial size when resizing the list. */
        void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "ObjectRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }
    private:
        ObjectPanel*    m_panel;
        int             m_object_id;
    };
}

void ObjectListWnd::Update() {
    const ObjectMap& objects = Objects();
    m_list_box->Clear();

    const GG::Pt row_size = m_list_box->ListRowSize();

    bool nested = true;

    if (nested) {
        // sort objects by containment associations
        std::map<int, const System*>                    systems;
        std::map<int, std::map<int, const Fleet*> >     system_fleets;
        std::map<int, std::map<int, const Ship*> >      fleet_ships;
        std::map<int, std::map<int, const Planet*> >    system_planets;
        std::map<int, std::map<int, const Building*> >  planet_buildings;
        for (ObjectMap::const_iterator it = objects.const_begin(); it != objects.const_end(); ++it) {
            const UniverseObject* obj = it->second;
            if (const System* system = universe_object_cast<const System*>(obj)) {
                systems[obj->ID()] = system;
            } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(obj)) {
                system_fleets[fleet->SystemID()][fleet->ID()] = fleet;
            } else if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
                fleet_ships[ship->FleetID()][ship->ID()] = ship;
            } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
                system_planets[planet->SystemID()][planet->ID()] = planet;
            } else if (const Building* building = universe_object_cast<const Building*>(obj)) {
                planet_buildings[building->PlanetID()][building->ID()] = building;
            }
        }

        ObjectRow* object_row = 0;
        int indent = 0;

        // add system rows
        for (std::map<int, const System*>::const_iterator sys_it = systems.begin(); sys_it != systems.end(); ++sys_it) {
            int system_id = sys_it->first;
            const System* system = sys_it->second;
            // add system row
            object_row = new ObjectRow(row_size.x, row_size.y, system, indent);
            m_list_box->Insert(object_row);
            object_row->Resize(row_size);

            ++indent;
            // add planet rows in this system
            std::map<int, std::map<int, const Planet*> >::const_iterator sp_it = system_planets.find(system_id);
            if (sp_it != system_planets.end()) {
                const std::map<int, const Planet*>& planets = sp_it->second;
                for (std::map<int, const Planet*>::const_iterator planet_it = planets.begin(); planet_it != planets.end(); ++planet_it) {
                    int planet_id = planet_it->first;
                    const Planet* planet = planet_it->second;
                    // add Planet row
                    object_row = new ObjectRow(row_size.x, row_size.y, planet, indent);
                    m_list_box->Insert(object_row);
                    object_row->Resize(row_size);

                    ++indent;
                    // add building rows on this planet
                    std::map<int, std::map<int, const Building*> >::const_iterator pb_it = planet_buildings.find(planet_id);
                    if (pb_it != planet_buildings.end()) {
                        const std::map<int, const Building*>& buildings = pb_it->second;
                        for (std::map<int, const Building*>::const_iterator building_it = buildings.begin(); building_it != buildings.end(); ++building_it) {
                            int building_id = building_it->first;
                            const Building* building = building_it->second;
                            // add Building row
                            object_row = new ObjectRow(row_size.x, row_size.y, building, indent);
                            m_list_box->Insert(object_row);
                            object_row->Resize(row_size);
                        }
                    }
                    indent--;
                }
            }

            // add fleet rows in this system
            std::map<int, std::map<int, const Fleet*> >::const_iterator sf_it = system_fleets.find(system_id);
            if (sf_it != system_fleets.end()) {
                const std::map<int, const Fleet*>& fleets = sf_it->second;
                for (std::map<int, const Fleet*>::const_iterator fleet_it = fleets.begin(); fleet_it != fleets.end(); ++fleet_it) {
                    int fleet_id = fleet_it->first;
                    const Fleet* fleet = fleet_it->second;
                    // add Fleet row
                    object_row = new ObjectRow(row_size.x, row_size.y, fleet, indent);
                    m_list_box->Insert(object_row);
                    object_row->Resize(row_size);

                    ++indent;
                    // add ship rows on this fleet
                    std::map<int, std::map<int, const Ship*> >::const_iterator fs_it = fleet_ships.find(fleet_id);
                    if (fs_it != fleet_ships.end()) {
                        const std::map<int, const Ship*>& ships = fs_it->second;
                        for (std::map<int, const Ship*>::const_iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
                            int ship_id = ship_it->first;
                            const Ship* ship = ship_it->second;
                            // add Building row
                            object_row = new ObjectRow(row_size.x, row_size.y, ship, indent);
                            m_list_box->Insert(object_row);
                            object_row->Resize(row_size);
                        }
                    }
                    indent--;
                }
            }

            indent--;
        }

        // add fleets outside systems...
        std::map<int, std::map<int, const Fleet*> >::const_iterator sf_it = system_fleets.find(INVALID_OBJECT_ID);
        if (sf_it != system_fleets.end()) {
            const std::map<int, const Fleet*>& fleets = sf_it->second;
            for (std::map<int, const Fleet*>::const_iterator fleet_it = fleets.begin(); fleet_it != fleets.end(); ++fleet_it) {
                int fleet_id = fleet_it->first;
                const Fleet* fleet = fleet_it->second;
                // add Fleet row
                object_row = new ObjectRow(row_size.x, row_size.y, fleet, indent);
                m_list_box->Insert(object_row);
                object_row->Resize(row_size);

                ++indent;
                // add ship rows on this fleet
                std::map<int, std::map<int, const Ship*> >::const_iterator fs_it = fleet_ships.find(fleet_id);
                if (fs_it != fleet_ships.end()) {
                    const std::map<int, const Ship*>& ships = fs_it->second;
                    for (std::map<int, const Ship*>::const_iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
                        int ship_id = ship_it->first;
                        const Ship* ship = ship_it->second;
                        // add Building row
                        object_row = new ObjectRow(row_size.x, row_size.y, ship, indent);
                        m_list_box->Insert(object_row);
                        object_row->Resize(row_size);
                    }
                }
                indent--;
            }
        }
    }
}

void ObjectListWnd::ObjectDoubleClicked(GG::ListBox::iterator it) {
    int object_id = ObjectInRow(it);
    if (object_id != INVALID_OBJECT_ID)
        ObjectDoubleClickedSignal(object_id);
    ClientUI::GetClientUI()->ZoomToObject(object_id);
}

void ObjectListWnd::ObjectRightClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    int object_id = ObjectInRow(it);
    if (object_id == INVALID_OBJECT_ID)
        return;

    // create popup menu with diplomacy options in it
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("DUMP"), 1, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor());
    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: {
            ObjectDumpSignal(object_id);
            break;
        }
        default:
            break;
        }
    }
}

int ObjectListWnd::ObjectInRow(GG::ListBox::iterator it) const {
    if (it == m_list_box->end())
        return INVALID_OBJECT_ID;

    if (ObjectRow* obj_row = dynamic_cast<ObjectRow*>(*it))
        return obj_row->ObjectID();

    return INVALID_OBJECT_ID;
}

void ObjectListWnd::FilterClicked() {
}

void ObjectListWnd::SortClicked() {
}

void ObjectListWnd::ColumnsClicked() {
}

void ObjectListWnd::CollapseExpandClicked() {
}
