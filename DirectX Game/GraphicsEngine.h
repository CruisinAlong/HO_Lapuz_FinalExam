#pragma once
#include <d3d11.h>

class SwapChain;
class DeviceContext;
class VertexBuffer;
class ConstantBuffer;
class IndexBuffer;
class VertexShader;
class PixelShader;

class RenderSystem;
class TextureManager;
class MeshManager;

class GraphicsEngine
{
private:
	GraphicsEngine();
	~GraphicsEngine();
	GraphicsEngine(GraphicsEngine const&) {}
	GraphicsEngine& operator=(GraphicsEngine const&) { return *this; }

	static GraphicsEngine* sharedInstance;

public:
    static GraphicsEngine* getInstance();
	// Controlled creation/release to manage initialization order and RAII
	static void create();
	static void destroy();

public:
	bool init();
	bool release();
public:
	static GraphicsEngine* get();

	// Access to the underlying RenderSystem
	RenderSystem* getRenderSystem();
    // Access to the TextureManager
	TextureManager* getTextureManager();
	// Access to the MeshManager
	MeshManager* getMeshManager();

private:
	RenderSystem* m_render = nullptr; // owned when initialized
    TextureManager* m_textureManager = nullptr;
	MeshManager* m_meshManager = nullptr;
};

