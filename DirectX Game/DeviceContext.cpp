#include "DeviceContext.h"
#include "SwapChain.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Debug.h"

DeviceContext::DeviceContext(ID3D11DeviceContext* context) : m_d3dContext(context),
	m_boundVertexBuffer(nullptr),
	m_boundInputLayout(nullptr),
	m_boundVertexStride(0),
	m_vsConstantBuffer(nullptr),
	m_psConstantBuffer(nullptr),
	m_boundInstanceBuffer(nullptr),
	m_boundInstanceStride(0)
{
}

bool DeviceContext::release()
{
    if (m_d3dContext)
    {
        m_d3dContext->Release();
        m_d3dContext = nullptr;
    }
    return true;
}

void DeviceContext::setVertexBuffer(VertexBuffer* vertex_buffer)
{
	if (!vertex_buffer) {
		return;
	}

	UINT stride = vertex_buffer->m_size_vertex;
	UINT offset = 0;

	if (vertex_buffer->m_buffer != m_boundVertexBuffer ||
		vertex_buffer->m_layout != m_boundInputLayout ||
		stride != m_boundVertexStride)
	{
		m_d3dContext->IASetVertexBuffers(0, 1, &vertex_buffer->m_buffer, &stride, &offset);
		m_d3dContext->IASetInputLayout(vertex_buffer->m_layout);

		m_boundVertexBuffer = vertex_buffer->m_buffer;
		m_boundInputLayout = vertex_buffer->m_layout;
		m_boundVertexStride = stride;
	}
}

void DeviceContext::setInstanceBuffer(ID3D11Buffer* instanceBuffer, UINT stride)
{
    UINT offset = 0;
    if (instanceBuffer != m_boundInstanceBuffer || stride != m_boundInstanceStride) {
        m_d3dContext->IASetVertexBuffers(1, 1, &instanceBuffer, &stride, &offset);
        m_boundInstanceBuffer = instanceBuffer;
        m_boundInstanceStride = stride;
    }
}

void DeviceContext::setInputLayout(ID3D11InputLayout* layout)
{
    if (layout != m_boundInputLayout) {
        m_d3dContext->IASetInputLayout(layout);
        m_boundInputLayout = layout;
    }
}

void DeviceContext::clearRenderTargetColor(SwapChain* swap_chain, float red, float green, float blue, float alpha)
{
    FLOAT clear_color[] = { red, green, blue, alpha };

    m_d3dContext->OMSetRenderTargets(1, &swap_chain->m_rtv, swap_chain->m_dsv);

    m_d3dContext->ClearRenderTargetView(swap_chain->m_rtv, clear_color);

    if (swap_chain->m_dsv)
    {
        m_d3dContext->ClearDepthStencilView(swap_chain->m_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}

void DeviceContext::drawTriangleList(UINT vertex_count, UINT start_vertex_location)
{
    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_d3dContext->Draw(vertex_count, start_vertex_location);
}

void DeviceContext::drawInstanced(UINT vertex_count, UINT instance_count, UINT start_vertex_location)
{
    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_d3dContext->DrawInstanced(vertex_count, instance_count, start_vertex_location, 0);
}

void DeviceContext::drawIndexedTriangleList(UINT index_count, UINT start_vertex_location, UINT start_index_location)
{
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_d3dContext->DrawIndexed(index_count, start_index_location, start_vertex_location);
}

void DeviceContext::drawTriangleStrip(UINT vertex_count, UINT start_vertex_location)
{
    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_d3dContext->Draw(vertex_count, start_vertex_location);
}

void DeviceContext::drawLineList(UINT vertex_count, UINT start_vertex_location)
{
    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_d3dContext->Draw(vertex_count, start_vertex_location);
}

void DeviceContext::setViewportSize(UINT width, UINT height)
{
    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    m_d3dContext->RSSetViewports(1, &vp);
}

void DeviceContext::setVertexShader(VertexShader* vertex_shader)
{
    m_d3dContext->VSSetShader(vertex_shader->m_vs, nullptr, 0);
}

void DeviceContext::setPixelShader(PixelShader* pixel_shader)
{
    m_d3dContext->PSSetShader(pixel_shader->m_ps, nullptr, 0);
}

void DeviceContext::setConstantBuffer(VertexShader* vertex_shader, ConstantBuffer* buffer)
{
    if (!buffer) {
        return;
    }

	if (buffer->m_buffer != m_vsConstantBuffer) {
		m_d3dContext->VSSetConstantBuffers(0, 1, &buffer->m_buffer);
		m_vsConstantBuffer = buffer->m_buffer;
	}
}

void DeviceContext::setConstantBuffer(PixelShader* pixel_shader, ConstantBuffer* buffer)
{
    if (!buffer) {
        return;
    }

	if (buffer->m_buffer != m_psConstantBuffer) {
		m_d3dContext->PSSetConstantBuffers(0, 1, &buffer->m_buffer);
		m_psConstantBuffer = buffer->m_buffer;
	}
}

void DeviceContext::setTexture(VertexShader* vertex_shader, ID3D11ShaderResourceView* srv)
{
    if (srv == m_vsTextureSRV) return;

    ID3D11ShaderResourceView* arr[1] = { srv };
    m_d3dContext->VSSetShaderResources(0, 1, arr);

    m_vsTextureSRV = srv;
}

void DeviceContext::setTexture(PixelShader* pixel_shader, ID3D11ShaderResourceView* srv)
{
    if (srv == m_psTextureSRV) return;

    ID3D11ShaderResourceView* arr[1] = { srv };
    m_d3dContext->PSSetShaderResources(0, 1, arr);

    m_psTextureSRV = srv;
}

void DeviceContext::resetStateBindings()
{
    m_boundVertexBuffer = nullptr;
    m_boundInputLayout = nullptr;
    m_boundVertexStride = 0;
    m_vsConstantBuffer = nullptr;
    m_psConstantBuffer = nullptr;
	m_boundInstanceBuffer = nullptr;
	m_boundInstanceStride = 0;

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_d3dContext->VSSetShaderResources(0, 1, nullSRV);
    m_d3dContext->PSSetShaderResources(0, 1, nullSRV);

    m_vsTextureSRV = nullptr;
    m_psTextureSRV = nullptr;
}

void DeviceContext::setIndexBuffer(IndexBuffer* index_buffer)
{
	m_d3dContext->IASetIndexBuffer(index_buffer->m_buffer, DXGI_FORMAT_R32_UINT, 0);
}

void DeviceContext::drawIndexedInstanced(UINT index_count, UINT instance_count, UINT start_vertex_location, UINT start_index_location)
{
    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_d3dContext->DrawIndexedInstanced(index_count, instance_count, start_index_location, start_vertex_location, 0);
}

DeviceContext::~DeviceContext()
{
}