#include <Windows.h>
#include "Debug.h"
#include "InputSystem.h"
#include "ImGui/imgui.h"

InputSystem* InputSystem::sharedInstance = nullptr;
// constructor performs initialization (RAII)
InputSystem::InputSystem() noexcept
{
	// nothing to initialize right now
}


InputSystem::~InputSystem() noexcept
{
	// cleanup listeners
	m_map_listeners.clear();
}

void InputSystem::update()
{
    POINT current_mouse_pos = {};
	::GetCursorPos(&current_mouse_pos);

	if (m_first_time) {
		m_old_mouse_pos = Point(current_mouse_pos.x, current_mouse_pos.y);
		m_first_time = false;
	}

	// If ImGui is present and wants to capture the mouse, suppress delivering
	// mouse movement and button events to the application so UI interactions
	// don't move the camera or recenter the cursor.
	bool imgui_wants_mouse = false;
	if (ImGui::GetCurrentContext()) {
		imgui_wants_mouse = ImGui::GetIO().WantCaptureMouse != 0;
	}

	Point deltaPos(0, 0);
	if (!imgui_wants_mouse) {
		deltaPos = Point(current_mouse_pos.x - m_old_mouse_pos.m_x, current_mouse_pos.y - m_old_mouse_pos.m_y);

		if (deltaPos.m_x != 0 || deltaPos.m_y != 0) {
			for (auto it = m_map_listeners.begin(); it != m_map_listeners.end(); ++it)
			{
				it->second->onMouseMove(deltaPos);
			}
		}
	} else {
		// Keep internal old mouse position in sync to avoid jumps when leaving UI
		m_old_mouse_pos = Point(current_mouse_pos.x, current_mouse_pos.y);
	}

    SHORT leftState = ::GetAsyncKeyState(VK_LBUTTON);
	bool leftDownNow = (leftState & 0x8000) != 0;
	if (!imgui_wants_mouse) {
		if (leftDownNow && !m_left_button_down) {
			for (auto it = m_map_listeners.begin(); it != m_map_listeners.end(); ++it)
			{
				it->second->onLeftMouseDown(deltaPos);
			}
		}
		else if (!leftDownNow && m_left_button_down) {
			for (auto it = m_map_listeners.begin(); it != m_map_listeners.end(); ++it)
			{
				it->second->onLeftMouseUp(deltaPos);
			}
		}
	}
	m_left_button_down = leftDownNow;

    SHORT rightState = ::GetAsyncKeyState(VK_RBUTTON);
	bool rightDownNow = (rightState & 0x8000) != 0;
	if (!imgui_wants_mouse) {
		if (rightDownNow && !m_right_button_down) {
			for (auto it = m_map_listeners.begin(); it != m_map_listeners.end(); ++it)
			{
				it->second->onRightMouseDown(deltaPos);
			}
		}
		else if (!rightDownNow && m_right_button_down) {
			for (auto it = m_map_listeners.begin(); it != m_map_listeners.end(); ++it)
			{
				it->second->onRightMouseUp(deltaPos);
			}
		}
	}
	m_right_button_down = rightDownNow;


    // If ImGui wants the mouse, do not recenter the cursor even if the app has capture.
	if (imgui_wants_mouse) {
		m_old_mouse_pos = Point(current_mouse_pos.x, current_mouse_pos.y);
	} else {
		HWND capWnd = ::GetCapture();
		if (capWnd != nullptr)
		{
			RECT clientRect;
			if (::GetClientRect(capWnd, &clientRect))
			{
				POINT center = {};
				center.x = (clientRect.left + clientRect.right) / 2;
				center.y = (clientRect.top + clientRect.bottom) / 2;
				::ClientToScreen(capWnd, &center);

				// If a forced cursor position is set, reapply it. Otherwise center the cursor.
				if (m_has_forced_cursor) {
					::SetCursorPos(m_forced_cursor_pos.m_x, m_forced_cursor_pos.m_y);
					m_old_mouse_pos = m_forced_cursor_pos;
				}
				else {
					::SetCursorPos(center.x, center.y);
					m_old_mouse_pos = Point(center.x, center.y);
				}
			}
			else
			{
				m_old_mouse_pos = Point(current_mouse_pos.x, current_mouse_pos.y);
			}
		}
		else
		{
			m_old_mouse_pos = Point(current_mouse_pos.x, current_mouse_pos.y);
		}
	}

	static unsigned long long lastSampleMs = 0;
	static size_t lastListenerCount = static_cast<size_t>(-1);
	static unsigned long long lastGetStateFailMs = 0;
	const unsigned long long now = ::GetTickCount64();

	bool ok = ::GetKeyboardState(m_key_states) != 0;

	if (!ok) {
		if (now - lastGetStateFailMs > 2000) {
			lastGetStateFailMs = now;
		}
		return;
	}

	size_t listenerCount = m_map_listeners.size();
	if (listenerCount != lastListenerCount) {
		lastListenerCount = listenerCount;
	}

	if (now - lastSampleMs > 500) {
		char sample[16] = {};
		for (int i = 0; i < 16; ++i) sample[i] = static_cast<char>(m_key_states[i]);
		lastSampleMs = now;
	}

	for (unsigned int i = 0; i < 256; i++)
	{
		bool isDown = (m_key_states[i] & 0x80) != 0;
		bool wasDown = (m_old_key_states[i] & 0x80) != 0;

		if (isDown && !wasDown) {
			for (auto it = m_map_listeners.begin(); it != m_map_listeners.end(); ++it)
			{
				it->second->onKeyDown(i);
			}
		}
		else if (!isDown && wasDown) {
			for (auto it = m_map_listeners.begin(); it != m_map_listeners.end(); ++it)
			{
				it->second->onKeyUp(i);
			}
		}
	}

	::memcpy(m_old_key_states, m_key_states, sizeof(unsigned char) * 256);
}

void InputSystem::addListener(InputListener* listener)
{
	m_map_listeners.insert(std::make_pair<InputListener *, InputListener *>(std::forward<InputListener *>(listener), std::forward<InputListener*>(listener)));

}

void InputSystem::removeListener(InputListener* listener)
{
	std::map<InputListener*, InputListener*>::iterator it = m_map_listeners.find(listener);

	if (it != m_map_listeners.end())
	{
		m_map_listeners.erase(it);
	}
}

void InputSystem::create()
{
	if (!sharedInstance) sharedInstance = new InputSystem();
}

void InputSystem::destroy()
{
	if (sharedInstance) {
		delete sharedInstance;
		sharedInstance = nullptr;
	}
}

InputSystem* InputSystem::get()
{
	return sharedInstance;
}

void InputSystem::SetCursorPosition(const Point& screenPos)
{
	::SetCursorPos(screenPos.m_x, screenPos.m_y);
	m_forced_cursor_pos = screenPos;
	m_has_forced_cursor = true;
	m_old_mouse_pos = screenPos;
}

void InputSystem::ClearCursorPosition()
{
	m_has_forced_cursor = false;
}