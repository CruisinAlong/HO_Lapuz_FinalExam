#include "GraphicsEngine.h"
#include "RenderSystem.h"
#include "Debug.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "ShaderLibrary.h"

GraphicsEngine* GraphicsEngine::sharedInstance = nullptr;

GraphicsEngine::GraphicsEngine() : m_render(nullptr)
{
    try {
        m_render = new RenderSystem();
        ShaderLibrary::initialize(m_render);
        m_textureManager = new TextureManager();
        m_meshManager = new MeshManager();
    }
    catch (...) {
        if (m_render) { delete m_render; m_render = nullptr; }
        throw;
    }
}

GraphicsEngine::~GraphicsEngine()
{

    if (m_textureManager) { delete m_textureManager; m_textureManager = nullptr; }
    if (m_meshManager) { delete m_meshManager; m_meshManager = nullptr; }
    ShaderLibrary::destroy();
    if (m_render) {
        delete m_render;
        m_render = nullptr;
    }
}

bool GraphicsEngine::init()
{

    return (m_render != nullptr);
}

bool GraphicsEngine::release()
{
    if (m_render)
    {
        delete m_render;
        m_render = nullptr;
    }
    return true;
}
RenderSystem* GraphicsEngine::getRenderSystem()
{
    return m_render;
}

void GraphicsEngine::create()
{
    if (!sharedInstance) {
        try {
            sharedInstance = new GraphicsEngine();
        }
        catch (const std::exception& ex) {
            LOG("GraphicsEngine::create failed: %s", ex.what());
            if (sharedInstance) { delete sharedInstance; sharedInstance = nullptr; }
        }
        catch (...) {
            LOG("GraphicsEngine::create failed: unknown exception");
            if (sharedInstance) { delete sharedInstance; sharedInstance = nullptr; }
        }
    }
}

void GraphicsEngine::destroy()
{
    if (sharedInstance) {
        delete sharedInstance;
        sharedInstance = nullptr;
    }
}

GraphicsEngine* GraphicsEngine::getInstance()
{
    return sharedInstance;
}

GraphicsEngine* GraphicsEngine::get()
{
    return sharedInstance;
}

TextureManager* GraphicsEngine::getTextureManager()
{
    return sharedInstance ? sharedInstance->m_textureManager : nullptr;
}

MeshManager* GraphicsEngine::getMeshManager()
{
    return sharedInstance ? sharedInstance->m_meshManager : nullptr;
}
