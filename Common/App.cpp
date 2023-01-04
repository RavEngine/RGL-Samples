#include "App.hpp"
#include <SDL.h>
#include <SDL_main.h>
#include <iostream>
#include <array>

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
using namespace std;

int AppBase::run(int argc, char** argv) {
#if _WIN32
	// windows highdpi
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		char buf[512];
		SDL_GetErrorMsg(buf, 512);
		cerr << "Failed to init SDL2 : " << buf  << endl;
		return 1;
	}
	window = SDL_CreateWindow(
		SampleName(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
	);
	if (!window) {
		char buf[512];
		SDL_GetErrorMsg(buf, 512);
		cerr << "Failed to create window: " << buf << endl;
	}

	init(argc, argv);

	SDL_Event event;
	bool exit = false;
	while (!exit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					exit = true;
					break;
				case SDL_WINDOWEVENT: {
					const SDL_WindowEvent& wev = event.window;
					switch (wev.event) {
						case SDL_WINDOWEVENT_RESIZED:
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							sizechanged(wev.data1, wev.data2);
							break;
					}
				}
			}
		}
		tick();
	}

	shutdown();
	SDL_DestroyWindow(window);
	window = nullptr;
	SDL_Quit();
	
	return 0;
}
