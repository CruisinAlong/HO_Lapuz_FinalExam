#include "CameraController.h"

CameraController::CameraController()
    : m_camera_pos(0.0f, 0.0f, -10.0f), m_rot_x(0.0f), m_rot_y(0.0f), m_rot_z(0.0f)
{
}

CameraController::~CameraController()
{
}

Matrix4x4 CameraController::getViewMatrix() const
{
    Matrix4x4 world_cam;
    world_cam.SetIdentity();

    Matrix4x4 temp;
    temp.SetIdentity();
    temp.SetRotationX(m_rot_x);
    world_cam *= temp;

    temp.SetIdentity();
    temp.SetRotationY(m_rot_y);
    world_cam *= temp;

    temp.SetIdentity();
    temp.SetTranslation(m_camera_pos);
    world_cam *= temp;

    world_cam.inverse();
    return world_cam;
}
