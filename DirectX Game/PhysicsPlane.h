#pragma once
#pragma once
#include "Plane.h"
#include <vector>

class PhysicsPlane : public Plane
{
public:
    PhysicsPlane();
    virtual ~PhysicsPlane();

    bool create() override;
    void destroy() override;

    void setNormal(float x, float y, float z) { (void)x; (void)y; (void)z; }

    void update(float dt) override { Plane::update(dt); (void)dt; }

    static const std::vector<PhysicsPlane*>& getAllPlanes() { return s_planes; }

private:
    static std::vector<PhysicsPlane*> s_planes;
};

