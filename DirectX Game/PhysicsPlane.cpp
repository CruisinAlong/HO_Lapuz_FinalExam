#include "PhysicsPlane.h"
#include <algorithm>

#include "PhysicsPlane.h"

std::vector<PhysicsPlane*> PhysicsPlane::s_planes;

PhysicsPlane::PhysicsPlane() {}

PhysicsPlane::~PhysicsPlane() {
    auto it = std::find(s_planes.begin(), s_planes.end(), this);
    if (it != s_planes.end()) s_planes.erase(it);
}

bool PhysicsPlane::create()
{
    if (!Plane::create()) return false;
    s_planes.push_back(this);
    return true;
}

void PhysicsPlane::destroy()
{
    auto it = std::find(s_planes.begin(), s_planes.end(), this);
    if (it != s_planes.end()) s_planes.erase(it);
    Plane::destroy();
}
