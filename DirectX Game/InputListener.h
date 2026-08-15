#pragma once	
#include "Point.h"
class InputListener
{
public:
	InputListener() {

	}
	~InputListener() {

	}

	virtual void onKeyDown(int key) = 0;
	virtual void onKeyUp(int key) = 0;

	virtual void onMouseDown(const Point& delta_mouse_pos) { (void)delta_mouse_pos; }

	virtual void onMouseMove(const Point& delta_mouse_pos) { (void)delta_mouse_pos; }

	virtual void onLeftMouseDown(const Point& delta_mouse_pos) { (void)delta_mouse_pos; }
	virtual void onLeftMouseUp(const Point& delta_mouse_pos) { (void)delta_mouse_pos; }

	virtual void onRightMouseDown(const Point& delta_mouse_pos) { (void)delta_mouse_pos; }
	virtual void onRightMouseUp(const Point& delta_mouse_pos) { (void)delta_mouse_pos; }


};