#include "App.hpp"
#if __APPLE__
#include "AppleFns.hpp"
#endif
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_main.h>
#include <iostream>
#include <array>
#include <filesystem>
#if __APPLE__
    #include <CoreFoundation/CFBundle.h>
#endif
#include <RGL/RGL.hpp>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#define _UWP 1   
#else
#define _UWP 0
#endif

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
using namespace std;

#undef min
#undef max

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

bool app_quit = false;
AppBase* currentApp = nullptr;

void main_loop(){
	currentApp->tickimpl();
}

void AppBase::tickimpl(){
		SDL_Event event;
#if __APPLE__
        AppleAutoreleasePoolInit();
#endif
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					app_quit = true;
					break;
				case SDL_WINDOWEVENT: {
					const SDL_WindowEvent& wev = event.window;
					switch (wev.event) {
						case SDL_WINDOWEVENT_RESIZED:
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							sizechanged(wev.data1 * wmScaleFactor, wev.data2 * wmScaleFactor);
							break;
					}
				}
			}
            onevent(event);
		}
		auto now = clocktype::now();
		auto deltaTimeMicroseconds = std::min(std::chrono::duration_cast<timeDiff>(now - lastframeTime), maxTimeStep);
		float deltaSeconds = std::chrono::duration<decltype(deltaSeconds)>(deltaTimeMicroseconds).count();
		currentScale = deltaSeconds * evalNormal;
        internaltick();
		tick();
		lastframeTime = now;
#if __APPLE__
        AppleAutoreleasePoolDrain();
#endif
}

int AppBase::run(int argc, char** argv) {
#if _WIN32 && !_UWP
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
#if __APPLE__
    {
        int dw, h;
        SDL_Metal_GetDrawableSize(window, &h, &h);
        SDL_GetWindowSize(window, &dw, &dw);
        wmScaleFactor = h / dw;
    }
#endif

#if __APPLE__
    // set pwd to bundle path
    CFBundleRef AppBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(AppBundle);
    CFURLRef absoluteResourceURL = CFURLCopyAbsoluteURL(resourcesURL);
    CFStringRef resourcePath = CFURLCopyPath( absoluteResourceURL);

    string bundlepath = CFStringGetCStringPtr(resourcePath, kCFStringEncodingUTF8);
    
    CFRelease(resourcesURL);
    CFRelease(absoluteResourceURL);
    CFRelease(resourcePath);
    if (std::filesystem::exists(bundlepath)){
        std::filesystem::current_path(bundlepath);
    }
#endif
    
	init(argc, argv);
	currentApp = this;

#if __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, 1);
#else
	while(!app_quit){
		main_loop();
	}
	shutdown();
	SDL_DestroyWindow(window);
	window = nullptr;
	SDL_Quit();
#endif

	
	
	return 0;
}
