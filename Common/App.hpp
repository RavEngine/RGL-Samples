#pragma once
#include <memory>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#define _UWP 1   
#else
#define _UWP 0
#endif

struct SDL_Window;

class AppBase {
protected:
	SDL_Window* window = nullptr;
public:
	int run(int argc, char** argv);
	virtual const char* SampleName() = 0;
	virtual void init(int argc, char** argv) = 0;
	virtual void tick() = 0;
	virtual void shutdown() = 0;
	virtual void sizechanged(int width, int height) {}
	virtual void onevent(union SDL_Event&) {}
    float wmScaleFactor = 1;
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
