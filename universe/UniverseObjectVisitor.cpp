#include "UniverseObjectVisitor.h"

#include "Building.h"
#include "Field.h"
#include "Fighter.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"


auto UniverseObjectVisitor::Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject>
{ return nullptr; }

auto UniverseObjectVisitor::Visit(const std::shared_ptr<Building>& obj) const -> std::shared_ptr<UniverseObject>
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

auto UniverseObjectVisitor::Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject>
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

auto UniverseObjectVisitor::Visit(const std::shared_ptr<Planet>& obj) const -> std::shared_ptr<UniverseObject>
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

auto UniverseObjectVisitor::Visit(const std::shared_ptr<Ship>& obj) const -> std::shared_ptr<UniverseObject>
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

auto UniverseObjectVisitor::Visit(const std::shared_ptr<System>& obj) const -> std::shared_ptr<UniverseObject>
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

auto UniverseObjectVisitor::Visit(const std::shared_ptr<Field>& obj) const -> std::shared_ptr<UniverseObject>
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

auto UniverseObjectVisitor::Visit(const std::shared_ptr<Fighter>& obj) const -> std::shared_ptr<UniverseObject>
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }
