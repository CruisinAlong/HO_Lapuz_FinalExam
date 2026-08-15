#include "IndexBuffer.h"
#include "RenderSystem.h"
#include "Debug.h"
#include <cstdint>
#include "ReferenceManager.h"
#include <stdexcept>
#include <stdexcept>

IndexBuffer::IndexBuffer() : m_system(nullptr), m_size_index(0), m_size_list(0), m_buffer(nullptr), m_format(DXGI_FORMAT_UNKNOWN)
{
}

IndexBuffer::IndexBuffer(RenderSystem* system, void* list_indices, UINT size_index, UINT size_list)
	: m_system(system), m_size_index(0), m_size_list(0), m_buffer(nullptr), m_format(DXGI_FORMAT_UNKNOWN)
{
    // initialize from data and throw on failure
	if (!load(list_indices, size_index, size_list)) {
		char buf[256];
		sprintf_s(buf, "IndexBuffer ctor: load failed (size_index=%u size_list=%u)", size_index, size_list);
		throw std::runtime_error(buf);
	}
}

IndexBuffer::IndexBuffer(RenderSystem* system) : m_system(system), m_size_index(0), m_size_list(0), m_buffer(nullptr), m_format(DXGI_FORMAT_UNKNOWN)
{
}

bool IndexBuffer::load(void* list_indices, UINT size_index, UINT size_list)
{
	LOG("IndexBuffer::load size_index=%u size_list=%u indices=%p", size_index, size_list, list_indices);

	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = nullptr;
	}

	if (size_index == 2)
		m_format = DXGI_FORMAT_R16_UINT;
	else
		m_format = DXGI_FORMAT_R32_UINT;

	D3D11_BUFFER_DESC buff_desc = {};
	buff_desc.Usage = D3D11_USAGE_DEFAULT;
	buff_desc.ByteWidth = size_index * size_list;
	buff_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buff_desc.CPUAccessFlags = 0;
	buff_desc.MiscFlags = 0;
	buff_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA init_data = {};
	init_data.pSysMem = list_indices;

	m_size_index = size_index;
	m_size_list = size_list;

    ID3D11Device* device = nullptr;
	if (m_system) device = m_system->getDevice();
	if (!device) {
		LOG("IndexBuffer::load - no D3D device available");
		return false;
	}

    HRESULT hr = device->CreateBuffer(&buff_desc, &init_data, &m_buffer);
	if (FAILED(hr)) {
		LOG("IndexBuffer::load CreateBuffer failed HR=0x%08X", hr);
		return false;
	}

	// register created buffer in reference manager
	ReferenceManager::acquire(m_buffer);

	UINT toLog = (m_size_list < 3) ? m_size_list : 3;
	if (m_size_index == 2) {
		uint16_t* indices = reinterpret_cast<uint16_t*>(list_indices);
		for (UINT i = 0; i < toLog; ++i) {
			LOG("IndexBuffer::load index[%u] = %u", i, (unsigned int)indices[i]);
		}
	}
	else {
		uint32_t* indices = reinterpret_cast<uint32_t*>(list_indices);
		for (UINT i = 0; i < toLog; ++i) {
			LOG("IndexBuffer::load index[%u] = %u", i, indices[i]);
		}
	}

	LOG("IndexBuffer::load succeeded m_buffer=%p format=%u", m_buffer, (unsigned int)m_format);
	return true;
}

UINT IndexBuffer::getSizeIndexList()
{
	return this->m_size_list;
}

bool IndexBuffer::release()
{
	if (m_buffer)
	{
        if (ReferenceManager::release(m_buffer)) {
			m_buffer->Release();
		}
		m_buffer = nullptr;
	}
	LOG("IndexBuffer::release");
	return true;
}

IndexBuffer::~IndexBuffer()
{
	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = nullptr;
	}
}