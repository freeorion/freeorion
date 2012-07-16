#include "CombatSetupWnd.h"

#include "CombatWnd.h"
#include "InfoPanels.h"
#include "FleetWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../util/Math.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/Layout.h>

#include <OgreEntity.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager.h>
#include <OgreSceneManager.h>

#include <boost/cast.hpp>


namespace {
    const GG::Y     SETUP_WND_HEIGHT(300);
    const int       ICON_SIZE = 38;
    const int       PAD = 4;
    const GG::Pt    INVALID_PRESS_POS(-GG::X1, -GG::Y1);

    GG::Y LabelHeight()
    { return GG::Y(ClientUI::Pts()*3/2); }

    GG::Pt StatIconSize() {
        const int font_size = std::max(ClientUI::Pts(), 12);
        return GG::Pt(GG::X(font_size*10/3), GG::Y(font_size*4/3));
    }

    GG::Y ListRowHeight()
    { return GG::Y(std::max(ICON_SIZE, Value(LabelHeight() + StatIconSize().y) + PAD)); }

    // HACK! This must be kept in synch with CombatWnd.cpp.
    const Ogre::uint32 REGULAR_OBJECTS_MASK = 1 << 0;
    const std::string UNPLACEABLE_MATERIAL_PREFIX = "unplaceable ";

    class ShipRow : public GG::ListBox::Row {
    public:
        ShipRow(Ship* ship, Fleet* fleet, GG::X w, GG::Y h) :
            GG::ListBox::Row(w, h, ""),
            m_ship(ship)
        {
            SetName("ShipRow");
            SetChildClippingMode(ClipToClient);

            std::vector<GG::X> widths(3);
            widths[0] = GG::X(275);
            widths[1] = (w - widths[0]) / 2;
            widths[2] = w - widths[0] - widths[1];

            push_back(new ShipDataPanel(widths[0], h, m_ship->ID()));

            const std::string& fleet_name = fleet->Name();
            push_back(new GG::TextControl(GG::X0, GG::Y0, widths[1], h, fleet_name,
                                            ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT));

            std::string arrival_starlane = "Present";
            if (fleet->ArrivedThisTurn())
                arrival_starlane = "From " + GetUniverseObject(fleet->ArrivalStarlane())->Name();
            push_back(new GG::TextControl(GG::X0, GG::Y0, widths[2], h, arrival_starlane,
                                            ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT));

            assert(widths[0] != ColWidth(0));
            assert(widths[1] != ColWidth(1));
            assert(widths[2] != ColWidth(2));
            SetColWidths(widths);
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            if (!empty() && old_size != Size())
                at(0)->Resize(Size());
        }

        int ShipID() const
        { return m_ship->ID(); }

        Ship* const m_ship;
    };

    Ogre::SceneNode* CreatePlacementRegionNode(Ogre::SceneManager* scene_manager,
                                               const CombatSetupRegion& region,
                                               bool allow,
                                               std::size_t group_index,
                                               std::size_t region_index)
    {
        Ogre::SceneNode* retval = 0;

        const int SLICES = 100;
        const double INCR = Ogre::Math::TWO_PI / SLICES;
        static std::vector<Ogre::Vector3> unit_circle_vertices;
        if (unit_circle_vertices.empty()) {
            unit_circle_vertices.resize(SLICES);
            for (std::size_t i = 0; i < unit_circle_vertices.size(); ++i) {
                double theta = i * INCR;
                unit_circle_vertices[i].x = static_cast<Ogre::Real>(std::cos(theta));
                unit_circle_vertices[i].y = static_cast<Ogre::Real>(std::sin(theta));
                unit_circle_vertices[i].z = static_cast<Ogre::Real>(0.0f);
            }
        }

        const std::string UNIT_CIRCLE_MESH_NAME = std::string(allow ? "allow" : "deny") + "_unit_circle_mesh";
        const std::string RING_MESH_NAME = "deny_ring_mesh";
        const Ogre::ColourValue COLOR(allow ? 0.0f : 1.0f, allow ? 1.0f : 0.0f, 0.0f, 0.5f);

        std::string base_name =
            boost::lexical_cast<std::string>(group_index) + "_" +
            boost::lexical_cast<std::string>(region_index) + "_";
        retval = scene_manager->getRootSceneNode()->createChildSceneNode(base_name + "node");

        switch (region.m_type) {
        case CombatSetupRegion::RING: {
            // HACK! This currently assumes all rings are starlane exclusion
            // zones, so we only create one mesh for all RING's and reuse it;
            // this may need to be generalized later.
            Ogre::MeshPtr ring_mesh = Ogre::MeshManager::getSingleton().getByName(RING_MESH_NAME);
            if (ring_mesh.isNull()) {
                Ogre::ManualObject manual_object("");
                manual_object.estimateVertexCount(unit_circle_vertices.size() * 2 + 2);
                manual_object.begin("", Ogre::RenderOperation::OT_TRIANGLE_STRIP);
                for (std::size_t i = 0; i < unit_circle_vertices.size(); ++i) {
                    manual_object.position(unit_circle_vertices[i] * region.m_radius_begin);
                    manual_object.colour(COLOR);
                    manual_object.position(unit_circle_vertices[i] * region.m_radius_end);
                    manual_object.colour(COLOR);
                }
                manual_object.position(unit_circle_vertices[0] * region.m_radius_begin);
                manual_object.colour(COLOR);
                manual_object.position(unit_circle_vertices[0] * region.m_radius_end);
                manual_object.colour(COLOR);
                manual_object.end();
                ring_mesh = manual_object.convertToMesh(RING_MESH_NAME);
            }
            Ogre::Entity* entity = scene_manager->createEntity(base_name + "entity", RING_MESH_NAME);
            entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
            entity->setMaterialName("effects/area");
            retval->attachObject(entity);
            break;
        }

        case CombatSetupRegion::ELLIPSE: {
            Ogre::MeshPtr unit_circle_mesh = Ogre::MeshManager::getSingleton().getByName(UNIT_CIRCLE_MESH_NAME);
            if (unit_circle_mesh.isNull()) {
                Ogre::ManualObject manual_object("");
                manual_object.estimateVertexCount(unit_circle_vertices.size() + 2);
                manual_object.begin("", Ogre::RenderOperation::OT_TRIANGLE_FAN);
                manual_object.position(0.0, 0.0, 0.0);
                manual_object.colour(COLOR);
                for (std::size_t i = 0; i < unit_circle_vertices.size(); ++i) {
                    manual_object.position(unit_circle_vertices[i]);
                    manual_object.colour(COLOR);
                }
                manual_object.position(unit_circle_vertices[0]);
                manual_object.colour(COLOR);
                manual_object.end();
                unit_circle_mesh = manual_object.convertToMesh(UNIT_CIRCLE_MESH_NAME);
            }
            Ogre::Entity* entity = scene_manager->createEntity(base_name + "entity", UNIT_CIRCLE_MESH_NAME);
            entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
            entity->setMaterialName("effects/area");
            retval->attachObject(entity);
            break;
        }

        case CombatSetupRegion::PARTIAL_ELLIPSE: {
            Ogre::ManualObject* manual_object = new Ogre::ManualObject(base_name + "manual_object");
            manual_object->estimateVertexCount(unit_circle_vertices.size());
            manual_object->begin("effects/area", Ogre::RenderOperation::OT_TRIANGLE_FAN);
            manual_object->position(0.0, 0.0, 0.0);
            manual_object->colour(COLOR);
            manual_object->position(std::cos(region.m_theta_begin), std::sin(region.m_theta_begin), 0.0);
            manual_object->colour(COLOR);
            for (std::size_t i = 0; i < unit_circle_vertices.size(); ++i) {
                double theta = i * INCR;
                if (region.m_theta_begin < theta && theta < region.m_theta_end) {
                    manual_object->position(unit_circle_vertices[i]);
                    manual_object->colour(COLOR);
                }
            }
            manual_object->position(std::cos(region.m_theta_end), std::sin(region.m_theta_end), 0.0);
            manual_object->colour(COLOR);
            manual_object->end();
            manual_object->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
            retval->attachObject(manual_object);
            break;
        }
        }

        if (region.m_type == CombatSetupRegion::ELLIPSE || region.m_type == CombatSetupRegion::PARTIAL_ELLIPSE) {
            retval->setPosition(region.m_centroid[0], region.m_centroid[1], 0.0);
            retval->setScale(region.m_radial_axis, region.m_tangent_axis, 1.0);
            // if non-circular, rotate
            if (region.m_radial_axis != region.m_tangent_axis) {
                retval->setOrientation(
                    Ogre::Quaternion(Ogre::Radian(std::atan2(region.m_centroid[1], region.m_centroid[0])),
                                     Ogre::Vector3(0.0, 0.0, 1.0)));
            }
        }

        return retval;
    }

    void SetRegionNodesVisibility(const std::vector<Ogre::SceneNode*>& nodes, bool visible) {
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            nodes[i]->setVisible(visible);
        }
    }

    std::size_t GroupIndexOfShip(const std::vector<CombatSetupGroup>& setup_groups, const Ship* ship) {
        std::size_t retval = setup_groups.size();
        for (std::size_t i = 0; i < setup_groups.size(); ++i) {
            if (setup_groups[i].m_ships.find(ship->ID()) != setup_groups[i].m_ships.end()) {
                retval = i;
                break;
            }
        }
        return retval;
    }
}

CombatSetupWnd::CombatSetupWnd(
    const std::vector<CombatSetupGroup>& setup_groups,
    CombatWnd* combat_wnd,
    const CombatData* combat_data,
    Ogre::SceneManager* scene_manager,
    boost::function<std::pair<bool, Ogre::Vector3> (const GG::Pt& pt)>
    intersect_mouse_with_ecliptic,
    boost::function<const Ogre::MaterialPtr& (const Ship&)>
    get_ship_material,
    boost::function<void (int, Ogre::SceneNode*, Ogre::Entity*, const Ogre::MaterialPtr&)>
    add_ship_node_to_combat_wnd,
    boost::function<Ogre::MovableObject* (const GG::Pt&)>
    get_object_under_pt,
    boost::function<void (int, const Ogre::Vector3&, const Ogre::Quaternion&)>
    reposition_ship_node,
    boost::function<void (int)>
    remove_ship,
    boost::function<void (const Ogre::Vector3&)>
    look_at,
    GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::DRAGABLE*/) :
    CUIWnd("Ships", GG::X(PAD), GG::GUI::GetGUI()->AppHeight() - SETUP_WND_HEIGHT - GG::Y(PAD),
           GG::X(500), SETUP_WND_HEIGHT, flags),
    m_setup_groups(setup_groups),
    m_current_setup_group(m_setup_groups.size()),
    m_setup_finished_waiting_for_server(false),
    m_dragging_placed_ship(false),
    m_button_press_on_placed_ship(INVALID_PRESS_POS),
    m_button_press_placed_ship_node(0),
    m_mouse_dragged(false),
    m_listbox(new CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1)),
    m_redo_placements_button(new CUIButton(GG::X0, GG::Y1, GG::X1, UserString("REDO_PLACEMENTS"))),
    m_auto_place_button(new CUIButton(GG::X0, GG::Y(2), GG::X1, UserString("AUTO_PLACE_SHIPS"))),
    m_done_button(new CUIButton(GG::X0, GG::Y(3), GG::X1, UserString("DONE"))),
    m_selected_placeable_ship(0),
    m_placeable_ship_node(0),
    m_scene_manager(scene_manager),
    m_combat_universe(combat_data->m_combat_universe),
    m_intersect_mouse_with_ecliptic(intersect_mouse_with_ecliptic),
    m_get_ship_material(get_ship_material),
    m_add_ship_node_to_combat_wnd(add_ship_node_to_combat_wnd),
    m_get_object_under_pt(get_object_under_pt),
    m_reposition_ship_node(reposition_ship_node),
    m_remove_ship(remove_ship),
    m_look_at(look_at)
{
    m_listbox->SetStyle(m_listbox->Style() | GG::LIST_SINGLESEL);
    Connect(m_redo_placements_button->ClickedSignal, &CombatSetupWnd::RedoPlacementsButtonClicked, this);
    Connect(m_auto_place_button->ClickedSignal, &CombatSetupWnd::AutoPlaceButtonClicked, this);
    Connect(m_done_button->ClickedSignal, &CombatSetupWnd::DoneButtonClicked, this);

    GG::Y original_button_height = m_done_button->Height();
    GG::Y original_all_buttons_height = original_button_height * 3;

    AttachChild(m_listbox);
    AttachChild(m_redo_placements_button);
    AttachChild(m_auto_place_button);
    AttachChild(m_done_button);
    VerticalLayout();
    GetLayout()->SetRowStretch(0, Value(Height() - original_all_buttons_height));
    GetLayout()->SetRowStretch(1, Value(original_button_height));
    GetLayout()->SetRowStretch(2, Value(original_button_height));
    GetLayout()->SetRowStretch(3, Value(original_button_height));
    GetLayout()->SetCellMargin(2);

    const GG::Pt row_size = ListRowSize();

    for (PathingEngine::const_iterator it = combat_data->m_pathing_engine.begin();
         it != combat_data->m_pathing_engine.end();
         ++it) {
        if (CombatShipPtr combat_ship = boost::dynamic_pointer_cast<CombatShip>(*it)) {
            // create scene node
            Ship& ship = combat_ship->GetShip();

            // TODO: Temporary!  Serialization of CombatData currently sends
            // everything to everyone.  Fix this.
            if (!ship.OwnedBy(HumanClientApp::GetApp()->EmpireID()))
                continue;

            Ogre::SceneNode* node = GetShipNode(ship);

            if (combat_ship->position() == OpenSteer::Vec3(0.0, 0.0, 0.0)) {
                UniverseObject* o = m_combat_universe[ship.FleetID()];
                Fleet* fleet = boost::polymorphic_downcast<Fleet*>(o);
                ShipRow* row = new ShipRow(&ship, fleet, row_size.x, row_size.y);
                m_listbox->Insert(row);
            } else {
                node->setPosition(ToOgre(combat_ship->position()));
                node->setOrientation(Ogre::Quaternion(ToOgre(combat_ship->side()),
                                                      ToOgre(combat_ship->forward()),
                                                      ToOgre(combat_ship->up())));
                PlaceShip(&ship, node);
                node->setVisible(true);
            }
        }
    }

    m_redo_placements_button->Disable(m_placed_nodes.empty());
    m_auto_place_button->Disable(m_listbox->Empty());
    m_done_button->Disable(!m_listbox->Empty());

    GG::Connect(m_listbox->SelChangedSignal, &CombatSetupWnd::PlaceableShipSelected_, this);

    combat_wnd->InstallEventFilter(this);
}

CombatSetupWnd::~CombatSetupWnd() {
    for (std::map<std::size_t, std::vector<Ogre::SceneNode*> >::iterator it =
             m_region_nodes_by_setup_group.begin();
         it != m_region_nodes_by_setup_group.end();
         ++it) {
        for (std::size_t i = 0; i < it->second.size(); ++i) {
            m_scene_manager->destroySceneNode(it->second[i]);
        }
    }
}

GG::Pt CombatSetupWnd::ListRowSize() const
{ return GG::Pt(m_listbox->Width() - ClientUI::ScrollWidth() - 5, ListRowHeight()); }

bool CombatSetupWnd::EventFilter(GG::Wnd* w, const GG::WndEvent& event)
{
    // Turn off all event handling to lock down CombatWnd.
    if (m_setup_finished_waiting_for_server)
        return true;

    bool retval = false;
    if (event.Type() == GG::WndEvent::LButtonDown) {
        Ogre::MovableObject* movable_object = 0;
        if (!PlaceableShipNode() && (movable_object = m_get_object_under_pt(event.Point()))) {
            Ogre::SceneNode* node = movable_object->getParentSceneNode();
            if (Ogre::any_cast<Ship*>(&node->getUserAny())) {
                m_button_press_placed_ship_node = node;
                assert(m_button_press_placed_ship_node);
                m_button_press_on_placed_ship = event.Point();
            }
        }
        m_dragging_placed_ship = false;
        m_mouse_dragged = false;
    } else if (event.Type() == GG::WndEvent::LDrag) {
        HandleMouseMoves(event.Point());
        if (m_button_press_placed_ship_node) {
            if (!m_dragging_placed_ship) {
                const int DRAG_THRESHOLD = 5;
                GG::Pt delta = event.Point() - m_button_press_on_placed_ship;
                if (DRAG_THRESHOLD < std::abs(Value(delta.x)) + std::abs(Value(delta.y)))
                    m_dragging_placed_ship = true;
            }
            if (m_dragging_placed_ship) {
                std::pair<bool, Ogre::Vector3> intersection = m_intersect_mouse_with_ecliptic(event.Point());
                Ship* ship = *Ogre::any_cast<Ship*>(&m_button_press_placed_ship_node->getUserAny());
                bool valid_location = intersection.first && ValidPlacement(ship, intersection.second);
                if (valid_location)
                    RepositionShip(ship, m_button_press_placed_ship_node, intersection.second);
            }
        }
        retval = true;
        m_mouse_dragged = true;
    } else if (event.Type() == GG::WndEvent::LButtonUp) {
        m_button_press_placed_ship_node = 0;
        m_mouse_dragged = false;
    } else if (event.Type() == GG::WndEvent::LClick) {
        Ogre::SceneNode* placement_node = PlaceableShipNode();
        bool valid_placement = false;
        if (placement_node) {
            std::pair<bool, Ogre::Vector3> intersection = m_intersect_mouse_with_ecliptic(event.Point());
            Ship* ship = *Ogre::any_cast<Ship*>(&placement_node->getUserAny());
            valid_placement = intersection.first && ValidPlacement(ship, intersection.second);
        }
        if (valid_placement) {
            PlaceCurrentShip();
            retval = true;
        } else if (m_mouse_dragged) {
            retval = true;
        }
        m_button_press_placed_ship_node = 0;
    } else if (event.Type() == GG::WndEvent::RClick) {
        CancelCurrentShipPlacement();
        retval = true;
    } else if (event.Type() == GG::WndEvent::RDoubleClick) {
        retval = true;
    } else if (event.Type() == GG::WndEvent::MouseEnter) {
        HandleMouseMoves(event.Point());
    } else if (event.Type() == GG::WndEvent::MouseHere) {
        HandleMouseMoves(event.Point());
    } else if (event.Type() == GG::WndEvent::MouseLeave) {
        if (Ogre::SceneNode* placement_node = PlaceableShipNode())
            placement_node->setVisible(false);
    }

    return retval;
}

Ogre::SceneNode* CombatSetupWnd::PlaceableShipNode() const
{ return m_selected_placeable_ship ? m_placeable_ship_node : 0; }

bool CombatSetupWnd::ValidPlacement(Ship* ship, const Ogre::Vector3& point) const
{
    bool retval = false;
    std::size_t i = GroupIndexOfShip(m_setup_groups, ship);
    if (i != m_setup_groups.size()) {
        const CombatSetupGroup& group = m_setup_groups[i];
        retval = !group.m_allow;
        for (std::size_t j = 0; j < group.m_regions.size(); ++j) {
            double point_d[2] = { point.x, point.y };
            if (PointInRegion(point_d, group.m_regions[j])) {
                retval = !retval;
                break;
            }
        }
    }
    if (retval) {
        // verify that this placement is not too near to other ships
        for (std::map<int, Ogre::SceneNode*>::const_iterator it = m_placed_nodes.begin();
             it != m_placed_nodes.end();
             ++it) {
            Ship* this_ship = *Ogre::any_cast<Ship*>(&it->second->getUserAny());
            if (this_ship == ship)
                continue;
            Ogre::SceneNode::ObjectIterator iterator = it->second->getAttachedObjectIterator();
            assert(iterator.hasMoreElements());
            const Ogre::Sphere& sphere =
                boost::polymorphic_downcast<Ogre::Entity*>(iterator.getNext())->getWorldBoundingSphere();
            if (sphere.intersects(point)) {
                retval = false;
                break;
            }
        }
    }
    return retval;
}

void CombatSetupWnd::HandleMouseMoves(const GG::Pt& pt)
{
    if (Ogre::SceneNode* node = PlaceableShipNode()) {
        std::pair<bool, Ogre::Vector3> intersection = m_intersect_mouse_with_ecliptic(pt);
        if (intersection.first) {
            Ship* ship = *Ogre::any_cast<Ship*>(&node->getUserAny());
            bool valid_location = ValidPlacement(ship, intersection.second);
            node->setVisible(true);
            node->setPosition(intersection.second);
            node->setOrientation(StarwardOrientationForPosition(intersection.second));
            if (valid_location) {
                Ogre::SceneNode::ObjectIterator iterator = node->getAttachedObjectIterator();
                assert(iterator.hasMoreElements());
                Ogre::Entity* entity = boost::polymorphic_downcast<Ogre::Entity*>(iterator.getNext());
                entity->setMaterialName(ShipMaterialName(*ship->Design(), ship->Owner()));
                entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
            } else {
                std::string base_material_name = ShipMaterialName(*ship->Design(), ship->Owner());
                std::string material_name = UNPLACEABLE_MATERIAL_PREFIX + base_material_name;
                if (!Ogre::MaterialManager::getSingleton().resourceExists(material_name)) {
                    Ogre::MaterialPtr unmodified_material =
                        Ogre::MaterialManager::getSingleton().getByName(base_material_name);
                    Ogre::MaterialPtr material = unmodified_material->clone(material_name);
                    Ogre::Pass* pass = material->getTechnique(0)->getPass(1);
                    assert(pass->hasFragmentProgram());
                    pass->getFragmentProgramParameters()->setNamedConstant("alpha", 0.25f);
                }
                Ogre::SceneNode::ObjectIterator iterator = node->getAttachedObjectIterator();
                assert(iterator.hasMoreElements());
                Ogre::Entity* entity = boost::polymorphic_downcast<Ogre::Entity*>(iterator.getNext());
                entity->setMaterialName(material_name);
                entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
            }
        } else {
            node->setVisible(false);
        }
    }
}

void CombatSetupWnd::CreateCombatOrder(int ship_id, Ogre::SceneNode* node)
{
    Ogre::Vector3 axes[3];
    node->getOrientation().ToAxes(axes);
    m_placement_orders[ship_id] = CombatOrder(ship_id,
                                              ToOpenSteer(node->getPosition()),
                                              ToOpenSteer(axes[1]));
}

Ogre::SceneNode* CombatSetupWnd::GetShipNode(Ship& ship)
{
    Ogre::SceneNode*& retval = m_ship_nodes[ship.ID()];
    if (!retval) {
        retval = CreateShipSceneNode(m_scene_manager, ship);
        retval->setUserAny(Ogre::Any(&ship));
        const Ogre::MaterialPtr& material = m_get_ship_material(ship);
        Ogre::Entity*& entity = m_ship_entities[ship.ID()];
        entity = CreateShipEntity(m_scene_manager, ship, material);
        retval->attachObject(entity);
    }
    retval->setVisible(false);
    return retval;
}

void CombatSetupWnd::PlaceableShipSelected_(const GG::ListBox::SelectionSet& sels)
{
    assert(sels.size() <= 1u);
    if (sels.empty()) {
        PlaceableShipSelected(0);
        UpdatePlacementIndicators(0);
    } else {
        GG::ListBox::Row* row = **sels.begin();
        ShipRow* ship_row = boost::polymorphic_downcast<ShipRow*>(row);
        PlaceableShipSelected(const_cast<Ship*>(ship_row->m_ship));
        UpdatePlacementIndicators(ship_row->m_ship);
    }
}

void CombatSetupWnd::PlaceableShipSelected(Ship* ship)
{
    if (ship != m_selected_placeable_ship) {
        m_selected_placeable_ship = ship;
        if (m_placeable_ship_node) {
            m_placeable_ship_node->setVisible(false);
            m_placeable_ship_node = 0;
        }
        if (m_selected_placeable_ship)
            m_placeable_ship_node = GetShipNode(*ship);
    }
}

void CombatSetupWnd::UpdatePlacementIndicators(const Ship* ship)
{
    if (ship) {
        std::size_t i = GroupIndexOfShip(m_setup_groups, ship);
        if (i != m_setup_groups.size()) {
            if (m_current_setup_group != i) {
                if (m_current_setup_group < m_setup_groups.size())
                    SetRegionNodesVisibility(m_region_nodes_by_setup_group[m_current_setup_group], false);
                if (m_region_nodes_by_setup_group.find(i) != m_region_nodes_by_setup_group.end()) {
                    SetRegionNodesVisibility(m_region_nodes_by_setup_group[i], true);
                } else {
                    CombatSetupGroup& group = m_setup_groups[i];
                    for (std::size_t j = 0; j < group.m_regions.size(); ++j) {
                        Ogre::SceneNode* node =
                            CreatePlacementRegionNode(m_scene_manager, group.m_regions[j],
                                                      group.m_allow, i, j);
                        m_region_nodes_by_setup_group[i].push_back(node);
                    }
                }
                m_current_setup_group = i;
                if (m_setup_groups[m_current_setup_group].m_allow &&
                    m_region_nodes_by_setup_group[m_current_setup_group].size() == 1u) {
                    m_look_at(m_region_nodes_by_setup_group[m_current_setup_group].back()->getPosition());
                }
            }
        }
    } else {
        if (m_current_setup_group < m_setup_groups.size())
            SetRegionNodesVisibility(m_region_nodes_by_setup_group[m_current_setup_group], false);
    }
}

void CombatSetupWnd::CancelCurrentShipPlacement()
{
    m_selected_placeable_ship = 0;
    if (m_placeable_ship_node) {
        m_placeable_ship_node->setVisible(false);
        m_placeable_ship_node = 0;
    }
    m_listbox->DeselectAll();
}

void CombatSetupWnd::PlaceCurrentShip()
{
    assert(m_selected_placeable_ship);
    assert(m_placeable_ship_node);

    RepositionShip(m_selected_placeable_ship, m_placeable_ship_node, m_placeable_ship_node->getPosition());

    const CUIListBox::SelectionSet& selections = m_listbox->Selections();
    assert(selections.size() == 1u);
    m_listbox->Erase(*selections.begin());

    m_selected_placeable_ship = 0;
    m_placeable_ship_node = 0;

    m_done_button->Disable(!m_listbox->Empty());
}

void CombatSetupWnd::PlaceShip(Ship* ship, Ogre::SceneNode* node)
{
    int ship_id = ship->ID();
    m_add_ship_node_to_combat_wnd(ship_id,
                                  node,
                                  m_ship_entities[ship_id],
                                  m_get_ship_material(*ship));
    CreateCombatOrder(ship_id, node);
    m_placed_nodes[ship_id] = node;
}

void CombatSetupWnd::RepositionShip(Ship* ship, Ogre::SceneNode* node, const Ogre::Vector3& position)
{
    int ship_id = ship->ID();
    m_reposition_ship_node(ship_id, position, StarwardOrientationForPosition(position));
    CreateCombatOrder(ship_id, node);
    m_placed_nodes[ship_id] = node;
}

void CombatSetupWnd::RedoPlacementsButtonClicked()
{
    const GG::Pt row_size = ListRowSize();
    for (std::map<int, Ogre::SceneNode*>::iterator it = m_placed_nodes.begin();
         it != m_placed_nodes.end();
         ++it) {
        Ship* ship = *Ogre::any_cast<Ship*>(&it->second->getUserAny());
        UniverseObject* o = m_combat_universe[ship->FleetID()];
        Fleet* fleet = boost::polymorphic_downcast<Fleet*>(o);
        ShipRow* row = new ShipRow(ship, fleet, row_size.x, row_size.y);
        m_listbox->Insert(row);
        m_remove_ship(it->first);
    }
    m_placed_nodes.clear();
}

void CombatSetupWnd::AutoPlaceButtonClicked()
{
    // TODO
    std::cerr << "AutoPlaceButtonClicked()\n";
}

void CombatSetupWnd::DoneButtonClicked()
{
    CombatOrderSet orders;
    for (std::map<int, CombatOrder>::iterator it = m_placement_orders.begin();
         it != m_placement_orders.end();
         ++it) {
        orders.push_back(it->second);
    }
    HumanClientApp::GetApp()->Networking().SendMessage(
        CombatTurnOrdersMessage(
            HumanClientApp::GetApp()->PlayerID(),
            orders));

    m_setup_finished_waiting_for_server = true;
    m_done_button->Disable(true);
    Hide();
}
