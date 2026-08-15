#pragma once
#include <d3d11.h>

class SwapChain;
class VertexBuffer;
class VertexShader;
class IndexBuffer;
class PixelShader;
class ConstantBuffer;

class DeviceContext
{
public:
    DeviceContext(ID3D11DeviceContext* context);
    void setVertexBuffer(VertexBuffer* vertex_buffer);
    void setIndexBuffer(IndexBuffer* index_buffer);
    void setInstanceBuffer(ID3D11Buffer* instanceBuffer, UINT stride);
    void setInputLayout(ID3D11InputLayout* layout);
    // Draw non-indexed instanced
    void drawInstanced(UINT vertex_count, UINT instance_count, UINT start_vertex_location);
    void drawIndexedInstanced(UINT index_count, UINT instance_count, UINT start_vertex_location, UINT start_index_location);
    void clearRenderTargetColor(SwapChain* swap_chain,float red, float green, float blue, float alpha);
    void drawTriangleList(UINT vertex_count, UINT start_vertex_location);
	void drawIndexedTriangleList(UINT index_count, UINT start_vertex_location, UINT start_index_location);
    void drawTriangleStrip(UINT vertex_count, UINT start_vertex_location);
    void drawLineList(UINT vertex_count, UINT start_vertex_location);
    void setViewportSize(UINT width, UINT height);
    void setVertexShader(VertexShader* vertex_shader);
    void setPixelShader(PixelShader* pixel_shader);

    void setConstantBuffer(VertexShader* vertex_shader, class ConstantBuffer* constant_buffer);
    void setConstantBuffer(PixelShader* pixel_shader, class ConstantBuffer* constant_buffer);
    void setTexture(VertexShader* vertex_shader, ID3D11ShaderResourceView* srv);
    void setTexture(PixelShader* pixel_shader, ID3D11ShaderResourceView* srv);
    void resetStateBindings();

    bool release();
    ~DeviceContext();
private:
    ID3D11DeviceContext* m_d3dContext;

    ID3D11Buffer* m_boundVertexBuffer = nullptr;
    ID3D11Buffer* m_boundInstanceBuffer = nullptr;
    ID3D11InputLayout* m_boundInputLayout = nullptr;
    UINT m_boundVertexStride = 0;
    UINT m_boundInstanceStride = 0;

    ID3D11Buffer* m_vsConstantBuffer = nullptr;
    ID3D11Buffer* m_psConstantBuffer = nullptr;
    ID3D11ShaderResourceView* m_vsTextureSRV = nullptr;
    ID3D11ShaderResourceView* m_psTextureSRV = nullptr;

    friend class ConstantBuffer;
};

