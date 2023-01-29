#pragma once
#include "App.hpp"

#include <RGL/RGL.hpp>
#include <RGL/CommandQueue.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Device.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/Synchronization.hpp>
#include <RGL/ShaderLibrary.hpp>

// all examples aside from HelloTriangle use this for convenience
struct ExampleFramework : public AppBase{
    void init(int argc, char** argv) final;
    void shutdown() final;
    
    virtual void sampleinit(int argc, char** argv) = 0;
    virtual void sampleshutdown() = 0;
    
    // things required by all RGL samples
    std::shared_ptr<RGL::IDevice> device;
    std::shared_ptr<RGL::ICommandQueue> commandQueue;
    std::shared_ptr<RGL::ISurface> surface;
    std::shared_ptr<RGL::ISwapchain> swapchain;
    std::shared_ptr<RGL::IFence> swapchainFence;
    std::shared_ptr<RGL::ISemaphore> imageAvailableSemaphore, renderCompleteSemaphore;
    
    std::shared_ptr<RGL::IShaderLibrary> GetShader(const std::string& name);
    
    int width = 0, height = 0;
};
