#pragma once
#include <memory>

struct SDL_Window;

class AppBase {
protected:
	SDL_Window* window = nullptr;
public:
	int run(int argc, char** argv);
	virtual const char* SampleName() = 0;
	virtual void init() = 0;
	virtual void tick() = 0;
	virtual void shutdown() = 0;
};

#define START_SAMPLE(name) int main(int argc, char** argv){ auto app = std::make_unique< name >(); return app->run(argc, argv); }