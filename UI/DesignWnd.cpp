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
    // functor connecting slot droplists from which parts are selected to adding parts to the design.
    // external is true if the list is for an external slot, or false if the list is for an internal slot
    // slot is the index of the slot in the external or internal array of parts (and droplists)
    // droplist_index is the index of the selected item in the droplist
    struct SelectPartFunctor {
        SelectPartFunctor(DesignWnd* design_wnd, bool external, unsigned int slot) :
            m_design_wnd(design_wnd), m_external(external), m_slot(slot) {}
        void operator()(int droplist_index) {m_design_wnd->PartSelected(droplist_index, m_external, m_slot);}
        DesignWnd* const m_design_wnd;
        bool m_external;
        unsigned int m_slot;
    };
}

//////////////////////////////////////////////////
// DesignWnd                                    //
//////////////////////////////////////////////////
DesignWnd::DesignWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::ONTOP | GG::CLICKABLE),
    m_hulls_list(0),
    m_external_parts_lists(),
    m_internal_parts_lists(),
    m_add_design_button(0),
    m_design_name_edit(0),
    m_design_description_edit(0),
    m_selected_hull(""),
    m_selected_external_parts(),
    m_selected_internal_parts()

{
    EnableChildClipping(true);

    m_add_design_button = new CUIButton(100, 100, 120, UserString("DESIGN_ADD_TEST"));
    AttachChild(m_add_design_button);
    GG::Connect(m_add_design_button->ClickedSignal, &DesignWnd::AddDesign, this);

    m_design_name_edit = new CUIEdit(100, 125, 150, UserString("DESIGN_NAME_DEFAULT"));
    AttachChild(m_design_name_edit);

    m_design_description_edit = new CUIEdit(100, 150, 150, UserString("DESIGN_DESCRIPTION_DEFAULT"));
    AttachChild(m_design_description_edit);

    GG::Connect(this->DesignChangedSignal, &DesignWnd::DesignChanged, this);

    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;
    m_hulls_list = new CUIDropDownList(300, 150, 200, DROPLIST_HEIGHT, 100);
    AttachChild(m_hulls_list);
    GG::Connect(m_hulls_list->SelChangedSignal, &DesignWnd::HullSelected, this);

    const HullTypeManager& hull_manager = GetHullTypeManager();
    for (HullTypeManager::iterator it = hull_manager.begin(); it != hull_manager.end(); ++it) {
        const std::string& hull_name = it->first;
        m_hulls_list->Insert(new CUISimpleDropDownListRow(UserString(hull_name)));
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
    return ShipDesign::ValidDesign(m_selected_hull, m_selected_external_parts, m_selected_internal_parts);
}

void DesignWnd::AddDesign() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) return;

    if (!ValidateCurrentDesign()) {
        Logger().errorStream() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
        return;
    }

    // make sure name isn't blank.  TODO: prevent duplicate names?
    std::string name = m_design_name_edit->WindowText();
    if (name == "")
        name = "User ShipDesign";

    // create design from stuff chosen in UI
    ShipDesign* design = new ShipDesign(name, m_design_description_edit->WindowText(), empire_id, CurrentTurn(),
                                        m_selected_hull, m_selected_external_parts, m_selected_internal_parts,
                                        "misc/base1.png", "some model");

    if (!design) {
        Logger().errorStream() << "DesignWnd::AddDesign failed to create a new ShipDesign object";
        return;
    }

    int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ShipDesignOrder(empire_id, new_design_id, *design)));

    Logger().errorStream() << "Added new design: " << design->Name();

    const Universe& universe = GetUniverse();
    for (Universe::ship_design_iterator it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it)
        Logger().errorStream() << "Shipdesign: " << it->second->Name();
}

void DesignWnd::HullSelected(int hull_index) {
    // need to convert index in list of hulls to a particular hull name.

    // if index is -1, no hull was selected
    if (hull_index < 0) {
        SetDesignHull("");
        return;
    }

    // an actual row was selected, so need to figure out which hull the row index corresponds to
    std::string hull_name = "";
    const HullTypeManager& manager = GetHullTypeManager();
    HullTypeManager::iterator it = manager.begin();
    int n = 0;
    // loop until selected hull is reached.  assuming list is sorted the same way as the hulls in the manager.
    while (n < hull_index && it != manager.end()) {
        ++n;
        ++it;
    }

    // ensure hull was found.  if not, default to intial empty hull_name = ""
    if (it != manager.end())
        hull_name = it->first;
    else
        Logger().debugStream() << "DesignWnd::HullSelected: couldn't find hull with hull_index " << hull_index;

    SetDesignHull(hull_name);
}

void DesignWnd::SetDesignHull(const std::string& hull) {
    m_selected_hull = hull;

    // get number of slots needed for this hull.  (defaults to zero if hull is invalid)
    unsigned int num_external_slots = 0;
    unsigned int num_internal_slots = 0;
    const HullTypeManager& hull_manager = GetHullTypeManager();
    const HullType* hull_type = hull_manager.GetHullType(hull);
    if (hull_type) {
        num_external_slots = hull_type->NumExternalSlots();
        num_internal_slots = hull_type->NumInternalSlots();
    }

    // remove extra slot droplists and parts from previous design, until at most the number of parts and lists
    // for the current hull remain
    while (static_cast<int>(m_external_parts_lists.size()) > num_external_slots) {
        m_external_parts_lists.pop_back();      // vector of shared_ptr, so this deletes the popped part list
        m_selected_external_parts.pop_back();
    }
    while (static_cast<int>(m_internal_parts_lists.size()) > num_internal_slots) {
        m_internal_parts_lists.pop_back();      // vector of shared_ptr, so this deletes the popped part list
        m_selected_internal_parts.pop_back();
    }


    // add extra slot drop lists to meet required numbers for the current hull
    const PartTypeManager& part_manager = GetPartTypeManager();
    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;

    // external part droplists...
    unsigned int n_external = static_cast<unsigned int>(m_external_parts_lists.size()); // used as index to set selected external part, and to determine y positions of additional lists to be created
    while (static_cast<unsigned int>(m_external_parts_lists.size()) < num_external_slots) {
        // create new list
        CUIDropDownList* list = new CUIDropDownList(500, 150 + n_external*DROPLIST_HEIGHT*3/2, 200, DROPLIST_HEIGHT, 200);
        m_external_parts_lists.push_back(boost::shared_ptr<CUIDropDownList>(list));
        AttachChild(list);
        GG::Connect(list->SelChangedSignal, SelectPartFunctor(this, true, n_external));

        // populate new list with externally-mountable parts
        for (PartTypeManager::iterator it = part_manager.begin(); it != part_manager.end(); ++it) {
            const std::string& part_name = it->first;
            const PartType* part = it->second;
            if (part->CanMountExternally())
                list->Insert(new CUISimpleDropDownListRow(UserString(part_name)));
        }

        // select no part by default
        m_selected_external_parts.push_back("");
        list->Select(-1);

        ++n_external;
    }

    // internal part droplists
    unsigned int n_internal = static_cast<unsigned int>(m_internal_parts_lists.size()); // used as index to set selected internal part, and to determine y positions of additional lists to be created
    while (static_cast<unsigned int>(m_internal_parts_lists.size()) < num_internal_slots) {
        // create new list
        CUIDropDownList* list = new CUIDropDownList(750, 150 + n_internal*DROPLIST_HEIGHT*3/2, 200, DROPLIST_HEIGHT, 200);
        m_internal_parts_lists.push_back(boost::shared_ptr<CUIDropDownList>(list));
        AttachChild(list);
        GG::Connect(list->SelChangedSignal, SelectPartFunctor(this, false, n_internal));

        // populate new list with internally-mountable parts
        for (PartTypeManager::iterator it = part_manager.begin(); it != part_manager.end(); ++it) {
            const std::string& part_name = it->first;
            const PartType* part = it->second;
            if (part->CanMountInternally())
                list->Insert(new CUISimpleDropDownListRow(UserString(part_name)));
        }

        // select no part by default
        m_selected_internal_parts.push_back("");
        list->Select(-1);

        ++n_internal;
    }

    DesignChangedSignal();
}

void DesignWnd::DoLayout() {
}

void DesignWnd::PartSelected(int part_index, bool external, unsigned int slot) {
    if (external) {
        if (slot >= static_cast<unsigned int>(m_external_parts_lists.size()))
            throw std::invalid_argument("DesignWnd::PartSelected called with invalid slot");
        if (part_index >= static_cast<int>(m_external_parts_lists[slot]->NumRows()))
            throw std::invalid_argument("DesignWnd::PartSelected called with invalid part index");
    } else {
        if (slot >= static_cast<unsigned int>(m_internal_parts_lists.size()))
            throw std::invalid_argument("DesignWnd::PartSelected called with invalid slot");
        if (part_index >= static_cast<int>(m_internal_parts_lists[slot]->NumRows()))
            throw std::invalid_argument("DesignWnd::PartSelected called with invalid part index");
    }

    // need to convert index in list of hulls to part name

    // if index is -1, no part was selected
    if (part_index < 0) {
        SetDesignPart("", external, slot);
        return;
    }

    // an actual row was selected, so need to figure out which part the row index corresponds to
    std::string part_name = "";
    const PartTypeManager& manager = GetPartTypeManager();
    PartTypeManager::iterator it = manager.begin();
    int part_index_counter = 0;
    // iterate through parts in part manager.  for every part that can be mounted in the type of slot (internal
    // or external) for which a part has been selected, increment part_index_counter.  keep iterating until
    // part_index counter equals part_index, at which point the iterator through parts should point to the
    // part that the passed part_index represents.
    // 
    // assumes parts are indexed in same order as parts of same type (internal or external) in manager
    while (part_index_counter < part_index && it != manager.end()) {
        const PartType* part = it->second;
        if ((external && part->CanMountExternally()) || (!external && part->CanMountInternally()))
            ++part_index_counter;
        ++it;
    }

    // make sure part was found.  if not, use default blank part_name = ""
    if (it != manager.end())
        part_name = it->first;
    else
        Logger().debugStream() << "DesignWnd::PartSelected: couldn't find " << (external ? "external" : "internal") << " part with part_index " << part_index;

    SetDesignPart(part_name, external, slot);
}

void DesignWnd::SetDesignPart(const std::string& part, bool external, unsigned int slot) {
    if (external)
        m_selected_external_parts.at(slot) = part;
    else
        m_selected_internal_parts.at(slot) = part;
    DesignChangedSignal();
}

void DesignWnd::DesignChanged() {
    if (ValidateCurrentDesign())
        m_add_design_button->Disable(false);
    else
        m_add_design_button->Disable();
}
