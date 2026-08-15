#pragma once
#include <unordered_map>
#include <string>

class VertexShader;
class PixelShader;
class RenderSystem;

class ShaderLibrary
{
public:
    typedef std::wstring String;

    static ShaderLibrary* getInstance();
    static void initialize(RenderSystem* rs);
    static void destroy();

    void requestVertexShaderData(const String& filename, void** shaderByteCode, size_t* sizeShader);
    void requestPixelShaderData(const String& filename, void** shaderByteCode, size_t* sizeShader);

    VertexShader* getVertexShader(const String& filename);
    PixelShader* getPixelShader(const String& filename);

private:
    ShaderLibrary(RenderSystem* rs);
    ~ShaderLibrary();
    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;

    static ShaderLibrary* sharedInstance;
    RenderSystem* m_rs = nullptr;
    std::unordered_map<String, VertexShader*> m_vertexShaders;
    std::unordered_map<String, PixelShader*> m_pixelShaders;
};
