#include <RGL/RGL.hpp>
#include "Common/App.hpp"
#include <RGL/Device.hpp>

struct HelloWorld : public AppBase {
	std::shared_ptr<RGL::IDevice> device;

	const char* SampleName() {
		return "HelloTriangle";
	}
	void init(int argc, char** argv) final {
		RGL::InitOptions options{
			.api = RGL::API::PlatformDefault,
			.appName = SampleName(),
			.engineName = "RGLSampleFramework",
			.appVersion = {0,0,0,1},
			.engineVersion = {0,0,0,1}
		};
		RGL::Init(options);

		device = RGL::IDevice::CreateSystemDefaultDevice();
	}
	void tick() final {

	}

	void shutdown() final {
		RGL::Shutdown();
	}
};

START_SAMPLE(HelloWorld);