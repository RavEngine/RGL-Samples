#include "ExampleFramework.hpp"
#include <SDL.h>
#include <SDL_syswm.h>
#include <fmt/format.h>
#include <unordered_map>
#include <iostream>
#include <stb_image.h>
#undef LoadImage

// angry!!!!!
#undef CreateSemaphore

void ExampleFramework::init(int argc, char **argv){
    RGL::API api = RGL::API::PlatformDefault;
    {
#if _UWP
        size_t n_elt;
        char* envv;
        _dupenv_s(&envv, &n_elt, "RGL_BACKEND");
#else
        auto envv = std::getenv("RGL_BACKEND");
#endif
        if (envv == nullptr){
            goto cont;
        }
        auto backend = std::string_view(envv);
        
        const std::unordered_map<std::string_view, RGL::API> apis{
            {"metal", decltype(apis)::value_type::second_type::Metal},
            {"d3d12", decltype(apis)::value_type::second_type::Direct3D12},
            {"vulkan", decltype(apis)::value_type::second_type::Vulkan},
        };
        
        auto it = apis.find(backend);
        if (it != apis.end()){
            api = (*it).second;
        }
        else{
            std::cerr << "No backend \"" << backend << "\", expected one of:\n";
            for(const auto& api : apis){
                std::cout << "\t - " << RGL::APIToString(api.second) << "\n";
            }
        }
    }
cont:
    RGL::InitOptions options{
        .api = api,
        .appName = SampleName(),
        .engineName = "RGLSampleFramework",
        .appVersion = {0,0,0,1},
        .engineVersion = {0,0,0,1}
    };
    RGL::Init(options);

    auto title = fmt::format("{} - {}", SampleName(), RGL::APIToString(RGL::CurrentAPI()));

    SDL_SetWindowTitle(window, title.c_str());
    device = RGL::IDevice::CreateSystemDefaultDevice();

    // create the main queue
    commandQueue = device->CreateCommandQueue(RGL::QueueType::AllCommands);

    // create a surface to render to
    // this requires some platform-specific code
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if (!SDL_GetWindowWMInfo(window, &wmi)) {
        throw std::runtime_error("Cannot get native window information");
    }
    surface = RGL::CreateSurfaceFromPlatformHandle(
#if _UWP
        &wmi.info.winrt.window,
#elif _WIN32
        &wmi.info.win.window,
#elif TARGET_OS_IPHONE
        wmi.info.uikit.window,
#elif __APPLE__
        wmi.info.cocoa.window,
#else
#error Unknown platform
#endif
        true
    );
    
    SDL_GetWindowSizeInPixels(window, &width, &height);
    
    // create a swapchain for the surface
    // provide it the queue which will be presented on
    swapchain = device->CreateSwapchain(surface, commandQueue, width * wmScaleFactor,height*wmScaleFactor);
    swapchainFence = device->CreateFence(true);
    imageAvailableSemaphore = device->CreateSemaphore();
    renderCompleteSemaphore = device->CreateSemaphore();
    
    sampleinit(argc, argv);
}

void ExampleFramework::sizechanged(int width, int height)
{
    this->width = width;
    this->height = height;
    commandQueue->WaitUntilCompleted();
    swapchain->Resize(width, height);
    onresize(width,height);
}

void ExampleFramework::shutdown(){
    commandQueue->WaitUntilCompleted();
    device->BlockUntilIdle();
    
    sampleshutdown();
    
    renderCompleteSemaphore.reset();
    imageAvailableSemaphore.reset();
    swapchainFence.reset();
    swapchain.reset();
    surface.reset();
    device.reset();
    RGL::Shutdown();
}

RGLShaderLibraryPtr ExampleFramework::GetShader(const std::string& name){
    auto getShaderPathname = [](const std::string& name) {
        const char* backendPath;
        const char* extension;
        switch (RGL::CurrentAPI()) {
        case RGL::API::Vulkan:
            backendPath = "Vulkan";
            extension = ".spv";
            return std::filesystem::path("../shaders") / backendPath / (name + extension);
            break;
        case RGL::API::Direct3D12:
            backendPath = "Direct3D12";
            extension = ".cso";
            return std::filesystem::path(name + extension);
            break;
        default:
            throw std::runtime_error("Shader loading not implemented");
        }
        
    };
#if __APPLE__
    // metal uses the Xcode-created metallib in resources, which have a different naming convention
    auto tempstr = name;
    std::replace(tempstr.begin(), tempstr.end(), '.', '_');
    return device->CreateShaderLibraryFromName(tempstr);
#else
    return device->CreateShaderLibraryFromPath(getShaderPathname(name));
#endif
}

ExampleFramework::TextureData ExampleFramework::LoadImage(const std::filesystem::path& path){
    constexpr auto nlayers = 1;
    
    TextureData data;
    auto pathstr = path.string();
#if !_WIN32
    std::replace( pathstr.begin(), pathstr.end(), '\\', '/'); // make unix pathnames
#endif
    
    auto bytes = stbi_load(pathstr.c_str(), reinterpret_cast<int*>(&data.width), reinterpret_cast<int*>(&data.height), reinterpret_cast<int*>(&data.nchannels), 4);

    if (!bytes) {
        std::cerr << stbi_failure_reason() << std::endl;
        throw std::runtime_error(stbi_failure_reason());
    }
    
    auto uncompressed_size = data.width * data.height * data.nchannels * nlayers;
    data.bytes.ptr = bytes;
    data.bytes.size_bytes = uncompressed_size;
        
    return data;
}
