#pragma once

class Vector4D
{
public:
	Vector4D() noexcept : m_x(0.0f), m_y(0.0f), m_z(0.0f), m_w(0.0f) {}
	Vector4D(float x, float y, float z, float w) noexcept : m_x(x), m_y(y), m_z(z), m_w(w) {}
	Vector4D(const Vector4D& other) noexcept = default;
	Vector4D& operator=(const Vector4D& other) noexcept = default;
	~Vector4D() noexcept = default;

	const float* data() const noexcept { return &m_x; }
	float* data() noexcept { return &m_x; }

	void cross(Vector4D& v1, Vector4D& v2, Vector4D& v3)
	{
		this->m_x = v1.m_y * (v2.m_z * v3.m_w - v3.m_z * v2.m_w) - v1.m_z * (v2.m_y * v3.m_w - v3.m_y * v2.m_w) + v1.m_w * (v2.m_y * v3.m_z - v2.m_z * v3.m_y);
		this->m_y = -(v1.m_x * (v2.m_z * v3.m_w - v3.m_z * v2.m_w) - v1.m_z * (v2.m_x * v3.m_w - v3.m_x * v2.m_w) + v1.m_w * (v2.m_x * v3.m_z - v3.m_x * v2.m_z));
		this->m_z = v1.m_x * (v2.m_y * v3.m_w - v3.m_y * v2.m_w) - v1.m_y * (v2.m_x * v3.m_w - v3.m_x * v2.m_w) + v1.m_w * (v2.m_x * v3.m_y - v3.m_x * v2.m_y);
		this->m_w = -(v1.m_x * (v2.m_y * v3.m_z - v3.m_y * v2.m_z) - v1.m_y * (v2.m_x * v3.m_z - v3.m_x * v2.m_z) + v1.m_z * (v2.m_x * v3.m_y - v3.m_x * v2.m_y));
	}

public:
	float m_x;
	float m_y;
	float m_z;
	float m_w;
};