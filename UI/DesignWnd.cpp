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
    GG::Wnd(0, 0, w, h, GG::ONTOP) {
    EnableChildClipping(true);

    const PartTypeManager& part_manager = GetPartTypeManager();
    for (PartTypeManager::iterator it = part_manager.begin(); it != part_manager.end(); ++it)
        Logger().errorStream() << "part: " << it->first;

    const HullTypeManager& hull_manager = GetHullTypeManager();
    for (HullTypeManager::iterator it = hull_manager.begin(); it != hull_manager.end(); ++it)
        Logger().errorStream() << "hull: " << it->first;

    //int empire_id = HumanClientApp::GetApp()->EmpireID();
    //Empire* empire = Empires().Lookup(empire_id);

    //std::vector<std::string> parts;
    //parts.push_back("SR_LASER");
    //parts.push_back("SR_LASER");
    //parts.push_back("SR_ION_CANNON");
    //ShipDesign* design = new ShipDesign("test design", 0, 1, "SH_SMALL", parts, "misc/base1.png", "unknown model");
    //int design_id = empire->AddShipDesign(design);
    //design->SetID(design_id);
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