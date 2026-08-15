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
	static void create();
	static void destroy();

public:
	bool init();
	bool release();
public:
	static GraphicsEngine* get();

	RenderSystem* getRenderSystem();
	TextureManager* getTextureManager();
	MeshManager* getMeshManager();

private:
	RenderSystem* m_render = nullptr; 
    TextureManager* m_textureManager = nullptr;
	MeshManager* m_meshManager = nullptr;
};

