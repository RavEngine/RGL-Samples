#pragma once
#include "App.hpp"
#include <RGL/Types.hpp>
#include <RGL/RGL.hpp>
#include <RGL/CommandQueue.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Device.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/Synchronization.hpp>
#include <RGL/ShaderLibrary.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <RGL/Span.hpp>
#include <cstdlib>

#include <RGL/../../src/VkDevice.hpp>
#include <RGL/../../src/VkSwapchain.hpp>
#include <RGL/../../src/VkBuffer.hpp>
#include <RGL/../../src/VkCommandBuffer.hpp>
#include <RGL/../../src/VkCommandQueue.hpp>
#include <RGL/../../src/VkRenderpass.hpp>
#include <RGL/../../src/VkRenderPipeline.hpp>
#include <RGL/../../src/VkSampler.hpp>
#include <RGL/../../src/VkShaderLibrary.hpp>
#include <RGL/../../src/VkSurface.hpp>
#include <RGL/../../src/VkSynchronization.hpp>
#include <RGL/../../src/VkTexture.hpp>
#undef LoadImage


/**
 @param val an angle in degrees
 @return the angle in radians
 */
inline float deg_to_rad(float val){
    return glm::radians(val);
}

struct Camera{
    glm::vec3 position{0};
    float yaw = 0, pitch = 0;
    
    float FOV = 60;
    float nearClip = 0.1;
    float farClip = 100;
    
    glm::mat4 ModelMatrix() const{
        return glm::translate(glm::mat4(1), position) * (glm::toMat4(glm::quat(glm::vec3(0,yaw,0))) * glm::toMat4(glm::quat(glm::vec3(pitch,0,0))));
    }
    
    glm::mat4 GenerateViewMatrix() const{
        return glm::inverse(ModelMatrix());
    }
    
    glm::mat4 GenerateProjectionMatrix(uint32_t width, uint32_t height) const{
        return glm::mat4(glm::perspective(deg_to_rad(FOV), (float)width / height, nearClip, farClip));
    }
    
    glm::mat4 GenerateViewProjMatrix(uint32_t width, uint32_t height){
        return GenerateProjectionMatrix(width, height) * GenerateViewMatrix();
    }
};

// all examples aside from HelloTriangle use this for convenience
struct ExampleFramework : public AppBase{
    void init(int argc, char** argv) final;
    void sizechanged(int, int) final;
    void shutdown() final;
    
    virtual void sampleinit(int argc, char** argv) = 0;
    virtual void sampleshutdown() = 0;
    virtual void onresize(int, int) {};
    
    // things required by all RGL samples
    RGLDevicePtr device;
    RGLCommandQueuePtr commandQueue;
    RGLSurfacePtr surface;
    RGLSwapchainPtr swapchain;
    RGLFencePtr swapchainFence;
    RGLSemaphorePtr imageAvailableSemaphore, renderCompleteSemaphore;
    
    RGLShaderLibraryPtr GetShader(const std::string& name);
    
    struct stbi_freer{
        void operator()(void* ptr) const{
            free(ptr);
        }
    };
    
    struct TextureData{
        RGL::untyped_owning_span<stbi_freer> bytes;
        uint32_t width, height;
        uint32_t nchannels;
    };
    
    TextureData LoadImage(const std::filesystem::path& path);
    
    Camera camera;
    
    int width = 0, height = 0;
};
