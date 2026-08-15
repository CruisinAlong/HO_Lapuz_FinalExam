#pragma once
#include "Cube.h"
#include "Vector3D.h"

class PhysicsCube : public Cube
{
public:
    PhysicsCube();
    virtual ~PhysicsCube();

    // create as cube with physical properties
    bool create() override;

    // enable/disable physics
    void setStatic(bool s) { m_static = s; }

    void setVelocity(const Vector3D& v) { m_velocity = v; }

    void update(float dt) override;

private:
    Vector3D m_velocity;
    bool m_static = false;
    const Vector3D m_gravity = Vector3D(0.0f, -9.81f, 0.0f);
};
