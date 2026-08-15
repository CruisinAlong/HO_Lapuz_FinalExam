#include <Windows.h>
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")
#include "AppWindow.h"
#include "CorePrereqs.h"
#include "SceneBuilder.h"
#include "SceneSerializer.h"
#include <unordered_map>
#include <cmath>
#include "ShaderLibrary.h"

static void DebugLog(const char* format, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, format);
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, format, args);
    va_end(args);
    LOG_INFO("%s", buf);
}

// Helper to log per-entity transform and world matrix for diagnostics
static void LogEntityTransform(const char* phase, int index, GameObject* obj, const ObjectInstance& inst)
{
	if (!obj) return;
    DebugLog("%s Entity %d: pos=(%.3f, %.3f, %.3f) rot=(%.3f, %.3f, %.3f) scale=(%.3f, %.3f, %.3f)",
        phase, index,
        inst.position.m_x, inst.position.m_y, inst.position.m_z,
        inst.rotation.m_x, inst.rotation.m_y, inst.rotation.m_z,
        inst.scale.m_x, inst.scale.m_y, inst.scale.m_z);
}

bool AppWindow::saveLevel(const std::wstring& filename)
{
    return SceneSerializer::saveInstances(filename, m_cubes);
}

bool AppWindow::loadLevel(const std::wstring& filename)
{
    DebugLog("[AppWindow] loadLevel: delegating to SceneSerializer for '%S'", filename.c_str());

    if (!m_scene_builder) m_scene_builder = new SceneBuilder();

    for (size_t i = 0; i < m_cubes.size(); ++i) {
        if (m_cubes[i].object) {
            m_cubes[i].object->destroy();
            delete m_cubes[i].object;
            m_cubes[i].object = nullptr;
        }
    }
    m_cubes.clear();

    bool ok = SceneSerializer::loadInstances(filename, m_cubes, m_scene_builder);
    if (!ok) {
        DebugLog("[AppWindow] loadLevel: SceneSerializer::loadInstances failed for '%S'", filename.c_str());
        return false;
    }

    DebugLog("[AppWindow] loadLevel: finished loading %zu objects from '%S'", m_cubes.size(), filename.c_str());
    return true;
}

AppWindow::AppWindow()
    : m_swap_chain(nullptr),
      m_sphere(nullptr),
      m_old_delta(0),
      m_new_delta(0),
      m_delta_time(0.0f),
      m_delta_pos(0.0f),
      m_delta_scale(0.0f),
      m_left_button_down(false),
      m_right_button_down(false),
      m_move_forward(false),
      m_move_back(false),
      m_move_left(false),
      m_move_right(false),
      m_move_speed(3.0f),
      m_rotation_x(0.0f),
      m_rotation_y(0.0f),
      m_rotation_z(0.0f),
      m_rot_vel_x(0.0f),
      m_rot_vel_y(0.0f),
      m_forward(0.0f),
      m_rightward(0.0f),
      m_camera_pos(0.0f, 0.0f, -10.0f),
      m_simulation_running(false),
      m_prev_time(std::chrono::steady_clock::now())
{
    m_ui_manager = nullptr;
}

void AppWindow::updateScene()
{

    processPendingRemovals();


    Matrix4x4 world_cam;
    world_cam.SetIdentity();

    Matrix4x4 temp;
    temp.SetIdentity();
    temp.SetRotationX(m_rotation_x);
    world_cam *= temp;

    temp.SetIdentity();
    temp.SetRotationY(m_rotation_y);
    world_cam *= temp;

    float moveAmount = m_move_speed * (m_delta_time > 0.0f ? m_delta_time : 0.0f);

    Vector3D forwardDir = world_cam.getZDirection();
    Vector3D rightDir = world_cam.getXDirection();

    if (m_forward != 0.0f) {
        m_camera_pos = m_camera_pos + forwardDir * (m_forward * moveAmount);
    }

    if (m_rightward != 0.0f) {
        m_camera_pos = m_camera_pos + rightDir * (m_rightward * moveAmount);
    }

    Matrix4x4 trans;
    trans.SetIdentity();
    trans.SetTranslation(m_camera_pos);
    world_cam *= trans;

    world_cam.inverse();

    m_view = world_cam;

    RECT rc = this->getClientWindowRect();
    int width = (rc.right - rc.left);
    int height = (rc.bottom - rc.top);
    Matrix4x4 proj;
    proj.setPerspectiveFovLH(1.57f, ((float)width / (float)height), 0.1f, 100.0f);

    bool running = this->isSimulationRunning();

    for (size_t i = 0; i < m_cubes.size(); ++i)
    {
        ObjectInstance& inst = m_cubes[i];
        if (!inst.object) continue;

        bool isPhysicsObject = inst.hasPhysics();

        if (!isPhysicsObject || !running) {
            inst.object->setPosition(inst.position);
            inst.object->setRotation(inst.rotation);
            inst.object->setScale(inst.scale);
            if (!running && isPhysicsObject) {
                if (auto pc = dynamic_cast<PhysicsCube*>(inst.object)) {
                    pc->setVelocity(Vector3D(0.0f, 0.0f, 0.0f));
                }
                else {
                    auto comps = inst.object->getComponentsOfType(AComponent::Physics);
                    for (auto c : comps) {
                        if (!c) continue;
                        PhysicsComponent* ph = dynamic_cast<PhysicsComponent*>(c);
                        if (ph && ph->getRigidBody()) {
                            reactphysics3d::Vector3 zero(0.0f, 0.0f, 0.0f);
                            ph->getRigidBody()->setLinearVelocity(zero);
                            ph->getRigidBody()->setAngularVelocity(zero);
                        }
                    }
                }
            }
        }

        float visualDt = running ? m_delta_time : 0.0f;
        if (auto c = dynamic_cast<Cube*>(inst.object)) {
            c->setView(m_view);
            c->setProjection(proj);
            c->update(visualDt);
        } else if (auto s = dynamic_cast<Sphere*>(inst.object)) {
            s->setView(m_view);
            s->setProjection(proj);
            s->update(visualDt);
        } else if (auto cap = dynamic_cast<Capsule*>(inst.object)) {
            cap->setView(m_view);
            cap->setProjection(proj);
            cap->update(visualDt);
        } else if (auto p = dynamic_cast<Plane*>(inst.object)) {
            p->setView(m_view);
            p->setProjection(proj);
            p->update(visualDt);
        } else {
            inst.object->update(visualDt);
        }
    }

    if (running) {
        if (m_delta_time <= 0.0f) {
        } else {
            if (auto base = BaseComponentSystem::getInstance()) {
                if (auto phys = base->getPhysicsSystem()) {
                    phys->updateAllComponents(m_delta_time);
                }
            }
        }
    }

    for (size_t i = 0; i < m_cubes.size(); ++i)
    {
        ObjectInstance& inst = m_cubes[i];
        if (!inst.object) continue;

        bool isPhysicsObject = inst.hasPhysics();
        if (isPhysicsObject && running) {
            inst.position = inst.object->getPosition();
            inst.rotation = inst.object->getRotation();
            inst.scale = inst.object->getScale();

            float visualDt = 0.0f; 
            if (auto c = dynamic_cast<Cube*>(inst.object)) {
                c->setView(m_view);
                c->setProjection(proj);
                c->update(visualDt);
            } else if (auto s = dynamic_cast<Sphere*>(inst.object)) {
                s->setView(m_view);
                s->setProjection(proj);
                s->update(visualDt);
            } else if (auto cap = dynamic_cast<Capsule*>(inst.object)) {
                cap->setView(m_view);
                cap->setProjection(proj);
                cap->update(visualDt);
            } else if (auto p = dynamic_cast<Plane*>(inst.object)) {
                p->setView(m_view);
                p->setProjection(proj);
                p->update(visualDt);
            } else {
                inst.object->update(visualDt);
            }
        }
    }

    processPendingSpawns();
}

AppWindow::~AppWindow()
{
}

void AppWindow::onCreate()
{
	createGraphicsWindow();
}

int AppWindow::addCube()
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    return m_scene_builder->addCubeTo(m_cubes, m_box_texture);
}

int AppWindow::addPhysicsCube()
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    return m_scene_builder->addPhysicsCubeTo(m_cubes, 1.0f, m_box_texture);
}

int AppWindow::addTexturedCube()
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    return m_scene_builder->addTexturedCubeTo(m_cubes, m_box_texture);
}

int AppWindow::addMeshInstance()
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    return m_scene_builder->addMeshInstanceTo(m_cubes, m_teapot, Vector3D(0.0f,0.0f,0.0f), Vector3D(0.8f,0.8f,0.8f));
}

int AppWindow::addMeshInstanceFrom(MeshPtr mesh, const Vector3D& pos, const Vector3D& scale)
{
    if (!mesh) return -1;
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    return m_scene_builder->addMeshInstanceTo(m_cubes, mesh, pos, scale);
}

int AppWindow::addCapsule(float height)
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    return m_scene_builder->addCapsuleTo(m_cubes, height);
}

int AppWindow::addSphere(float radius)
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    return m_scene_builder->addSphereTo(m_cubes, radius);
}

void AppWindow::removeAllCubes()
{
    // Schedule removal to avoid deleting while UI may be iterating m_cubes
	m_pending_remove_all = true;
	m_pending_removals.clear();
}

size_t AppWindow::getEntityCount() const
{
	return m_cubes.size();
}

ObjectInstance* AppWindow::getEntity(size_t index)
{
	if (index >= m_cubes.size()) return nullptr;
	return &m_cubes[index];
}

void AppWindow::removeEntity(size_t index)
{
	if (index >= m_cubes.size()) return;
	m_pending_removals.push_back(index);
}

void AppWindow::processPendingRemovals()
{
	if (m_pending_remove_all) {
        for (size_t i = 0; i < m_cubes.size(); ++i)
		{
			if (m_cubes[i].object) {
				m_cubes[i].object->destroy();
				delete m_cubes[i].object;
				m_cubes[i].object = nullptr;
			}
		}
		m_cubes.clear();
		m_pending_remove_all = false;
		m_pending_removals.clear();
		return;
	}

	if (m_pending_removals.empty()) return;

	std::sort(m_pending_removals.begin(), m_pending_removals.end());
	m_pending_removals.erase(std::unique(m_pending_removals.begin(), m_pending_removals.end()), m_pending_removals.end());
	for (auto it = m_pending_removals.rbegin(); it != m_pending_removals.rend(); ++it)
	{
		size_t index = *it;
        if (index >= m_cubes.size()) continue;
		if (m_cubes[index].object) {
			m_cubes[index].object->destroy();
			delete m_cubes[index].object;
			m_cubes[index].object = nullptr;
		}
		m_cubes.erase(m_cubes.begin() + index);
	}
	m_pending_removals.clear();
}

void AppWindow::requestAddCube()
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    m_scene_builder->addCubeTo(m_cubes, m_box_texture);
}

void AppWindow::requestAddTexturedCube()
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    m_scene_builder->addTexturedCubeTo(m_cubes, m_box_texture);
}

void AppWindow::requestAddMeshInstance()
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    m_scene_builder->addMeshInstanceTo(m_cubes, m_teapot, Vector3D(0.0f,0.0f,0.0f), Vector3D(0.8f,0.8f,0.8f));
}

void AppWindow::requestAddPhysicsCube(int count)
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    int c = (count > 0) ? count : 1;
    for (int i = 0; i < c; ++i) m_scene_builder->addPhysicsCubeTo(m_cubes, 1.0f, m_box_texture);
}

void AppWindow::requestAddPhysicsPlane()
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    m_scene_builder->addPhysicsPlaneTo(m_cubes, Vector3D(0.0f, -1.5f, 0.0f), Vector3D(10.0f, 0.1f, 10.0f));
}

void AppWindow::requestAddCapsule(float height)
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    m_scene_builder->addCapsuleTo(m_cubes, height);
}

void AppWindow::requestAddSphere(float radius)
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    m_scene_builder->addSphereTo(m_cubes, radius);
}

void AppWindow::processPendingSpawns()
{
    if (m_pending_spawns.empty()) return;

    for (const auto& req : m_pending_spawns)
    {
        switch (req.type)
        {
        case SpawnRequest::AddCube:
            {
                if (!m_scene_builder) m_scene_builder = new SceneBuilder();
                ObjectInstance ci = m_scene_builder->createCube(Vector3D(0.0f,0.0f,0.0f));
                if (ci.object) {
                    m_cubes.push_back(ci);
                    int idx = static_cast<int>(m_cubes.size() - 1);
                    LogEntityTransform("Created", idx, m_cubes[idx].object, m_cubes[idx]);
                    auto comps = ci.object->getComponentsOfType(AComponent::Physics);
                    if (comps.empty()) {
                        PhysicsComponent* pc = new PhysicsComponent("PhysicsCube", 1.0f, ci.object);
                        if (pc) { ci.object->attachComponent(pc); pc->syncOwnerToBody(); }
                    }
                }
            }
            break;
        case SpawnRequest::AddTexturedCube:
            {
                if (!m_scene_builder) m_scene_builder = new SceneBuilder();
                ObjectInstance ci = m_scene_builder->createTexturedCube(m_box_texture, Vector3D(0.0f,0.0f,0.0f));
                if (ci.object) {
                    m_cubes.push_back(ci);
                    int idx = static_cast<int>(m_cubes.size() - 1);
                    LogEntityTransform("CreatedTexturedCube", idx, m_cubes[idx].object, m_cubes[idx]);
                    auto comps = ci.object->getComponentsOfType(AComponent::Physics);
                    if (comps.empty()) {
                        PhysicsComponent* pc = new PhysicsComponent("PhysicsCube", 1.0f, ci.object);
                        if (pc) ci.object->attachComponent(pc);
                    }
                }
            }
            break;
        case SpawnRequest::AddMeshInstance:
            {
                if (!m_scene_builder) m_scene_builder = new SceneBuilder();
                ObjectInstance ci = m_scene_builder->createMeshInstanceFrom(m_teapot, Vector3D(0.0f,0.0f,0.0f), Vector3D(0.8f,0.8f,0.8f));
                if (ci.object) {
                    m_cubes.push_back(ci);
                    int idx = static_cast<int>(m_cubes.size() - 1);
                    LogEntityTransform("CreatedMeshInstance", idx, m_cubes[idx].object, m_cubes[idx]);
                }
            }
            break;
        case SpawnRequest::AddPhysicsCube:
            {
                if (!m_scene_builder) m_scene_builder = new SceneBuilder();
                if (req.count <= 0) break;
                int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(req.count))));
                float spacing = 1.5f; 
                for (int i = 0; i < req.count; ++i) {
                    int row = i % cols;
                    int col = i / cols;
                    float x = (row - (cols - 1) * 0.5f) * spacing;
                    float z = (col - (cols - 1) * 0.5f) * spacing;
                    Vector3D pos(x, 2.0f, z);
                    ObjectInstance ci = m_scene_builder->createPhysicsCube(1.0f, pos,
                                                                           Vector3D(0.0f,0.0f,0.0f),
                                                                           Vector3D(1.0f,1.0f,1.0f),
                                                                           m_box_texture);
                    if (ci.object) {
                        m_cubes.push_back(ci);
                        int idx = static_cast<int>(m_cubes.size() - 1);
                        LogEntityTransform("CreatedPhysicsCube", idx, m_cubes[idx].object, m_cubes[idx]);
                    }
                }
            }
            break;
        case SpawnRequest::AddPhysicsPlane:
            {
                if (!m_scene_builder) m_scene_builder = new SceneBuilder();
                ObjectInstance ci = m_scene_builder->createPhysicsPlane(Vector3D(0.0f, -1.5f, 0.0f), Vector3D(10.0f, 0.1f, 10.0f));
                if (ci.object) {
                    m_cubes.push_back(ci);
                    int idx = static_cast<int>(m_cubes.size() - 1);
                    LogEntityTransform("CreatedPlane", idx, m_cubes[idx].object, m_cubes[idx]);
                }
            }
            break;
        case SpawnRequest::AddCapsule:
            {
                if (!m_scene_builder) m_scene_builder = new SceneBuilder();
                ObjectInstance ci = m_scene_builder->createCapsule(req.capsuleHeight > 0.0f ? req.capsuleHeight : 2.0f, Vector3D(0.0f,0.0f,0.0f));
                if (ci.object) {
                    m_cubes.push_back(ci);
                    int idx = static_cast<int>(m_cubes.size() - 1);
                    LogEntityTransform("CreatedCapsule", idx, m_cubes[idx].object, m_cubes[idx]);
                }
            }
            break;
        default:
            break;
        }
    }

    m_pending_spawns.clear();
}

bool AppWindow::createGraphicsWindow()
{
    InputSystem::create();
    InputSystem::get()->addListener(this);
    GraphicsEngine::create();

    BaseComponentSystem::create();

    GraphicsEngine* graphics = GraphicsEngine::getInstance();
    if (!graphics) return false;

	RECT rc = this->getClientWindowRect();
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
    RenderSystem* rs = graphics->getRenderSystem();
	if (!rs) return false;
	m_swap_chain = rs->createSwapChainPtr(this->m_hwnd, w, h);
	if (!m_swap_chain) return false;

	if (graphics) {
		ID3D11Device* device = graphics->getRenderSystem()->getDevice();
		ID3D11DeviceContext* context = graphics->getRenderSystem()->getContext();
		if (device && context) {
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui_ImplWin32_Init(this->m_hwnd);
			ImGui_ImplDX11_Init(device, context);
			m_imgui_initialized = true;

			if (!m_ui_manager) {
             m_ui_manager = new UIManager(this);
			}

			RenderSystem* rs = graphics->getRenderSystem();
			if (rs) {
				if (!Cube::InitSharedResources(rs)) {
					DebugLog("Warning: Cube::InitSharedResources failed");
				}
			}

			TextureManager* tm = graphics->getTextureManager();
			if (tm) {
				m_box_texture = tm->createTextureFromFile(L"Images\\brick.png");
				if (!m_box_texture) DebugLog("Warning: failed to load Images\\box.jpg via TextureManager");
			}

            MeshManager* mm = graphics->getMeshManager();
			if (mm) {
               m_teapot = mm->createMeshFromFile(L"Meshes\\teapot.obj");
				if (!m_teapot) DebugLog("Warning: failed to load Meshes\\teapot.obj via MeshManager");
				m_bunny = mm->createMeshFromFile(L"Meshes\\bunny.obj");
				if (!m_bunny) DebugLog("Warning: failed to load Meshes\\bunny.obj via MeshManager");
				m_armadillo = mm->createMeshFromFile(L"Meshes\\armadillo.obj");
				if (!m_armadillo) DebugLog("Warning: failed to load Meshes\\armadillo.obj via MeshManager");
				m_cube_mesh = mm->createCubeMesh();
				if (!m_cube_mesh) DebugLog("Warning: failed to create procedural cube mesh");
				m_plane_mesh = mm->createPlaneMesh();
				if (!m_plane_mesh) DebugLog("Warning: failed to create procedural plane mesh");
			}
            m_scene_builder = new SceneBuilder();
		}
	}

    m_cubes.clear();
    m_cubes.reserve(1);

    if (m_scene_builder) {
        ObjectInstance gi = m_scene_builder->createPhysicsPlane(Vector3D(0.0f, -1.5f, 0.0f), Vector3D(10.0f, 0.1f, 10.0f));
        if (gi.object) {
            m_cubes.push_back(gi);
        }
    } else {
        int idx = addPhysicsPlane();
        if (idx >= 0 && static_cast<size_t>(idx) < m_cubes.size()) {
        }
    }

	DebugLog("Created %zu scene objects (ground plane)", m_cubes.size());

	return true;
}

void AppWindow::onUpdate()
{
    if (!m_swap_chain) return;

    Window::onUpdate();
    InputSystem::get()->update();

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = now - m_prev_time;
    m_delta_time = static_cast<float>(diff.count()); // seconds
    m_prev_time = now;

    GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get()->clearRenderTargetColor(
		this->m_swap_chain.get(), 0.3f, 0.5f, 0.7f, 1.0f);

	RECT rc = this->getClientWindowRect();
    auto ctx = GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get();
	ctx->setViewportSize(rc.right - rc.left, rc.bottom - rc.top);

	updateScene();
	
    // Build batches: per-Mesh/texture batches, and per-primitive-type batches for primitives without MeshComponent
    using BatchKey = std::pair<Mesh*, ID3D11ShaderResourceView*>;
    struct BatchKeyHash {
        std::size_t operator()(BatchKey const& k) const noexcept {
            auto a = reinterpret_cast<uintptr_t>(k.first);
            auto b = reinterpret_cast<uintptr_t>(k.second);
            return static_cast<std::size_t>(a ^ (b + 0x9e3779b97f4a7c15ULL + (a<<6) + (a>>2)));
        }
    };
    struct BatchKeyEq { bool operator()(BatchKey const& a, BatchKey const& b) const noexcept { return a.first == b.first && a.second == b.second; } };

    std::unordered_map<BatchKey, std::vector<Matrix4x4>, BatchKeyHash, BatchKeyEq> meshBatches;
    std::vector<Matrix4x4> sphereMatrices;
    std::vector<Matrix4x4> capsuleMatrices;
    std::vector<Matrix4x4> planeMatrices;
    std::vector<ObjectInstance*> nonBatchedObjects;

    meshBatches.reserve(64);
    sphereMatrices.reserve(64);
    capsuleMatrices.reserve(64);
    planeMatrices.reserve(64);

    auto buildWorldMatrix = [](const Vector3D& pos, const Vector3D& rot, const Vector3D& scale) {
        Matrix4x4 scaleMat; scaleMat.SetScale(scale);
        Matrix4x4 rotX; rotX.SetRotationX(rot.m_x);
        Matrix4x4 rotY; rotY.SetRotationY(rot.m_y);
        Matrix4x4 rotZ; rotZ.SetRotationZ(rot.m_z);
        Matrix4x4 rotMat = rotY; rotMat *= rotX; rotMat *= rotZ;
        Matrix4x4 trans; trans.SetTranslation(pos);
        Matrix4x4 world = scaleMat; world *= rotMat; world *= trans;
        return world;
    };

    for (size_t i = 0; i < m_cubes.size(); ++i) {
        ObjectInstance& inst = m_cubes[i];
        if (!inst.object) continue;
        if (!inst.visible) continue;

        // Primitive-specific instancing
        if (dynamic_cast<Sphere*>(inst.object)) {
            sphereMatrices.push_back(buildWorldMatrix(inst.position, inst.rotation, inst.scale));
            continue;
        }
        if (dynamic_cast<Capsule*>(inst.object)) {
            capsuleMatrices.push_back(buildWorldMatrix(inst.position, inst.rotation, inst.scale));
            continue;
        }
        if (dynamic_cast<Plane*>(inst.object)) {
            planeMatrices.push_back(buildWorldMatrix(inst.position, inst.rotation, inst.scale));
            continue;
        }

        // MeshComponent-based batching for other objects
        AComponent* mcComp = inst.object->findComponentOfType(AComponent::MeshComp);
        MeshComponent* mc = mcComp ? dynamic_cast<MeshComponent*>(mcComp) : nullptr;
        if (mc && mc->getMesh()) {
            Mesh* mesh = mc->getMesh();
            ID3D11ShaderResourceView* srv = mc->getTexture() ? mc->getTexture()->getSRV() : nullptr;
            meshBatches[BatchKey(mesh, srv)].push_back(buildWorldMatrix(inst.position, inst.rotation, inst.scale));
        } else {
            nonBatchedObjects.push_back(&inst);
        }
    }

    // Get rendering contexts
    RenderSystem* rs2 = GraphicsEngine::getInstance()->getRenderSystem();
    DeviceContext* ctxWrap = rs2 ? rs2->getImmediateDeviceContext().get() : nullptr;
    ID3D11DeviceContext* d3dCtx = rs2 ? rs2->getContext() : nullptr;
    ShaderLibrary* lib = ShaderLibrary::getInstance();

    // Draw sphere batch
    if (!sphereMatrices.empty() && ctxWrap && d3dCtx) {
        Sphere::InitInstanceBuffer(rs2, 10000);
        if (Sphere::UpdateInstanceBuffer(d3dCtx, sphereMatrices.data(), static_cast<UINT>(sphereMatrices.size()))) {
            // bind sphere shared shaders inside RenderInstanced
            Sphere::RenderInstanced(ctxWrap, static_cast<UINT>(sphereMatrices.size()));
        }
    }

    // Draw capsule batch
    if (!capsuleMatrices.empty() && ctxWrap && d3dCtx) {
        Capsule::InitInstanceBuffer(rs2, 10000);
        if (Capsule::UpdateInstanceBuffer(d3dCtx, capsuleMatrices.data(), static_cast<UINT>(capsuleMatrices.size()))) {
            Capsule::RenderInstanced(ctxWrap, static_cast<UINT>(capsuleMatrices.size()));
        }
    }

    // Draw plane batch
    if (!planeMatrices.empty() && ctxWrap && d3dCtx) {
        Plane::InitInstanceBuffer(rs2, 10000);
        if (Plane::UpdateInstanceBuffer(d3dCtx, planeMatrices.data(), static_cast<UINT>(planeMatrices.size()))) {
            Plane::RenderInstanced(ctxWrap, static_cast<UINT>(planeMatrices.size()));
        }
    }

    // Draw mesh-based batches
    for (auto& kv : meshBatches) {
        Mesh* mesh = kv.first.first;
        ID3D11ShaderResourceView* srv = kv.first.second;
        auto& mats = kv.second;
        if (!mesh || mats.empty()) continue;
        // ensure per-mesh instance buffer
        if (!mesh->getVertexBuffer() || !mesh->getIndexBuffer()) continue;
        mesh->InitInstanceBuffer(rs2, 10000);
        if (mesh->UpdateInstanceBuffer(d3dCtx, mats.data(), static_cast<UINT>(mats.size()))) {
            VertexShader* vs = srv ? lib->getVertexShader(ShaderNames::TEX_VS) : lib->getVertexShader(ShaderNames::BASIC_VS);
            PixelShader* ps = srv ? lib->getPixelShader(ShaderNames::TEX_PS) : lib->getPixelShader(ShaderNames::BASIC_PS);
            if (vs) ctxWrap->setVertexShader(vs);
            if (ps) ctxWrap->setPixelShader(ps);
            if (srv && ps) ctxWrap->setTexture(ps, srv);
            mesh->RenderInstanced(ctxWrap, static_cast<UINT>(mats.size()));
            if (srv && ps) ctxWrap->setTexture(ps, nullptr);
        }
    }

    // Render non-batched objects individually
    for (auto pInst : nonBatchedObjects) {
        ObjectInstance& inst = *pInst;
        if (!inst.object) continue;
        if (auto c = dynamic_cast<Cube*>(inst.object)) c->render();
        else if (auto s = dynamic_cast<Sphere*>(inst.object)) s->render();
        else if (auto cap = dynamic_cast<Capsule*>(inst.object)) cap->render();
        else inst.object->render();
    }

    if (m_imgui_initialized)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (m_ui_manager) m_ui_manager->drawUI(m_delta_time);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	m_swap_chain->present(true);
}

void AppWindow::onDestroy()
{
	Window::onDestroy();

    for (size_t i = 0; i < m_cubes.size(); ++i)
	{
		if (m_cubes[i].object)
		{
			m_cubes[i].object->destroy();
			delete m_cubes[i].object;
			m_cubes[i].object = nullptr;
		}
	}
	m_cubes.clear();

	if (m_swap_chain) {
		m_swap_chain.reset();
	}

	if (m_imgui_initialized)
	{
		if (m_ui_manager) { delete m_ui_manager; m_ui_manager = nullptr; }

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		m_imgui_initialized = false;
	}

    if (m_scene_builder) { delete m_scene_builder; m_scene_builder = nullptr; }

    Cube::ReleaseSharedResources();

	GraphicsEngine::destroy();
	InputSystem::destroy();
}

void AppWindow::onFocus()
{
	InputSystem::get()->addListener(this);
}

void AppWindow::onKillFocus()
{
	InputSystem::get()->removeListener(this);
}

void AppWindow::onKeyDown(int key)
{
	if (key == 'W') { m_move_forward = true;  m_forward = 1.0f; }
	else if (key == 'S') { m_move_back = true;   m_forward = -1.0f; }
	else if (key == 'A') { m_move_left = true;   m_rightward = -1.0f; }
	else if (key == 'D') { m_move_right = true;  m_rightward = 1.0f; }
}

void AppWindow::onKeyUp(int key)
{
	if (key == 'W') { m_move_forward = false; if (m_forward > 0.0f) m_forward = 0.0f; }
	else if (key == 'S') { m_move_back = false;  if (m_forward < 0.0f) m_forward = 0.0f; }
	else if (key == 'A') { m_move_left = false;  if (m_rightward < 0.0f) m_rightward = 0.0f; }
	else if (key == 'D') { m_move_right = false; if (m_rightward > 0.0f) m_rightward = 0.0f; }
}

void AppWindow::onLeftMouseDown(const Point& delta_mouse_pos)
{
	m_left_button_down = true;
	::SetCapture(this->m_hwnd);
	::ShowCursor(FALSE);
}

void AppWindow::onLeftMouseUp(const Point& delta_mouse_pos)
{
	m_left_button_down = false;
	::ReleaseCapture();
	::ShowCursor(TRUE);
}

void AppWindow::onRightMouseDown(const Point& delta_mouse_pos)
{
	m_right_button_down = true;
}

void AppWindow::onRightMouseUp(const Point& delta_mouse_pos)
{
	m_right_button_down = false;
}

void AppWindow::onMouseMove(const Point& delta_mouse_pos)
{
	// Rotate camera while left mouse button is held
	if (!m_left_button_down) return;

	const float sensitivity = 0.005f; // adjust as needed
	// Apply raw mouse delta (non-inverted)
	m_rotation_y += delta_mouse_pos.m_x * sensitivity; // yaw
	m_rotation_x += delta_mouse_pos.m_y * sensitivity; // pitch

	// Clamp pitch to avoid gimbal flip (~ +/- 89 degrees)
	const float maxPitch = 1.4f; // ~80 degrees
	if (m_rotation_x > maxPitch) m_rotation_x = maxPitch;
	if (m_rotation_x < -maxPitch) m_rotation_x = -maxPitch;
}

int AppWindow::addPhysicsPlane()	
{
    if (!m_scene_builder) m_scene_builder = new SceneBuilder();
    return m_scene_builder->addPhysicsPlaneTo(m_cubes, Vector3D(0.0f, -1.5f, 0.0f), Vector3D(10.0f, 0.1f, 10.0f));
}



void AppWindow::setSimulationRunning(bool r)
{
    // Starting simulation: capture current editor-state transforms
    if (r && !m_simulation_running) {
		m_old_delta = ::GetTickCount64();
        m_pre_sim_states.clear();
        m_pre_sim_states.reserve(m_cubes.size());
        for (auto &inst : m_cubes) {
            PreSimState s;
            s.position = inst.position;
            s.rotation = inst.rotation;
            s.scale = inst.scale;
            m_pre_sim_states.push_back(s);
        }
        {
            size_t physCount = 0;
            if (auto base = BaseComponentSystem::getInstance()) {
                if (auto phys = base->getPhysicsSystem()) {
                    physCount = phys->getAllComponents().size();
                }
            }
            DebugLog("[AppWindow] setSimulationRunning: START play components=%zu entities=%zu", physCount, m_cubes.size());
        }
        m_simulation_running = true;
        return;
    }

    if (!r && m_simulation_running) {
        size_t n = std::min(m_cubes.size(), m_pre_sim_states.size());
        for (size_t i = 0; i < n; ++i) {
            ObjectInstance& inst = m_cubes[i];
            const PreSimState& s = m_pre_sim_states[i];
            if (inst.object) {
                inst.position = s.position;
                inst.rotation = s.rotation;
                inst.scale = s.scale;
                inst.object->setPosition(s.position);
                inst.object->setRotation(s.rotation);
                inst.object->setScale(s.scale);

                auto comps = inst.object->getComponentsOfType(AComponent::Physics);
                for (auto c : comps) {
                    if (!c) continue;
                    PhysicsComponent* pc = dynamic_cast<PhysicsComponent*>(c);
                    if (pc && pc->getRigidBody()) {
                        // move the rigid body to owner transform and zero velocities
                        pc->syncOwnerToBody();
                        reactphysics3d::Vector3 zero(0.0f, 0.0f, 0.0f);
                        pc->getRigidBody()->setLinearVelocity(zero);
                        pc->getRigidBody()->setAngularVelocity(zero);
                    }
                }
            }
        }
        DebugLog("[AppWindow] setSimulationRunning: STOP pause entities_restored=%zu", n);
        m_pre_sim_states.clear();
        m_simulation_running = false;
        return;
    }

    // Otherwise, simple state set
    m_simulation_running = r;
}

bool AppWindow::saveLevelAs()
{
    std::wstring initialDir = L".\\DirectX Game";

    WCHAR filebuf[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = this->m_hwnd;
    ofn.lpstrFilter = L"Level Files\0*.level\0All Files\0*.*\0";
    ofn.lpstrFile = filebuf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = initialDir.c_str();
    ofn.lpstrDefExt = L"level";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetSaveFileNameW(&ofn)) {
        std::wstring chosen(filebuf);
        return SceneSerializer::saveInstances(chosen, m_cubes);
    }
    return false;
}

bool AppWindow::loadLevelFromDialog()
{
    std::wstring initialDir = L".\\DirectX Game";

    WCHAR filebuf[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = this->m_hwnd;
    ofn.lpstrFilter = L"Level Files\0*.level\0All Files\0*.*\0";
    ofn.lpstrFile = filebuf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = initialDir.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn)) {
        std::wstring chosen(filebuf);
        DebugLog("[AppWindow] loadLevelFromDialog: chosen file '%S' (cwd preserved)", chosen.c_str());
        bool ok = loadLevel(chosen);
        return ok;
    }

    return false;
}

bool ObjectInstance::hasPhysics() const {
    if (!object) return false;
    return (object->findComponentOfType(AComponent::Physics) != nullptr);
}