#include "AppWindow.h"
#include <Windows.h>

int main()
{

	try {
		GraphicsEngine::create();
	}
	catch(...) {
		return -1;
	}

	AppWindow app;
	if (!app.init())
	{
		return -1;
	}

	while (app.isRun())
	{
		app.broadcast();
	}

	GraphicsEngine::destroy();

	return 0;
}