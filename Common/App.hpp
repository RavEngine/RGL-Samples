#pragma once
#include <memory>
#include <chrono>
#include <SDL_main.h>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#define _UWP 1
#else
#define _UWP 0
#endif

struct SDL_Window;

class AppBase {
protected:
	SDL_Window* window = nullptr;
	float currentScale = 0;
	typedef std::chrono::duration<double, std::micro> timeDiff;
	typedef std::chrono::high_resolution_clock clocktype;
	clocktype::time_point lastframeTime = clocktype::now();
	constexpr static float evalNormal = 60;
	const timeDiff maxTimeStep = std::chrono::milliseconds((long)1000);
public:
	int run(int argc, char** argv);
	virtual const char* SampleName() = 0;
	virtual void init(int argc, char** argv) = 0;
	virtual void tick() = 0;
    virtual void internaltick(){}
	virtual void shutdown() = 0;
	virtual void sizechanged(int width, int height) {}
	virtual void onevent(union SDL_Event&) {}
    float wmScaleFactor = 1;
	
	auto getCurrentScale() const {
		return currentScale;
	}
};

#if _UWP
// UWP startup requires extra effort
#undef main
#define START_SAMPLE(APP) \
int DoProgram(int argc, char** argv){\
auto a = std::make_unique<APP>(); return a->run(argc, argv);\
}\
int main(int argc, char** argv) { \
	return SDL_WinRTRunApp(DoProgram, NULL);\
}
#else
#define START_SAMPLE(name) int main(int argc, char** argv){ auto app = std::make_unique< name >(); return app->run(argc, argv); }
#endif
