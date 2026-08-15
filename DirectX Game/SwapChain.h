#pragma once
#include "Prerequisites.h"
#include <d3d11.h>

class DeviceContext;

class SwapChain
{
public:
    SwapChain();
	// RAII constructor: initialize and create swap chain for given window
	SwapChain(RenderSystem* system, HWND hwnd, UINT width, UINT height);
	// Construct with system only (deferred init)
	SwapChain(RenderSystem* system);
	~SwapChain();
  bool init(HWND hwnd, UINT width, UINT height);


	bool present(bool vsync);

	bool release();
private:
	IDXGISwapChain* m_swap_chain;
	ID3D11RenderTargetView* m_rtv;

	ID3D11Texture2D* m_depthStencilTex = nullptr;
	ID3D11DepthStencilView* m_dsv = nullptr;

private:
    friend class DeviceContext;
	RenderSystem* m_system = nullptr;
};

// SwapChainPtr typedef moved to Prerequisites.h

