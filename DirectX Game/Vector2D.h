#pragma once

class Vector2D
{
public:
    Vector2D() noexcept : m_x(0.0f), m_y(0.0f) {}
    Vector2D(float x, float y) noexcept : m_x(x), m_y(y) {}
    Vector2D(const Vector2D& other) noexcept = default;
    Vector2D& operator=(const Vector2D& other) noexcept = default;
    ~Vector2D() noexcept = default;

    const float* data() const noexcept { return &m_x; }
    float* data() noexcept { return &m_x; }

    Vector2D operator *(float num) const { return Vector2D(m_x * num, m_y * num); }
    Vector2D operator +(Vector2D vec) const { return Vector2D(m_x + vec.m_x, m_y + vec.m_y); }

public:
    float m_x;
    float m_y;
};
