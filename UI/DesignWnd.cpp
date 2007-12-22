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


//////////////////////////////////////////////////
// DesignWnd                                    //
//////////////////////////////////////////////////
DesignWnd::DesignWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::ONTOP),
    m_add_design_button(0)
{
    EnableChildClipping(true);

    const PartTypeManager& part_manager = GetPartTypeManager();
    for (PartTypeManager::iterator it = part_manager.begin(); it != part_manager.end(); ++it)
        Logger().errorStream() << "part: " << it->first;

    const HullTypeManager& hull_manager = GetHullTypeManager();
    for (HullTypeManager::iterator it = hull_manager.begin(); it != hull_manager.end(); ++it)
        Logger().errorStream() << "hull: " << it->first;

    m_add_design_button = new CUIButton(100, 100, 120, "Add Test Design");
    AttachChild(m_add_design_button);
    GG::Connect(m_add_design_button->ClickedSignal, &DesignWnd::AddDesign, this);
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
    return true;
}

void DesignWnd::AddDesign() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) return;

    if (!ValidateCurrentDesign()) return;

    // create design from stuff chosen in UI
    ShipDesign* design = new ShipDesign("procedural dummy design", empire_id, CurrentTurn(), "SH_SMALL",
                                        std::vector<std::string>(), "some graphic", "some model");

    if (!design) {
        Logger().errorStream() << "DesignWnd::AddDesign failed to create a new ShipDesign object";
        return;
    }

    int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
    HumanClientApp::GetApp()->Orders().IssueOrder(new ShipDesignOrder(empire_id, new_design_id, *design));
}
