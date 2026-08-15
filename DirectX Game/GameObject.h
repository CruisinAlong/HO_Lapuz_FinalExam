#pragma once
#include "Vector3D.h"
#include "Matrix4x4.h"
#include "AComponent.h"
#include "PhysicsComponent.h" 
#include <vector>
#include <string>
#include <algorithm>

class GameObject
{
public:
	typedef std::vector<AComponent*> ComponentList;

	GameObject() :
		m_position(0.0f, 0.0f, 0.0f),
		m_rotation(0.0f, 0.0f, 0.0f),
		m_scale(1.0f, 1.0f, 1.0f)
	{
	}

	// Ensure attached components are deleted so their destructors can
	// unregister from systems (for example, remove physics colliders).
	virtual ~GameObject()
	{
		// Delete and detach components in the same order they were attached.
		for (auto c : m_components) {
			if (c) {
				c->detachOwner();
				delete c;
			}
		}
		m_components.clear();
	}

	void setPosition(const Vector3D& pos) { m_position = pos; }
	void setRotation(const Vector3D& rot) { m_rotation = rot; }

	// When scale changes, update attached PhysicsComponent colliders so physics matches visuals.
	void setScale(const Vector3D& scale)
	{
		m_scale = scale;
		// Notify Physics components to recreate their collider to match new scale
		for (auto c : m_components) {
			if (!c) continue;
			if (c->getType() == AComponent::Physics) {
				// safe to static_cast because we checked type
				PhysicsComponent* pc = static_cast<PhysicsComponent*>(c);
				if (pc) pc->recreateCollider();
			}
		}
	}

	Vector3D getPosition() const { return m_position; }
	Vector3D getRotation() const { return m_rotation; }
	Vector3D getScale() const { return m_scale; }

	Matrix4x4 getWorldMatrix() const
	{
    	// Correct transform order: scale, then rotate, then translate.
		// Using row-vector convention (v' = v * M) the world matrix should be
		// S * R * T so vertices are scaled in local space, then rotated, then translated.
		Matrix4x4 scaleMat; scaleMat.SetScale(m_scale);

		Matrix4x4 rotX; rotX.SetRotationX(m_rotation.m_x);
		Matrix4x4 rotY; rotY.SetRotationY(m_rotation.m_y);
		Matrix4x4 rotZ; rotZ.SetRotationZ(m_rotation.m_z);

		Matrix4x4 rot = rotY;
		rot *= rotX;
		rot *= rotZ;

		Matrix4x4 trans; trans.SetTranslation(m_position);

		Matrix4x4 world = scaleMat;
		world *= rot;
		world *= trans;
		return world;
	}

	// lifecycle
	virtual bool create() { return true; }
	virtual void update(float dt) { (void)dt; }
	virtual void render() {}
	virtual void destroy() {}
	// called when object is first created/activated so components can initialize
	virtual void awake() {}

	// component management
	void attachComponent(AComponent* component)
	{
		if (!component) return;
		component->attachOwner(this);
		m_components.push_back(component);
	}

	void detachComponent(AComponent* component)
	{
		if (!component) return;
		auto it = std::find(m_components.begin(), m_components.end(), component);
		if (it != m_components.end()) {
			component->detachOwner();
			m_components.erase(it);
		}
	}

	AComponent* findComponentByName(const std::string& name)
	{
		for (auto c : m_components) {
			if (c && c->getName() == name) return c;
		}
		return nullptr;
	}

	AComponent* findComponentOfType(AComponent::ComponentType type, const std::string& name = "")
	{
		for (auto c : m_components) {
			if (!c) continue;
			if (c->getType() == type) {
				if (name.empty() || c->getName() == name) return c;
			}
		}
		return nullptr;
	}

	ComponentList getComponentsOfType(AComponent::ComponentType type)
	{
		ComponentList out;
		for (auto c : m_components) {
			if (c && c->getType() == type) out.push_back(c);
		}
		return out;
	}

	// no child hierarchy in this simple GameObject; behave same as getComponentsOfType
	ComponentList getComponentsOfTypeRecursive(AComponent::ComponentType type)
	{
		return getComponentsOfType(type);
	}

	// Return all attached components
	ComponentList getAllComponents() const { return m_components; }

private:
	Vector3D m_position;
	Vector3D m_rotation;
	Vector3D m_scale;

	ComponentList m_components;
};
