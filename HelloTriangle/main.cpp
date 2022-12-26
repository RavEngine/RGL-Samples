#include <RGL/RGL.hpp>
#include "Common/App.hpp"

struct HelloWorld : public AppBase {
	std::shared_ptr<RGL::IDevice> device;

	const char* SampleName() {
		return "HelloTriangle";
	}
	void init() final {

	}
	void tick() final {

	}

	void shutdown() final {

	}
};

START_SAMPLE(HelloWorld);