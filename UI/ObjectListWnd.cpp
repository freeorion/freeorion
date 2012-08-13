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
// ObjectListBox
////////////////////////////////////////////////
class ObjectListBox : public CUIListBox {
public:
    ObjectListBox() :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1)
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
};

ObjectListWnd::ObjectListWnd(GG::X w, GG::Y h) :
    CUIWnd(UserString("MAP_BTN_OBJECTS"), GG::X1, GG::Y1, w - 1, h - 1, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE),
    m_list_box(0)
{
    m_list_box = new ObjectListBox();
    m_list_box->SetHiliteColor(GG::CLR_ZERO);
    m_list_box->SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);

    AttachChild(m_list_box);
    DoLayout();
}

void ObjectListWnd::DoLayout()
{ m_list_box->SizeMove(GG::Pt(), GG::Pt(ClientWidth(), ClientHeight() - GG::Y(INNER_BORDER_ANGLE_OFFSET))); }

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

            GG::X left(ICON_WIDTH * m_indent);
            GG::Y top(GG::Y0);
            GG::Y bottom(ClientHeight());
            GG::X PAD(3);

            GG::X ctrl_width = ICON_WIDTH;
            m_icon->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
            left += ctrl_width + PAD;

            ctrl_width = GG::X(ClientUI::Pts()*12);
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

    for (ObjectMap::const_iterator it = objects.const_begin(); it != objects.const_end(); ++it) {
        const UniverseObject* obj = it->second;
        ObjectRow* object_row = new ObjectRow(row_size.x, row_size.y, obj, 0);
        m_list_box->Insert(object_row);
        object_row->Resize(row_size);
    }
}
