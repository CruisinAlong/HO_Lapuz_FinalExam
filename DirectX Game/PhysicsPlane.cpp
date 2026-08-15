#include "PhysicsPlane.h"
#include <algorithm>

#include "PhysicsPlane.h"

std::vector<PhysicsPlane*> PhysicsPlane::s_planes;

PhysicsPlane::PhysicsPlane() {}

PhysicsPlane::~PhysicsPlane() {
    // ensure unregistered
    auto it = std::find(s_planes.begin(), s_planes.end(), this);
    if (it != s_planes.end()) s_planes.erase(it);
}

bool PhysicsPlane::create()
{
    // reuse Plane creation (plane mesh + shaders)
    if (!Plane::create()) return false;
    // register this plane for simple physics collision queries
    s_planes.push_back(this);
    return true;
}

void PhysicsPlane::destroy()
{
    // unregister
    auto it = std::find(s_planes.begin(), s_planes.end(), this);
    if (it != s_planes.end()) s_planes.erase(it);
    Plane::destroy();
}
