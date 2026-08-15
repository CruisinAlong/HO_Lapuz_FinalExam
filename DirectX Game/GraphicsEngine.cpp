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
        // initialize shader library with the render system
        ShaderLibrary::initialize(m_render);
        // create texture and mesh managers after render system is available
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
    // ensure resources are freed
    // shutdown internal systems
    // delete texture and mesh managers before render system (they may depend on render system)
    if (m_textureManager) { delete m_textureManager; m_textureManager = nullptr; }
    if (m_meshManager) { delete m_meshManager; m_meshManager = nullptr; }
    // destroy shader library before destroying render system
    ShaderLibrary::destroy();
    if (m_render) {
        delete m_render;
        m_render = nullptr;
    }
}

bool GraphicsEngine::init()
{
    // legacy: nothing to do because ctor performs initialization
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
