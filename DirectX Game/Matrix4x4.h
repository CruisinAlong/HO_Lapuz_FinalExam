#pragma once
#include "Vector3D.h"
#include "Vector4D.h"
#include <memory>
#include <cmath>
#include <cstring>

class Matrix4x4
{
public:
	Matrix4x4() {
		SetIdentity();
	}

	void SetIdentity() {
		::memset(m_mat, 0, sizeof(float) * 16);
		m_mat[0][0] = 1.0f;
		m_mat[1][1] = 1.0f;
		m_mat[2][2] = 1.0f;
		m_mat[3][3] = 1.0f;
	}

	void SetTranslation(const Vector3D& translation) {
		SetIdentity();
		m_mat[3][0] = translation.m_x;
		m_mat[3][1] = translation.m_y;
		m_mat[3][2] = translation.m_z;
	}

	void SetScale(const Vector3D& scale) {
		SetIdentity();
		m_mat[0][0] = scale.m_x;
		m_mat[1][1] = scale.m_y;
		m_mat[2][2] = scale.m_z;
	}

	void SetRotationX(float angle)
	{
		SetIdentity();
		float c = std::cosf(angle);
		float s = std::sinf(angle);
		m_mat[1][1] = c;  m_mat[1][2] = s;
		m_mat[2][1] = -s; m_mat[2][2] = c;
	}

	void SetRotationY(float angle)
	{
		SetIdentity();
		float c = std::cosf(angle);
		float s = std::sinf(angle);
		m_mat[0][0] = c;  m_mat[0][2] = -s;
		m_mat[2][0] = s;  m_mat[2][2] = c;
	}

	void SetRotationZ(float angle)
	{
		SetIdentity();
		float c = std::cosf(angle);
		float s = std::sinf(angle);
		m_mat[0][0] = c;  m_mat[0][1] = s;
		m_mat[1][0] = -s; m_mat[1][1] = c;
	}

	void SetRotationYawPitchRoll(float yaw, float pitch, float roll)
	{
		Matrix4x4 ry, rx, rz;
		ry.SetRotationY(yaw);
		rx.SetRotationX(pitch);
		rz.SetRotationZ(roll);

		*this = ry;
		*this *= rx;
		*this *= rz;
	}

	void operator*=(const Matrix4x4& rhs)
	{
		Matrix4x4 out;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				float sum = 0.0f;
				for (int k = 0; k < 4; ++k)
				{
					sum += m_mat[i][k] * rhs.m_mat[k][j];
				}
				out.m_mat[i][j] = sum;
			}
		}
		::memcpy(m_mat, out.m_mat, sizeof(m_mat));
	}

	void setOrthoLH(float width, float height, float z_near, float z_far) {
		SetIdentity();
		m_mat[0][0] = 2.0f / width;
		m_mat[1][1] = 2.0f / height;
		m_mat[2][2] = 1.0f / (z_far - z_near);
		m_mat[3][2] = -z_near / (z_far - z_near);
	}
	void setPerspectiveFovLH(float fovYRadians, float aspect, float z_near, float z_far)
	{
		if (aspect <= 0.0f) aspect = 1.0f;
		if (z_far == z_near) z_far = z_near + 1.0f;

		float yScale = 1.0f / std::tanf(fovYRadians * 0.5f);
		float xScale = yScale / aspect;


		SetIdentity();
		m_mat[0][0] = xScale;
		m_mat[1][1] = yScale;
		m_mat[2][2] = z_far / (z_far - z_near);
		m_mat[2][3] = 1.0f;
		m_mat[3][2] = (-z_near * z_far) / (z_far - z_near);
		m_mat[3][3] = 0.0f;
	}


	void SetLookAtLH(const Vector3D& eye, const Vector3D& target, const Vector3D& up)
	{
		Vector3D z;
		z.m_x = target.m_x - eye.m_x;
		z.m_y = target.m_y - eye.m_y;
		z.m_z = target.m_z - eye.m_z;
		{
			float len = std::sqrt(z.m_x*z.m_x + z.m_y*z.m_y + z.m_z*z.m_z);
			if (len > 0.0f) { z.m_x /= len; z.m_y /= len; z.m_z /= len; }
		}
		Vector3D x;
		x.m_x = up.m_y * z.m_z - up.m_z * z.m_y;
		x.m_y = up.m_z * z.m_x - up.m_x * z.m_z;
		x.m_z = up.m_x * z.m_y - up.m_y * z.m_x;
		{
			float len = std::sqrt(x.m_x*x.m_x + x.m_y*x.m_y + x.m_z*x.m_z);
			if (len > 0.0f) { x.m_x /= len; x.m_y /= len; x.m_z /= len; }
		}
		Vector3D y;
		y.m_x = z.m_y * x.m_z - z.m_z * x.m_y;
		y.m_y = z.m_z * x.m_x - z.m_x * x.m_z;
		y.m_z = z.m_x * x.m_y - z.m_y * x.m_x;

		SetIdentity();
		m_mat[0][0] = x.m_x; m_mat[0][1] = x.m_y; m_mat[0][2] = x.m_z; m_mat[0][3] = 0.0f;
		m_mat[1][0] = y.m_x; m_mat[1][1] = y.m_y; m_mat[1][2] = y.m_z; m_mat[1][3] = 0.0f;
		m_mat[2][0] = z.m_x; m_mat[2][1] = z.m_y; m_mat[2][2] = z.m_z; m_mat[2][3] = 0.0f;
		m_mat[3][0] = -(x.m_x*eye.m_x + x.m_y*eye.m_y + x.m_z*eye.m_z);
		m_mat[3][1] = -(y.m_x*eye.m_x + y.m_y*eye.m_y + y.m_z*eye.m_z);
		m_mat[3][2] = -(z.m_x*eye.m_x + z.m_y*eye.m_y + z.m_z*eye.m_z);
		m_mat[3][3] = 1.0f;
	}

	void setMatrix(const Matrix4x4& other)
	{
		::memcpy(this->m_mat, other.m_mat, sizeof(float) * 16);
	}

	float getDeterminant()
	{
		Vector4D minor, v1, v2, v3;
		float det;

		v1 = Vector4D(this->m_mat[0][0], this->m_mat[1][0], this->m_mat[2][0], this->m_mat[3][0]);
		v2 = Vector4D(this->m_mat[0][1], this->m_mat[1][1], this->m_mat[2][1], this->m_mat[3][1]);
		v3 = Vector4D(this->m_mat[0][2], this->m_mat[1][2], this->m_mat[2][2], this->m_mat[3][2]);


		minor.cross(v1, v2, v3);
		det = -(this->m_mat[0][3] * minor.m_x + this->m_mat[1][3] * minor.m_y + this->m_mat[2][3] * minor.m_z +
			this->m_mat[3][3] * minor.m_w);
		return det;
	}

	void inverse()
	{
		int a, i, j;
		Matrix4x4 out;
		Vector4D v, vec[3];
		float det = 0.0f;

		det = this->getDeterminant();
		if (!det) return;
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 4; j++)
			{
				if (j != i)
				{
					a = j;
					if (j > i) a = a - 1;
					vec[a].m_x = (this->m_mat[j][0]);
					vec[a].m_y = (this->m_mat[j][1]);
					vec[a].m_z = (this->m_mat[j][2]);
					vec[a].m_w = (this->m_mat[j][3]);
				}
			}
			v.cross(vec[0], vec[1], vec[2]);

			out.m_mat[0][i] = pow(-1.0f, i) * v.m_x / det;
			out.m_mat[1][i] = pow(-1.0f, i) * v.m_y / det;
			out.m_mat[2][i] = pow(-1.0f, i) * v.m_z / det;
			out.m_mat[3][i] = pow(-1.0f, i) * v.m_w / det;
		}

		this->setMatrix(out);
	}

	Vector3D getZDirection() {
		return Vector3D(m_mat[2][0], m_mat[2][1], m_mat[2][2]);
	}

	Vector3D getXDirection() {
		return Vector3D(m_mat[0][0], m_mat[0][1], m_mat[0][2]);
	}

	Vector3D getTranslation() {
		return Vector3D(m_mat[3][0], m_mat[3][1], m_mat[3][2]);
	}




	~Matrix4x4() {}

public:
	float m_mat[4][4] = {};
};