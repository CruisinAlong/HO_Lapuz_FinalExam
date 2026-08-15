#pragma once

class Vector3D
{
public:
    Vector3D() noexcept : m_x(0.0f), m_y(0.0f), m_z(0.0f) {}
    Vector3D(float x, float y, float z) noexcept : m_x(x), m_y(y), m_z(z) {}
    Vector3D(const Vector3D& other) noexcept = default;
    Vector3D& operator=(const Vector3D& other) noexcept = default;
    ~Vector3D() noexcept = default;

    static Vector3D lerp(const Vector3D& start, const Vector3D& end, float delta)
    {
        Vector3D v;
		v.m_x = start.m_x + (end.m_x - start.m_x) * delta;
		v.m_y = start.m_y + (end.m_y - start.m_y) * delta;
        v.m_z = start.m_z + (end.m_z - start.m_z) * delta;
		return v;
	}


    const float* data() const noexcept { return &m_x; }
    float* data() noexcept { return &m_x; }

    Vector3D operator *(float num) const {
        return Vector3D(m_x * num, m_y * num, m_z * num);
    }

    Vector3D operator +(const Vector3D& vec) const {
        return Vector3D(m_x + vec.m_x, m_y + vec.m_y, m_z + vec.m_z);
    }

public:
    float m_x;
    float m_y;
    float m_z;
};