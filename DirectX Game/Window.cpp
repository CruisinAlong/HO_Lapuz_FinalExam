
#include "Window.h"
#include <stdexcept>
#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window* window = nullptr;

Window::Window() : m_hwnd(nullptr), m_isRun(false), m_initialized(false)
{

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch(msg)
	{
	case WM_CREATE:
	{
		Window* window = (Window*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		if (window)
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
			window->setHWND(hwnd);
			window->onCreate();
		}
		break;
	}
	case WM_SETFOCUS:
	{
		Window* w = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (w)
		{
			w->onFocus();
		}
		break;
	}
	case WM_KILLFOCUS:
	{
		Window* w = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (w)
		{
			w->onKillFocus();
		}
		break;
	}
	case WM_DESTROY:
	{
		Window* window = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (window)
		{
			window->onDestroy();
		}
		::PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

bool Window::init()
{
    if (m_initialized) return true;
	WNDCLASSEX wc;
	wc.cbClsExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = NULL;
	wc.lpszClassName = L"MyWindowClass";
	wc.lpszMenuName = L"";
	wc.style = NULL;
	wc.lpfnWndProc = WndProc;

	if(!::RegisterClassEx(&wc))
	{
		return false;
	}

	if(!window)
		window = this;

	m_hwnd=::CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"MyWindowClass", L"DirectX Game", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, NULL, NULL, NULL, this);

	if(!m_hwnd)
	{
		return false;
	}

	::ShowWindow(m_hwnd, SW_SHOW);
	::UpdateWindow(m_hwnd);
	
	m_isRun = true;
    m_initialized = true;
	return true;
}

bool Window::broadcast()
{
	if (!window)
		return false;

	window->onUpdate();

	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Sleep(0);

	return true;
}

bool Window::release()
{
    if (!m_initialized) return true;

	if(!::DestroyWindow(m_hwnd))
	{
		return false;
	}

	m_initialized = false;
	m_isRun = false;
	return true;
}

bool Window::isRun()
{
	return m_isRun;
}

RECT Window::getClientWindowRect()
{
	RECT rc;
	::GetClientRect(this->m_hwnd, &rc);
	return rc;
}

void Window::setHWND(HWND hwnd)
{
	this->m_hwnd = hwnd;
}

void Window::onCreate()
{
}

void Window::onUpdate()
{
}

void Window::onDestroy()
{
	m_isRun = false;
}

Window::~Window()
{
    // ensure native resources are released
	release();
}
