#pragma once
#include "InputListener.h"
#include "Point.h"
#include <map>
class InputSystem
{
private:
	InputSystem() noexcept;
	~InputSystem() noexcept;
	InputSystem(const InputSystem&) = delete;
	InputSystem& operator=(const InputSystem&) = delete;

	static InputSystem* sharedInstance;



public:
	static void create();
	static void destroy();
	static InputSystem* get();

	bool init() { return true; }
	bool shutdown() { return true; }

	void update();
	void addListener(InputListener* listener);
	void removeListener(InputListener* listener);


	void SetCursorPosition(const Point& screenPos);
	void ClearCursorPosition();

private:
	std::map<InputListener*, InputListener*> m_map_listeners;
	unsigned char m_key_states[256] = {};
	unsigned char m_old_key_states[256] = {};
	Point m_old_mouse_pos;
	bool m_first_time = true;
	bool m_left_button_down = false;
	bool m_right_button_down = false;

	Point m_forced_cursor_pos;
	bool m_has_forced_cursor = false;
};