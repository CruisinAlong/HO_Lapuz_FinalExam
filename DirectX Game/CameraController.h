#pragma once
#include "CorePrereqs.h"

class CameraController {
public:
    CameraController();
    ~CameraController();

    void setPosition(const Vector3D& pos) { m_camera_pos = pos; }
    Vector3D getPosition() const { return m_camera_pos; }

    void setRotation(float pitch, float yaw, float roll) { m_rot_x = pitch; m_rot_y = yaw; m_rot_z = roll; }
    void getRotation(float& pitch, float& yaw, float& roll) const { pitch = m_rot_x; yaw = m_rot_y; roll = m_rot_z; }

    Matrix4x4 getViewMatrix() const;

    // Simple helpers used by AppWindow: move and rotate
    void move(const Vector3D& delta) { m_camera_pos = m_camera_pos + delta; }
    void rotate(float dx, float dy) { m_rot_y += dx; m_rot_x += dy; }

private:
    Vector3D m_camera_pos;
    float m_rot_x;
    float m_rot_y;
    float m_rot_z;
};
