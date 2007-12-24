#include "DesignWnd.h"

#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/ShipDesign.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/format.hpp>
#include <cmath>

using boost::lexical_cast;

namespace {
    struct SelectPartFunctor {
        SelectPartFunctor(DesignWnd* design_wnd, int slot) : m_design_wnd(design_wnd), m_slot(slot) {}
        void operator()(int index) {m_design_wnd->PartSelected(index, m_slot);}
        DesignWnd* const m_design_wnd;
        int m_slot;
    };
}

//////////////////////////////////////////////////
// DesignWnd                                    //
//////////////////////////////////////////////////
DesignWnd::DesignWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::ONTOP | GG::CLICKABLE),
    m_add_design_button(0),
    m_hulls_list(0),
    m_design_name_edit(0),
    m_design_description_edit(0),
    m_parts_lists(),
    m_selected_hull(""),
    m_selected_parts()
{
    EnableChildClipping(true);

    m_add_design_button = new CUIButton(100, 100, 120, "Add Test Design");
    AttachChild(m_add_design_button);
    GG::Connect(m_add_design_button->ClickedSignal, &DesignWnd::AddDesign, this);

    m_design_name_edit = new CUIEdit(100, 125, 150, "ShipDesign Name");
    AttachChild(m_design_name_edit);

    m_design_description_edit = new CUIEdit(100, 150, 150, "ShipDesign Description");
    AttachChild(m_design_description_edit);

    GG::Connect(this->DesignChangedSignal, &DesignWnd::DesignChanged, this);

    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;
    m_hulls_list = new CUIDropDownList(300, 150, 200, DROPLIST_HEIGHT, 100);
    AttachChild(m_hulls_list);
    GG::Connect(m_hulls_list->SelChangedSignal, &DesignWnd::HullSelected, this);

    const HullTypeManager& hull_manager = GetHullTypeManager();
    for (HullTypeManager::iterator it = hull_manager.begin(); it != hull_manager.end(); ++it) {
        const std::string& hull_name = it->first;
        m_hulls_list->Insert(new CUISimpleDropDownListRow(hull_name));
    }
    m_hulls_list->Select(m_hulls_list->NumRows() - 1);
}

void DesignWnd::Reset() {
}

void DesignWnd::Sanitize() {
}

void DesignWnd::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);
    glEnable(GL_TEXTURE_2D);
}

bool DesignWnd::ValidateCurrentDesign() {
    // TODO: determine if design specified in UI is valid
    return ShipDesign::ValidDesign(m_selected_hull, m_selected_parts);
}

void DesignWnd::AddDesign() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) return;

    if (!ShipDesign::ValidDesign(m_selected_hull, m_selected_parts)) {
        Logger().errorStream() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
        return;
    }

    // make sure name isn't blank.  TODO: prevent duplicate names?
    std::string name = m_design_name_edit->WindowText();
    if (name == "")
        name = "User ShipDesign";

    // create design from stuff chosen in UI
    ShipDesign* design = new ShipDesign(name, m_design_description_edit->WindowText(), empire_id, CurrentTurn(),
                                        m_selected_hull, m_selected_parts, "misc/base1.png", "some model");

    if (!design) {
        Logger().errorStream() << "DesignWnd::AddDesign failed to create a new ShipDesign object";
        return;
    }

    int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
    HumanClientApp::GetApp()->Orders().IssueOrder(new ShipDesignOrder(empire_id, new_design_id, *design));

    Logger().errorStream() << "Added new design: " << design->Name();

    const Universe& universe = GetUniverse();
    for (Universe::ship_design_iterator it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it)
        Logger().errorStream() << "Shipdesign: " << it->second->Name();
}

void DesignWnd::HullSelected(int index) {
    // need to convert index in list of hulls to a particular hull name.

    // if index is -1, no hull was selected
    if (index < 0) {
        SetDesignHull("");
        return;
    }

    // an actual row was selected, so need to figure out which hull the row index corresponds to
    std::string hull = "";
    const HullTypeManager& manager = GetHullTypeManager();
    HullTypeManager::iterator it = manager.begin();
    int n = 0;
    // loop until selected hull is reached.  assuming list is sorted the same way as the hulls in the manager.
    while (n < index && it != manager.end()) {
        ++n;
        ++it;
    }
    if (it != manager.end())
        hull = it->first;

    SetDesignHull(hull);
}

void DesignWnd::SetDesignHull(const std::string& hull) {
    m_selected_hull = hull;

    int num_slots = 0;

    // get number of slots needed for this hull.  (defaults to zero if hull is invalid)
    const HullTypeManager& hull_manager = GetHullTypeManager();
    const HullType* hull_type = hull_manager.GetHullType(hull);
    if (hull_type)
        num_slots = hull_type->NumberSlots();

    // remove extra slot droplists
    while (static_cast<int>(m_parts_lists.size()) > num_slots) {
        CUIDropDownList* list = m_parts_lists.back();
        delete list;
        m_parts_lists.pop_back();
        m_selected_parts.pop_back();
    }

    // add extra slot drop lists to meet required number
    const PartTypeManager& part_manager = GetPartTypeManager();

    int n = static_cast<int>(m_parts_lists.size()); // used as index to set selected part, and to determine y positions of additional lists to be created
    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;

    while (static_cast<int>(m_parts_lists.size()) < num_slots) {
        // create new list
        CUIDropDownList* list = new CUIDropDownList(500, 150 + n*DROPLIST_HEIGHT*3/2, 200, DROPLIST_HEIGHT, 200);
        m_parts_lists.push_back(list);
        AttachChild(list);
        GG::Connect(list->SelChangedSignal, SelectPartFunctor(this, n));

        // populate new list with parts
        for (PartTypeManager::iterator it = part_manager.begin(); it != part_manager.end(); ++it) {
            const std::string& part_name = it->first;
            list->Insert(new CUISimpleDropDownListRow(part_name));
        }

        // select no part by default
        m_selected_parts.push_back("");
        list->Select(-1);

        ++n;
    }
    DesignChangedSignal();
}

void DesignWnd::DoLayout() {
}

void DesignWnd::PartSelected(int index, int slot) {
    if (slot >= static_cast<int>(m_parts_lists.size()) || slot < 0)
        throw std::invalid_argument("DesignWnd::PartSelected called with invalid slot number: " + lexical_cast<std::string>(slot));
    if (index >= static_cast<int>(m_parts_lists[slot]->NumRows()))
        throw std::invalid_argument("DesignWnd::PartSelected called with invalid part number: " + lexical_cast<std::string>(slot));

    // need to convert index in list of hulls to a particular hull name.

    // if index is -1, no hull was selected
    if (index < 0) {
        SetDesignPart("", slot);
        return;
    }

    // an actual row was selected, so need to figure out which hull the row index corresponds to
    std::string part = "";
    const PartTypeManager& manager = GetPartTypeManager();
    PartTypeManager::iterator it = manager.begin();
    int n = 0;
    // loop until selected part is reached.  assuming list is sorted the same way as the hulls in the manager.
    while (n < index && it != manager.end()) {
        ++n;
        ++it;
    }
    if (it != manager.end())
        part = it->first;

    SetDesignPart(part, slot);
}

void DesignWnd::SetDesignPart(const std::string& part, int slot) {
    m_selected_parts.at(slot) = part;
    DesignChangedSignal();
}

void DesignWnd::DesignChanged() {
    if (ValidateCurrentDesign())
        m_add_design_button->Disable(false);
    else
        m_add_design_button->Disable();
}