#pragma once
#include "Window.h"
#include "GraphicsEngine.h"
#include "Prerequisites.h"
#include "SwapChain.h"
#include "DeviceContext.h"

#include "InputListener.h"
#include "Cube.h"
#include "Sphere.h"
#include "Capsule.h"
#include "SceneEditor.h"
#include "Vector3D.h"
#include "Matrix4x4.h"
#include "AComponent.h"

class GameObject;
#include <vector>
#include <chrono>

class UIManager;
class SceneBuilder;

struct ObjectInstance {
	GameObject* object = nullptr;
	Vector3D position = Vector3D(0.0f, 0.0f, 0.0f);
	Vector3D rotation = Vector3D(0.0f, 0.0f, 0.0f);
	Vector3D scale = Vector3D(1.0f, 1.0f, 1.0f);
	bool visible = true;

    bool hasPhysics() const;
};

class AppWindow : public Window, public InputListener
{
public:
	AppWindow();

	UIManager* getUIManager() { return m_ui_manager; }

	int addCube();
    int addTexturedCube();
    int addMeshInstance();
    int addMeshInstanceFrom(MeshPtr mesh, const Vector3D& pos = Vector3D(0.0f,0.0f,0.0f), const Vector3D& scale = Vector3D(0.8f,0.8f,0.8f));
    int addCapsule(float height = 2.0f);
    int addSphere(float radius = 0.5f);
	int addPhysicsCube();
	int addPhysicsPlane();
	bool saveLevel(const std::wstring& filename);
	bool loadLevel(const std::wstring& filename);
	bool saveLevelAs();
	bool loadLevelFromDialog();
	void removeAllCubes();
	size_t getEntityCount() const;
    struct ObjectInstance* getEntity(size_t index);
	void removeEntity(size_t index);

	void requestAddCube();
	void requestAddTexturedCube();
	void requestAddMeshInstance();
	void requestAddPhysicsCube(int count = 1); 
	void requestAddPhysicsPlane();
	void requestAddCapsule(float height = 2.0f);
	void requestAddSphere(float radius = 0.5f);

	void updateScene();

	~AppWindow();

	void onCreate() override;
	void onUpdate() override;
	void onDestroy() override;

	void onFocus() override;
	void onKillFocus() override;

	void onKeyDown(int key) override;
	void onKeyUp(int key) override;

	void onLeftMouseDown(const Point& delta_mouse_pos) override;
	void onLeftMouseUp(const Point& delta_mouse_pos) override;
	void onRightMouseDown(const Point& delta_mouse_pos) override;
	void onRightMouseUp(const Point& delta_mouse_pos) override;

	void onMouseMove(const Point& delta_mouse_pos) override;

private:
	bool createGraphicsWindow();

private:
    SwapChainPtr m_swap_chain;
    std::vector<ObjectInstance> m_cubes;
	Sphere* m_sphere;


private:
	unsigned long long m_old_delta;
	unsigned long long m_new_delta;
	float m_delta_time;

	float m_delta_pos;
	float m_delta_scale;

	bool m_left_button_down;
	bool m_right_button_down;

	bool m_move_forward;
	bool m_move_back;
	bool m_move_left;
	bool m_move_right;
	float m_move_speed;

	float m_rotation_x;
	float m_rotation_y;
	float m_rotation_z;
	float m_rot_vel_x;
	float m_rot_vel_y;

	float m_forward = 0.0f;

	float m_rightward = 0.0f;

	Matrix4x4 m_world_cam;
	Matrix4x4 m_view;

	Vector3D m_camera_pos;

	UIManager* m_ui_manager = nullptr;
	bool m_imgui_initialized = false;
	std::vector<size_t> m_pending_removals;
	bool m_pending_remove_all = false;
	void processPendingRemovals();

    struct SpawnRequest {
        enum Type {
            AddCube,
            AddTexturedCube,
            AddMeshInstance,
            AddPhysicsCube,
            AddPhysicsPlane,
            AddCapsule
        } type;
        int count;          
        float capsuleHeight; 
    };
    std::vector<SpawnRequest> m_pending_spawns;
    void processPendingSpawns();

	TexturePtr m_box_texture;
    MeshPtr m_teapot;
    MeshPtr m_bunny;
	MeshPtr m_armadillo;

	MeshPtr m_cube_mesh;
	MeshPtr m_plane_mesh;

	SceneBuilder* m_scene_builder = nullptr;

	bool m_simulation_running = false;

public:
	void setSimulationRunning(bool r);
	bool isSimulationRunning() const { return m_simulation_running; }

private:
	struct PreSimState {
		Vector3D position;
		Vector3D rotation;
		Vector3D scale;
	};
	std::vector<PreSimState> m_pre_sim_states;
	std::chrono::steady_clock::time_point m_prev_time;
};